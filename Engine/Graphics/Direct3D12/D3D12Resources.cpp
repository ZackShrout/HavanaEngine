#include "D3D12Resources.h"
#include "D3D12Core.h"

namespace havana::graphics::d3d12
{
	//// DESCRIPTOR HEAP //////////////////////////////////////////////////////////////////////////
	bool descriptor_heap::initialize(u32 capacity, bool isShaderVisible)
	{
		std::lock_guard lock{ _mutex };
		assert(capacity && capacity < D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2);
		assert(!(_type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER &&
			capacity > D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE));
		
		if (_type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV ||
			_type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
		{
			isShaderVisible = false;
		}

		release();

		auto* const device{ core::device() };
		assert(device);

		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.Flags = isShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NodeMask = 0;
		desc.NumDescriptors = capacity;
		desc.Type = _type;

		HRESULT hr{ S_OK };
		DXCall(hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&_heap)));
		if (FAILED(hr)) return false;

		_free_handles = std::move(std::make_unique<u32[]>(capacity));
		_capacity = capacity;
		_size = 0;

		for (u32 i{ 0 }; i < capacity; i++)
			_free_handles[i] = i;

		DEBUG_OP(for (u32 i{ 0 }; i < frame_buffer_count; i++) assert(_deferred_free_indices[i].empty()));

		_descriptor_size = device->GetDescriptorHandleIncrementSize(_type);
		_cpu_start = _heap->GetCPUDescriptorHandleForHeapStart();
		_gpu_start = isShaderVisible ? _heap->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE{ 0 };

		return true;
	}
	
	void descriptor_heap::process_deferred_free(u32 frameIdx)
	{
		std::lock_guard lock{ _mutex };
		assert(frameIdx < frame_buffer_count);

		utl::vector<u32>& indices{ _deferred_free_indices[frameIdx] };
		if (!indices.empty())
		{
			for (auto index : indices)
			{
				_size--;
				_free_handles[_size] = index;
			}
			indices.clear();
		}
	}
	
	void descriptor_heap::release()
	{
		assert(!_size);
		core::deferred_release(_heap);
	}

	descriptor_handle descriptor_heap::allocate()
	{
		std::lock_guard lock{ _mutex };
		assert(_heap);
		assert(_size < _capacity);

		const u32 index{ _free_handles[_size] };
		const u32 offset{ index * _descriptor_size };
		_size++;

		descriptor_handle handle;
		handle.cpu.ptr = _cpu_start.ptr + offset;
		
		if (is_shader_visible())
		{
			handle.gpu.ptr = _gpu_start.ptr + offset;
		}

		handle.index = index;
		DEBUG_OP(handle.container = this);

		return handle;
	}

	void descriptor_heap::free(descriptor_handle& handle)
	{
		if (!handle.is_vaild()) return;
		std::lock_guard lock{ _mutex };
		assert(_heap && _size);
		assert(handle.container = this);
		assert(handle.cpu.ptr >= _cpu_start.ptr);
		assert((handle.cpu.ptr - _cpu_start.ptr) % _descriptor_size == 0);
		assert(handle.index < _capacity);
		const u32 index{ (u32)(handle.cpu.ptr - _cpu_start.ptr) / _descriptor_size };
		assert(handle.index == index);

		const u32 frameIdx{ core::current_frame_index() };
		_deferred_free_indices[frameIdx].push_back(index);
		core::set_deferred_releases_flag();
		handle = {};
	}

	//// D3D12 TEXTURE ////////////////////////////////////////////////////////////////////////////
	d3d12_texture::d3d12_texture(d3d12_texture_init_info info)
	{
		auto* const device{ core::device() };
		assert(device);

		D3D12_CLEAR_VALUE* const clear_value
		{
			(info.desc &&
			(info.desc->Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET ||
				info.desc->Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
			? &info.clear_value : nullptr
		};

		if (info.resource)
		{
			assert(!info.heap);
			_resource = info.resource;
		}
		else if (info.heap && info.desc)
		{
			assert(!info.resource);
			DXCall(device->CreatePlacedResource(
				info.heap, info.allocation_info.Offset, info.desc, info.initial_state,
				clear_value, IID_PPV_ARGS(&_resource)));
		}
		else if(info.desc)
		{
			assert(!info.resource);

			DXCall(device->CreateCommittedResource(
				&d3dx::heap_properties.default_heap, D3D12_HEAP_FLAG_NONE, info.desc, info.initial_state,
				clear_value, IID_PPV_ARGS(&_resource)));
		}

		assert(_resource);
		_srv = core::srv_heap().allocate();
		device->CreateShaderResourceView(_resource, info.srv_desc, _srv.cpu);
	}

	void d3d12_texture::release()
	{
		core::srv_heap().free(_srv);
		core::deferred_release(_resource);
	}

	//// RENDER TEXTURE ///////////////////////////////////////////////////////////////////////////
	d3d12_render_texture::d3d12_render_texture(d3d12_texture_init_info info) : _texture{ info }
	{
		_mip_count = resource()->GetDesc().MipLevels;
		assert(_mip_count && _mip_count <= d3d12_texture::max_mips);

		descriptor_heap& rtvHeap{ core::rtv_heap() };
		D3D12_RENDER_TARGET_VIEW_DESC desc{};
		desc.Format = info.desc->Format;
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;

		auto* const device{ core::device() };
		assert(device);

		for (u32 i{ 0 }; i < _mip_count; i++)
		{
			_rtv[i] = rtvHeap.allocate();
			device->CreateRenderTargetView(resource(), &desc, _rtv[i].cpu);
			++desc.Texture2D.MipSlice;
		}
	}

	void d3d12_render_texture::release()
	{
		for (u32 i{ 0 }; i < _mip_count; i++) core::rtv_heap().free(_rtv[i]);
		_texture.release();
		_mip_count = 0;
	}

	//// DEPTH BUFFER /////////////////////////////////////////////////////////////////////////////
	d3d12_depth_buffer::d3d12_depth_buffer(d3d12_texture_init_info info)
	{
		assert(info.desc);
		const DXGI_FORMAT dsvFormat{ info.desc->Format };

		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
		// depth stencil view cannot be the same format as the shader resource view
		if (info.desc->Format == DXGI_FORMAT_D32_FLOAT)
		{
			info.desc->Format = DXGI_FORMAT_R32_TYPELESS;
			srv_desc.Format = DXGI_FORMAT_R32_FLOAT;
		}

		srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srv_desc.Texture2D.MipLevels = 1;
		srv_desc.Texture2D.MostDetailedMip = 0;
		srv_desc.Texture2D.PlaneSlice = 0;
		srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;

		assert(!info.srv_desc && !info.resource);
		info.srv_desc = &srv_desc;
		_texture = d3d12_texture(info);

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.Format = dsvFormat; // Now that info was used to make the texture, revert the depth stencil view to it's previous format
		dsvDesc.Texture2D.MipSlice = 0;

		_dsv = core::dsv_heap().allocate();
		auto* const device{ core::device() };
		assert(device);
		device->CreateDepthStencilView(resource(), &dsvDesc, _dsv.cpu);
	}
	
	void d3d12_depth_buffer::release()
	{
		core::dsv_heap().free(_dsv);
		_texture.release();
	}
}
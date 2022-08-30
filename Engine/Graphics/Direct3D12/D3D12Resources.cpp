#include "D3D12Resources.h"
#include "D3D12Core.h"

namespace havana::graphics::d3d12
{
	//// DESCRIPTOR HEAP //////////////////////////////////////////////////////////////////////////
	bool descriptor_heap::initialize(u32 capacity, bool isShaderVisible)
	{
		std::lock_guard lock{ m_mutex };
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
		DXCall(hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_heap)));
		if (FAILED(hr)) return false;

		m_freeHandles = std::move(std::make_unique<u32[]>(capacity));
		_capacity = capacity;
		_size = 0;

		for (u32 i{ 0 }; i < capacity; i++)
			m_freeHandles[i] = i;

		DEBUG_OP(for (u32 i{ 0 }; i < frame_buffer_count; i++) assert(m_deferredFreeIndices[i].empty()));

		m_descriptorSize = device->GetDescriptorHandleIncrementSize(_type);
		m_cpuStart = m_heap->GetCPUDescriptorHandleForHeapStart();
		m_gpuStart = isShaderVisible ? m_heap->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE{ 0 };

		return true;
	}
	
	void descriptor_heap::process_deferred_free(u32 frameIdx)
	{
		std::lock_guard lock{ m_mutex };
		assert(frameIdx < frame_buffer_count);

		utl::vector<u32>& indices{ m_deferredFreeIndices[frameIdx] };
		if (!indices.empty())
		{
			for (auto index : indices)
			{
				_size--;
				m_freeHandles[_size] = index;
			}
			indices.clear();
		}
	}
	
	void descriptor_heap::release()
	{
		assert(!_size);
		core::deferred_release(m_heap);
	}

	DescriptorHandle descriptor_heap::Allocate()
	{
		std::lock_guard lock{ m_mutex };
		assert(m_heap);
		assert(_size < _capacity);

		const u32 index{ m_freeHandles[_size] };
		const u32 offset{ index * m_descriptorSize };
		_size++;

		DescriptorHandle handle;
		handle.cpu.ptr = m_cpuStart.ptr + offset;
		
		if (IsShaderVisible())
		{
			handle.gpu.ptr = m_gpuStart.ptr + offset;
		}

		handle.index = index;
		DEBUG_OP(handle.container = this);

		return handle;
	}

	void descriptor_heap::Free(DescriptorHandle& handle)
	{
		if (!handle.IsVaild()) return;
		std::lock_guard lock{ m_mutex };
		assert(m_heap && _size);
		assert(handle.container = this);
		assert(handle.cpu.ptr >= m_cpuStart.ptr);
		assert((handle.cpu.ptr - m_cpuStart.ptr) % m_descriptorSize == 0);
		assert(handle.index < _capacity);
		const u32 index{ (u32)(handle.cpu.ptr - m_cpuStart.ptr) / m_descriptorSize };
		assert(handle.index == index);

		const u32 frameIdx{ core::current_frame_index() };
		m_deferredFreeIndices[frameIdx].push_back(index);
		core::set_deferred_releases_flag();
		handle = {};
	}

	//// D3D12 TEXTURE ////////////////////////////////////////////////////////////////////////////
	D3D12Texture::D3D12Texture(D3D12TextureInitInfo info)
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
			m_resource = info.resource;
		}
		else if (info.heap && info.desc)
		{
			assert(!info.resource);
			DXCall(device->CreatePlacedResource(
				info.heap, info.allocationInfo.Offset, info.desc, info.initialState,
				clear_value, IID_PPV_ARGS(&m_resource)));
		}
		else if(info.desc)
		{
			assert(!info.resource);

			DXCall(device->CreateCommittedResource(
				&d3dx::heap_properties.default_heap, D3D12_HEAP_FLAG_NONE, info.desc, info.initialState,
				clear_value, IID_PPV_ARGS(&m_resource)));
		}

		assert(m_resource);
		m_srv = core::srv_heap().Allocate();
		device->CreateShaderResourceView(m_resource, info.srvDesc, m_srv.cpu);
	}

	void D3D12Texture::release()
	{
		core::srv_heap().Free(m_srv);
		core::deferred_release(m_resource);
	}

	//// RENDER TEXTURE ///////////////////////////////////////////////////////////////////////////
	d3d12_render_texture::d3d12_render_texture(D3D12TextureInitInfo info) : m_texture{ info }
	{
		m_mipCount = resource()->GetDesc().MipLevels;
		assert(m_mipCount && m_mipCount <= D3D12Texture::maxMips);

		descriptor_heap& rtvHeap{ core::rtv_heap() };
		D3D12_RENDER_TARGET_VIEW_DESC desc{};
		desc.Format = info.desc->Format;
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;

		auto* const device{ core::device() };
		assert(device);

		for (u32 i{ 0 }; i < m_mipCount; i++)
		{
			m_rtv[i] = rtvHeap.Allocate();
			device->CreateRenderTargetView(resource(), &desc, m_rtv[i].cpu);
			++desc.Texture2D.MipSlice;
		}
	}

	void d3d12_render_texture::release()
	{
		for (u32 i{ 0 }; i < m_mipCount; i++) core::rtv_heap().Free(m_rtv[i]);
		m_texture.release();
		m_mipCount = 0;
	}

	//// DEPTH BUFFER /////////////////////////////////////////////////////////////////////////////
	d3d12_depth_buffer::d3d12_depth_buffer(D3D12TextureInitInfo info)
	{
		assert(info.desc);
		const DXGI_FORMAT dsvFormat{ info.desc->Format };

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		// depth stencil view cannot be the same format as the shader resource view
		if (info.desc->Format == DXGI_FORMAT_D32_FLOAT)
		{
			info.desc->Format = DXGI_FORMAT_R32_TYPELESS;
			srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		}

		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.PlaneSlice = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

		assert(!info.srvDesc && !info.resource);
		info.srvDesc = &srvDesc;
		m_texture = D3D12Texture(info);

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.Format = dsvFormat; // Now that info was used to make the texture, revert the depth stencil view to it's previous format
		dsvDesc.Texture2D.MipSlice = 0;

		m_dsv = core::dsv_heap().Allocate();
		auto* const device{ core::device() };
		assert(device);
		device->CreateDepthStencilView(resource(), &dsvDesc, m_dsv.cpu);
	}
	
	void d3d12_depth_buffer::release()
	{
		core::dsv_heap().Free(m_dsv);
		m_texture.release();
	}
}
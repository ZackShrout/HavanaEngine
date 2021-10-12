#include "D3D12Resources.h"
#include "D3D12Core.h"
#include "D3D12Helpers.h"

namespace Havana::Graphics::D3D12
{
	//// DESCRIPTOR HEAP //////////////////////////////////////////////////////////////////////////
	bool DescriptorHeap::Initialize(u32 capacity, bool isShaderVisible)
	{
		std::lock_guard lock{ m_mutex };
		assert(capacity && capacity < D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2);
		assert(!(m_type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER &&
			capacity > D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE));
		
		if (m_type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV ||
			m_type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
		{
			isShaderVisible = false;
		}

		Release();

		auto* const device{ Core::Device() };
		assert(device);

		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.Flags = isShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NodeMask = 0;
		desc.NumDescriptors = capacity;
		desc.Type = m_type;

		HRESULT hr{ S_OK };
		DXCall(hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_heap)));
		if (FAILED(hr)) return false;

		m_freeHandles = std::move(std::make_unique<u32[]>(capacity));
		m_capacity = capacity;
		m_size = 0;

		for (u32 i{ 0 }; i < capacity; i++)
			m_freeHandles[i] = i;

		DEBUG_OP(for (u32 i{ 0 }; i < frameBufferCount; i++) assert(m_deferredFreeIndices[i].empty()));

		m_descriptorSize = device->GetDescriptorHandleIncrementSize(m_type);
		m_cpuStart = m_heap->GetCPUDescriptorHandleForHeapStart();
		m_gpuStart = isShaderVisible ? m_heap->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE{ 0 };

		return true;
	}
	
	void DescriptorHeap::ProcessDeferredFree(u32 frameIdx)
	{
		std::lock_guard lock{ m_mutex };
		assert(frameIdx < frameBufferCount);

		Utils::vector<u32>& indices{ m_deferredFreeIndices[frameIdx] };
		if (!indices.empty())
		{
			for (auto index : indices)
			{
				m_size--;
				m_freeHandles[m_size] = index;
			}
			indices.clear();
		}
	}
	
	void DescriptorHeap::Release()
	{
		assert(!m_size);
		Core::DeferredRelease(m_heap);
	}

	DescriptorHandle DescriptorHeap::Allocate()
	{
		std::lock_guard lock{ m_mutex };
		assert(m_heap);
		assert(m_size < m_capacity);

		const u32 index{ m_freeHandles[m_size] };
		const u32 offset{ index * m_descriptorSize };
		m_size++;

		DescriptorHandle handle;
		handle.cpu.ptr = m_cpuStart.ptr + offset;
		
		if (IsShaderVisible())
		{
			handle.gpu.ptr = m_gpuStart.ptr + offset;
		}

		DEBUG_OP(handle.container = this);
		DEBUG_OP(handle.index = index);

		return handle;
	}

	void DescriptorHeap::Free(DescriptorHandle& handle)
	{
		if (!handle.IsVaild()) return;
		std::lock_guard lock{ m_mutex };
		assert(m_heap && m_size);
		assert(handle.container = this);
		assert(handle.cpu.ptr >= m_cpuStart.ptr);
		assert((handle.cpu.ptr - m_cpuStart.ptr) % m_descriptorSize == 0);
		assert(handle.index < m_capacity);
		const u32 index{ (u32)(handle.cpu.ptr - m_cpuStart.ptr) / m_descriptorSize };
		assert(handle.index == index);

		const u32 frameIdx{ Core::CurrentFrameIndex() };
		m_deferredFreeIndices[frameIdx].push_back(index);
		Core::SetDeferredReleasesFlag();
		handle = {};
	}

	//// D3D12 TEXTURE ////////////////////////////////////////////////////////////////////////////
	D3D12Texture::D3D12Texture(D3D12TextureInitInfo info)
	{
		auto* const device{ Core::Device() };
		assert(device);

		D3D12_CLEAR_VALUE* const clearValue
		{
			(info.desc &&
			(info.desc->Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET ||
				info.desc->Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
			? &info.clearValue : nullptr
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
				clearValue, IID_PPV_ARGS(&m_resource)));
		}
		else if(info.desc)
		{
			assert(!info.resource);

			DXCall(device->CreateCommittedResource(
				&D3DX::heapProperties.defaultHeap, D3D12_HEAP_FLAG_NONE, info.desc, info.initialState,
				clearValue, IID_PPV_ARGS(&m_resource)));
		}

		assert(m_resource);
		m_srv = Core::SRVHeap().Allocate();
		device->CreateShaderResourceView(m_resource, info.srvDesc, m_srv.cpu);
	}

	void D3D12Texture::Release()
	{
		Core::SRVHeap().Free(m_srv);
		Core::DeferredRelease(m_resource);
	}

	//// RENDER TEXTURE ///////////////////////////////////////////////////////////////////////////
	D3D12RenderTexture::D3D12RenderTexture(D3D12TextureInitInfo info) : m_texture{ info }
	{
		m_mipCount = Resource()->GetDesc().MipLevels;
		assert(m_mipCount && m_mipCount <= D3D12Texture::maxMips);

		DescriptorHeap& rtvHeap{ Core::RTVHeap() };
		D3D12_RENDER_TARGET_VIEW_DESC desc{};
		desc.Format = info.desc->Format;
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;

		auto* const device{ Core::Device() };
		assert(device);

		for (u32 i{ 0 }; i < m_mipCount; i++)
		{
			m_rtv[i] = rtvHeap.Allocate();
			device->CreateRenderTargetView(Resource(), &desc, m_rtv[i].cpu);
			++desc.Texture2D.MipSlice;
		}
	}

	void D3D12RenderTexture::Release()
	{
		for (u32 i{ 0 }; i < m_mipCount; i++) Core::RTVHeap().Free(m_rtv[i]);
		m_mipCount = 0;
	}

	//// DEPTH BUFFER /////////////////////////////////////////////////////////////////////////////
	D3D12DepthBuffer::D3D12DepthBuffer(D3D12TextureInitInfo info)
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

		m_dsv = Core::DSVHeap().Allocate();
		auto* const device{ Core::Device() };
		assert(device);
		device->CreateDepthStencilView(Resource(), &dsvDesc, m_dsv.cpu);
	}
	
	void D3D12DepthBuffer::Release()
	{
		Core::DSVHeap().Free(m_dsv);
		m_texture.Release();
	}
}
#include "D3D12Resources.h"
#include "D3D12Core.h"

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

		ID3D12Device* const device{ Core::Device() };
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
}
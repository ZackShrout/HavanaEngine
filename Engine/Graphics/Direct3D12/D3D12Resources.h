#pragma once
#include "D3D12CommonHeaders.h"

namespace Havana::Graphics::D3D12
{
	class DescriptorHeap;

	struct DescriptorHandle
	{
		D3D12_CPU_DESCRIPTOR_HANDLE cpu{};
		D3D12_GPU_DESCRIPTOR_HANDLE gpu{};

		constexpr bool IsVaild() const { return cpu.ptr != 0; }
		constexpr bool IsShaderVisible() const { return gpu.ptr != 0; }
#ifdef _DEBUG
	private:
		friend class DescriptorHeap;
		DescriptorHeap* container{ nullptr };
		u32				index{ U32_INVALID_ID };
#endif // _DEBUG

	};

	class DescriptorHeap
	{
	public:
		explicit DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type) : m_type{ type } {}
		DISABLE_COPY_AND_MOVE(DescriptorHeap);
		~DescriptorHeap() { assert(!m_heap); }

		// Implemented in the translation unit for this header
		bool Initialize(u32 capacity, bool isShaderVisible);
		void ProcessDeferredFree(u32 frameIdx);
		void Release();
		[[nodiscard]] DescriptorHandle Allocate();
		void Free(DescriptorHandle& handle);

		constexpr D3D12_DESCRIPTOR_HEAP_TYPE Type() { return m_type; }
		constexpr D3D12_CPU_DESCRIPTOR_HANDLE CpuStart() { return m_cpuStart; }
		constexpr D3D12_GPU_DESCRIPTOR_HANDLE GpuStart() { return m_gpuStart; }
		constexpr ID3D12DescriptorHeap* const Heap() { return m_heap; }
		constexpr u32 Capactity() { return m_capacity; }
		constexpr u32 Size() { return m_size; }
		constexpr u32 DescriptorSize() { return m_descriptorSize; }
		constexpr bool IsShaderVisible() { return m_gpuStart.ptr != 0; }

	private:
		ID3D12DescriptorHeap*				m_heap;
		D3D12_CPU_DESCRIPTOR_HANDLE			m_cpuStart{};
		D3D12_GPU_DESCRIPTOR_HANDLE			m_gpuStart{};
		std::unique_ptr<u32[]>				m_freeHandles{};
		Utils::vector<u32>					m_deferredFreeIndices[frameBufferCount]{};
		std::mutex							m_mutex{};
		u32									m_capacity{ 0 };
		u32									m_size{ 0 };
		u32									m_descriptorSize{ 0 };
		const D3D12_DESCRIPTOR_HEAP_TYPE	m_type{};
	};

	struct D3D12TextureInitInfo
	{
		ID3D12Heap1*						heap{ nullptr };
		ID3D12Resource*						resource{ nullptr };
		D3D12_SHADER_RESOURCE_VIEW_DESC*	srvDesc{ nullptr };
		D3D12_RESOURCE_DESC*				desc{ nullptr };
		D3D12_RESOURCE_ALLOCATION_INFO1		allocationInfo{};
		D3D12_RESOURCE_STATES				initialState{};
		D3D12_CLEAR_VALUE					clearValue{};
	};
	
	class D3D12Texture
	{
	public:
		D3D12Texture() = default;
		explicit D3D12Texture(D3D12TextureInitInfo info);
		DISABLE_COPY(D3D12Texture);
		constexpr D3D12Texture(D3D12Texture&& o) : m_resource{ o.m_resource }, m_srv{ o.m_srv }
		{
			o.Reset();
		}
		constexpr D3D12Texture& operator=(D3D12Texture&& o)
		{
			assert(this != &o);
			if (this != &o)
			{
				Release();
				Move(o);
			}
			return *this;
		}

		void Release();
		constexpr ID3D12Resource* const Resource() const { return m_resource; }
		constexpr DescriptorHandle SRV() const { return m_srv; }

	private:
		constexpr void Move(D3D12Texture& o)
		{
			m_resource = o.m_resource;
			m_srv = o.m_srv;
			o.Reset();
		}

		constexpr void Reset()
		{
			m_resource = nullptr;
			m_srv = {};
		}

		ID3D12Resource* m_resource{ nullptr };
		DescriptorHandle m_srv;
	};
}
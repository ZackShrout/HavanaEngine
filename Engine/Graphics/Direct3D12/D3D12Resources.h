#pragma once
#include "D3D12CommonHeaders.h"

namespace havana::graphics::d3d12
{
	class DescriptorHeap;

	struct DescriptorHandle
	{
		D3D12_CPU_DESCRIPTOR_HANDLE cpu{};
		D3D12_GPU_DESCRIPTOR_HANDLE gpu{};
		u32							index{ u32_invalid_id };


		[[nodiscard]] constexpr bool IsVaild() const { return cpu.ptr != 0; }
		[[nodiscard]] constexpr bool IsShaderVisible() const { return gpu.ptr != 0; }
#ifdef _DEBUG
	private:
		friend class DescriptorHeap;
		DescriptorHeap* container{ nullptr };
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

		[[nodiscard]] constexpr D3D12_DESCRIPTOR_HEAP_TYPE type() { return m_type; }
		[[nodiscard]] constexpr D3D12_CPU_DESCRIPTOR_HANDLE CpuStart() { return m_cpuStart; }
		[[nodiscard]] constexpr D3D12_GPU_DESCRIPTOR_HANDLE GpuStart() { return m_gpuStart; }
		[[nodiscard]] constexpr ID3D12DescriptorHeap* const Heap() { return m_heap; }
		[[nodiscard]] constexpr u32 Capactity() { return m_capacity; }
		[[nodiscard]] constexpr u32 Size() { return m_size; }
		[[nodiscard]] constexpr u32 DescriptorSize() { return m_descriptorSize; }
		[[nodiscard]] constexpr bool IsShaderVisible() { return m_gpuStart.ptr != 0; }

	private:
		ID3D12DescriptorHeap*				m_heap;
		D3D12_CPU_DESCRIPTOR_HANDLE			m_cpuStart{};
		D3D12_GPU_DESCRIPTOR_HANDLE			m_gpuStart{};
		std::unique_ptr<u32[]>				m_freeHandles{};
		utl::vector<u32>					m_deferredFreeIndices[frameBufferCount]{};
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
		constexpr static u32 maxMips{ 14 }; // Support up to 16k resolution
		D3D12Texture() = default;
		explicit D3D12Texture(D3D12TextureInitInfo info);
		~D3D12Texture() { Release(); }
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
		[[nodiscard]] constexpr ID3D12Resource* const Resource() const { return m_resource; }
		[[nodiscard]] constexpr DescriptorHandle SRV() const { return m_srv; }

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

	class D3D12RenderTexture
	{
	public:
		D3D12RenderTexture() = default;
		explicit D3D12RenderTexture(D3D12TextureInitInfo info);
		~D3D12RenderTexture() { Release(); }
		DISABLE_COPY(D3D12RenderTexture);
		constexpr D3D12RenderTexture(D3D12RenderTexture&& o) : m_texture{ std::move(o.m_texture) }, m_mipCount{ o.m_mipCount }
		{
			for (u32 i{ 0 }; i < m_mipCount; i++) m_rtv[i] = o.m_rtv[i];
			o.Reset();
		}
		constexpr D3D12RenderTexture& operator=(D3D12RenderTexture&& o)
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
		[[nodiscard]] constexpr u32 MipCount() const { return m_mipCount; }
		[[nodiscard]] constexpr D3D12_CPU_DESCRIPTOR_HANDLE RTV(u32 mipIndex) const { assert(mipIndex < m_mipCount); return m_rtv[mipIndex].cpu; }
		[[nodiscard]] constexpr DescriptorHandle SRV() const { return m_texture.SRV(); }
		[[nodiscard]] constexpr ID3D12Resource* const Resource() const { return m_texture.Resource(); }

	private:
		constexpr void Move(D3D12RenderTexture& o)
		{
			m_texture = std::move(o.m_texture);
			m_mipCount = o.m_mipCount;
			for (u32 i{ 0 }; i < m_mipCount; i++) m_rtv[i] = o.m_rtv[i];
			o.Reset();
		}

		constexpr void Reset()
		{
			for (u32 i{ 0 }; i < m_mipCount; i++) m_rtv[i] = {};
			m_mipCount = 0;
		}

		D3D12Texture		m_texture{};
		DescriptorHandle	m_rtv[D3D12Texture::maxMips]{};
		u32					m_mipCount{ 0 };
	};

	class D3D12DepthBuffer
	{
	public:
		D3D12DepthBuffer() = default;
		explicit D3D12DepthBuffer(D3D12TextureInitInfo info);
		~D3D12DepthBuffer() { Release(); }
		DISABLE_COPY(D3D12DepthBuffer);
		constexpr D3D12DepthBuffer(D3D12DepthBuffer&& o) : m_texture{ std::move(o.m_texture) }, m_dsv{ o.m_dsv }
		{
			o.m_dsv = {};
		}
		constexpr D3D12DepthBuffer& operator=(D3D12DepthBuffer&& o)
		{
			assert(this != &o);
			if (this != &o)
			{
				m_texture = std::move(o.m_texture);
				m_dsv = o.m_dsv;
				o.m_dsv = {};
			}
			return *this;
		}

		void Release();
		[[nodiscard]] constexpr D3D12_CPU_DESCRIPTOR_HANDLE DSV() const { return m_dsv.cpu; }
		[[nodiscard]] constexpr DescriptorHandle SRV() const { return m_texture.SRV(); }
		[[nodiscard]] constexpr ID3D12Resource* const Resource() const { return m_texture.Resource(); }

	private:
		D3D12Texture		m_texture{};
		DescriptorHandle	m_dsv{};
	};
}
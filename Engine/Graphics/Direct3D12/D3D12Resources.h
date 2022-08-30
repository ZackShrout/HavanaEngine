#pragma once
#include "D3D12CommonHeaders.h"

namespace havana::graphics::d3d12
{
	class descriptor_heap;

	struct DescriptorHandle
	{
		D3D12_CPU_DESCRIPTOR_HANDLE cpu{};
		D3D12_GPU_DESCRIPTOR_HANDLE gpu{};
		u32							index{ u32_invalid_id };


		[[nodiscard]] constexpr bool IsVaild() const { return cpu.ptr != 0; }
		[[nodiscard]] constexpr bool IsShaderVisible() const { return gpu.ptr != 0; }
#ifdef _DEBUG
	private:
		friend class descriptor_heap;
		descriptor_heap* container{ nullptr };
#endif // _DEBUG

	};

	class descriptor_heap
	{
	public:
		explicit descriptor_heap(D3D12_DESCRIPTOR_HEAP_TYPE type) : _type{ type } {}
		DISABLE_COPY_AND_MOVE(descriptor_heap);
		~descriptor_heap() { assert(!m_heap); }

		// Implemented in the translation unit for this header
		bool initialize(u32 capacity, bool isShaderVisible);
		void process_deferred_free(u32 frameIdx);
		void release();
		[[nodiscard]] DescriptorHandle Allocate();
		void Free(DescriptorHandle& handle);

		[[nodiscard]] constexpr D3D12_DESCRIPTOR_HEAP_TYPE type() { return _type; }
		[[nodiscard]] constexpr D3D12_CPU_DESCRIPTOR_HANDLE CpuStart() { return m_cpuStart; }
		[[nodiscard]] constexpr D3D12_GPU_DESCRIPTOR_HANDLE GpuStart() { return m_gpuStart; }
		[[nodiscard]] constexpr ID3D12DescriptorHeap* const Heap() { return m_heap; }
		[[nodiscard]] constexpr u32 Capactity() { return _capacity; }
		[[nodiscard]] constexpr u32 Size() { return _size; }
		[[nodiscard]] constexpr u32 DescriptorSize() { return m_descriptorSize; }
		[[nodiscard]] constexpr bool IsShaderVisible() { return m_gpuStart.ptr != 0; }

	private:
		ID3D12DescriptorHeap*				m_heap;
		D3D12_CPU_DESCRIPTOR_HANDLE			m_cpuStart{};
		D3D12_GPU_DESCRIPTOR_HANDLE			m_gpuStart{};
		std::unique_ptr<u32[]>				m_freeHandles{};
		utl::vector<u32>					m_deferredFreeIndices[frame_buffer_count]{};
		std::mutex							m_mutex{};
		u32									_capacity{ 0 };
		u32									_size{ 0 };
		u32									m_descriptorSize{ 0 };
		const D3D12_DESCRIPTOR_HEAP_TYPE	_type{};
	};

	struct D3D12TextureInitInfo
	{
		ID3D12Heap1*						heap{ nullptr };
		ID3D12Resource*						resource{ nullptr };
		D3D12_SHADER_RESOURCE_VIEW_DESC*	srvDesc{ nullptr };
		D3D12_RESOURCE_DESC*				desc{ nullptr };
		D3D12_RESOURCE_ALLOCATION_INFO1		allocationInfo{};
		D3D12_RESOURCE_STATES				initialState{};
		D3D12_CLEAR_VALUE					clear_value{};
	};
	
	class D3D12Texture
	{
	public:
		constexpr static u32 maxMips{ 14 }; // Support up to 16k resolution
		D3D12Texture() = default;
		explicit D3D12Texture(D3D12TextureInitInfo info);
		~D3D12Texture() { release(); }
		DISABLE_COPY(D3D12Texture);
		constexpr D3D12Texture(D3D12Texture&& o) : m_resource{ o.m_resource }, m_srv{ o.m_srv }
		{
			o.reset();
		}
		constexpr D3D12Texture& operator=(D3D12Texture&& o)
		{
			assert(this != &o);
			if (this != &o)
			{
				release();
				move(o);
			}
			return *this;
		}

		void release();
		[[nodiscard]] constexpr ID3D12Resource* const resource() const { return m_resource; }
		[[nodiscard]] constexpr DescriptorHandle SRV() const { return m_srv; }

	private:
		constexpr void move(D3D12Texture& o)
		{
			m_resource = o.m_resource;
			m_srv = o.m_srv;
			o.reset();
		}

		constexpr void reset()
		{
			m_resource = nullptr;
			m_srv = {};
		}

		ID3D12Resource* m_resource{ nullptr };
		DescriptorHandle m_srv;
	};

	class d3d12_render_texture
	{
	public:
		d3d12_render_texture() = default;
		explicit d3d12_render_texture(D3D12TextureInitInfo info);
		~d3d12_render_texture() { release(); }
		DISABLE_COPY(d3d12_render_texture);
		constexpr d3d12_render_texture(d3d12_render_texture&& o) : m_texture{ std::move(o.m_texture) }, m_mipCount{ o.m_mipCount }
		{
			for (u32 i{ 0 }; i < m_mipCount; i++) m_rtv[i] = o.m_rtv[i];
			o.reset();
		}
		constexpr d3d12_render_texture& operator=(d3d12_render_texture&& o)
		{
			assert(this != &o);
			if (this != &o)
			{
				release();
				move(o);
			}
			return *this;
		}

		void release();
		[[nodiscard]] constexpr u32 MipCount() const { return m_mipCount; }
		[[nodiscard]] constexpr D3D12_CPU_DESCRIPTOR_HANDLE rtv(u32 mipIndex) const { assert(mipIndex < m_mipCount); return m_rtv[mipIndex].cpu; }
		[[nodiscard]] constexpr DescriptorHandle SRV() const { return m_texture.SRV(); }
		[[nodiscard]] constexpr ID3D12Resource* const resource() const { return m_texture.resource(); }

	private:
		constexpr void move(d3d12_render_texture& o)
		{
			m_texture = std::move(o.m_texture);
			m_mipCount = o.m_mipCount;
			for (u32 i{ 0 }; i < m_mipCount; i++) m_rtv[i] = o.m_rtv[i];
			o.reset();
		}

		constexpr void reset()
		{
			for (u32 i{ 0 }; i < m_mipCount; i++) m_rtv[i] = {};
			m_mipCount = 0;
		}

		D3D12Texture		m_texture{};
		DescriptorHandle	m_rtv[D3D12Texture::maxMips]{};
		u32					m_mipCount{ 0 };
	};

	class d3d12_depth_buffer
	{
	public:
		d3d12_depth_buffer() = default;
		explicit d3d12_depth_buffer(D3D12TextureInitInfo info);
		~d3d12_depth_buffer() { release(); }
		DISABLE_COPY(d3d12_depth_buffer);
		constexpr d3d12_depth_buffer(d3d12_depth_buffer&& o) : m_texture{ std::move(o.m_texture) }, m_dsv{ o.m_dsv }
		{
			o.m_dsv = {};
		}
		constexpr d3d12_depth_buffer& operator=(d3d12_depth_buffer&& o)
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

		void release();
		[[nodiscard]] constexpr D3D12_CPU_DESCRIPTOR_HANDLE dsv() const { return m_dsv.cpu; }
		[[nodiscard]] constexpr DescriptorHandle SRV() const { return m_texture.SRV(); }
		[[nodiscard]] constexpr ID3D12Resource* const resource() const { return m_texture.resource(); }

	private:
		D3D12Texture		m_texture{};
		DescriptorHandle	m_dsv{};
	};
}
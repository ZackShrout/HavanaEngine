#pragma once
#include "D3D12CommonHeaders.h"

namespace havana::Graphics::D3D12::Upload
{
	class D3D12UploadContext
	{
	public:
		D3D12UploadContext(u32 alignedSize);
		DISABLE_COPY_AND_MOVE(D3D12UploadContext);
		~D3D12UploadContext() { assert(m_frameIndex == u32_invalid_id); }

		void EndUpload();

		[[nodiscard]] constexpr id3d12GraphicsCommandList* const CommandList() const { return m_cmdList; }
		[[nodiscard]] constexpr ID3D12Resource* const UploadBuffer() const { return m_uploadBuffer; }
		[[nodiscard]] constexpr void* const CPUAddress() const { return m_cpuAddress; }

	private:
		DEBUG_OP(D3D12UploadContext() = default);
		id3d12GraphicsCommandList*		m_cmdList{ nullptr };
		ID3D12Resource*					m_uploadBuffer{ nullptr };
		void*							m_cpuAddress{ nullptr };
		u32								m_frameIndex{ u32_invalid_id };
	};

	bool Initialize();
	void Shutdown();
}
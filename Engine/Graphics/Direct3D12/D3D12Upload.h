#pragma once
#include "D3D12CommonHeaders.h"

namespace Havana::Graphics::D3D12::Upload
{
	class D3D12UploadContext
	{
	public:
		D3D12UploadContext(u32 alignedSize);
		DISABLE_COPY_AND_MOVE(D3D12UploadContext);
		~D3D12UploadContext() {}

		void EndUpload();

		constexpr id3d12GraphicsCommandList* const CommandList() const { return m_cmdList; }
		constexpr ID3D12Resource* const UploadBuffer() const { return m_uploadBuffer; }
		constexpr void* const CPUAddress() const { return m_cpuAddress; }

	private:
		id3d12GraphicsCommandList*		m_cmdList{ nullptr };
		ID3D12Resource*					m_uploadBuffer{ nullptr };
		void*							m_cpuAddress{ nullptr };
		u32								m_frameIndex{ U32_INVALID_ID };
	};

	bool Initialize();
	void Shutdown();
}
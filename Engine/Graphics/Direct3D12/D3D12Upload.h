#pragma once
#include "D3D12CommonHeaders.h"

namespace havana::graphics::d3d12::upload
{
	class d3d12_upload_context
	{
	public:
		d3d12_upload_context(u32 alignedSize);
		DISABLE_COPY_AND_MOVE(d3d12_upload_context);
		~d3d12_upload_context() { assert(_frame_index == u32_invalid_id); }

		void end_upload();

		[[nodiscard]] constexpr id3d12_graphics_command_list* const command_list() const { return m_cmdList; }
		[[nodiscard]] constexpr ID3D12Resource* const upload_buffer() const { return m_uploadBuffer; }
		[[nodiscard]] constexpr void* const cpu_address() const { return m_cpuAddress; }

	private:
		DEBUG_OP(d3d12_upload_context() = default);
		id3d12_graphics_command_list*		m_cmdList{ nullptr };
		ID3D12Resource*					m_uploadBuffer{ nullptr };
		void*							m_cpuAddress{ nullptr };
		u32								_frame_index{ u32_invalid_id };
	};

	bool initialize();
	void shutdown();
}
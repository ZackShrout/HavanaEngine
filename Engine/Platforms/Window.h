#pragma once

#include "CommonHeaders.h"

namespace havana::Platform
{
	DEFINE_TYPED_ID(window_id);

	class Window
	{
	public:
		constexpr Window() = default;
		constexpr explicit Window(window_id id) : m_id{ id } {}
		constexpr window_id GetID() const { return m_id; }
		constexpr bool is_valid() const { return id::is_valid(m_id); }

		void SetFullscreen(bool isFullscreen) const;
		bool IsFullscreen() const;
		void* Handle() const;
		void* Display() const;
		void SetCaption(const wchar_t* caption) const;
		math::u32v4 Size() const;
		void Resize(u32 width, u32 height) const;
		u32 Width() const;
		u32 Height() const;
		bool IsClosed() const;
		void Close();

	private:
		window_id m_id{ id::invalid_id };
	};
}
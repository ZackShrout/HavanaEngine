#pragma once

#include "CommonHeaders.h"

namespace havana::platform
{
	DEFINE_TYPED_ID(window_id);

	class window
	{
	public:
		constexpr window() = default;
		constexpr explicit window(window_id id) : _id{ id } {}
		constexpr window_id get_id() const { return _id; }
		constexpr bool is_valid() const { return id::is_valid(_id); }

		void set_fullscreen(bool isFullscreen) const;
		bool is_fullscreen() const;
		void* handle() const;
		void* display() const;
		void set_caption(const wchar_t* caption) const;
		math::u32v4 size() const;
		void resize(u32 width, u32 height) const;
		u32 width() const;
		u32 height() const;
		bool is_closed() const;
		void close();

	private:
		window_id _id{ id::invalid_id };
	};
}
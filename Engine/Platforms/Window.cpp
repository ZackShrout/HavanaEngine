#if INCLUDE_WINDOW_CPP
#include "Window.h"

namespace havana::platform
{
	void window::set_fullscreen(bool isFullscreen) const
	{
		assert(is_valid());
		set_window_fullscreen(m_id, isFullscreen);
	}

	bool window::is_fullscreen() const
	{
		assert(is_valid());
		return is_window_fullscreen(m_id);
	}

	void* window::handle() const
	{
		assert(is_valid());
		return get_window_handle(m_id);
	}

	void* window::display() const
	{
		assert(is_valid());
		return get_display(m_id);
	}

	void window::set_caption(const wchar_t* caption) const
	{
		assert(is_valid());
		set_window_caption(m_id, caption);
	}

	math::u32v4 window::size() const
	{
		assert(is_valid());
		return get_window_size(m_id);
	}

	void window::resize(u32 width, u32 height) const
	{
		assert(is_valid());
		resize_window(m_id, width, height);
	}

	u32 window::width() const
	{
		math::u32v4 s{ size() };
		return s.z - s.x;
	}

	u32 window::height() const
	{
		math::u32v4 s{ size() };
		return s.w - s.y;
	}

	bool window::is_closed() const
	{
		assert(is_valid());
		return is_window_closed(m_id);
	}

	void window::close()
	{
		set_window_closed(m_id);
	}
}
#endif // INCLUDE_WINDOW_CPP
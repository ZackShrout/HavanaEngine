#if INCLUDE_WINDOW_CPP
#include "Window.h"

namespace havana::Platform
{
	void Window::SetFullscreen(bool isFullscreen) const
	{
		assert(is_valid());
		SetWindowFullscreen(m_id, isFullscreen);
	}

	bool Window::IsFullscreen() const
	{
		assert(is_valid());
		return IsWindowFullscreen(m_id);
	}

	void* Window::Handle() const
	{
		assert(is_valid());
		return GetWindowHandle(m_id);
	}

	void* Window::Display() const
	{
		assert(is_valid());
		return GetDisplay(m_id);
	}

	void Window::SetCaption(const wchar_t* caption) const
	{
		assert(is_valid());
		SetWindowCaption(m_id, caption);
	}

	math::u32v4 Window::Size() const
	{
		assert(is_valid());
		return GetWindowSize(m_id);
	}

	void Window::Resize(u32 width, u32 height) const
	{
		assert(is_valid());
		ResizeWindow(m_id, width, height);
	}

	u32 Window::Width() const
	{
		math::u32v4 size{ Size() };
		return size.z - size.x;
	}

	u32 Window::Height() const
	{
		math::u32v4 size{ Size() };
		return size.w - size.y;
	}

	bool Window::IsClosed() const
	{
		assert(is_valid());
		return IsWindowClosed(m_id);
	}

	void Window::Close()
	{
		SetWindowClosed(m_id);
	}
}
#endif // INCLUDE_WINDOW_CPP
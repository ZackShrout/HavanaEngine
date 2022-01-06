#if INCLUDE_WINDOW_CPP
#include "Window.h"

namespace Havana::Platform
{
	void Window::SetFullscreen(bool isFullscreen) const
	{
		assert(IsValid());
		SetWindowFullscreen(m_id, isFullscreen);
	}

	bool Window::IsFullscreen() const
	{
		assert(IsValid());
		return IsWindowFullscreen(m_id);
	}

	void* Window::Handle() const
	{
		assert(IsValid());
		return GetWindowHandle(m_id);
	}

	void* Window::Display() const
	{
		assert(IsValid());
		return GetDisplay(m_id);
	}

	void Window::SetCaption(const wchar_t* caption) const
	{
		assert(IsValid());
		SetWindowCaption(m_id, caption);
	}

	Math::Vec4u32 Window::Size() const
	{
		assert(IsValid());
		return GetWindowSize(m_id);
	}

	void Window::Resize(u32 width, u32 height) const
	{
		assert(IsValid());
		ResizeWindow(m_id, width, height);
	}

	u32 Window::Width() const
	{
		Math::Vec4u32 size{ Size() };
		return size.z - size.x;
	}

	u32 Window::Height() const
	{
		Math::Vec4u32 size{ Size() };
		return size.w - size.y;
	}

	bool Window::IsClosed() const
	{
		assert(IsValid());
		return IsWindowClosed(m_id);
	}

	void Window::Close()
	{
		SetWindowClosed(m_id);
	}
}
#endif // INCLUDE_WINDOW_CPP
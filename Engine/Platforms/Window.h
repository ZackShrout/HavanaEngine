#pragma once

#include "..\Common\CommonHeaders.h"

namespace Havana::Platform
{
	DEFINE_TYPED_ID(window_id);

	class Window
	{
	public:
		constexpr Window() : id{ Id::INVALID_ID } {}
		constexpr explicit Window(window_id id) : id{ id } {}
		constexpr window_id GetID() const { return id; }
		constexpr bool IsValid() const { return Id::IsValid(id); }

		void SetFullscreen(bool isFullscreen) const;
		bool IsFullscreen() const;
		void* Handle() const;
		void SetCaption(const wchar_t* caption) const;
		Math::Vec4u32 Size() const;
		void Resize(u32 width, u32 height) const;
		u32 Width() const;
		u32 Height() const;
		bool IsClosed() const;

	private:
		window_id id{ Id::INVALID_ID };
	};
}
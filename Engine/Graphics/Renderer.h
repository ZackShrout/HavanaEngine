#pragma once
#include "../Common/CommonHeaders.h"
#include "../Platforms/Platform.h"

namespace Havana::Graphics
{
	DEFINE_TYPED_ID(surface_id);
	
	class Surface
	{
	public:
		constexpr Surface() = default;
		constexpr explicit Surface(surface_id id) : m_id{ id } {}
		constexpr surface_id GetID() const { return m_id; }
		constexpr bool IsValid() const { return Id::IsValid(m_id); }

		void Resize(u32 width, u32 height) const;
		u32 Width() const;
		u32 Height() const;
		void Render() const;

	private:
		surface_id m_id{ Id::INVALID_ID };
	};

	struct RenderSurface
	{
		Platform::Window window{};
		Surface surface{};
	};

	enum GraphicsPlatform : u32
	{
		Direct3D12 = 0,
		OpenGL = 1
	};
	
	bool Initialize(GraphicsPlatform platform);
	void Shutdown();

	Surface CreateSurface(Platform::Window window);
	void RemoveSurface(surface_id id);
}
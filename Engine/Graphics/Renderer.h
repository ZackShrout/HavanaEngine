#pragma once
#include "CommonHeaders.h"
#include "../Platforms/Platform.h"
#include "EngineAPI/Camera.h"

namespace havana::graphics
{
	DEFINE_TYPED_ID(surface_id);
	
	class surface
	{
	public:
		constexpr surface() = default;
		constexpr explicit surface(surface_id id) : m_id{ id } {}
		constexpr surface_id get_id() const { return m_id; }
		constexpr bool is_valid() const { return id::is_valid(m_id); }

		void Resize(u32 width, u32 height) const;
		u32 Width() const;
		u32 Height() const;
		void Render() const;

	private:
		surface_id m_id{ id::invalid_id };
	};

	struct RenderSurface
	{
		platform::window window{};
		surface surface{};
	};

	struct CameraParameter
	{
		enum Parameter : u32
		{
			UpVector,
			FieldOfView,
			AspectRatio,
			ViewWidth,
			ViewHeight,
			NearZ,
			FarZ,
			View,
			Projection,
			InverseProjection,
			ViewProjection,
			InverseViewProjection,
			type,
			EntityID,

			count
		};
	};

	struct CameraInitInfo
	{
		id::id_type		entityId{ id::invalid_id };
		Camera::type	type{};
		math::v3		up;
		f32				nearZ;
		f32				farZ;
		union
		{
			f32	fieldOfView;
			f32	viewWidth;
		};
		union
		{
			f32	aspectRatio;
			f32	viewHeight;
		};
	};

	struct PerspectiveCameraInitInfo : public CameraInitInfo
	{
		explicit PerspectiveCameraInitInfo(id::id_type id)
		{
			assert(id::is_valid(id));
			entityId = id;
			type = Camera::Perspective;
			up = { 0.0f, 1.0f, 0.0f };
			nearZ = 0.001f;
			farZ = 10000.0f;
			fieldOfView = 0.25f;
			aspectRatio = 16.0f / 10.0f;
		}
	};

	struct OrthographicCameraInitInfo : public CameraInitInfo
	{
		explicit OrthographicCameraInitInfo(id::id_type id)
		{
			assert(id::is_valid(id));
			entityId = id;
			type = Camera::Orthographic;
			up = { 0.0f, 1.0f, 0.0f };
			nearZ = 0.001f;
			farZ = 10000.0f;
			viewHeight = 1920;
			viewWidth = 1080;
		}
	};

	enum graphics_platform : u32
	{
		Direct3D12 = 0,
		vulkan_1 = 1,
		OpenGraphicsL = 2
	};
	
	bool Initialize(graphics_platform platform);
	void Shutdown();

	// Get the location of the compiled engine shaders relative to the executable's path.
	// The path is for the graphics API that is currently in use.
	const char* get_engine_shaders_path();
	// Get the location of the compiled engine shaders, for the specified platform, relative to the executable's path.
	const char* get_engine_shaders_path(graphics_platform platform);

	surface CreateSurface(platform::window window);
	void RemoveSurface(surface_id id);

	Camera CreateCamera(CameraInitInfo info);
	void RemoveCamera(camera_id id);

	id::id_type add_submesh(const u8*& data);
	void remove_submesh(id::id_type id);
}
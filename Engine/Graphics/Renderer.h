#pragma once
#include "CommonHeaders.h"
#include "../Platforms/Platform.h"
#include "EngineAPI/Camera.h"

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
			Type,
			EntityID,

			Count
		};
	};

	struct CameraInitInfo
	{
		Id::id_type		entityId{ Id::INVALID_ID };
		Camera::Type	type{};
		Math::Vec3		up;
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
		explicit PerspectiveCameraInitInfo(Id::id_type id)
		{
			assert(Id::IsValid(id));
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
		explicit OrthographicCameraInitInfo(Id::id_type id)
		{
			assert(Id::IsValid(id));
			entityId = id;
			type = Camera::Orthographic;
			up = { 0.0f, 1.0f, 0.0f };
			nearZ = 0.001f;
			farZ = 10000.0f;
			viewHeight = 1920;
			viewWidth = 1080;
		}
	};

	enum GraphicsPlatform : u32
	{
		Direct3D12 = 0,
		VulkanAPI = 1,
		OpenGraphicsL = 2
	};
	
	bool Initialize(GraphicsPlatform platform);
	void Shutdown();

	// Get the location of the compiled engine shaders relative to the executable's path.
	// The path is for the graphics API that is currently in use.
	const char* GetEngineShadersPath();
	// Get the location of the compiled engine shaders, for the specified platform, relative to the executable's path.
	const char* GetEngineShadersPath(GraphicsPlatform platform);

	Surface CreateSurface(Platform::Window window);
	void RemoveSurface(surface_id id);

	Camera CreateCamera(CameraInitInfo info);
	void RemoveCamera(camera_id id);

	Id::id_type AddSubmesh(const u8*& data);
	void RemoveSubmesh(Id::id_type id);
}
#include "Renderer.h"
#include "GraphicsPlatformInterface.h"
#include "../Graphics/Direct3D12/D3D12Interface.h"
#include "../Graphics/Vulkan/VulkanInterface.h"
#include "../Graphics/OpenGL/OpenGLInterface.h"

namespace havana::Graphics
{
	namespace
	{
		// Defines where the compiled engine shaders files is located for each one of the supported APIs.
		constexpr const char* engineShaderPaths[]
		{
			"./Shaders/D3D12/shaders.bin",
			"./Shaders/Vulkan/shaders.bin"
		};

		PlatformInterface gfx{};

		bool SetGraphicsPlatform(GraphicsPlatform platform)
		{
			switch (platform)
			{
#ifdef _WIN64
			case GraphicsPlatform::Direct3D12:
				D3D12::GetPlatformInterface(gfx);
				break;
#endif // _WIN64
			case GraphicsPlatform::VulkanAPI:
				Vulkan::GetPlatformInterface(gfx);
				break;
			case GraphicsPlatform::OpenGraphicsL:
				OpenGL::GetPlatformInterface(gfx);
				break;
			default:
				return false;
			}
			assert(gfx.platform == platform);
			return true;
		}

	} // anonymous namespace

	bool Initialize(GraphicsPlatform platform)
	{
		return SetGraphicsPlatform(platform) && gfx.Initialize();
	}

	void Shutdown()
	{
		if (gfx.platform != (GraphicsPlatform)-1) gfx.Shutdown();
	}

	const char* GetEngineShadersPath()
	{
		return engineShaderPaths[(u32)gfx.platform];
	}

	const char* GetEngineShadersPath(GraphicsPlatform platform)
	{
		return engineShaderPaths[(u32)platform];
	}

	Surface CreateSurface(Platform::Window window)
	{
		return gfx.Surface.Create(window);
	}

	void RemoveSurface(surface_id id)
	{
		assert(Id::is_valid(id));
		gfx.Surface.Remove(id);
	}

	void Surface::Resize(u32 width, u32 height) const
	{
		assert(is_valid());
		return gfx.Surface.Resize(m_id, width, height);
	}

	u32 Surface::Width() const
	{
		assert(is_valid());
		return gfx.Surface.Width(m_id);
	}

	u32 Surface::Height() const
	{
		assert(is_valid());
		return gfx.Surface.Height(m_id);
	}

	void Surface::Render() const
	{
		assert(is_valid());
		gfx.Surface.Render(m_id);
	}

	Camera CreateCamera(CameraInitInfo info)
	{
		return gfx.Camera.Create(info);
	}

	void RemoveCamera(camera_id id)
	{
		gfx.Camera.Remove(id);
	}

	void Camera::Up(math::v3 up) const
	{
		assert(is_valid());
		gfx.Camera.SetParamter(m_id, CameraParameter::UpVector, &up, sizeof(up));
	}

	void Camera::FieldOfView(f32 fov) const
	{
		assert(is_valid());
		gfx.Camera.SetParamter(m_id, CameraParameter::FieldOfView, &fov, sizeof(fov));
	}

	void Camera::AspectRatio(f32 aspectRatio) const
	{
		assert(is_valid());
		gfx.Camera.SetParamter(m_id, CameraParameter::AspectRatio, &aspectRatio, sizeof(aspectRatio));
	}

	void Camera::ViewWidth(f32 width) const
	{
		assert(is_valid());
		gfx.Camera.SetParamter(m_id, CameraParameter::ViewWidth, &width, sizeof(width));
	}

	void Camera::ViewHeight(f32 height) const
	{
		assert(is_valid());
		gfx.Camera.SetParamter(m_id, CameraParameter::ViewHeight, &height, sizeof(height));
	}

	void Camera::Range(f32 nearZ, f32 farZ) const
	{
		assert(is_valid());
		gfx.Camera.SetParamter(m_id, CameraParameter::NearZ, &nearZ, sizeof(nearZ));
		gfx.Camera.SetParamter(m_id, CameraParameter::FarZ, &farZ, sizeof(farZ));
	}

	math::Mat4 Camera::View() const
	{
		assert(is_valid());
		math::Mat4 matrix;
		gfx.Camera.GetParamter(m_id, CameraParameter::View, &matrix, sizeof(matrix));
		return matrix;
	}

	math::Mat4 Camera::Projection() const
	{
		assert(is_valid());
		math::Mat4 matrix;
		gfx.Camera.GetParamter(m_id, CameraParameter::Projection, &matrix, sizeof(matrix));
		return matrix;
	}

	math::Mat4 Camera::InverseProjection() const
	{
		assert(is_valid());
		math::Mat4 matrix;
		gfx.Camera.GetParamter(m_id, CameraParameter::InverseProjection, &matrix, sizeof(matrix));
		return matrix;
	}

	math::Mat4 Camera::ViewProjection() const
	{
		assert(is_valid());
		math::Mat4 matrix;
		gfx.Camera.GetParamter(m_id, CameraParameter::ViewProjection, &matrix, sizeof(matrix));
		return matrix;
	}

	math::Mat4 Camera::InverseViewProjection() const
	{
		assert(is_valid());
		math::Mat4 matrix;
		gfx.Camera.GetParamter(m_id, CameraParameter::InverseViewProjection, &matrix, sizeof(matrix));
		return matrix;
	}

	math::v3 Camera::Up() const
	{
		assert(is_valid());
		math::v3 upVector;
		gfx.Camera.GetParamter(m_id, CameraParameter::UpVector, &upVector, sizeof(upVector));
		return upVector;
	}

	f32 Camera::NearZ() const
	{
		assert(is_valid());
		f32 nearZ;
		gfx.Camera.GetParamter(m_id, CameraParameter::NearZ, &nearZ, sizeof(nearZ));
		return nearZ;
	}

	f32 Camera::FarZ() const
	{
		assert(is_valid());
		f32 farZ;
		gfx.Camera.GetParamter(m_id, CameraParameter::FarZ, &farZ, sizeof(farZ));
		return farZ;
	}

	f32 Camera::FieldOfView() const
	{
		assert(is_valid());
		f32 fov;
		gfx.Camera.GetParamter(m_id, CameraParameter::FieldOfView, &fov, sizeof(fov));
		return fov;
	}

	f32 Camera::AspectRatio() const
	{
		assert(is_valid());
		f32 aspectRatio;
		gfx.Camera.GetParamter(m_id, CameraParameter::AspectRatio, &aspectRatio, sizeof(aspectRatio));
		return aspectRatio;
	}

	f32 Camera::ViewWidth() const
	{
		assert(is_valid());
		f32 width;
		gfx.Camera.GetParamter(m_id, CameraParameter::ViewWidth, &width, sizeof(width));
		return width;
	}

	f32 Camera::ViewHeight() const
	{
		assert(is_valid());
		f32 height;
		gfx.Camera.GetParamter(m_id, CameraParameter::ViewHeight, &height, sizeof(height));
		return height;
	}

	Camera::type Camera::ProjectionType() const
	{
		assert(is_valid());
		type type;
		gfx.Camera.GetParamter(m_id, CameraParameter::type, &type, sizeof(type));
		return type;
	}

	Id::id_type Camera::EntityID() const
	{
		assert(is_valid());
		Id::id_type id;
		gfx.Camera.GetParamter(m_id, CameraParameter::EntityID, &id, sizeof(id));
		return id;
	}

	Id::id_type AddSubmesh(const u8*& data)
	{
		return gfx.Resources.AddSubmesh(data);
	}

	void RemoveSubmesh(Id::id_type id)
	{
		gfx.Resources.RemoveSubmesh(id);
	}
}
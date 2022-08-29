#include "Renderer.h"
#include "GraphicsPlatformInterface.h"
#include "Graphics/Direct3D12/D3D12Interface.h"
#include "Graphics/Vulkan/VulkanInterface.h"

namespace havana::graphics
{
	namespace
	{
		// Defines where the compiled engine shaders files is located for each one of the supported APIs.
		constexpr const char* engineShaderPaths[]
		{
			"./Shaders/D3D12/shaders.bin",
			"./Shaders/Vulkan/shaders.bin"
		};

		platform_interface gfx{};

		bool set_graphics_platform(graphics_platform platform)
		{
			switch (platform)
			{
#ifdef _WIN64
			case graphics_platform::Direct3D12:
				d3d12::get_platform_interface(gfx);
				break;
#endif // _WIN64
			case graphics_platform::vulkan_1:
				vulkan::get_platform_interface(gfx);
				break;
			default:
				return false;
			}
			assert(gfx.platform == platform);
			return true;
		}

	} // anonymous namespace

	bool Initialize(graphics_platform platform)
	{
		return set_graphics_platform(platform) && gfx.initialize();
	}

	void Shutdown()
	{
		if (gfx.platform != (graphics_platform)-1) gfx.shutdown();
	}

	const char* get_engine_shaders_path()
	{
		return engineShaderPaths[(u32)gfx.platform];
	}

	const char* get_engine_shaders_path(graphics_platform platform)
	{
		return engineShaderPaths[(u32)platform];
	}

	surface CreateSurface(platform::window window)
	{
		return gfx.surface.create(window);
	}

	void RemoveSurface(surface_id id)
	{
		assert(id::is_valid(id));
		gfx.surface.remove(id);
	}

	void surface::Resize(u32 width, u32 height) const
	{
		assert(is_valid());
		return gfx.surface.resize(m_id, width, height);
	}

	u32 surface::Width() const
	{
		assert(is_valid());
		return gfx.surface.width(m_id);
	}

	u32 surface::Height() const
	{
		assert(is_valid());
		return gfx.surface.height(m_id);
	}

	void surface::Render() const
	{
		assert(is_valid());
		gfx.surface.render(m_id);
	}

	Camera CreateCamera(CameraInitInfo info)
	{
		return gfx.Camera.create(info);
	}

	void RemoveCamera(camera_id id)
	{
		gfx.Camera.remove(id);
	}

	void Camera::Up(math::v3 up) const
	{
		assert(is_valid());
		gfx.Camera.set_paramter(m_id, CameraParameter::UpVector, &up, sizeof(up));
	}

	void Camera::FieldOfView(f32 fov) const
	{
		assert(is_valid());
		gfx.Camera.set_paramter(m_id, CameraParameter::FieldOfView, &fov, sizeof(fov));
	}

	void Camera::AspectRatio(f32 aspectRatio) const
	{
		assert(is_valid());
		gfx.Camera.set_paramter(m_id, CameraParameter::AspectRatio, &aspectRatio, sizeof(aspectRatio));
	}

	void Camera::ViewWidth(f32 width) const
	{
		assert(is_valid());
		gfx.Camera.set_paramter(m_id, CameraParameter::ViewWidth, &width, sizeof(width));
	}

	void Camera::ViewHeight(f32 height) const
	{
		assert(is_valid());
		gfx.Camera.set_paramter(m_id, CameraParameter::ViewHeight, &height, sizeof(height));
	}

	void Camera::Range(f32 nearZ, f32 farZ) const
	{
		assert(is_valid());
		gfx.Camera.set_paramter(m_id, CameraParameter::NearZ, &nearZ, sizeof(nearZ));
		gfx.Camera.set_paramter(m_id, CameraParameter::FarZ, &farZ, sizeof(farZ));
	}

	math::Mat4 Camera::View() const
	{
		assert(is_valid());
		math::Mat4 matrix;
		gfx.Camera.get_paramter(m_id, CameraParameter::View, &matrix, sizeof(matrix));
		return matrix;
	}

	math::Mat4 Camera::Projection() const
	{
		assert(is_valid());
		math::Mat4 matrix;
		gfx.Camera.get_paramter(m_id, CameraParameter::Projection, &matrix, sizeof(matrix));
		return matrix;
	}

	math::Mat4 Camera::InverseProjection() const
	{
		assert(is_valid());
		math::Mat4 matrix;
		gfx.Camera.get_paramter(m_id, CameraParameter::InverseProjection, &matrix, sizeof(matrix));
		return matrix;
	}

	math::Mat4 Camera::ViewProjection() const
	{
		assert(is_valid());
		math::Mat4 matrix;
		gfx.Camera.get_paramter(m_id, CameraParameter::ViewProjection, &matrix, sizeof(matrix));
		return matrix;
	}

	math::Mat4 Camera::InverseViewProjection() const
	{
		assert(is_valid());
		math::Mat4 matrix;
		gfx.Camera.get_paramter(m_id, CameraParameter::InverseViewProjection, &matrix, sizeof(matrix));
		return matrix;
	}

	math::v3 Camera::Up() const
	{
		assert(is_valid());
		math::v3 upVector;
		gfx.Camera.get_paramter(m_id, CameraParameter::UpVector, &upVector, sizeof(upVector));
		return upVector;
	}

	f32 Camera::NearZ() const
	{
		assert(is_valid());
		f32 nearZ;
		gfx.Camera.get_paramter(m_id, CameraParameter::NearZ, &nearZ, sizeof(nearZ));
		return nearZ;
	}

	f32 Camera::FarZ() const
	{
		assert(is_valid());
		f32 farZ;
		gfx.Camera.get_paramter(m_id, CameraParameter::FarZ, &farZ, sizeof(farZ));
		return farZ;
	}

	f32 Camera::FieldOfView() const
	{
		assert(is_valid());
		f32 fov;
		gfx.Camera.get_paramter(m_id, CameraParameter::FieldOfView, &fov, sizeof(fov));
		return fov;
	}

	f32 Camera::AspectRatio() const
	{
		assert(is_valid());
		f32 aspectRatio;
		gfx.Camera.get_paramter(m_id, CameraParameter::AspectRatio, &aspectRatio, sizeof(aspectRatio));
		return aspectRatio;
	}

	f32 Camera::ViewWidth() const
	{
		assert(is_valid());
		f32 width;
		gfx.Camera.get_paramter(m_id, CameraParameter::ViewWidth, &width, sizeof(width));
		return width;
	}

	f32 Camera::ViewHeight() const
	{
		assert(is_valid());
		f32 height;
		gfx.Camera.get_paramter(m_id, CameraParameter::ViewHeight, &height, sizeof(height));
		return height;
	}

	Camera::type Camera::ProjectionType() const
	{
		assert(is_valid());
		type type;
		gfx.Camera.get_paramter(m_id, CameraParameter::type, &type, sizeof(type));
		return type;
	}

	id::id_type Camera::EntityID() const
	{
		assert(is_valid());
		id::id_type id;
		gfx.Camera.get_paramter(m_id, CameraParameter::EntityID, &id, sizeof(id));
		return id;
	}

	id::id_type add_submesh(const u8*& data)
	{
		return gfx.Resources.add_submesh(data);
	}

	void remove_submesh(id::id_type id)
	{
		gfx.Resources.remove_submesh(id);
	}
}
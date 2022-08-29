#include "D3D12Camera.h"
#include "EngineAPI/GameEntity.h"

namespace havana::graphics::d3d12::camera
{
	namespace
	{
		utl::free_list<D3D12Camera> cameras;

		void SetUpVector(D3D12Camera camera, const void* const data, [[maybe_unused]] u32 size)
		{
			math::v3 upVector{ *(math::v3*)data };
			assert(sizeof(upVector) == size);
			camera.up(upVector);
		}

		void SetFieldOfView(D3D12Camera camera, const void* const data, [[maybe_unused]] u32 size)
		{
			assert(camera.projection_type() == graphics::camera::perspective);
			f32 fov{ *(f32*)data };
			assert(sizeof(fov) == size);
			camera.field_of_view(fov);
		}

		void SetAspectRatio(D3D12Camera camera, const void* const data, [[maybe_unused]] u32 size)
		{
			assert(camera.projection_type() == graphics::camera::perspective);
			f32 aspect_ratio{ *(f32*)data };
			assert(sizeof(aspect_ratio) == size);
			camera.aspect_ratio(aspect_ratio);
		}

		void SetViewWidth(D3D12Camera camera, const void* const data, [[maybe_unused]] u32 size)
		{
			assert(camera.projection_type() == graphics::camera::orthographic);
			f32 view_width{ *(f32*)data };
			assert(sizeof(view_width) == size);
			camera.view_width(view_width);
		}

		void SetViewHeight(D3D12Camera camera, const void* const data, [[maybe_unused]] u32 size)
		{
			assert(camera.projection_type() == graphics::camera::orthographic);
			f32 view_height{ *(f32*)data };
			assert(sizeof(view_height) == size);
			camera.view_height(view_height);
		}

		void SetNearZ(D3D12Camera camera, const void* const data, [[maybe_unused]] u32 size)
		{
			f32 near_z{ *(f32*)data };
			assert(sizeof(near_z) == size);
			camera.near_z(near_z);
		}

		void SetFarZ(D3D12Camera camera, const void* const data, [[maybe_unused]] u32 size)
		{
			f32 far_z{ *(f32*)data };
			assert(sizeof(far_z) == size);
			camera.far_z(far_z);
		}

		void GetView(D3D12Camera camera, void* const data, [[maybe_unused]] u32 size)
		{
			math::m4x4* const matrix{ (math::m4x4* const)data };
			assert(sizeof(math::m4x4) == size);
			DirectX::XMStoreFloat4x4(matrix, camera.view());
		}

		void GetProjection(D3D12Camera camera, void* const data, [[maybe_unused]] u32 size)
		{
			math::m4x4* const matrix{ (math::m4x4* const)data };
			assert(sizeof(math::m4x4) == size);
			DirectX::XMStoreFloat4x4(matrix, camera.projection());
		}

		void GetInverseProjection(D3D12Camera camera, void* const data, [[maybe_unused]] u32 size)
		{
			math::m4x4* const matrix{ (math::m4x4* const)data };
			assert(sizeof(math::m4x4) == size);
			DirectX::XMStoreFloat4x4(matrix, camera.inverse_projection());
		}

		void GetViewProjection(D3D12Camera camera, void* const data, [[maybe_unused]] u32 size)
		{
			math::m4x4* const matrix{ (math::m4x4* const)data };
			assert(sizeof(math::m4x4) == size);
			DirectX::XMStoreFloat4x4(matrix, camera.view_projection());
		}

		void GetInverseViewProjection(D3D12Camera camera, void* const data, [[maybe_unused]] u32 size)
		{
			math::m4x4* const matrix{ (math::m4x4* const)data };
			assert(sizeof(math::m4x4) == size);
			DirectX::XMStoreFloat4x4(matrix, camera.inverse_view_projection());
		}

		void GetUpVector(D3D12Camera camera, void* const data, [[maybe_unused]] u32 size)
		{
			math::v3* const upVector{ (math::v3* const)data };
			assert(sizeof(math::v3) == size);
			DirectX::XMStoreFloat3(upVector, camera.up());
		}

		void GetFieldOfView(D3D12Camera camera, void* const data, [[maybe_unused]] u32 size)
		{
			assert(camera.projection_type() == graphics::camera::perspective);
			f32* const fov{ (f32* const)data };
			assert(sizeof(f32) == size);
			*fov = camera.field_of_view();
		}

		void GetAspectRatio(D3D12Camera camera, void* const data, [[maybe_unused]] u32 size)
		{
			assert(camera.projection_type() == graphics::camera::perspective);
			f32* const aspect_ratio{ (f32* const)data };
			assert(sizeof(f32) == size);
			*aspect_ratio = camera.aspect_ratio();
		}

		void GetViewWidth(D3D12Camera camera, void* const data, [[maybe_unused]] u32 size)
		{
			assert(camera.projection_type() == graphics::camera::orthographic);
			f32* const view_width{ (f32* const)data };
			assert(sizeof(f32) == size);
			*view_width = camera.view_width();
		}

		void GetViewHeight(D3D12Camera camera, void* const data, [[maybe_unused]] u32 size)
		{
			assert(camera.projection_type() == graphics::camera::orthographic);
			f32* const view_height{ (f32* const)data };
			assert(sizeof(f32) == size);
			*view_height = camera.view_height();
		}

		void GetNearZ(D3D12Camera camera, void* const data, [[maybe_unused]] u32 size)
		{
			f32* const near_z{ (f32* const)data };
			assert(sizeof(f32) == size);
			*near_z = camera.near_z();
		}

		void GetFarZ(D3D12Camera camera, void* const data, [[maybe_unused]] u32 size)
		{
			f32* const far_z{ (f32* const)data };
			assert(sizeof(f32) == size);
			*far_z = camera.far_z();
		}

		void GetProjectionType(D3D12Camera camera, void* const data, [[maybe_unused]] u32 size)
		{
			graphics::camera::type* const type{ (graphics::camera::type* const)data };
			assert(sizeof(graphics::camera::type) == size);
			*type = camera.projection_type();
		}

		void GetEntityID(D3D12Camera camera, void* const data, [[maybe_unused]] u32 size)
		{
			id::id_type* const entityID{ (id::id_type* const)data };
			assert(sizeof(id::id_type) == size);
			*entityID = camera.entity_id();
		}

		void DummySet(D3D12Camera, const void* const, u32) {}

		using SetFunction = void(*)(D3D12Camera, const void* const, u32);
		using GetFunction = void(*)(D3D12Camera, void* const, u32);
		constexpr SetFunction setFunctions[]
		{
			SetUpVector,
			SetFieldOfView,
			SetAspectRatio,
			SetViewWidth,
			SetViewHeight,
			SetNearZ,
			SetFarZ,
			DummySet,
			DummySet,
			DummySet,
			DummySet,
			DummySet,
			DummySet,
			DummySet,
		};
		static_assert(_countof(setFunctions) == camera_parameter::count);

		constexpr GetFunction getFunctions[]
		{
			GetUpVector,
			GetFieldOfView,
			GetAspectRatio,
			GetViewWidth,
			GetViewHeight,
			GetNearZ,
			GetFarZ,
			GetView,
			GetProjection,
			GetInverseProjection,
			GetViewProjection,
			GetInverseViewProjection,
			GetProjectionType,
			GetEntityID,
		};
		static_assert(_countof(getFunctions) == camera_parameter::count);

	} // anonymous namespace


	D3D12Camera::D3D12Camera(camera_init_info info)
		: m_up{ DirectX::XMLoadFloat3(&info.up) }, m_nearZ{ info.near_z }, m_farZ{ info.far_z },
		m_fieldOfView{ info.field_of_view }, m_aspectRatio{ info.aspect_ratio },
		m_projectionType{ info.type }, m_entityID{ info.entity_id }, m_isDirty{ true }
	{
		assert(id::is_valid(m_entityID));
		update();
	}

	void D3D12Camera::update()
	{
		game_entity::entity entity{ game_entity::entity_id{m_entityID} };
		using namespace DirectX;
		math::v3 pos{ entity.transform().position() };
		math::v3 dir{ entity.transform().orientation() };
		XMVECTOR position{ XMLoadFloat3(&pos) };
		XMVECTOR direction{ XMLoadFloat3(&dir) };
		m_view = XMMatrixLookToRH(position, direction, m_up);

		if (m_isDirty)
		{
			m_projection = (m_projectionType == graphics::camera::perspective)
				? XMMatrixPerspectiveFovRH(m_fieldOfView * XM_PI, m_aspectRatio, m_nearZ, m_farZ)
				: XMMatrixOrthographicRH(m_viewWidth, m_viewHeight, m_nearZ, m_farZ);
			m_inverseProjection = XMMatrixInverse(nullptr, m_projection);
			m_isDirty = false;
		}

		m_viewProjection = XMMatrixMultiply(m_view, m_projection);
		m_inverseViewProjection = XMMatrixInverse(nullptr, m_viewProjection);
	}

	void D3D12Camera::up(math::v3 up)
	{
		m_up = DirectX::XMLoadFloat3(&up);
	}

	void D3D12Camera::field_of_view(f32 fov)
	{
		assert(m_projectionType == graphics::camera::perspective);
		m_fieldOfView = fov;
		m_isDirty = true;
	}

	void D3D12Camera::aspect_ratio(f32 aspect_ratio)
	{
		assert(m_projectionType == graphics::camera::perspective);
		m_aspectRatio = aspect_ratio;
		m_isDirty = true;
	}

	void D3D12Camera::view_width(f32 width)
	{
		assert(width);
		assert(m_projectionType == graphics::camera::orthographic);
		m_viewWidth = width;
		m_isDirty = true;
	}

	void D3D12Camera::view_height(f32 height)
	{
		assert(height);
		assert(m_projectionType == graphics::camera::orthographic);
		m_viewHeight = height;
		m_isDirty = true;
	}

	void D3D12Camera::near_z(f32 near_z)
	{
		m_nearZ = near_z;
		m_isDirty = true;
	}

	void D3D12Camera::far_z(f32 far_z)
	{
		m_farZ = far_z;
		m_isDirty = true;
	}

	graphics::camera Create(camera_init_info info)
	{
		return graphics::camera{ camera_id{ cameras.add(info) } };
	}

	void Remove(camera_id id)
	{
		assert(id::is_valid(id));
		cameras.remove(id);
	}

	void SetParamter(camera_id id, camera_parameter::parameter parameter, const void *const data, u32 dataSize)
	{
		assert(data && dataSize);
		assert(parameter < camera_parameter::count);
		D3D12Camera& camera{ Get(id) };
		setFunctions[parameter](camera, data, dataSize);
	}

	void GetParamter(camera_id id, camera_parameter::parameter parameter, void *const data, u32 dataSize)
	{
		assert(data && dataSize);
		assert(parameter < camera_parameter::count);
		D3D12Camera& camera{ Get(id) };
		getFunctions[parameter](camera, data, dataSize);
	}

	D3D12Camera& Get(camera_id id)
	{
		assert(id::is_valid(id));
		return cameras[id];
	}
}
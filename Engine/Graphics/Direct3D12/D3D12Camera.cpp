#include "D3D12Camera.h"
#include "EngineAPI/GameEntity.h"

namespace Havana::Graphics::D3D12::Camera
{
	namespace
	{
		Utils::free_list<D3D12Camera> cameras;

		void SetUpVector(D3D12Camera camera, const void* const data, u32 size)
		{
			Math::Vec3 upVector{ *(Math::Vec3*)data };
			assert(sizeof(upVector) == size);
			camera.Up(upVector);
		}

		void SetFieldOfView(D3D12Camera camera, const void* const data, u32 size)
		{
			assert(camera.ProjectionType() == Graphics::Camera::Perspective);
			f32 fov{ *(f32*)data };
			assert(sizeof(fov) == size);
			camera.FieldOfView(fov);
		}

		void SetAspectRatio(D3D12Camera camera, const void* const data, u32 size)
		{
			assert(camera.ProjectionType() == Graphics::Camera::Perspective);
			f32 aspectRatio{ *(f32*)data };
			assert(sizeof(aspectRatio) == size);
			camera.AspectRatio(aspectRatio);
		}

		void SetViewWidth(D3D12Camera camera, const void* const data, u32 size)
		{
			assert(camera.ProjectionType() == Graphics::Camera::Orthographic);
			f32 viewWidth{ *(f32*)data };
			assert(sizeof(viewWidth) == size);
			camera.ViewWidth(viewWidth);
		}

		void SetViewHeight(D3D12Camera camera, const void* const data, u32 size)
		{
			assert(camera.ProjectionType() == Graphics::Camera::Orthographic);
			f32 viewHeight{ *(f32*)data };
			assert(sizeof(viewHeight) == size);
			camera.ViewHeight(viewHeight);
		}

		void SetNearZ(D3D12Camera camera, const void* const data, u32 size)
		{
			f32 nearZ{ *(f32*)data };
			assert(sizeof(nearZ) == size);
			camera.NearZ(nearZ);
		}

		void SetFarZ(D3D12Camera camera, const void* const data, u32 size)
		{
			f32 farZ{ *(f32*)data };
			assert(sizeof(farZ) == size);
			camera.FarZ(farZ);
		}

		void GetView(D3D12Camera camera, void* const data, u32 size)
		{
			Math::Mat4* const matrix{ (Math::Mat4* const)data };
			assert(sizeof(Math::Mat4) == size);
			DirectX::XMStoreFloat4x4(matrix, camera.View());
		}

		void GetProjection(D3D12Camera camera, void* const data, u32 size)
		{
			Math::Mat4* const matrix{ (Math::Mat4* const)data };
			assert(sizeof(Math::Mat4) == size);
			DirectX::XMStoreFloat4x4(matrix, camera.Projection());
		}

		void GetInverseProjection(D3D12Camera camera, void* const data, u32 size)
		{
			Math::Mat4* const matrix{ (Math::Mat4* const)data };
			assert(sizeof(Math::Mat4) == size);
			DirectX::XMStoreFloat4x4(matrix, camera.InverseProjection());
		}

		void GetViewProjection(D3D12Camera camera, void* const data, u32 size)
		{
			Math::Mat4* const matrix{ (Math::Mat4* const)data };
			assert(sizeof(Math::Mat4) == size);
			DirectX::XMStoreFloat4x4(matrix, camera.ViewProjection());
		}

		void GetInverseViewProjection(D3D12Camera camera, void* const data, u32 size)
		{
			Math::Mat4* const matrix{ (Math::Mat4* const)data };
			assert(sizeof(Math::Mat4) == size);
			DirectX::XMStoreFloat4x4(matrix, camera.InverseViewProjection());
		}

		void GetUpVector(D3D12Camera camera, void* const data, u32 size)
		{
			Math::Vec3* const upVector{ (Math::Vec3* const)data };
			assert(sizeof(Math::Vec3) == size);
			DirectX::XMStoreFloat3(upVector, camera.Up());
		}

		void GetFieldOfView(D3D12Camera camera, void* const data, u32 size)
		{
			assert(camera.ProjectionType() == Graphics::Camera::Perspective);
			f32* const fov{ (f32* const)data };
			assert(sizeof(f32) == size);
			*fov = camera.FieldOfView();
		}

		void GetAspectRatio(D3D12Camera camera, void* const data, u32 size)
		{
			assert(camera.ProjectionType() == Graphics::Camera::Perspective);
			f32* const aspectRatio{ (f32* const)data };
			assert(sizeof(f32) == size);
			*aspectRatio = camera.AspectRatio();
		}

		void GetViewWidth(D3D12Camera camera, void* const data, u32 size)
		{
			assert(camera.ProjectionType() == Graphics::Camera::Orthographic);
			f32* const viewWidth{ (f32* const)data };
			assert(sizeof(f32) == size);
			*viewWidth = camera.ViewWidth();
		}

		void GetViewHeight(D3D12Camera camera, void* const data, u32 size)
		{
			assert(camera.ProjectionType() == Graphics::Camera::Orthographic);
			f32* const viewHeight{ (f32* const)data };
			assert(sizeof(f32) == size);
			*viewHeight = camera.ViewHeight();
		}

		void GetNearZ(D3D12Camera camera, void* const data, u32 size)
		{
			f32* const nearZ{ (f32* const)data };
			assert(sizeof(f32) == size);
			*nearZ = camera.NearZ();
		}

		void GetFarZ(D3D12Camera camera, void* const data, u32 size)
		{
			f32* const farZ{ (f32* const)data };
			assert(sizeof(f32) == size);
			*farZ = camera.FarZ();
		}

		void GetProjectionType(D3D12Camera camera, void* const data, u32 size)
		{
			Graphics::Camera::Type* const type{ (Graphics::Camera::Type* const)data };
			assert(sizeof(Graphics::Camera::Type) == size);
			*type = camera.ProjectionType();
		}

		void GetEntityID(D3D12Camera camera, void* const data, u32 size)
		{
			Id::id_type* const entityID{ (Id::id_type* const)data };
			assert(sizeof(Id::id_type) == size);
			*entityID = camera.EntityID();
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
		static_assert(_countof(setFunctions) == CameraParameter::Count);

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
		static_assert(_countof(getFunctions) == CameraParameter::Count);

	} // anonymous namespace


	D3D12Camera::D3D12Camera(CameraInitInfo info)
		: m_up{ DirectX::XMLoadFloat3(&info.up) }, m_nearZ{ info.nearZ }, m_farZ{ info.farZ },
		m_fieldOfView{ info.fieldOfView }, m_aspectRatio{ info.aspectRatio },
		m_projectionType{ info.type }, m_entityID{ info.entityId }, m_isDirty{ true }
	{
		assert(Id::IsValid(m_entityID));
		Update();
	}

	void D3D12Camera::Update()
	{
		Entity::Entity entity{ Entity::entity_id{m_entityID} };
		using namespace DirectX;
		Math::Vec3 pos{ entity.Transform().Position() };
		Math::Vec3 dir{ entity.Transform().Orientation() };
		XMVECTOR position{ XMLoadFloat3(&pos) };
		XMVECTOR direction{ XMLoadFloat3(&dir) };
		m_view = XMMatrixLookToRH(position, direction, m_up);

		if (m_isDirty)
		{
			m_projection = (m_projectionType == Graphics::Camera::Perspective)
				? XMMatrixPerspectiveFovRH(m_fieldOfView * XM_PI, m_aspectRatio, m_nearZ, m_farZ)
				: XMMatrixOrthographicRH(m_viewWidth, m_viewHeight, m_nearZ, m_farZ);
			m_inverseProjection = XMMatrixInverse(nullptr, m_projection);
			m_isDirty = false;
		}

		m_viewProjection = XMMatrixMultiply(m_view, m_projection);
		m_inverseViewProjection = XMMatrixInverse(nullptr, m_viewProjection);
	}

	void D3D12Camera::Up(Math::Vec3 up)
	{
		m_up = DirectX::XMLoadFloat3(&up);
	}

	void D3D12Camera::FieldOfView(f32 fov)
	{
		assert(m_projectionType == Graphics::Camera::Perspective);
		m_fieldOfView = fov;
		m_isDirty = true;
	}

	void D3D12Camera::AspectRatio(f32 aspectRatio)
	{
		assert(m_projectionType == Graphics::Camera::Perspective);
		m_aspectRatio = aspectRatio;
		m_isDirty = true;
	}

	void D3D12Camera::ViewWidth(f32 width)
	{
		assert(width);
		assert(m_projectionType == Graphics::Camera::Orthographic);
		m_viewWidth = width;
		m_isDirty = true;
	}

	void D3D12Camera::ViewHeight(f32 height)
	{
		assert(height);
		assert(m_projectionType == Graphics::Camera::Orthographic);
		m_viewHeight = height;
		m_isDirty = true;
	}

	void D3D12Camera::NearZ(f32 nearZ)
	{
		m_nearZ = nearZ;
		m_isDirty = true;
	}

	void D3D12Camera::FarZ(f32 farZ)
	{
		m_farZ = farZ;
		m_isDirty = true;
	}

	Graphics::Camera Create(CameraInitInfo info)
	{
		return Graphics::Camera{ camera_id{ cameras.add(info) } };
	}

	void Remove(camera_id id)
	{
		assert(Id::IsValid(id));
		cameras.remove(id);
	}

	void SetParamter(camera_id id, CameraParameter::Parameter parameter, const void *const data, u32 dataSize)
	{
		assert(data && dataSize);
		assert(parameter < CameraParameter::Count);
		D3D12Camera& camera{ Get(id) };
		setFunctions[parameter](camera, data, dataSize);
	}

	void GetParamter(camera_id id, CameraParameter::Parameter parameter, void *const data, u32 dataSize)
	{
		assert(data && dataSize);
		assert(parameter < CameraParameter::Count);
		D3D12Camera& camera{ Get(id) };
		getFunctions[parameter](camera, data, dataSize);
	}

	D3D12Camera& Get(camera_id id)
	{
		assert(Id::IsValid(id));
		return cameras[id];
	}
}
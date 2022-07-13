#include "D3D12Camera.h"
#include "EngineAPI/GameEntity.h"

namespace Havana::Graphics::D3D12::Camera
{
	namespace
	{

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
}
#pragma once
#include "D3D12CommonHeaders.h"

namespace havana::Graphics::D3D12::Camera
{
	class D3D12Camera
	{
	public:
		explicit D3D12Camera(CameraInitInfo info);
		
		void Update();
		void Up(math::v3 up);
		void FieldOfView(f32 fov);
		void AspectRatio(f32 aspectRatio);
		void ViewWidth(f32 width);
		void ViewHeight(f32 height);
		void NearZ(f32 nearZ);
		void FarZ(f32 farZ);

		[[nodiscard]] constexpr DirectX::XMMATRIX View() const { return m_view; }
		[[nodiscard]] constexpr DirectX::XMMATRIX Projection() const { return m_projection; }
		[[nodiscard]] constexpr DirectX::XMMATRIX InverseProjection() const { return m_inverseProjection; }
		[[nodiscard]] constexpr DirectX::XMMATRIX ViewProjection() const { return m_viewProjection; }
		[[nodiscard]] constexpr DirectX::XMMATRIX InverseViewProjection() const { return m_inverseViewProjection; }
		[[nodiscard]] constexpr DirectX::XMVECTOR Up() const { return m_up; }
		[[nodiscard]] constexpr f32 NearZ() const { return m_nearZ; }
		[[nodiscard]] constexpr f32 FarZ() const { return m_farZ; }
		[[nodiscard]] constexpr f32 FieldOfView() const { return m_fieldOfView; }
		[[nodiscard]] constexpr f32 AspectRatio() const { return m_aspectRatio; }
		[[nodiscard]] constexpr f32 ViewWidth() const { return m_viewWidth; }
		[[nodiscard]] constexpr f32 ViewHeight() const { return m_viewHeight; }
		[[nodiscard]] constexpr Graphics::Camera::type ProjectionType() const { return m_projectionType; }
		[[nodiscard]] constexpr Id::id_type EntityID() const { return m_entityID; }

	private:
		DirectX::XMMATRIX		m_view;
		DirectX::XMMATRIX		m_projection;
		DirectX::XMMATRIX		m_inverseProjection;
		DirectX::XMMATRIX		m_viewProjection;
		DirectX::XMMATRIX		m_inverseViewProjection;
		DirectX::XMVECTOR		m_up;
		f32						m_nearZ;
		f32						m_farZ;
		union
		{
			f32					m_fieldOfView;	// use with perspective camera
			f32					m_viewWidth;	// use with orthographic camera
		};
		union
		{
			f32					m_aspectRatio;	// use with perspective camera
			f32					m_viewHeight;	// use with orthographic camera
		};
		Graphics::Camera::type	m_projectionType;
		Id::id_type				m_entityID;
		bool					m_isDirty;
	};

	Graphics::Camera Create(CameraInitInfo info);
	void Remove(camera_id id);
	void SetParamter(camera_id id, CameraParameter::Parameter parameter, const void *const data, u32 dataSize);
	void GetParamter(camera_id id, CameraParameter::Parameter parameter, void *const data, u32 dataSize);
	[[nodiscard]] D3D12Camera& Get(camera_id id);
}
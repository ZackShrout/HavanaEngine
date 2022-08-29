#pragma once
#include "D3D12CommonHeaders.h"

namespace havana::graphics::d3d12::camera
{
	class D3D12Camera
	{
	public:
		explicit D3D12Camera(camera_init_info info);
		
		void update();
		void up(math::v3 up);
		void field_of_view(f32 fov);
		void aspect_ratio(f32 aspect_ratio);
		void view_width(f32 width);
		void view_height(f32 height);
		void near_z(f32 near_z);
		void far_z(f32 far_z);

		[[nodiscard]] constexpr DirectX::XMMATRIX view() const { return m_view; }
		[[nodiscard]] constexpr DirectX::XMMATRIX projection() const { return m_projection; }
		[[nodiscard]] constexpr DirectX::XMMATRIX inverse_projection() const { return m_inverseProjection; }
		[[nodiscard]] constexpr DirectX::XMMATRIX view_projection() const { return m_viewProjection; }
		[[nodiscard]] constexpr DirectX::XMMATRIX inverse_view_projection() const { return m_inverseViewProjection; }
		[[nodiscard]] constexpr DirectX::XMVECTOR up() const { return m_up; }
		[[nodiscard]] constexpr f32 near_z() const { return m_nearZ; }
		[[nodiscard]] constexpr f32 far_z() const { return m_farZ; }
		[[nodiscard]] constexpr f32 field_of_view() const { return m_fieldOfView; }
		[[nodiscard]] constexpr f32 aspect_ratio() const { return m_aspectRatio; }
		[[nodiscard]] constexpr f32 view_width() const { return m_viewWidth; }
		[[nodiscard]] constexpr f32 view_height() const { return m_viewHeight; }
		[[nodiscard]] constexpr graphics::camera::type projection_type() const { return m_projectionType; }
		[[nodiscard]] constexpr id::id_type entity_id() const { return m_entityID; }

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
		graphics::camera::type	m_projectionType;
		id::id_type				m_entityID;
		bool					m_isDirty;
	};

	graphics::camera Create(camera_init_info info);
	void Remove(camera_id id);
	void SetParamter(camera_id id, camera_parameter::parameter parameter, const void *const data, u32 dataSize);
	void GetParamter(camera_id id, camera_parameter::parameter parameter, void *const data, u32 dataSize);
	[[nodiscard]] D3D12Camera& Get(camera_id id);
}
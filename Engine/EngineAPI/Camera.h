#pragma once
#include "CommonHeaders.h"

namespace havana::Graphics
{
	DEFINE_TYPED_ID(camera_id);

	class Camera
	{
	public:
		enum type : u32
		{
			Perspective,
			Orthographic
		};
		
		constexpr explicit Camera(camera_id id) : m_id{ id } {}
		constexpr Camera() = default;
		constexpr camera_id GetID() const { return m_id; }
		constexpr bool is_valid() const { return Id::is_valid(m_id); }


		void Up(math::v3 up) const;
		void FieldOfView(f32 fov) const;
		void AspectRatio(f32 aspectRatio) const;
		void ViewWidth(f32 width) const;
		void ViewHeight(f32 height) const;
		void Range(f32 nearZ, f32 farZ) const;

		math::Mat4 View() const;
		math::Mat4 Projection() const;
		math::Mat4 InverseProjection() const;
		math::Mat4 ViewProjection() const;
		math::Mat4 InverseViewProjection() const;
		math::v3 Up() const;
		f32 NearZ() const;
		f32 FarZ() const;
		f32 FieldOfView() const;
		f32 AspectRatio() const;
		f32 ViewWidth() const;
		f32 ViewHeight() const;
		type ProjectionType() const;
		Id::id_type EntityID() const;

	private:
		camera_id m_id{ Id::INVALID_ID };
	};
}
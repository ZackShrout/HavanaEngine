#pragma once
#include "CommonHeaders.h"

namespace Havana::Graphics
{
	DEFINE_TYPED_ID(camera_id);

	class Camera
	{
	public:
		enum Type : u32
		{
			Perspective,
			Orthographic
		};
		
		constexpr explicit Camera(camera_id id) : m_id{ id } {}
		constexpr Camera() = default;
		constexpr camera_id GetID() const { return m_id; }
		constexpr bool IsValid() const { return Id::IsValid(m_id); }


		void Up(Math::Vec3 up) const;
		void FieldOfView(f32 fov) const;
		void AspectRatio(f32 aspectRatio) const;
		void ViewWidth(f32 width) const;
		void ViewHeight(f32 height) const;
		void Range(f32 nearZ, f32 farZ) const;

		Math::Mat4 View() const;
		Math::Mat4 Projection() const;
		Math::Mat4 InverseProjection() const;
		Math::Mat4 ViewProjection() const;
		Math::Mat4 InverseViewProjection() const;
		Math::Vec3 Up() const;
		f32 NearZ() const;
		f32 FarZ() const;
		f32 FieldOfView() const;
		f32 AspectRatio() const;
		f32 ViewWidth() const;
		f32 ViewHeight() const;
		Type ProjectionType() const;
		Id::id_type EntityID() const;

	private:
		camera_id m_id{ Id::INVALID_ID };
	};
}
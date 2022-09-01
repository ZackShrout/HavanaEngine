#pragma once
#include "CommonHeaders.h"
#include "Platforms/Platform.h"
#include "EngineAPI/Camera.h"

namespace havana::graphics
{
	DEFINE_TYPED_ID(surface_id);
	
	class surface
	{
	public:
		constexpr surface() = default;
		constexpr explicit surface(surface_id id) : _id{ id } {}
		constexpr surface_id get_id() const { return _id; }
		constexpr bool is_valid() const { return id::is_valid(_id); }

		void resize(u32 width, u32 height) const;
		u32 width() const;
		u32 height() const;
		void render() const;

	private:
		surface_id _id{ id::invalid_id };
	};

	struct render_surface
	{
		platform::window window{};
		havana::graphics::surface surface{};
	};

	struct camera_parameter
	{
		enum parameter : u32
		{
			up_vector,
			field_of_view,
			aspect_ratio,
			view_width,
			view_height,
			near_z,
			far_z,
			view,
			projection,
			inverse_projection,
			view_projection,
			inverse_view_projection,
			type,
			entity_id,

			count
		};
	};

	struct camera_init_info
	{
		id::id_type		entity_id{ id::invalid_id };
		camera::type	type{};
		math::v3		up;
		f32				near_z;
		f32				far_z;
		union
		{
			f32	field_of_view;
			f32	view_width;
		};
		union
		{
			f32	aspect_ratio;
			f32	view_height;
		};
	};

	struct perspective_camera_init_info : public camera_init_info
	{
		explicit perspective_camera_init_info(id::id_type id)
		{
			assert(id::is_valid(id));
			entity_id = id;
			type = camera::perspective;
			up = { 0.0f, 1.0f, 0.0f };
			near_z = 0.001f;
			far_z = 10000.0f;
			field_of_view = 0.25f;
			aspect_ratio = 16.0f / 10.0f;
		}
	};

	struct orthographic_camera_init_info : public camera_init_info
	{
		explicit orthographic_camera_init_info(id::id_type id)
		{
			assert(id::is_valid(id));
			entity_id = id;
			type = camera::orthographic;
			up = { 0.0f, 1.0f, 0.0f };
			near_z = 0.001f;
			far_z = 10000.0f;
			view_height = 1920;
			view_width = 1080;
		}
	};

	struct primitive_topology
	{
		enum type : u32
		{
			point_list = 1,
			line_list,
			line_strip,
			triangle_list,
			triangle_strip,

			count
		};
	};

	#include "Graphics/GraphicsPlatform.h"
	
	bool initialize(graphics_platform platform);
	void shutdown();

	// Get the location of the compiled engine shaders relative to the executable's path.
	// The path is for the graphics API that is currently in use.
	const char* get_engine_shaders_path();
	// Get the location of the compiled engine shaders, for the specified platform, relative to the executable's path.
	const char* get_engine_shaders_path(graphics_platform platform);

	surface create_surface(platform::window window);
	void remove_surface(surface_id id);

	camera create_camera(camera_init_info info);
	void remove_camera(camera_id id);

	id::id_type add_submesh(const u8*& data);
	void remove_submesh(id::id_type id);
}
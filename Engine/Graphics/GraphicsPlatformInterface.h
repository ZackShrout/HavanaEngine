#pragma once
#include "CommonHeaders.h"
#include "Renderer.h"

namespace havana::graphics
{
	struct platform_interface
	{
		bool(*initialize)(void);
		void(*shutdown)(void);

		struct
		{
			surface(*create)(platform::window);
			void(*remove)(surface_id);
			void(*resize)(surface_id, u32, u32);
			u32(*width)(surface_id);
			u32(*height)(surface_id);
			void(*render)(surface_id);
		} surface;

		struct
		{
			Camera(*create)(CameraInitInfo);
			void(*remove)(camera_id);
			void(*set_paramter)(camera_id, CameraParameter::Parameter, const void *const, u32);
			void(*get_paramter)(camera_id, CameraParameter::Parameter, void *const, u32);
		} Camera;

		struct
		{
			id::id_type (*add_submesh)(const u8*&);
			void (*remove_submesh)(id::id_type);
		} Resources;

		graphics_platform platform = (graphics_platform)-1;
	};
}
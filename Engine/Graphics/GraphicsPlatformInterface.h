#pragma once
#include "CommonHeaders.h"
#include "Renderer.h"
//#include "../Platforms/Window.h"

namespace havana::Graphics
{
	struct PlatformInterface
	{
		bool(*Initialize)(void);
		void(*Shutdown)(void);

		struct
		{
			Surface(*Create)(Platform::Window);
			void(*Remove)(surface_id);
			void(*Resize)(surface_id, u32, u32);
			u32(*Width)(surface_id);
			u32(*Height)(surface_id);
			void(*Render)(surface_id);
		} Surface;

		struct
		{
			Camera(*Create)(CameraInitInfo);
			void(*Remove)(camera_id);
			void(*SetParamter)(camera_id, CameraParameter::Parameter, const void *const, u32);
			void(*GetParamter)(camera_id, CameraParameter::Parameter, void *const, u32);
		} Camera;

		struct
		{
			id::id_type (*AddSubmesh)(const u8*&);
			void (*RemoveSubmesh)(id::id_type);
		} Resources;

		GraphicsPlatform platform = (GraphicsPlatform)-1;
	};
}
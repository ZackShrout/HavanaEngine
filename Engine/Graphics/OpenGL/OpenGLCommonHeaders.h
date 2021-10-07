#pragma once

#ifdef __linux__
#include <GL/glx.h>
#include <X11/Xlib.h>
#elif _WIN64
#include <gl.h>
#endif // __linux__

#include "CommonHeaders.h"
#include "../Renderer.h"


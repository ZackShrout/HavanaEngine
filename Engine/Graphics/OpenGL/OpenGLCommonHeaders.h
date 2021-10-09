#pragma once

#ifdef __linux__
#include <GL/gl.h>
#include <GL/glx.h>
#elif _WIN64
#ifndef NOMINMAX
#define NOMINMAX
#endif // !NOMINMAX
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#endif // __linux__

#include "CommonHeaders.h"
#include "../Renderer.h"


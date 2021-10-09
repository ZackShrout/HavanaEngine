#pragma once

#ifdef __linux__
#include <GL/gl.h>
#include <GL/glx.h>
#elif _WIN64
#include <GL/gl.h>
#endif // __linux__

#include "CommonHeaders.h"
#include "../Renderer.h"


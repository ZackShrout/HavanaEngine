#pragma once
#pragma warning(disable: 4530) // disable exception warning

// C/C++
// NOTE: don't include any headers that include std::vector or std::deque
#include <stdint.h>
#include <assert.h>
#include <typeinfo>
#include <memory>
#include <unordered_map>
#include <mutex>

#if defined (_WIN64)
	#include <DirectXMath.h>
#endif

#if defined (__linux__)
	#include <glm/glm.hpp>
	#include <cstring>
#endif

#ifndef DISABLE_COPY
#define DISABLE_COPY(T)					\
		explicit T(const T&) = delete;	\
		T& operator=(const T&) = delete;
#endif // !DISABLE_COPY

#ifndef DISABLE_MOVE
#define DISABLE_MOVE(T)				\
		explicit T(T&&) = delete;	\
		T& operator=(T&&) = delete;
#endif // !DISABLE_MOVE

#ifndef DISABLE_COPY_AND_MOVE
#define DISABLE_COPY_AND_MOVE(T) DISABLE_COPY(T) DISABLE_MOVE(T)
#endif // !DISABLE_COPY_AND_MOVE

#ifdef _DEBUG
#define DEBUG_OP(x) x
#else
#define DEBUG_OP(x)
#endif // _DEBUG

// COMMON HEADERS
#include "PrimitiveTypes.h"
#include "../Utilities/Math.h"
#include "../Utilities/Utilities.h"
#include "Id.h"
#include "../Utilities/MathTypes.h"
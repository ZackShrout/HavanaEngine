#pragma once
#pragma warning(disable: 4530) // disable exception warning

// C/C++
#include <stdint.h>
#include <assert.h>
#include <typeinfo>
#include <memory>
#include <unordered_map>

#if defined (_WIN64)
	#include <DirectXMath.h>
#endif

// COMMON HEADERS
#include "PrimitiveTypes.h"
#include "..\Utilities\Utilities.h"
#include "..\Utilities\MathTypes.h"


#pragma once
#include "CommonHeaders.h"
#ifdef _WIN64
#include <combaseapi.h>
#endif

#ifndef EDITOR_INTERFACE
#define EDITOR_INTERFACE extern"C" __declspec(dllexport)
#endif // !EDITOR_INTERFACE
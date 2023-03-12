#pragma once
#ifdef __linux__

#include <X11/Xlib.h>
#include <X11/keysym.h>

namespace havana::input
{
	// TODO: change this from Win32 to XLib
    void process_input_message();
}

#endif // !_WIN64
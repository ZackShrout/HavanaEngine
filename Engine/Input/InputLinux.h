#pragma once
#ifdef __linux__

#include <X11/Xlib.h>
#include <X11/keysym.h>

namespace havana::input
{
    void process_input_message(XEvent xev, Display* display);
}

#endif // !__linux__
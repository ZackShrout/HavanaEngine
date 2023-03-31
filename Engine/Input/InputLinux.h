#pragma once
#ifdef __linux__

#include "../Platforms/LinuxWindowManager.h"

namespace havana::input
{
    void process_input_message(const platform::event* const ev, Display* display);
}

#endif // !__linux__
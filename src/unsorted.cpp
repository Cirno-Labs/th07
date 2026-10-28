#include "unsorted.h"

void DebugPrint(const char *fmt, ...)
{
}

int Unknown45e1f0::GetField2c()
{
    return field2c;
}

int SetButtonFromJoystick(WORD *buttons, short button, WORD bit, DWORD joyButtons)
{
    if (button < 0)
    {
        return 0;
    }
    DWORD mask = 1 << button;
    *buttons |= (joyButtons & mask) ? bit : 0;
    return (joyButtons & mask) ? bit : 0;
}

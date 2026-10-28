// cflags: /Os
// The file from 0x437903: the App chain callbacks, DirectInput set-up,
// the frame timer, the config and the music, as in th06's
// unsorted42333b.cpp. Built with /Os: its functions are laid out without
// alignment.

#include "unsorted.h"

void DebugPrint2(const char *fmt, ...)
{
}

int App::OnDraw(App *arg)
{
    DrawFps(1);
    return 1;
}

// cflags: /Os -/Oi
// The /Os file from 0x4553fa to 0x45c6e0, with the title menu, th06's
// mainmenu.cpp.

#include "unsorted.h"

void App::ResetClockDriftCounters()
{
    this->unk_2a4 = 0;
    this->unk_2ac = 0;
    this->unk_2b8 = 0;
    this->unk_2b0 = 0;
    this->unk_2b4 = 0;
}

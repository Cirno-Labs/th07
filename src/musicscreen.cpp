// cflags: /Os
// Part of the /Os file starting at 0x437903, as in th06.

#include "musicscreen.h"

int MusicScreen::UpdateIntro()
{
    int i;

    if (this->stateFrames == 0)
    {
        for (i = 0; i < 0x1f; i++)
        {
            if (this->cursor == i)
            {
                this->titles[i].pendingInterrupt = 1;
            }
            else
            {
                this->titles[i].pendingInterrupt = 2;
            }
        }
        for (i = 0; i < 8; i++)
        {
            this->comment[i].pendingInterrupt = 1;
        }
    }
    if (this->stateFrames >= 8)
    {
        this->state = 1;
    }
    return 0;
}

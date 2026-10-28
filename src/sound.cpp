// The file with SoundPlayer, the sound effects and wave music, from
// 0x44b510.

#include "unsorted.h"

SoundPlayer::SoundPlayer()
{
    int i;

    memset(this, 0, sizeof(*this));
    for (i = 0; i < 0x80; i++)
    {
        this->unk_408[i] = -1;
    }
}

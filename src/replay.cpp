// The file with ReplayManager, around 0x442c60.

#include "replay.h"
#include "unsorted.h"

int ReplayManager::RecordOnUpdate2(ReplayManager *replay)
{
    replay->unk_d6 = 0;
    replay->unk_d4 = g_Rng.seed;
    g_Rng.generated = 0;
    if (g_Unknown626270.unk_93d0)
    {
        replay->unk_d6 |= 0x100;
    }
    g_Unknown626270.unk_93d0 = 0;
    return 1;
}

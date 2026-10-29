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

int ReplayManager::RecordOnUpdate(ReplayManager *replay)
{
    int idx;
    WORD buttons;

    if (!(g_Unknown626270.unk_93d8 >> 2 & 1))
    {
        return 1;
    }
    g_Unknown4b9e58 = g_Unknown4b9e50;
    g_Unknown4b9e50 = g_Buttons;
    if (g_Unknown626270.unk_4->unk_25 != 0)
    {
        return 1;
    }
    if (g_App.unk_18c >> 3 & 1)
    {
        return 1;
    }
    idx = g_Unknown626270.stage - 1;
    if (idx >= 7)
    {
        idx = 6;
    }
    buttons = g_Buttons;
    g_Unknown4b9e50 = buttons;
    replay->cursor++;
    replay->stageEnds[idx] = replay->cursor + 1;
    replay->cursor->input = buttons;
    replay->cursor->events = replay->unk_d6;
    if (replay->frame % 30 == 0)
    {
        *replay->extraCursor = (BYTE)g_App.unk_188 | (g_App.unk_2a4 != 0 ? 0x80 : 0);
        replay->extraCursor[1] = (BYTE)g_App.unk_188;
        replay->extraEnds[idx] = replay->extraCursor + 2;
        replay->extraCursor++;
    }
    replay->frame++;
    return 1;
}

int ReplayManager::PlayOnUpdate2(ReplayManager *replay)
{
    if (!(g_Unknown626270.unk_93d8 >> 2 & 1))
    {
        return 1;
    }
    if (g_Unknown49fbf0.Func42ad66() && g_Unknown49fbf0.Func427be2() && replay->frame % 3 != 2)
    {
        return 6;
    }
    if (g_Unknown626270.unk_93df == 2 && !g_Unknown9a9b00.Func4232a0() && replay->frame % 5 != 4)
    {
        return 6;
    }
    return 1;
}

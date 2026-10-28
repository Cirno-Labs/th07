// Full-screen effects such as fades, each with its own chain elements.

#pragma once

#include <windows.h>
#include <d3d8.h>

#include "chain.h"
#include "common.h"
#include "timer.h"

// One effect (0x30 bytes), as in th06: `type` picks its update and draw
// callbacks.
class ScreenEffect
{
public:
    // Type 0's update callback: fades `alpha` from 255 to 0 over
    // `duration` frames. Returns 0 when done, else 1.
    static int OnUpdateFadeIn(ScreenEffect *effect);

    int type;
    ChainElem *calcElem;
    ChainElem *drawElem;
    DWORD unk_0c;
    int alpha;
    // How many frames the effect lasts.
    int duration;
    union
    {
        // Fades: the color faded.
        D3DCOLOR color;
        // Shakes: how far the play area shakes at the start.
        int shakeFrom;
    };
    int shakeTo;
    int unk_20;
    Timer timer;
};
ASSERT_SIZE(ScreenEffect, 0x30);

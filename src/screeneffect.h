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
    // Type 2's update callback: fades `alpha` from 0 to 255 over
    // `duration` frames. Returns 0 when done, else 1.
    static int OnUpdateFadeOut(ScreenEffect *effect);
    // Type 0's draw callback: covers the whole screen in `color` at
    // `alpha`. Returns 1.
    static int OnDrawScreen(ScreenEffect *effect);
    // Type 2's draw callback: covers the play area in `color` at `alpha`.
    // Returns 1.
    static int OnDrawPlayArea(ScreenEffect *effect);

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

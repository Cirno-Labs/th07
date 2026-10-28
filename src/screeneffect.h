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
    // Creates an effect of `type` (0 fade in, 1 shake, 2 fade out, 3
    // flashes, 4 fade out over the whole screen) that lasts `duration`
    // frames, with `a` and `b` as its color, shake range or flashes,
    // and adds its chain elements (update priority 0xf, draw 0x11).
    // Returns it, or NULL if adding fails.
    static ScreenEffect *StartEffect(int type, int duration, int a, int b, int c);
    // The update element's added callback: restarts `timer`. Returns 0.
    static int AddedCallback(ScreenEffect *effect);
    // The update element's deleted callback: removes the draw element
    // and deletes the effect. Returns 0.
    static int DeletedCallback(ScreenEffect *effect);
    // Type 0's update callback: fades `alpha` from 255 to 0 over
    // `duration` frames. Returns 0 when done, else 1.
    static int OnUpdateFadeIn(ScreenEffect *effect);
    // Type 1's update callback: shakes the screen, moving SpriteEngine's
    // shake offset at random by from shakeFrom to shakeTo pixels over
    // `duration` frames. Holds still while g_Unknown626270.isFrozen is
    // set. Returns 0 when done, else 1.
    static int OnUpdateShake(ScreenEffect *effect);
    // Type 2's update callback: fades `alpha` from 0 to 255 over
    // `duration` frames. Returns 0 when done, else 1.
    static int OnUpdateFadeOut(ScreenEffect *effect);
    // Type 0's draw callback: covers the whole screen in `color` at
    // `alpha`. Returns 1.
    static int OnDrawScreen(ScreenEffect *effect);
    // Type 2's draw callback: covers the play area in `color` at `alpha`.
    // Returns 1.
    static int OnDrawPlayArea(ScreenEffect *effect);
    // Type 3's update callback: fades `alpha` from flashColor's alpha to
    // 0 over `duration` frames, `flashes` times over. Returns 0 when
    // done, else 1. Not in th06.
    static int OnUpdateFlash(ScreenEffect *effect);
    // Type 3's draw callback: covers the play area in flashColor at
    // `alpha`. Returns 1. Not in th06.
    static int OnDrawFlash(ScreenEffect *effect);

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
        // Type 3: how many more times the color flashes.
        int flashes;
    };
    union
    {
        // Shakes: how far the play area shakes at the end.
        int shakeTo;
        // Type 3: the color flashed, starting at its alpha.
        D3DCOLOR flashColor;
    };
    int unk_20;
    Timer timer;
};
ASSERT_SIZE(ScreenEffect, 0x30);

// Short-lived sprite effects. Only what is known so far.

#pragma once

#include <d3d8.h>
#include <d3dx8math.h>

#include "common.h"
#include "sprite.h"

// One effect. Only the fields known so far.
struct Effect
{
    Sprite sprite;
    D3DXVECTOR3 pos;
    unsigned char unk_258[0x3c];
    // Player::StopBombState sets them to the cosine and sine of the
    // direction its effects burst in.
    float unk_294;
    float unk_298;
    unsigned char unk_29c[0x30];
    // Whether it runs, as th06's.
    char active;
    unsigned char unk_2cd[0xb];
};
ASSERT_SIZE(Effect, 0x2d8);

// The effects, at 0x12fe250. Its effects are 0x2d8 bytes, 400 of them.
class EffectManager
{
public:
    // Starts `count` effects of `type` at `pos` in `color`. Returns the
    // last effect started, or the spare one if there were not enough
    // free slots.
    Effect *SpawnEffect(int type, D3DXVECTOR3 *pos, int count, D3DCOLOR color);
    // Starts an effect of `type` at `pos` in `color` in reserved slot
    // `slot` (from 400 on). `count` is unused. Returns it.
    Effect *Func41c610(int type, D3DXVECTOR3 *pos, int slot, int count, D3DCOLOR color);
};
extern EffectManager g_EffectManager;

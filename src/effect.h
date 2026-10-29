// Short-lived sprite effects, drawn with the stage's effect sprites
// (data/eff01.anm, ...), as th06's.

#pragma once

#include <d3d8.h>
#include <d3dx8math.h>

#include "chain.h"
#include "common.h"
#include "sprite.h"
#include "timer.h"

// One effect. Only the fields known so far.
struct Effect
{
    Sprite sprite;
    D3DXVECTOR3 pos;
    D3DXVECTOR3 velocity;
    // Three vectors, as th06's acceleration, origin and axis maybe; the
    // type is load-bearing: with fewer, EffectManager's constructor does
    // not expand Effect's.
    D3DXVECTOR3 unk_264;
    D3DXVECTOR3 unk_270;
    D3DXVECTOR3 unk_27c;
    unsigned char unk_288[0xc];
    // Player::StopBombState sets them to the cosine and sine of the
    // direction its effects burst in.
    float unk_294;
    float unk_298;
    unsigned char unk_29c[0x1c];
    Timer timer;
    unsigned char unk_2c4[0x4];
    // From g_EffectInfos. Called every frame while set; the effect ends
    // when it returns anything but 1.
    int (*update)(Effect *effect);
    // Whether it runs, as th06's.
    char active;
    // The type it was started as.
    char type;
    char fadeOut;
    char fadeFrames;
    // Which of EffectManager's draw lists it goes in: 0 the first (the
    // fourth with sprite flag bit 4), 1 and 3 the second, others the
    // third.
    char drawList;
    unsigned char unk_2d1[0x3];
    // The next effect in its draw list.
    Effect *next;
};
ASSERT_SIZE(Effect, 0x2d8);

// What each type of effect starts with.
struct EffectInfo
{
    int script;
    int (*update)(Effect *effect);
    // Called once the effect is started, if set; the effect ends at once
    // when it returns nonzero.
    int (*init)(Effect *effect);
};
extern EffectInfo g_EffectInfos[];

// The effects, at 0x12fe250: 400 started in turn, 8 reserved slots and
// a spare.
class EffectManager
{
public:
    // Constructs the effects, zeroes everything and sets colorScale to
    // 1.
    EffectManager();
    // Zeroes everything.
    void Reset();
    // Starts `count` effects of `type` at `pos` in `color`. Returns the
    // last effect started, or the spare one if there were not enough
    // free slots.
    Effect *SpawnEffect(int type, D3DXVECTOR3 *pos, int count, D3DCOLOR color);
    // SpawnEffect with a starting `velocity`, without SpawnEffect's
    // sprite flag 0x1000.
    Effect *SpawnEffectWithVelocity(int type, D3DXVECTOR3 *pos, D3DXVECTOR3 *velocity, int count, D3DCOLOR color);
    // Starts an effect of `type` at `pos` in `color` in reserved slot
    // `slot` (from 400 on). `count` is unused. Returns it.
    Effect *SpawnReservedEffect(int type, D3DXVECTOR3 *pos, int slot, int count, D3DCOLOR color);
    // The update chain element's callback: runs the active effects
    // (all but the spare) and sorts them into the draw lists; every 300
    // frames checks the player's numbers. Returns 4 if they were
    // tampered with, otherwise 1.
    static int OnUpdate(EffectManager *mgr);
    // The draw chain element's callback: draws the first, third and
    // fourth draw lists, the first and fourth moved into the play area.
    // Returns 1.
    static int OnDraw(EffectManager *mgr);
    // Draws the second draw list, unless App::unk_13c is 0; with 1 it
    // returns at the first effect. Type 20 is drawn with its color
    // scaled by colorScale. Returns 1.
    int OnDraw2();

    // Where SpawnEffect looks for a free effect next.
    int next;
    unsigned char unk_4[0x4];
    // How many effects were active this frame.
    int activeCount;
    // What OnDraw2 scales type 20's red, green, blue and alpha by.
    float colorScale[4];
    Effect effects[0x199];
    // The heads of the draw lists, and their last effects.
    Effect drawList0;
    Effect drawList1;
    Effect drawList2;
    Effect drawList3;
    Effect *drawListTails[4];
    // Counts OnUpdate's calls.
    int frames;
};
ASSERT_SIZE(EffectManager, 0x496a8);
extern EffectManager g_EffectManager;

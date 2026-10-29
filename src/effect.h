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
    // Set by the spawn functions: zero, or the velocity
    // EffectManager::SpawnEffectWithVelocity is given.
    D3DXVECTOR3 velocity;
    // Added to the position each frame by the moving types, as th06's
    // velocity, and changed by acceleration.
    D3DXVECTOR3 step;
    D3DXVECTOR3 acceleration;
    D3DXVECTOR3 unk_27c;
    // The point an effect circles or comes in to, and its axis or
    // direction, as th06's. The vector and quaternion types are
    // load-bearing: with fewer, the Effect constructor copy (0x41a2f0)
    // takes a larger frame.
    D3DXVECTOR3 origin;
    D3DXVECTOR3 axis;
    D3DXQUATERNION rotation;
    float radius;
    float angle;
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

// An effect init, as the first frame of th06's EffectUpdateDrift: a
// random step that slows to a stop over 19 frames, both scaled by
// App::unk_178. Returns 0.
int EffectInitDrift(Effect *effect);

// An effect update: moves by step, which changes by acceleration, as
// th06's (without frameAdvance). Returns 1.
int EffectUpdateMove(Effect *effect);

// Like EffectInitDrift, faster, slowing to a stop over 20 frames, as
// th06's EffectUpdateDriftFast.
int EffectInitDriftFast(Effect *effect);

// An effect init: puts it in the third draw list. Returns 0.
int EffectInitForeground(Effect *effect);

// An effect update, as th06's EffectUpdateOrbit: circles origin at
// `radius` about `axis`, at `angle` (flattened); once fadeOut is set,
// fades out and grows over 16 frames, then ends.
int EffectUpdateOrbit(Effect *effect);

// An effect init, as the first frame of th06's EffectUpdateConverge:
// sets origin to where it starts (at z 0) and a random direction.
// Returns 0.
int EffectInitConverge(Effect *effect);

// An effect update, as th06's EffectUpdateConverge without its first
// frame: comes in from 256 along axis to origin over 60 frames, at z 0.
// Returns 1.
int EffectUpdateConverge(Effect *effect);

// An effect update: follows the player while its script runs. Returns
// 1, or 0 once the script has ended.
int EffectUpdateFollowPlayer(Effect *effect);

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
    // Resets and loads the stage's effect sprites: data/eff01.anm to
    // eff08.anm, from one to two files a stage. Returns 0, or -1 on
    // failure.
    static int AddedCallback(EffectManager *mgr);
    // Frees the effect sprites. Returns 0.
    static int DeletedCallback(EffectManager *mgr);
    // Resets and adds the update element (priority 11) and the draw
    // element (priority 9). Returns 0, or -1 if adding fails.
    static int RegisterChain();
    // Removes the two chain elements.
    static void RemoveChain();

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

// EffectManager's statically allocated chain elements.
extern ChainElem g_EffectCalcChain;
extern ChainElem g_EffectDrawChain;

// Set by EffectManager::AddedCallback to 1 or 2, by stage. Not in th06.
extern int g_Unknown1348020;
// Cleared by EffectManager::AddedCallback.
extern int g_Unknown1348024;

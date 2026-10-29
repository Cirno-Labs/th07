// Enemies, as th06's enemy.h. Only the fields known so far.

#pragma once

#include <d3dx8math.h>

#include "common.h"

// Enemy's flags, in bytes. The first byte is th06's.
struct EnemyFlags
{
    unsigned char moveMode : 2;
    unsigned char moveEasing : 3;
    unsigned char holdFire : 1;
    // Mirrors the horizontal movement.
    unsigned char mirrored : 1;
    // In use.
    unsigned char active : 1;
};

// One enemy (0x4f48 bytes).
struct Enemy
{
    // Moves by the velocity scaled by App::unk_178, mirrored with
    // flags.mirrored, as th06's; first keeps the last move in posDelta
    // and the position in prevPos.
    void Move();

    unsigned char unk_0[0x2b0c];
    D3DXVECTOR3 pos;
    D3DXVECTOR3 velocity;
    // Where Move found the enemy, and how far it had moved since the
    // call before.
    D3DXVECTOR3 prevPos;
    D3DXVECTOR3 posDelta;
    unsigned char unk_2b3c[0x2ec];
    EnemyFlags flags;
    unsigned char unk_2e29[0x211f];
};
ASSERT_SIZE(Enemy, 0x4f48);

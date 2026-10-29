// The enemies' bullets and lasers, as th06's bullet.h.

#pragma once

#include <d3dx8math.h>

#include "chain.h"
#include "common.h"
#include "sprite.h"
#include "timer.h"

// A bullet type's sprites: the bullet's and those of its spawning
// effects, as th06's. Its implicit constructor is 0x423420.
struct BulletTypeSprites
{
    Sprite sprite0;
    Sprite sprite1;
    Sprite sprite2;
    Sprite sprite3;
    Sprite sprite4;
    // The size of the bullet's hitbox.
    D3DXVECTOR3 hitboxSize;
    BYTE unk_b88;
    BYTE unk_b89;
    BYTE unk_b8a;
    BYTE unk_b8b;
};
ASSERT_SIZE(BulletTypeSprites, 0xb8c);

// One step of a bullet's behavior, which th06 kept in BulletShooter's
// exFloats and exInts: `flag` is the Bullet::flags bit it runs under.
struct BulletCommand
{
    float unk_0;
    float unk_4;
    float unk_8;
    float unk_c;
    DWORD flag;
    int unk_14;
};
ASSERT_SIZE(BulletCommand, 0x18);

// The state of one of a bullet's running behaviors.
struct BulletEx
{
    Timer timer;
    float unk_c;
    float unk_10;
    float unk_14;
    float unk_18;
    int unk_1c;
    float unk_20;
    float unk_24;
    int unk_28;
};
ASSERT_SIZE(BulletEx, 0x2c);

// One enemy bullet. Its implicit constructor is 0x423510.
struct Bullet
{
    BulletTypeSprites sprites;
    D3DXVECTOR3 pos;
    D3DXVECTOR3 velocity;
    D3DXVECTOR3 unk_ba4;
    float speed;
    float unk_bb4;
    float unk_bb8;
    float angle;
    float unk_bc0;
    float unk_bc4;
    Timer timer;
    Timer unk_bd4;
    unsigned char unk_be0[0x10];
    float unk_bf0;
    // The behaviors running, by BulletCommand::flag.
    WORD activeFlags;
    // BulletShooter::flags.
    WORD flags;
    short color;
    WORD unk_bfa;
    // 0 unused; 6 marks the spare bullet after the last.
    WORD state;
    WORD unk_bfe;
    BYTE unk_c00;
    BYTE grazed;
    unsigned char unk_c02[0x6];
    int unk_c08;
    // The sound the bullet's behaviors play as they start.
    int sound;
    // The command to start next.
    int commandIndex;
    BulletCommand commands[5];
    BulletEx ex[5];
};
ASSERT_SIZE(Bullet, 0xd68);

// What a bullet pattern fires, as th06's.
struct BulletShooter
{
    // The bullet type, and its color.
    short sprite;
    short color;
    D3DXVECTOR3 pos;
    float angle1;
    float angle2;
    float speed1;
    float speed2;
    // Copied into each bullet.
    BulletCommand commands[5];
    // Extra values for the lasers.
    float exFloats[4];
    int exInts[4];
    DWORD laserEndHitFrames;
    short count1;
    short count2;
    WORD aimMode;
    WORD unk_c2;
    int flags;
    int sound;
    // Bullet::sound.
    int unk_cc;
    // FirePattern sets it to the bullet type's sprites.
    BulletTypeSprites *typeSprites;
};

// One enemy laser, as th06's. Its implicit constructor is 0x4235d0.
struct Laser
{
    Sprite sprite0;
    Sprite sprite1;
    D3DXVECTOR3 pos;
    float angle;
    float startOffset;
    float endOffset;
    float maxLength;
    float width;
    float unk_4b8;
    float speed;
    int startFrames;
    int hitStartFrame;
    int durationFrames;
    int endFrames;
    int endHitFrames;
    int active;
    Timer timer;
    WORD flags;
    short color;
    BYTE state;
    BYTE unk_4e9;
    unsigned char unk_4ea[0x2];
};
ASSERT_SIZE(Laser, 0x4ec);

// The bullets, at 0x62f958.
class BulletManager
{
public:
    // Zeroes everything and marks the spare bullet.
    void Reset();
    // Removes the bullets (Player's respawn step calls it with 0).
    void ClearAll(int unk);

    BulletTypeSprites typeSprites[0x10];
    // The last one is a spare whose state 6 sends searches back to the
    // first.
    Bullet bullets[0x401];
    Laser lasers[0x40];
    int bulletCount;
    int unk_37a12c;
    Timer unk_37a130;
    int unk_37a13c;
    // RegisterChain's argument.
    char *anmPath;
    int unk_37a144[6];
    // Where SpawnBullet looks for a free bullet first.
    Bullet *nextBullet;
    // The item type cleared bullets turn into (g_Unknown9a9ab8).
    int unk_37a160;
};
ASSERT_SIZE(BulletManager, 0x37a164);
extern BulletManager g_BulletManager;

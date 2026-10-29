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
    // Frames, for most flags.
    int unk_8;
    int unk_c;
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
    // Sets command `idx` to run under `flag`, adds `flag` to flags and
    // starts over from the first command. Returns the command.
    BulletCommand *RestartCommand(int idx, int unk_14, DWORD flag);
    // Sets command `idx` up for flag 0x10.
    void SetAccelerateCommand(int idx, int unk_14, int unk_8, float unk_0, float unk_4);
    // Sets command `idx` up for flag 0x20.
    void SetTurnCommand(int idx, int unk_14, int unk_8, float unk_4, float unk_0);

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
    // Sets command `idx` to run under `flag` and adds `flag` to flags.
    // Returns the command.
    BulletCommand *StartCommand(int idx, int unk_14, DWORD flag);
    // Sets command `idx` up for flag 0x20.
    void SetShooterTurnCommand(int idx, int unk_14, int unk_8, float unk_4, float unk_0);
    // Sets command `idx` up for flag 0x80.
    void SetStopTurnCommand(int idx, int unk_14, int unk_8, int unk_c, float unk_0, float unk_4);
    // Sets command `idx` up for flag 0x10.
    void SetShooterAccelerateCommand(int idx, int unk_14, int unk_8, float unk_0, float unk_4);
    // Sets command `idx` up for flag 0x2000.
    void SetHiddenCommand(int idx, int unk_14, int unk_8);

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
    // Constructs the members and resets.
    BulletManager();

    // Zeroes everything and marks the spare bullet.
    void Reset();
    // Ends every bullet, and every laser without flag 4 (all of them
    // for `mode` 10): with `mode` 0 or over 8 they fade out; with 1 or 2
    // bullets, and points every 32 along each laser, turn into
    // `mode` items of type unk_37a160, with 3 to 8 into one item of
    // type 8. Then sets unk_37a12c to 10.
    void ClearAll(int mode);
    // Spawns bullet (`col`, `row`) of `shooter`'s pattern, aimed at
    // `aimAngle` by the aiming modes that use it. Returns 0, or 1 if no
    // bullet is free.
    int SpawnBullet(BulletShooter *shooter, int col, int row, float aimAngle);
    // Fires `shooter`'s pattern with its type's sprites, aimed at the
    // player, until no bullet is free, and plays its sound with flag
    // 0x200; nothing while 0x400 bullets are out. Returns 0.
    int FirePattern(BulletShooter *shooter);
    // Starts a laser from `shooter` in a free slot: exFloats give its
    // start and end offsets, length and width, exInts its timings.
    // Returns it, or the first laser while unk_37a12c is set and flag 4
    // is not, or past the last one if none is free.
    Laser *SpawnLaser(BulletShooter *shooter);
    // Stops every bullet of g_BulletManager where it is and turns it to
    // color 0. Not in th06.
    void StopAllBullets();

    // Resets, remembers `anmPath`, points g_EffectColors at
    // g_EffectColorsFull (th06 picked by an option), and adds the update
    // element (priority 0xc) and the draw element (priority 10). Returns
    // 0, or -1 if adding fails.
    static int RegisterChain(char *anmPath);
    static int OnUpdate(BulletManager *mgr);
    static int OnDraw(BulletManager *mgr);
    static int AddedCallback(BulletManager *mgr);
    // Frees the bullet sprites (slots 0xb to 0xe), unless the scene is
    // 3, 0xb or 0xc. Returns 0.
    static int DeletedCallback(BulletManager *mgr);
    // Removes the two chain elements and zeroes g_BulletManager.
    static void RemoveChain();

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

// Sprite offsets by color, for bullets up to 16 and up to 32 high, as
// th06's.
extern int g_BulletColorOffsets16[16];
extern int g_BulletColorOffsets32[16];

// Shows sprite `src` in `color` on `dst`, by the size of `bullet`'s
// sprite, unless `dst` shows it already: th06's SET_BULLET_SPRITE macro
// for the spawning sprites, out of line and without its 0x207 case.
void ShowSpawnSprite(Sprite *dst, Sprite *src, Bullet *bullet, int color);

// The colors of effect 0xd by index, as th06's; RegisterChain points
// g_EffectColors at them.
extern DWORD g_EffectColorsFull[0x1c];
extern DWORD *g_EffectColors;

// BulletManager's statically allocated chain elements.
extern ChainElem g_BulletCalcChain;
extern ChainElem g_BulletDrawChain;

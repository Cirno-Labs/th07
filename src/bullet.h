// The enemies' bullets and lasers, as th06's bullet.h.

#pragma once

#include <d3dx8math.h>

#include "chain.h"
#include "common.h"
#include "sprite.h"
#include "timer.h"

// A Sprite of its own type: BulletTypeSprites' constructor only reserves
// its unused slots as in the original when the third sprite's
// constructor is one more inline call deep.
struct BulletSprite : Sprite
{
};

// A bullet type's sprites: the bullet's and those of its spawning
// effects, as th06's.
struct BulletTypeSprites
{
    // Out of line: as an inline that isn't expanded, its BulletSprite
    // would reserve more slots in BulletManager's and Bullet's
    // constructors than the original's.
    BulletTypeSprites();

    Sprite sprite0;
    Sprite sprite1;
    BulletSprite sprite2;
    Sprite sprite3;
    Sprite sprite4;
    // The size of the bullet's hitbox.
    D3DXVECTOR3 hitboxSize;
    BYTE spawnCopiedFlag;
    // The height of the bullet's sprite.
    BYTE height;
    // A size class: 5 up to 8 high, 4 or 3 up to 16, 2 or 1 up to 32,
    // 0 above.
    BYTE sizeClass;
    BYTE unusedByteB8b;
};
ASSERT_SIZE(BulletTypeSprites, 0xb8c);

// One step of a bullet's behavior, which th06 kept in BulletShooter's
// exFloats and exInts: `flag` is the Bullet::flags bit it runs under.
struct BulletCommand
{
    float arg0;
    float arg1;
    // Frames, for most flags.
    int frames;
    int maxTurns;
    DWORD flag;
    int concurrentFlag;
};
ASSERT_SIZE(BulletCommand, 0x18);

// The state of one of a bullet's running behaviors.
struct BulletEx
{
    Timer timer;
    float turnSpeed;
    float turnAngle;
    // Added to the velocity (flag 0x10).
    D3DXVECTOR3 accel;
    // How long the behavior runs, in frames.
    int duration;
    int maxTurns;
    int turnCount;
};
ASSERT_SIZE(BulletEx, 0x2c);

// One enemy bullet. Its implicit constructor is 0x423510.
struct Bullet
{
    // Sets command `idx` to run under `flag`, adds `flag` to flags and
    // starts over from the first command. Returns the command.
    BulletCommand *RestartCommand(int idx, int concurrentFlag, DWORD flag);
    // Sets command `idx` up for flag 0x10.
    void SetAccelerateCommand(int idx, int concurrentFlag, int frames, float arg0, float arg1);
    // Frees the bullet: state 0 and both timers reset. Inline, defined
    // after BulletManager::OnUpdate, which calls it out of line.
    void FreeBullet();
    // Sets command `idx` up for flag 0x20.
    void SetTurnCommand(int idx, int concurrentFlag, int frames, float arg1, float arg0);
    // Clears command `idx`'s flag, so the commands stop there.
    void ClearCommandFlag(int idx)
    {
        this->commands[idx].flag = 0;
    }

    BulletTypeSprites sprites;
    D3DXVECTOR3 pos;
    D3DXVECTOR3 velocity;
    D3DXVECTOR3 accel;
    float speed;
    float accelSpeed;
    float turnSpeed;
    float angle;
    float angularVelocity;
    float turnAngle;
    Timer timer;
    Timer grazeDelayTimer;
    unsigned char pad_be0[0x10];
    // Set by a command with flag 0x2000.
    int boundsCheckDelay;
    // The behaviors running, by BulletCommand::flag.
    WORD activeFlags;
    // BulletShooter::flags.
    WORD flags;
    short color;
    WORD unusedWordBfa;
    // 0 unused; 6 marks the spare bullet after the last.
    WORD state;
    WORD outOfBoundsFrames;
    BYTE spawnedFlag;
    BYTE grazed;
    unsigned char pad_c02[0x2];
    // The next bullet in the same BulletManager draw list.
    Bullet *next;
    int deflectState;
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
    BulletShooter()
    {
        memset(this, 0, sizeof(BulletShooter));
        this->bulletSound = -1;
    }

    // Sets command `idx` to run under `flag` and adds `flag` to flags.
    // Returns the command.
    BulletCommand *StartCommand(int idx, int concurrentFlag, DWORD flag);
    // Sets command `idx` up for flag 0x20.
    void SetShooterTurnCommand(int idx, int concurrentFlag, int frames, float arg1, float arg0);
    // Sets command `idx` up for flag 0x80.
    void SetStopTurnCommand(int idx, int concurrentFlag, int frames, int maxTurns, float arg0, float arg1);
    // Sets command `idx` up for flag 0x10.
    void SetShooterAccelerateCommand(int idx, int concurrentFlag, int frames, float arg0, float arg1);
    // Sets command `idx` up for flag 0x2000.
    void SetHiddenCommand(int idx, int concurrentFlag, int frames);
    // A random angle from angle2 up to angle1, and a random speed from
    // speed2 up to speed1. Inline, defined in bullet.cpp.
    float RandomAngle();
    float RandomSpeed();

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
    WORD patternCounter;
    int flags;
    int sound;
    // Bullet::sound.
    int bulletSound;
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
    float baseWidth;
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
    BYTE drawDelayFlag;
    unsigned char pad_4ea[0x2];
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
    // `mode` items of type clearItemType, with 3 to 8 into one item of
    // type 8. Then sets clearAllFrames to 10.
    void ClearAll(int mode);
    // Like ClearAll(1), but each bullet scores a bonus, from 2000 up by
    // 20 per bullet to `maxBonus`, shown as a popup; lasers turn into
    // items only with `toItems`. Unlike th06's it adds nothing to the
    // score and shows no total. Returns the total.
    int ClearAllWithBonus(int maxBonus, int toItems);
    // Turns every bullet of g_BulletManager within `radius` of `pos` into
    // an item of type 6. Not in th06.
    void ConvertBulletsToItems(D3DXVECTOR3 *pos, float radius);
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
    // Returns it, or the first laser while clearAllFrames is set and flag 4
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
    int clearAllFrames;
    Timer updateTimer;
    int updateFrameCount;
    // RegisterChain's argument.
    char *anmPath;
    // Six lists of the bullets to draw, in order, linked by
    // Bullet::next.
    Bullet *drawLists[6];
    // Where SpawnBullet looks for a free bullet first.
    Bullet *nextBullet;
    // The item type cleared bullets turn into (g_ClearedBulletItem).
    int clearItemType;
};
ASSERT_SIZE(BulletManager, 0x37a164);
extern BulletManager g_BulletManager;

// The scripts of each bullet type's five sprites, as th06's.
extern int g_BulletTypeScripts[11][5];

// Sprite offsets by color, for bullets up to 16 and up to 32 high, as
// th06's.
extern int g_BulletColorOffsets16[16];
extern int g_BulletColorOffsets32[16];

// Shows sprite `src` in `color` on `dst`, by the size of `bullet`'s
// sprite, unless `dst` shows it already: th06's SET_BULLET_SPRITE macro
// for the spawning sprites, out of line and without its 0x207 case.
void ShowSpawnSprite(Sprite *dst, Sprite *src, Bullet *bullet, int color);

// Starts the bullet's commands from commandIndex on: skips those whose
// flag the bullet lacks, and stops at an empty one, at one whose concurrentFlag
// is 0 while a behavior is still running, or after starting one
// behavior in its BulletEx (flag 0x2000 only sets boundsCheckDelay and goes on).
// Behaviors started after the first play the bullet's sound.
void RunBulletCommands(Bullet *bullet);

// The bullet behaviors BulletManager::OnUpdate runs by activeFlags, th06's
// sections of its OnUpdate as functions, each on its own BulletEx.
// Flag 1, ex[0]: for 16 frames the bullet runs 5 to 0 faster than its
// speed.
void SlowBulletStart(Bullet *bullet);
// Flag 0x10, ex[1]: for its duration the bullet speeds up by accel,
// turned along its velocity.
void AccelerateAlongVelocity(Bullet *bullet);
// Flag 0x20, ex[2]: for its duration the bullet turns by turnAngle and
// speeds up by turnSpeed a frame.
void TurnAndAccelerate(Bullet *bullet);
// Flags 0x40, 0x100 and 0x80, ex[3]: the bullet slows to a stop over
// each `duration` frames, then plays its sound, turns (by turnAngle, to
// turnAngle, or to the player plus turnAngle) and runs at turnSpeed again,
// maxTurns times (counted in turnCount).
void StopTurnAndRestart(Bullet *bullet);
void StopTurnToAngle(Bullet *bullet);
void StopTurnToPlayer(Bullet *bullet);
// Flags 0x400 and 0x800, ex[4]: a bullet leaving the play area plays
// its sound and bounces off the sides and the top (and the bottom with
// 0x400) at speed turnSpeed, up to maxTurns times (counted in duration).
void BounceOffWalls(Bullet *bullet);

// Draws a bullet with the sprite for its state (spawning effects, fading
// out), placed in the play area and turned along its angle where the
// sprite asks for it, as th06's.
void DrawBullet(Bullet *bullet);

// The colors of effect 0xd by index, as th06's; RegisterChain points
// g_EffectColors at them.
extern DWORD g_EffectColorsFull[0x1c];
extern DWORD *g_EffectColors;

// BulletManager's statically allocated chain elements.
extern ChainElem g_BulletCalcChain;
extern ChainElem g_BulletDrawChain;

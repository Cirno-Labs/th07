// The player and its shots. Only the fields known so far.

#pragma once

#include <d3dx8math.h>

#include "chain.h"
#include "common.h"
#include "sprite.h"
#include "timer.h"

// One of the player's bullets. Its implicit constructor is 0x43ba10.
struct PlayerBullet
{
    Sprite sprite;
    D3DXVECTOR3 pos;
    D3DXVECTOR3 unk_258[0x10];
    D3DXVECTOR3 size;
    // A laser keeps its offset from its option in velocity.z and
    // laserOffsetY.
    D3DXVECTOR3 velocity;
    float laserOffsetY;
    float speed;
    // The direction it flies in.
    float angle;
    Timer timer;
    short damage;
    // 1 while it flies.
    short state;
    // ShotData::type.
    short type;
    // A laser's: its entry in Player::lasers, and its option, from 1.
    short laserIndex;
    short option;
    // FireTrailLaser sets it to ShotData::interval.
    short unk_352;
    unsigned char unk_354[0x10];
};
ASSERT_SIZE(PlayerBullet, 0x364);

// One shot of a shot type, as th06's ShotData.
struct ShotData
{
    // Fired on frames where frame % interval is phase.
    short interval;
    short phase;
    // Where it starts, from the player or its option.
    float offsetX;
    float offsetY;
    float sizeX;
    float sizeY;
    float angle;
    float speed;
    short damage;
    // 0 for the player, else the option, from 1.
    char option;
    // PlayerBullet::type.
    char type;
    // Its script, and its sound or -1.
    short script;
    short sound;
};
ASSERT_SIZE(ShotData, 0x24);

class Player;

// Fills `bullet` with shot `sd` and starts it: from the player or its
// option, its script and its sound. th06's FireShot did this inline.
void FireShot(Player *player, PlayerBullet *bullet, ShotData *sd);

// The shot types' fire functions, by ShotData: fire `sd` into
// `bullet` if it fires on `frame`, returning 1 if it did, else 0.
// Fires on frames where frame % interval is phase.
int TryFireShot(Player *player, PlayerBullet *bullet, int frame, ShotData *sd);
// A laser, while Player::unk_240a is 1: fires if laser sd->phase is
// free, running its timer for sd->interval frames; while it flies from
// another shot, sends its bullet interrupt 1 and frees it.
int FireLaser(Player *player, PlayerBullet *bullet, int frame, ShotData *sd);
// The same while Player::unk_240a is 3, with the timer at 999: also
// keeps sd->interval in bullet->unk_352 and moves its pos and the 16
// unk_258 to x = -999.
int FireTrailLaser(Player *player, PlayerBullet *bullet, int frame, ShotData *sd);
// A plain shot that, when there is a homingTarget, flies at it, turned
// by sd->angle + pi / 2, at 1.5 times its speed.
int FireHomingShot(Player *player, PlayerBullet *bullet, int frame, ShotData *sd);
// A plain shot that flies at Player::unk_b7e58, turned by sd->angle +
// pi / 2.
int FireAimedShot(Player *player, PlayerBullet *bullet, int frame, ShotData *sd);

// Steers a flying homing bullet, as th06's UpdateBullets did Reimu A's
// orbs: for its first 40 frames towards Player::unk_2428, if any, at up
// to 10; otherwise speeds it up by a third up to 10. Returns 0.
int UpdateHomingOrb(Player *player, PlayerBullet *bullet);
// The same at up to 18, speeding up by 0.6.
int UpdateHomingOrbFast(Player *player, PlayerBullet *bullet);
// A flying bullet accelerating up, as th06's UpdateBullets case 2, by
// a random 0.27 to 0.37 a frame. Returns 0.
int UpdateAcceleratingBullet(Player *player, PlayerBullet *bullet);
// A laser, as th06's UpdateBullets case 3: cut short to 20 frames while
// g_Unknown49fbf0.Func42ad66() or a bomb runs; ends it (returning 1)
// once its timer runs out, else keeps it on its option, and unless
// Player::unk_2408 is 2 (returning 1) stretches it to the top. Returns
// 0 otherwise.
int UpdateLaserBullet(Player *player, PlayerBullet *bullet);
// FireTrailLaser's lasers: ended and cut short like UpdateLaserBullet's; the
// first unk_352 points of the bullet's trail (unk_258, from pos, newest
// first) that are set (x at least -900) become Player hitboxes 0x60 on,
// with the bullet's size. Unless Player::unk_2408 is 2 (returning 1),
// the laser then runs from the player to the top. Returns 0 otherwise.
int UpdateTrailLaserBullet(Player *player, PlayerBullet *bullet);
// Draws a UpdateTrailLaserBullet laser's sprite at each set point of its trail,
// fading towards the tail. Returns 0.
int DrawTrailLaserBullet(Player *player, PlayerBullet *bullet);
// A bomb-like shot: once it has hit (state 2), every other frame its
// damage drops to a third (at least 1) and it slows by 12%, returning
// 1 on the frames between; otherwise it is sized by its script (0x441
// to 0x448) and sent upwards, up to 45 degrees off at random. Every 6
// frames it bursts effect 5 at `pos`. Returns 0.
int UpdateBombShot(Player *player, PlayerBullet *bullet, D3DXVECTOR3 *pos);

// One of the 0x70 entries at Player+0x9dc, probably th06's bomb
// hitboxes (bombHitPos, bombHitSize, ...) as one struct.
struct PlayerBombHit
{
    D3DXVECTOR3 pos;
    D3DXVECTOR3 size;
    int unk_18;
    int unk_1c;
};
ASSERT_SIZE(PlayerBombHit, 0x20);

// One of PlayerBomb's 0x80 parts. Its implicit constructor is 0x43bb10.
struct PlayerBombPart
{
    unsigned char unk_0[0x20];
    D3DXVECTOR3 unk_20[0x20];
    unsigned char unk_1a0[0x18];
    Sprite sprites[8];
    unsigned char unk_1418[0x4];
    Timer timer;
};
ASSERT_SIZE(PlayerBombPart, 0x1428);

// Presumably th06's PlayerBomb. Its implicit constructor is 0x43baa0.
struct PlayerBomb
{
    // Whether a bomb runs, as in th06.
    int running;
    unsigned char unk_4[0xc];
    Timer timer;
    unsigned char unk_1c[0x10];
    PlayerBombPart parts[0x80];
};
ASSERT_SIZE(PlayerBomb, 0xa142c);

// One of the player's lasers, th06's laserTimers grown: its clock and
// its bullet, NULL when none flies.
struct PlayerLaser
{
    Timer timer;
    PlayerBullet *bullet;
};
ASSERT_SIZE(PlayerLaser, 0x10);

// The player, the global at 0x4bdad8.
class Player
{
public:
    // Empty, and defined out of line: constructs the members (0x43b7f0).
    Player();

    Sprite sprite;
    Sprite optionSprites[3];
    D3DXVECTOR3 position;
    unsigned char unk_93c[0x78];
    // Where the two options are.
    D3DXVECTOR3 optionPos[2];
    unsigned char unk_9cc[0x10];
    PlayerBombHit unk_9dc[0x70];
    unsigned char unk_17dc[0xc04];
    // The shot each laser was fired from, by ShotData::phase.
    ShotData *laserShots[3];
    unsigned char unk_23ec[0x1c];
    // While it is 2, UpdateLaserBullet leaves its lasers' length alone.
    char unk_2408;
    unsigned char unk_2409[0x1];
    // FireLaser fires its lasers while it is 1, FireTrailLaser while it
    // is 3.
    char unk_240a;
    unsigned char unk_240b[0x5];
    Timer unk_2410;
    unsigned char unk_241c[0xc];
    // What UpdateHomingOrb's and UpdateHomingOrbFast's shots home in on; x is -100 or
    // less when there is nothing.
    D3DXVECTOR3 unk_2428;
    // What the homing shots aim at; x is -100 or less when there is
    // nothing.
    D3DXVECTOR3 homingTarget;
    unsigned char unk_2440[0x4];
    PlayerBullet bullets[0x60];
    PlayerLaser lasers[3];
    Timer unk_169f4;
    Timer unk_16a00;
    Timer unk_16a0c;
    unsigned char unk_16a18[0x8];
    PlayerBomb bomb;
    unsigned char unk_b7e4c[0xc];
    // The angle FireAimedShot's shots fly at, turned by sd->angle + pi / 2.
    float unk_b7e58;
    // The update element and the two draw elements, as in th06
    // (removed together by 0x442b13's function).
    ChainElem *calcChain;
    ChainElem *drawChain;
    ChainElem *drawChain2;
};
ASSERT_SIZE(Player, 0xb7e68);
extern Player g_Player;

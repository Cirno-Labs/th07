// The player and its shots. Only the fields known so far.

#pragma once

#include <d3dx8math.h>

#include "chain.h"
#include "common.h"
#include "sprite.h"
#include "timer.h"

class Player;
struct Effect;
struct PlayerBullet;
struct ShotData;

// A shot's callbacks. Under /Gr these are __fastcall.
typedef int (*ShotFireFunc)(Player *player, PlayerBullet *bullet, int frame, ShotData *sd);
typedef int (*ShotUpdateFunc)(Player *player, PlayerBullet *bullet);
typedef int (*ShotHitFunc)(Player *player, PlayerBullet *bullet, D3DXVECTOR3 *pos);

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
    // 0 unused, 1 flying, 2 hit, as in th06.
    short state;
    // ShotData::type.
    short type;
    // A laser's: its entry in Player::lasers, and its option, from 1.
    short laserIndex;
    short option;
    // FireTrailLaser sets it to ShotData::interval.
    short unk_352;
    // The shot's callbacks, and the shot.
    ShotUpdateFunc update;
    ShotUpdateFunc draw;
    ShotHitFunc hit;
    ShotData *shot;
};
ASSERT_SIZE(PlayerBullet, 0x364);

// One shot of a shot type: th06's ShotData, with callbacks. A table of
// them ends with an interval below 0.
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
    // Fires it; TryFireShot when NULL.
    ShotFireFunc fire;
    // Copied into the bullet it fires.
    ShotUpdateFunc update;
    ShotUpdateFunc draw;
    ShotHitFunc hit;
};
ASSERT_SIZE(ShotData, 0x34);

// The shots of a shot type up to a power level.
struct ShotTable
{
    ShotData *shots;
    // The power it lasts below; FireShots takes the first above the
    // player's.
    int maxPower;
};

// A shot type's data, behind Player::shotType and shotTypeFocused. Only
// the fields known so far.
struct ShotTypeData
{
    unsigned char unk_0[0x8];
    // Player::deathbombFrames once the player has respawned.
    int deathbombFrames;
    unsigned char unk_c[0x28];
    ShotTable tables[1];
};

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
// Player::playerState is 2 (returning 1) stretches it to the top. Returns
// 0 otherwise.
int UpdateLaserBullet(Player *player, PlayerBullet *bullet);
// FireTrailLaser's lasers: ended and cut short like UpdateLaserBullet's; the
// first unk_352 points of the bullet's trail (unk_258, from pos, newest
// first) that are set (x at least -900) become Player hitboxes 0x60 on,
// with the bullet's size. Unless Player::playerState is 2 (returning 1),
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
// A hit at `pos`: on every eighth (Player::hitCounter) bursts effect 5
// at the bullet's x and pos's y. Returns 0.
int BurstHitEffect(Player *player, PlayerBullet *bullet, D3DXVECTOR3 *pos);
// th06's FireShots: fires the shots of the shot type's table for the
// player's power, unfocused or focused, for `frame` into free bullets,
// through each shot's fire function, giving the bullets its callbacks.
void FireShots(Player *player, int frame);
// th06's UpdateBullets: drops lasers whose options are gone (lasers 0
// and 1 unless unk_240a is 1, laser 2 unless it is 3) or all of them
// while playerState is 2, runs down the laser timers (cut to 50 while
// fireTimer is idle), then moves the flying bullets, through their
// update callback first, ending those that leave the play area (but
// types 4 and 5) or whose script ends.
void UpdateBullets(Player *player);
// th06's DrawBullets: draws the flying bullets (state 1), turned along
// their angle and offset by the play area, then their draw callbacks.
void DrawBullets(Player *player);
// th06's DrawHitBullets: the same for the bullets that hit (state 2),
// without the draw callback.
void DrawHitBullets(Player *player);
// th06's UpdateFiring: runs fireTimer, firing (FireShots) on each new
// frame, except during a bomb of character 1 shot type 1, and stops it
// after 30 frames or while respawning or dying. Returns 0.
int UpdateFiring(Player *player);

// OnUpdate's steps, not decompiled yet. Func43ee50 runs unless the
// player is respawning or dying, Func440cf0 while dying, and UpdateRespawningState
// while respawning or when Func440cf0 returns nonzero.
int Func43ee50(Player *player);
// OnUpdate's first step, as the start of th06's: clears the sizes of
// the 0x70 unk_9dc hitboxes, then counts down each PlayerArea's unk_18,
// growing unk_10 by unk_14 meanwhile, and frees it once it is 0.
void UpdateHitboxAreas(Player *player);
void Func4409f0(Player *player);
int Func440cf0(Player *player);
// OnUpdate's respawning step, as th06's: for 60 frames clears bullets
// (respawnClearFrames), fades the player in over 30 frames of
// stateTimer, scaling it from 3 by 0 to 1 by 1, then makes it
// invulnerable (playerState 3) for 240 frames, with deathbombFrames
// from the shot type.
void UpdateRespawningState(Player *player);
// OnUpdate's step after the dying and respawning ones, as the end of
// th06's OnUpdate: runs down respawnClearFrames, clearing the bullets
// meanwhile. While invulnerable (playerState 3) keeps unk_b7e68 on the
// player and counts stateTimer down, blinking the player, and at 0 ends
// unk_b7e68 and returns to playerState 0. While playerState is 4 keeps
// unk_b7e6c on the player, sets Unknown626270::unk_9620 from the time
// left, and counts stateTimer down, blinking the player red and setting
// Unknown1347b00's color, then calls Func441670 at 0. Otherwise ticks
// stateTimer.
void UpdateInvulnerability(Player *player);
// OnUpdate's last step: clears unk_2428, homingTarget and unk_2440 for
// the next frame, then sets AsciiText's unk_74d4 to 2 while the player
// is in the bottom left (y at least 400, x below 160) and back to 3
// once it leaves.
void ResetHomingAndText(Player *player);

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

// One of the 0x60 entries at Player+0x17dc, maybe th06's
// bulletClearAreas grown. Free while width and unk_10 are 0.
struct PlayerArea
{
    float x;
    float y;
    float width;
    float height;
    float unk_10;
    float unk_14;
    int unk_18;
    int unk_1c;
};
ASSERT_SIZE(PlayerArea, 0x20);

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

    // Starts fireTimer from 0 unless it already runs, as th06's.
    static void StartFiring(Player *player);
    // A draw callback, as th06's: draws the bullets that hit
    // (DrawHitBullets). Returns 1.
    static int OnDraw2(Player *player);
    // The damage the player's bullets and bomb hitboxes deal to an
    // enemy's `size` box at `pos`, as th06's; none while stateTimer
    // has not ticked. A flying bullet (or a hit one of type 3) that
    // meets it deals its damage (a third, at least 1, while a bomb
    // runs), unless it is of type 4 or 5 on an odd frame or its hit
    // callback returns nonzero; then, unless of type 4 or 5, it is
    // marked hit (restarting its script 0x20 on and bursting effect 5
    // if it was flying) and, unless of type 3, slowed to an eighth.
    // Each unk_9dc box that meets it deals its unk_18 and adds it to
    // its unk_1c; every fourth such hit bursts effect 3 at `pos` (5
    // for the last 0x10 boxes), and during a bomb sets *hitByBomb.
    int CalcDamage(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, int *hitByBomb);
    // Whether a `size` box at `pos` meets one of unk_17dc's areas: a box
    // while its width is set, else a circle of radius unk_10 if that is
    // set. Keeps the area's unk_1c in unk_2404 and returns 2 if so, else
    // returns 0. Not in th06.
    int CheckAreaHit(D3DXVECTOR3 *pos, D3DXVECTOR3 *size);
    // The angle from `pos` to the player, pi / 2 when they coincide.
    float AngleFrom(D3DXVECTOR3 *pos);
    // Starts dying, as th06's: rerolls the checksums, bursts the death
    // effects, sets playerState to 2 and stateTimer to 0, and plays
    // sound 4.
    void Die();
    // Whether a `size` box at `pos` hits the player, as th06's: 2 if it
    // meets one of unk_17dc's areas (CheckAreaHit, unk_2404 6 otherwise),
    // else 1 if it meets the hitbox, else 0. A hit marks bit 2 of
    // g_ReplayManager->unk_d6; it calls Func441bd0(0) while playerState
    // is 4 and kills the player while it is 0.
    int CheckHit(D3DXVECTOR3 *pos, D3DXVECTOR3 *size);
    // Whether a `size` box at `pos`, grown by 20 on each side, grazes the
    // player, as th06's: 2 if the box itself meets one of unk_17dc's
    // areas, else 0 while respawning or dying or off the graze box, else
    // grazes `pos` (Graze) and returns 1.
    int CheckGraze(D3DXVECTOR3 *pos, D3DXVECTOR3 *size);
    // Removes the player's update and draw elements from the chain.
    // Takes the first free one of unk_17dc's first 0x5f entries, or the
    // last entry, and sets it to a `width` by `height` area at `pos`
    // with unk_18 0. Returns it.
    PlayerArea *AddHitboxArea(D3DXVECTOR3 *pos, float width, float height, int unk_1c);
    // The same, setting unk_10 to unk_1c instead of width and height.
    PlayerArea *AddHitboxAreaCircle(D3DXVECTOR3 *pos, float unk_10, float unk_14, int unk_18, int unk_1c);
    // Clears the player and adds it to the chains (update priority 8,
    // draw 6 and 8), keeping `unk` in unk_2409. Returns 0, or -1 if
    // adding fails.
    static int RegisterChain(int unk);
    // Whether a `size` box at `pos` meets the unk_978 box while
    // playerState is 0, 3 or 4, as th06's (which tested the graze box).
    int TouchesGrazeBox(D3DXVECTOR3 *pos, D3DXVECTOR3 *size);
    // Whether a laser hits or grazes the player, as th06's: `pos` and
    // `size` are its box in its own frame, into which the hitbox is
    // turned by `angle` around `origin`. On a hit, marks bit 2 of g_ReplayManager->unk_d6, then
    // calls Func441bd0(0) while playerState is 4 and kills the player
    // while it is 0, returning 1 in both cases, else 0. Otherwise, with
    // `canGraze`, grazes the player within 48 of the box unless it is
    // respawning or dying, returning 2; else returns 0.
    int CheckLaserHit(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, D3DXVECTOR3 *origin, float angle, int canGraze);
    // Scores a graze of the bullet at `pos`, as th06's: counts it
    // unless a bomb runs, bursts effect 8 halfway to it (three red ones
    // while unk_240d is 1 and not focused), adds 6 hundredths to the
    // rank, plays sound 0x1e, adds to g_Unknown12fe0d0 and 2000 points,
    // and while unk_240d is 1 calls Unknown626270's Func42f789 and
    // Func42f736 with 30 (focused) or 80.
    void Graze(D3DXVECTOR3 *pos);
    // CheckHit calls it with 0 on a hit while playerState is 4.
    void Func441bd0(int unk);
    // UpdateInvulnerability calls it when playerState 4 runs out.
    void Func441670();
    // The update callback: unless the game is frozen, runs the steps
    // (UpdateHitboxAreas on), steps the sprites (the options' while unk_240a
    // is set), the bullets (UpdateBullets) and the firing (UpdateFiring).
    // Returns 1.
    static int OnUpdate(Player *player);
    static int OnDraw(Player *player);
    static int AddedCallback(Player *player);
    // Unless the scene is 3, 0xb or 0xc, releases ANM 10 and sets
    // AsciiText's sprite interrupts (99) and unk_74d4; frees the shot
    // type data. Returns 0.
    static int DeletedCallback(Player *player);
    static void RemoveChain();

    Sprite sprite;
    Sprite optionSprites[3];
    D3DXVECTOR3 position;
    unsigned char unk_93c[0xc];
    // The corners of the hitbox and of the graze box, as th06's.
    D3DXVECTOR3 hitboxMin;
    D3DXVECTOR3 hitboxMax;
    D3DXVECTOR3 grazeMin;
    D3DXVECTOR3 grazeMax;
    // The corners of the box TouchesGrazeBox tests.
    D3DXVECTOR3 unk_978;
    D3DXVECTOR3 unk_984;
    // The hitbox's half size, as th06's.
    D3DXVECTOR3 hitboxSize;
    unsigned char unk_99c[0x18];
    // Where the two options are.
    D3DXVECTOR3 optionPos[2];
    unsigned char unk_9cc[0x10];
    PlayerBombHit unk_9dc[0x70];
    PlayerArea unk_17dc[0x60];
    unsigned char unk_23dc[0x4];
    // The shot each laser was fired from, by ShotData::phase.
    ShotData *laserShots[3];
    unsigned char unk_23ec[0x4];
    // Scale the player's movement, as th06's.
    float moveScaleX;
    float moveScaleY;
    // As th06's: while the player dies, the frames left to bomb.
    int deathbombFrames;
    unsigned char unk_23fc[0x4];
    // Frames left of clearing all bullets after a respawn, as th06's.
    int respawnClearFrames;
    // The unk_1c of the PlayerArea CheckAreaHit met last.
    int unk_2404;
    // As th06's: 0 normally, 1 respawning, 2 dying, 3 invulnerable.
    // Firing stops at 1 and 2.
    char playerState;
    // RegisterChain's argument, as th06's unk_09e1.
    BYTE unk_2409;
    // FireLaser fires its lasers while it is 1, FireTrailLaser while it
    // is 3. Where th06 has optionState.
    char unk_240a;
    // Picks shotTypeFocused over shotType, as th06's focused.
    char focused;
    // Counts hits on enemies, to burst an effect every few, as in th06.
    BYTE hitCounter;
    // Graze treats 1 specially.
    char unk_240d;
    unsigned char unk_240e[0x2];
    Timer unk_2410;
    unsigned char unk_241c[0xc];
    // What UpdateHomingOrb's and UpdateHomingOrbFast's shots home in on; x is -100 or
    // less when there is nothing.
    D3DXVECTOR3 unk_2428;
    // What the homing shots aim at; x is -100 or less when there is
    // nothing.
    D3DXVECTOR3 homingTarget;
    // Cleared every frame by ResetHomingAndText.
    int unk_2440;
    PlayerBullet bullets[0x60];
    PlayerLaser lasers[3];
    // Runs while firing, as th06's: -1 when idle, stopped after 30
    // frames.
    Timer fireTimer;
    // Started from 0 by RegisterChain, as th06's.
    Timer stateTimer;
    Timer unk_16a0c;
    unsigned char unk_16a18[0x8];
    PlayerBomb bomb;
    unsigned char unk_b7e4c[0xc];
    // The angle FireAimedShot's shots fly at, turned by sd->angle + pi / 2.
    float unk_b7e58;
    // The update element and the two draw elements, as in th06
    // (removed together by RemoveChain).
    ChainElem *calcChain;
    ChainElem *drawChain;
    ChainElem *drawChain2;
    // Effects that follow the player while playerState is 3 and 4.
    Effect *unk_b7e68;
    Effect *unk_b7e6c;
    // The shot type's data, unfocused and focused.
    ShotTypeData *shotType;
    ShotTypeData *shotTypeFocused;
};
ASSERT_SIZE(Player, 0xb7e78);
extern Player g_Player;

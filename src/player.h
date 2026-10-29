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
    unsigned char unk_0[0x2];
    // How many entries tables has.
    WORD tableCount;
    // The bombs the player respawns with.
    float bombs;
    // Player::deathbombFrames once the player has respawned.
    int deathbombFrames;
    // The hitbox and the graze box, both square.
    float hitboxSize;
    float grazeSize;
    unsigned char unk_14[0x8];
    // The share of GameManager::unk_961c over stats->unk_88 a death
    // costs.
    float unk_1c;
    unsigned char unk_20[0x14];
    ShotTable tables[1];
};

// A shot type's bomb callbacks, by character * 2 + shot type: those
// PlayerBomb runs.
struct BombData
{
    void (*update)(Player *player);
    void (*draw)(Player *player);
    void (*updateFocused)(Player *player);
    void (*drawFocused)(Player *player);
};
extern BombData g_Bombs[];

// The shot type files, unfocused and focused, by character * 2 + shot
// type.
extern char *g_ShotFiles[];
extern char *g_ShotFilesFocused[];

// The shot callbacks a shot file names by index, which LoadShotTypeFile turns
// into pointers. Entry 0 is NULL.
extern ShotFireFunc g_ShotFireFuncs[];
extern ShotUpdateFunc g_ShotUpdateFuncs[];
extern ShotUpdateFunc g_ShotDrawFuncs[];
extern ShotHitFunc g_ShotHitFuncs[];

// Loads the shot type file at `path` into `*out`: turns its table
// offsets into pointers and its shots' callback indices into the
// callbacks. Returns 0, or -1 if the file does not load.
int LoadShotTypeFile(ShotTypeData **out, char *path);

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
// player is respawning or dying, UpdateDyingState while dying, and UpdateRespawningState
// while respawning or when UpdateDyingState returns nonzero.
int Func43ee50(Player *player);
// OnUpdate's first step, as the start of th06's: clears the sizes of
// the 0x70 unk_9dc hitboxes, then counts down each PlayerArea's unk_18,
// growing unk_10 by unk_14 meanwhile, and frees it once it is 0.
void UpdateHitboxAreas(Player *player);
// OnUpdate's bomb step, the bomb part of th06's: with unk_240d set and
// button 2 held outside a bomb, ends playerState 4 (StopBombState(1)) and
// collects all items instead. Otherwise tries StartBombState while
// unk_240d is 2 and runs down unk_23fc. A running bomb takes its unk_c
// from GameManager::unk_961c on each new frame (down to stats->
// unk_88) and runs. Else, with button 2 held, deathbombFrames left,
// bombs left, unk_23fc at 0, intact checksums and no
// g_Unknown49fbf0.Func42ad66(), starts a bomb: marks bit 1 of
// g_ReplayManager->unk_d6, uses a bomb, runs it for the player's
// focus, takes 2 from the rank and gives 6 more deathbombFrames (up to
// the shot type's). unk_23dc is 1 on the frame a bomb starts, else 0.
void UpdateBombState(Player *player);
// OnUpdate's dying step, the dying part of th06's: while
// deathbombFrames run, ends playerState 4 at once (StopBombState(0)) if
// unk_240d is 1, else counts them down and at 0 loses a life: marks bit
// 4 of g_ReplayManager->unk_d6, counts the death, and with lives left
// drops 16 power and spawns six power items and takes a share of
// GameManager::unk_961c (at most 100000, or 60000 for character 2);
// without, drops all power and spawns five type 4 items; then takes 16
// from the rank. Afterwards it fades and stretches the player out over
// 30 frames, then puts it back at the bottom of the play area,
// respawning, and takes a life and refills the bombs, returning 1, or
// with no lives left opens the continue menu. Returns 0 otherwise.
int UpdateDyingState(Player *player);
// Tries to start playerState 4 (UpdateBombState calls it while unk_240d is
// 2): sets unk_240d to 2 during a bomb or while
// g_Unknown49fbf0.Func42ad66(), or while playerState is 1 or 3, or
// while dying without deathbombFrames; while dying with them calls
// StopBombState(0). Otherwise starts playerState 4 for 540 frames of
// stateTimer (kept in unk_16a0c), with unk_240d 1: ends unk_b7e68 (and
// lets unk_b7e6c end), starts effect 0x1c in slot 4 to follow the
// player in unk_b7e6c, calls Gui's Func427c81(0, 2), plays
// sounds 0x20 and 0x24 and marks bit 8 of g_ReplayManager->unk_d6.
void StartBombState(Player *player);
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
// unk_b7e6c on the player, sets GameManager::unk_9620 from the time
// left, and counts stateTimer down, blinking the player red and setting
// Stage's color, then calls EndBombState at 0. Otherwise ticks
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
    // Reimu A: 0 unused, 1 flying, 2 and up after it bursts, while
    // unk_4 counts to 30.
    int state;
    int unk_4;
    union
    {
        // Reimu A focused: the orb's speed, as th06's partSpeed.
        float homingSpeed;
        // Marisa B: the laser's direction.
        float laserAngle;
        // Sakuya A: added to speed every frame.
        float speedStep;
    };
    float speed;
    float angle;
    D3DXVECTOR3 pos;
    // Marisa A focused: the star's last positions, newest first.
    D3DXVECTOR3 trail[0x20];
    D3DXVECTOR3 velocity;
    D3DXVECTOR3 acceleration;
    Sprite sprites[8];
    // Sakuya A, part 0: an effect 0x15 at the player.
    Effect *effect;
    Timer timer;
};
ASSERT_SIZE(PlayerBombPart, 0x1428);

// Presumably th06's PlayerBomb. Its implicit constructor is 0x43baa0.
struct PlayerBomb
{
    // Whether a bomb runs, as in th06.
    int running;
    // Player::focused when the bomb started: picks updateFocused and
    // drawFocused over update and draw.
    int focused;
    // Set to 999 when a bomb starts, as th06's.
    int duration;
    // What each frame of the bomb takes from GameManager::unk_961c.
    int unk_c;
    Timer timer;
    // Run the bomb and draw it, unfocused and focused, as th06's update
    // and draw.
    void (*update)(Player *player);
    void (*draw)(Player *player);
    void (*updateFocused)(Player *player);
    void (*drawFocused)(Player *player);
    PlayerBombPart parts[0x80];
    // Where the player was when the bomb started (Reimu A, Reimu B
    // focused, Marisa B, Sakuya A).
    D3DXVECTOR3 startPos;
};
ASSERT_SIZE(PlayerBomb, 0xa1438);

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
    // g_ReplayManager->unk_d6; it calls StopBombState(0) while playerState
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
    // calls StopBombState(0) while playerState is 4 and kills the player
    // while it is 0, returning 1 in both cases, else 0. Otherwise, with
    // `canGraze`, grazes the player within 48 of the box unless it is
    // respawning or dying, returning 2; else returns 0.
    int CheckLaserHit(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, D3DXVECTOR3 *origin, float angle, int canGraze);
    // Scores a graze of the bullet at `pos`, as th06's: counts it
    // unless a bomb runs, bursts effect 8 halfway to it (three red ones
    // while unk_240d is 1 and not focused), adds 6 hundredths to the
    // rank, plays sound 0x1e, adds to g_Unknown12fe0d0 and 2000 points,
    // and while unk_240d is 1 calls GameManager's IncreaseBonusCap and
    // AddBonusPointsCapped with 30 (focused) or 80.
    void Graze(D3DXVECTOR3 *pos);
    // Ends playerState 4 early (the hit checks call it on a hit,
    // StartBombState while dying with deathbombFrames): ends unk_b7e6c and
    // starts a new effect 0x1c in slot 4 there, makes the player
    // invulnerable (playerState 3) for 40 frames with unk_23fc 40 and
    // unk_240d 0, resets GameManager::unk_9620, adds an area at the
    // player (AddHitboxAreaCircle with 32, 16, 0x32 and 8), bursts 32 effects
    // 0x1d in a circle and plays sounds 7 and 0x21, marking bit 0x10 of
    // g_ReplayManager->unk_d6. `unk` is unused.
    void StopBombState(int unk);
    // Ends playerState 4 (UpdateInvulnerability calls it when it runs out): calls
    // GameManager's IncreaseBonusCap and AddBonusPointsCapped with 10000, scores ten
    // times unk_961c over stats->unk_88 (shown through Gui's
    // Func427c81), resets unk_9620, plays sound 0x21, finishes a respawn
    // if one runs, then makes the player invulnerable (playerState 3)
    // for 40 frames, with unk_23fc 40 and unk_240d 0, and ends the
    // unk_b7e6c effect.
    void EndBombState();
    // The update callback: unless the game is frozen, runs the steps
    // (UpdateHitboxAreas on), steps the sprites (the options' while unk_240a
    // is set), the bullets (UpdateBullets) and the firing (UpdateFiring).
    // Returns 1.
    static int OnUpdate(Player *player);
    // The draw callback, as th06's: draws the flying bullets
    // (DrawBullets) and a running bomb, then, unless the game is in the
    // continue menu, the player and, while unk_240a is set and
    // playerState is 0, 3 or 4, the options, all offset by the play
    // area. While playerState is 4 and stateTimer runs, blinks the
    // player red and sets Stage's color, as UpdateInvulnerability.
    // Returns 1.
    static int OnDraw(Player *player);
    // The update element's added callback, as th06's: loads the shot
    // type files and, outside the replay and other scenes (3, 11 and
    // 12), the character's ANM, then sets up the player, its boxes from
    // the shot type, options, bullets, timers and bomb callbacks, and
    // (outside those scenes) sends g_AsciiText's sprites interrupt 1,
    // its unk_b7c interrupt 2. Caps GameManager::unk_9620 as
    // PlayAddedCallback does. Returns 0, or -1 if a file fails to load.
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
    D3DXVECTOR3 grazeSize;
    D3DXVECTOR3 unk_9a8;
    // Where the two options are.
    D3DXVECTOR3 optionPos[2];
    unsigned char unk_9cc[0x10];
    PlayerBombHit unk_9dc[0x70];
    PlayerArea unk_17dc[0x60];
    // 1 on the frame a bomb starts, else 0 (UpdateBombState).
    int unk_23dc;
    // The shot each laser was fired from, by ShotData::phase.
    ShotData *laserShots[3];
    unsigned char unk_23ec[0x4];
    // Scale the player's movement, as th06's.
    float moveScaleX;
    float moveScaleY;
    // As th06's: while the player dies, the frames left to bomb.
    int deathbombFrames;
    // Set to 40 by EndBombState.
    int unk_23fc;
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
    // Cleared by AddedCallback, as th06's direction.
    int unk_241c;
    // The last frame's movement, as th06's.
    float velocityX;
    float velocityY;
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

// Bomb helpers, in bomb.cpp.

// Probably th06's DimPlayAreaForBomb, turned into a stage tint: over
// the bomb's first and last 60 frames the stage fades between gray
// (0x80) and 0x30, through Stage::SetTintColor, after resetting the
// SpriteEngine's color; sets the stage's unk_52b0, which keeps OnDraw2
// from resetting that color.
void DimPlayAreaForBomb(Player *player);

// Ends the effect at unk_b7e68 and starts a red effect 0x19 at the
// player in slot 0 that runs as long as stateTimer, keeping it there.
void SpawnBombAura(Player *player);

// Sets the bomb's unk_c, what each of its frames takes from
// GameManager::unk_961c: `share` of unk_961c over stats->unk_88, halved
// on difficulty 2, quartered on 3 and a third on 4 and 5, spread over
// the bomb's duration, but at least `minimum` spread the same way,
// both rounded down to tens.
void SetBombDrain(Player *player, int minimum, float share);

// Reimu A's bomb, th06's UpdateReimuABomb reworked: 140 frames with
// stateTimer at 200, an aura (SpawnBombAura), all items collected and
// an area (AddHitboxAreaCircle); from frame 8 one orb every 6 frames (8), fanned
// out by startPos's side of the play area. Each one slows down by 0.4
// a frame, flies off backwards and bursts once its speed is below -10
// (a 256 box of 400 damage, an area, sound 0xf and a shake), then stays
// a 256 box of 2 damage for 30 frames. Stops at the end.
void UpdateReimuABomb(Player *player);

// Draws Reimu A's bomb, as th06's DrawReimuABomb: the stage tint
// (DimPlayAreaForBomb), then each live orb's four sprites at its
// position in the play area.
void DrawReimuABomb(Player *player);

// Reimu A's focused bomb, closest to th06's UpdateReimuABomb: 300
// frames with stateTimer at 360 and the player slowed to 0.6; from
// frame 60 an orb every 16 frames (7) in a random direction, homing
// in on unk_2428 (else the player), until it has hit 100 times or 30
// frames are left, then bursts as Reimu A's. Stops at the end,
// restoring the player's speed.
void UpdateReimuABombFocused(Player *player);

// Draws Reimu A's focused bomb, as DrawReimuABomb.
void DrawReimuABombFocused(Player *player);

// Reimu B's bomb, as th06's UpdateReimuBBomb: 140 frames with
// stateTimer at 200 and an aura; four barriers, two crossing the play
// area each way through the player, each an area (AddHitboxArea) and
// a box of 16 damage that follow its sprite on odd frames.
void UpdateReimuBBomb(Player *player);

// Draws Reimu B's bomb, as th06's DrawReimuBBomb: the stage tint, then
// each barrier's sprite at its position in the play area.
void DrawReimuBBomb(Player *player);

// Reimu B's focused bomb: 190 frames with stateTimer at 250, an aura
// and the player slowed to 0.4; a barrier of three sprites around
// where the bomb started, a 256 box of 18 damage, and one area
// (AddHitboxAreaCircle, 192) for the whole bomb.
void UpdateReimuBBombFocused(Player *player);

// Draws Reimu B's focused bomb: the stage tint, then the barrier's
// three sprites around startPos in the play area.
void DrawReimuBBombFocused(Player *player);

// Marisa A's bomb, as th06's UpdateMarisaABomb: 200 frames with
// stateTimer at 250 and an aura; eight stars fly out in a circle at
// speed 2, each an area (AddHitboxAreaCircle, 96) and a 128 box of 8
// damage on two frames out of three.
void UpdateMarisaABomb(Player *player);

// Draws Marisa A's bomb, as th06's DrawMarisaABomb: the stage tint,
// then each star three times, at shrinking sizes along its trail.
void DrawMarisaABomb(Player *player);

// Marisa A's focused bomb: 260 frames with stateTimer at 310, an aura
// and the player slowed to 0.4; a star every 6 frames (24) shot
// upwards from the player at a slightly random angle and accelerating,
// keeping a trail of its last 8 positions, each an area
// (AddHitboxAreaCircle, 96) and, until it has hit 80 times, a 128 box
// of 12 damage. A star ends once its y is below -256.
void UpdateMarisaABombFocused(Player *player);

// Draws Marisa A's focused bomb: the stage tint, then each star at
// its position and at trail[3] and trail[7], shrinking.
void DrawMarisaABombFocused(Player *player);

// Marisa B's bomb: 300 frames with stateTimer at 300, an aura and the
// player slowed to 0.4; three lasers from the player, 120 degrees
// apart, turning faster and faster (one way or the other by startPos's
// side of the play area), each six 128 boxes of 10 damage and areas
// (AddHitboxAreaCircle, 64) along its length.
void UpdateMarisaBBomb(Player *player);

// Draws Marisa B's bomb: the stage tint, then each laser's sprite
// halfway along it from the player, turned along laserAngle.
void DrawMarisaBBomb(Player *player);

// Marisa B's focused bomb, th06's Master Spark (UpdateMarisaBBomb):
// 340 frames with stateTimer at 390, an aura and the player slowed to
// 0.2; on three frames out of four a box of 23 damage and an area
// (AddHitboxArea) from the player up to the top of the play area.
void UpdateMarisaBBombFocused(Player *player);

// Draws Marisa B's focused bomb, as th06's DrawMarisaBBomb: the stage
// tint, then the beam's four sprites fanned out upwards from the
// player.
void DrawMarisaBBombFocused(Player *player);

// Sakuya A's bomb: 160 frames with stateTimer at 210, an aura and an
// effect 0x15; from frame 60 to 120 five knives a frame (up to 0x60)
// start 24 out from startPos in random directions, each speeding up
// and turning at its own random rate, an area (AddHitboxAreaCircle,
// 32) and a 24 box of 10 damage until it has hit 30 times (then it
// restarts its sprite at script 0x460 and stops), gone once it leaves
// the play area.
void UpdateSakuyaABomb(Player *player);

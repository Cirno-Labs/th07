// Enemies, as th06's enemy.h. Only the fields known so far.

#pragma once

#include <d3dx8math.h>

#include "common.h"
#include "bullet.h"
#include "effect.h"
#include "sprite.h"
#include "timer.h"

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
    // Can be shot and killed, as th06's.
    unsigned char shootable : 1;
    unsigned char unk_9 : 1;
    unsigned char unk_a : 1;
    // Hides the effects that follow the enemy.
    unsigned char unk_b : 1;
    unsigned char unk_c : 1;
    unsigned char unk_d : 1;
    // A boss, as th06's isBoss.
    unsigned char isBoss : 1;
    // Keeps the position within lowerBound and upperBound, as th06's.
    unsigned char clampPos : 1;
    // What dying does, as th06's.
    unsigned char deathMode : 3;
    unsigned char unk_13 : 1;
    // Turns the main sprite with the enemy's angle, as th06's.
    unsigned char rotateSprite : 1;
    unsigned char unk_15 : 1;
    // CheckTimerCallback leaves the spell card and the bullets alone, as
    // th06's.
    unsigned char keepSpellCard : 1;
};

// The variables of an enemy's script.
struct EnemyScriptVars
{
    int unk_0[0x1a];
};
ASSERT_SIZE(EnemyScriptVars, 0x68);

// Where one enemy is in its script (0x218 bytes), th06's grown. Its
// implicit constructor is 0x41f220.
struct EnemyScriptState
{
    unsigned char unk_0[0x18];
    EnemyScriptVars vars;
    unsigned char unk_80[0x198];
};
ASSERT_SIZE(EnemyScriptState, 0x218);

// One of Enemy's 0x122 boxes (0x1c bytes); Initialize hides the first
// 0x60 at x -999.
struct EnemyBox
{
    D3DXVECTOR3 pos;
    unsigned char unk_c[0x10];
};
ASSERT_SIZE(EnemyBox, 0x1c);

// One enemy (0x4f48 bytes).
struct Enemy
{
    // Moves by the velocity scaled by App::unk_178, mirrored with
    // flags.mirrored, as th06's; first keeps the last move in posDelta
    // and the position in prevPos.
    void Move();
    // Once the life falls below one of lifeCallbackLife: sets it to
    // that, starts the matching lifeCallbackSub, drops the timer
    // callback, resets the rank values, the call stack and the shooter
    // from the template's, and clears the other enemies, as th06's.
    // Returns 1 then, else 0.
    int CheckLifeCallback();
    // Once callbackTimer reaches timerCallbackTime: restores the life
    // from the highest lifeCallbackLife, starts timerCallbackSub, makes
    // the death subroutine the next, ends the spell card (taking a
    // quarter of Unknown626270::unk_961c above stats->unk_88, in tens)
    // and clears the bullets unless flags.keepSpellCard, clears the
    // other enemies and resets as CheckLifeCallback, as th06's. While
    // it is boss 0, shows the seconds left first. Returns 1 then, else
    // 0.
    int CheckTimerCallback();
    // After the enemy leaves the play area or its script ends: frees it
    // (flags.deathMode 0) or makes it unshootable, hides the boss bar
    // for bosses 0 to 3 and releases its effects, as th06's. A boss
    // also leaves Unknown9a9b00::bosses. Marks the replay frame with
    // 0x20.
    void Despawn();
    // Keeps the position within the bounds, with flags.clampPos, as
    // th06's.
    void ClampPos();
    // Contact with the player, from th06's OnUpdate: with flags.unk_d,
    // every sixth frame of callbackTimer lets the player graze `size`
    // / 0.7 at `pos`; touching `size` / 1.5 there costs a shootable
    // enemy that is neither a boss nor flags.unk_d 10 life.
    void CheckPlayerContact(D3DXVECTOR3 *pos, D3DXVECTOR3 *size);

    Sprite sprite;
    Sprite sprites[2];
    EnemyScriptState ctx;
    // Contexts saved by calls, as th06's.
    EnemyScriptState savedCtx[16];
    int stackDepth;
    int unk_2a80;
    // The subroutine the script runs when the enemy dies; negative for
    // none.
    int deathSub;
    int interrupts[0x20];
    // The interrupt to run next; negative for none.
    int pendingInterrupt;
    D3DXVECTOR3 pos;
    D3DXVECTOR3 velocity;
    // Where Move found the enemy, and how far it had moved since the
    // call before.
    D3DXVECTOR3 prevPos;
    D3DXVECTOR3 posDelta;
    // The size of the enemy's hitbox.
    D3DXVECTOR3 hitboxSize;
    unsigned char unk_2b48[0xc];
    float angle;
    float angularVelocity;
    unsigned char unk_2b5c[0x8];
    float speed;
    float acceleration;
    unsigned char unk_2b6c[0x8];
    // Where shots start, from the position.
    D3DXVECTOR3 shootOffset;
    unsigned char unk_2b80[0x18];
    Timer moveTimer;
    int unk_2ba4;
    // How the shots' speed and counts grow with the rank, as th06's.
    float rankSpeedLow;
    float rankSpeedHigh;
    short rankCount1Low;
    short rankCount1High;
    short rankCount2Low;
    short rankCount2High;
    int life;
    int maxLife;
    int score;
    Timer callbackTimer;
    // The sprite's color when spawned.
    D3DCOLOR color;
    BulletShooter shooter;
    // Fires `shooter` every shootInterval frames, counted by shootTimer.
    int shootInterval;
    Timer shootTimer;
    BulletShooter laserShooter;
    Laser *lasers[0x20];
    int laserIndex;
    int item;
    BYTE deathEffect;
    BYTE unk_2e15;
    BYTE unk_2e16;
    // The enemy's slot in Unknown9a9b00::bosses, while it is one.
    BYTE bossIndex;
    BYTE hitFlashTimer;
    unsigned char unk_2e19[0x3];
    Timer unk_2e1c;
    EnemyFlags flags;
    unsigned char unk_2e2b[0x5];
    // The sprite scripts for turning, as th06's.
    short turnStill;
    short turnStillFromLeft;
    short turnStillFromRight;
    short turnLeft;
    short turnRight;
    short unk_2e3a;
    // The bounds of the position, with flags.clampPos.
    float lowerBound[2];
    float upperBound[2];
    unsigned char unk_2e4c[0x4];
    // Effects that follow the enemy, as th06's.
    Effect *effects[25];
    int effectCount;
    // How far the effects following the enemy widen.
    float effectRadius;
    // When the life falls below one of lifeCallbackLife, the script
    // runs the matching lifeCallbackSub; -1 for none.
    int lifeCallbackLife[4];
    int lifeCallbackSub[4];
    // When callbackTimer reaches timerCallbackTime, the script runs
    // timerCallbackSub; -1 for none.
    int timerCallbackTime;
    int timerCallbackSub;
    int unk_2ee4;
    unsigned char unk_2ee8[0x68];
    Timer unk_2f50;
    Timer unk_2f5c;
    int unk_2f68;
    Timer unk_2f6c;
    EnemyBox unk_2f78[0x60];
    EnemyBox unk_39f8[0xc2];
    unsigned char unk_4f30[0x8];
    Timer unk_4f38;
    unsigned char unk_4f44[0x4];
};
ASSERT_SIZE(Enemy, 0x4f48);

// Keeps the effects following `enemy` on it, widening their circle up
// to effectRadius and turning them, as th06's; they are shown unless
// flags.unk_b is set.
void MoveEnemyEffects(Enemy *enemy);
// Lets go of the effects following `enemy`: each fades out, as th06's.
void ReleaseEnemyEffects(Enemy *enemy);

// The lower of angles `a` and `b` plus the fraction `t` of the shorter
// of their direct and wrapped-around (2 pi) distances. Not in th06.
float LerpAngle(float a, float b, float t);

// The enemies, at 0x9a9b00, th06's EnemyManager. Placeholder name; only
// what is known so far.
class Unknown9a9b00
{
public:
    // Zeroes everything and sets the template up, as th06's Initialize.
    void Initialize();
    // Spawns an enemy from the template at `pos`, starting script
    // subroutine `sub` and running its first frame, as th06's Spawn.
    // `life` and `score` replace the script's when not negative. Frees
    // the enemy again if its script ends at once. Returns the enemy,
    // or the one past the last if all are in use.
    Enemy *Spawn(short sub, D3DXVECTOR3 *pos, int life, char item, int score, BYTE mirrored);
    // Spawn without the mirroring, with the script's variables from
    // `vars` once the subroutine starts.
    Enemy *SpawnWithVars(short sub, D3DXVECTOR3 *pos, int life, char item, int score,
                      EnemyScriptVars *vars);
    // GuiImpl::StartMsgScript calls it with (0, 0) as a dialogue starts.
    void Func423090(int unk0, int unk1);
    // Whether one of the eight pointers at +0x954598 is set.
    int Func4232a0();

    // Draws the enemies of the lists at +0x954700, from `from` up to
    // `to`. Returns 1.
    static int Func422170(Unknown9a9b00 *mgr, int from, int to);
    // A draw callback: Func422170 for lists 0 and 1.
    static int OnDraw(Unknown9a9b00 *mgr);
    // The other draw callback: lists 2 and 3.
    static int OnDraw2(Unknown9a9b00 *mgr);
    // Loads the enemy sprites into slots 0xf and 0x10 (either path may
    // be NULL), picks dropCounter and dropIndex at random, ends the spell
    // card and hides AsciiText's unk_b7c sprites at -999. Returns 0, or
    // -1 if loading fails.
    static int AddedCallback(Unknown9a9b00 *mgr);

    // The stage's two enemy sprite files.
    char *anmPath;
    char *anmPath2;
    // The template new enemies start from, and the enemies.
    Enemy enemyTemplate;
    Enemy enemies[0x1e1];
    // The bosses, by Enemy::bossIndex.
    Enemy *bosses[8];
    // Counts the enemies dropping one of th06's g_EnemyItemDrops, and
    // the next of them, as th06's; start below 3 and below 8.
    WORD dropCounter;
    WORD dropIndex;
    unsigned char unk_9545bc[0xc];
    // th06's spellCardActive; other code reaches it as g_Unknown12fe0c8.
    int spellCardActive;
    unsigned char unk_9545cc[0x144];
};
ASSERT_SIZE(Unknown9a9b00, 0x954710);
extern Unknown9a9b00 g_Unknown9a9b00;

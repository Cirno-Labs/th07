// Enemies, as th06's enemy.h. Only the fields known so far.

#pragma once

#include <d3dx8math.h>

#include "bullet.h"
#include "chain.h"
#include "common.h"
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
    unsigned char contactDamage : 1;
    unsigned char unk_a : 1;
    // Hides the effects that follow the enemy.
    unsigned char hideFollowEffects : 1;
    unsigned char takesDamage : 1;
    unsigned char grazeable : 1;
    // A boss, as th06's isBoss.
    unsigned char isBoss : 1;
    // Keeps the position within lowerBound and upperBound, as th06's.
    unsigned char clampPos : 1;
    // What dying does, as th06's.
    unsigned char deathMode : 3;
    unsigned char onScreen : 1;
    // Turns the main sprite with the enemy's angle, as th06's.
    unsigned char rotateSprite : 1;
    unsigned char disableCallStack : 1;
    // CheckTimerCallback leaves the spell card and the bullets alone, as
    // th06's.
    unsigned char keepSpellCard : 1;
    unsigned char staysOffscreen : 1;
    // Set by WarpToBoss.
    unsigned char isWarpingToBoss : 1;
    // Cleared as a spell card starts.
    unsigned char hidden : 1;
    unsigned char unk_1a : 1;
    // While a bomb runs or the player is not in playerState 0, OnUpdate
    // skips the enemy's script, movement and damage, and holds
    // callbackTimer back.
    unsigned char haltedByBomb : 1;
};

// The variables of an enemy's script, by number from 10000: 10000 to
// 10003 ints, 10004 to 10011 floats, 10012 to 10015 ints, 10072 and
// 10073 floats, 10029 to 10032 ints, 10033 to 10036 floats.
struct EnemyScriptVars
{
    int ints[4];
    float floats[8];
    int ints2[4];
    float floats2[2];
    int ints3[4];
    float floats3[4];
};
ASSERT_SIZE(EnemyScriptVars, 0x68);

struct Enemy;
struct EnemyScriptInstr;

// A function of g_EnemyScriptFuncs: runs with the instruction that
// started it, or with NULL each frame from RunEnemy.
typedef void (*EnemyScriptFunc)(Enemy *enemy, EnemyScriptInstr *instr);
// Moves a variable along a curve: g_EnemyScriptInterps.
typedef void (*EnemyScriptInterp)(Enemy *enemy, EnemyScriptInstr *instr, float t);

// The functions script opcodes 121 and 122 run, th06's
// g_EnemyScriptFuncs.
extern EnemyScriptFunc g_EnemyScriptFuncs[24];
// How opcode 27 moves a variable, by its kind: ScriptMoveLinear for 0
// to 6, ScriptMoveHermite for 7.
extern EnemyScriptInterp g_EnemyScriptInterps[8];

// One of EnemyScriptState's eight interpolations of a variable. Laid
// out as an instruction (arguments 4 to 8 at +0x1c) for the
// EnemyScriptInterp it runs.
struct EnemyScriptLoop
{
    EnemyScriptInterp func;
    Timer timer;
    // Its length in frames, its kind (picks func) and easing.
    int duration;
    int type;
    int mode;
    // The start and end values, and the curve's tangents.
    float args[4];
    // The variable it sets.
    float target;
};
ASSERT_SIZE(EnemyScriptLoop, 0x30);

// Where one enemy is in its script (0x218 bytes), th06's grown. Its
// implicit constructor is 0x41f220.
struct EnemyScriptState
{
    void *code;
    Timer timer;
    // Runs each frame with funcInstr, from opcode 122.
    EnemyScriptFunc func;
    EnemyScriptInstr *funcInstr;
    EnemyScriptVars vars;
    // While above 0 the script waits, holding timer back.
    Timer waitTimer;
    EnemyScriptLoop loops[8];
    // The result of the last laser test, as th06's compareResult.
    int compareResult;
    // Set by the timed call of opcode 144: returning from it copies the
    // variables back into Enemy::unk_2ee8.
    int timedCallActive;
    short sub;
    short unk_216;
};
ASSERT_SIZE(EnemyScriptState, 0x218);

// One of Enemy's 0x122 boxes (0x1c bytes); Initialize hides the first
// 0x60 at x -999.
struct EnemyBox
{
    D3DXVECTOR3 pos;
    // The enemy's velocity there.
    D3DXVECTOR3 velocity;
    float angle;
};
ASSERT_SIZE(EnemyBox, 0x1c);

// One enemy (0x4f48 bytes).
struct Enemy
{
    // Moves by the velocity scaled by App::unk_178, mirrored with
    // flags.mirrored, as th06's; first keeps the last move in posDelta
    // and the position in prevPos. An inline too large to expand (in
    // enemy.cpp): OnUpdate calls the copy and reserves its temporaries.
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
    // quarter of GameManager::unk_961c above stats->unk_88, in tens)
    // and clears the bullets unless flags.keepSpellCard, clears the
    // other enemies and resets as CheckLifeCallback, as th06's. While
    // it is boss 0, shows the seconds left first. Returns 1 then, else
    // 0.
    int CheckTimerCallback();
    // After the enemy leaves the play area or its script ends: frees it
    // (flags.deathMode 0) or makes it unshootable, hides the boss bar
    // for bosses 0 to 3 and releases its effects, as th06's. A boss
    // also leaves EnemyManager::bosses. Marks the replay frame with
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
    int unusedInt2a80;
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
    // A second hitbox size, tested while its x is above 0.
    D3DXVECTOR3 extraHitboxSize;
    float angle;
    float angularVelocity;
    float orbitAngle;
    float orbitAngularVelocity;
    float speed;
    float acceleration;
    float orbitRadius;
    float orbitRadiusVelocity;
    // Where shots start, from the position.
    D3DXVECTOR3 shootOffset;
    // A timed move, as th06's: the distance, where it starts, and its
    // length in frames, counted down by moveTimer.
    D3DXVECTOR3 moveDelta;
    D3DXVECTOR3 moveStart;
    Timer moveTimer;
    int moveDuration;
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
    char deathEffect;
    BYTE deathEffectIndex;
    BYTE deathEffectVariant;
    // The enemy's slot in EnemyManager::bosses, while it is one.
    BYTE bossIndex;
    BYTE hitFlashTimer;
    unsigned char pad_2e19[0x3];
    Timer unusedTimer2e1c;
    EnemyFlags flags;
    WORD keepSpellCardDelay;
    // The turning sprite script last started, as th06's
    // flags.spriteDirection: 0 still, 1 left, 2 right, 0xff none yet.
    BYTE spriteDirection;
    // The draw list (EnemyManager::unk_954700) OnUpdate puts it in.
    BYTE drawListIndex;
    // The sprite scripts for turning, as th06's.
    short turnStill;
    short turnStillFromLeft;
    short turnStillFromRight;
    short turnLeft;
    short turnRight;
    short unusedShort2e3a;
    // The bounds of the position, with flags.clampPos.
    float lowerBound[2];
    float upperBound[2];
    int lastDamage;
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
    int periodicSubIndex;
    EnemyScriptVars periodicSubVars;
    Timer periodicSubInterval;
    Timer periodicSubTimer;
    int unusedInt2f68;
    Timer unusedTimer2f6c;
    EnemyBox trailBoxes[0x60];
    EnemyBox spriteStripVertices[0xc2];
    BYTE trailFlags;
    unsigned char pad_4f31[0x1];
    short trailSegmentCount;
    // The trail boxes (unk_2f78) below it touch the player, every sixth
    // from 1.
    short trailContactSegments;
    short trailSegmentStep;
    Timer invulnTimer;
    Enemy *nextInDrawList;
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

// The arguments of an enemy timeline entry, as th06's.
struct EnemyTimelineArgs
{
    D3DXVECTOR3 pos;
    int life;
    int item;
    int score;
};

// One entry of an enemy timeline, ended by a negative time, as th06's.
struct EnemyTimelineEntry
{
    // The frame it runs on.
    short time;
    short arg0;
    short opcode;
    // The size of the entry, arguments included.
    short size;
    EnemyTimelineArgs args;
};

// One of EnemyManager's sixteen unk_9545f4: th06's timelinePos and
// timelineTime, one per timeline.
struct EnemyTimeline
{
    // Runs the entries due this frame, then ticks the timer, as th06's
    // EnemyManager::RunTimeline. Spawning entries only spawn while no
    // boss fight runs. Opcode 9 waits while the dialogue holds the
    // timeline and 12 while boss arg0 is active. Without th06's rank
    // and progress counting.
    void RunTimeline();

    Timer timer;
    // The next entry.
    EnemyTimelineEntry *pos;
};

// The enemies, at 0x9a9b00, th06's EnemyManager. Placeholder name; only
// what is known so far.
class EnemyManager
{
public:
    // Constructs the members and initializes, as th06's.
    EnemyManager();

    // Zeroes everything and sets the template up, as th06's Initialize.
    void Initialize();
    // Spawns an enemy from the template at `pos`, starting script
    // subroutine `sub` and running its first frame, as th06's Spawn.
    // `life` and `score` replace the script's when not negative. Frees
    // the enemy again if its script ends at once. Returns the enemy,
    // or the one past the last if all are in use.
    Enemy *Spawn(int sub, D3DXVECTOR3 *pos, int life, int item, int score, BYTE mirrored);
    // Spawn without the mirroring, with the script's variables from
    // `vars` once the subroutine starts.
    Enemy *SpawnWithVars(int sub, D3DXVECTOR3 *pos, int life, int item, int score,
                      EnemyScriptVars *vars);
    // Kills every enemy but the bosses, as th06's spell card ends do.
    // Those with flags.unk_d, and their first unk_4f32 boxes (every
    // sixth, while unk_4f30 is set), each drop item 6 and show a bonus
    // that starts at 2000 and grows by 30 up to `maxBonus` (white
    // below it, yellow from it). Returns `start` plus the bonuses.
    // GuiImpl::StartMsgScript calls it with (0, 0) as a dialogue starts.
    int ClearEnemiesForBonus(int maxBonus, int start);
    // Whether any of bosses is set.
    int AnyBossActive();

    // Draws the enemies of the lists at +0x954700, from `from` up to
    // `to`, as th06's OnDraw: sprites[0], the main sprite (unless
    // unk_4f30 bit 4 or flags.unk_1a) and sprites[1], each at the
    // enemy's position plus posOffset. With unk_4f30 set, also its trail
    // along unk_2f78 (every unk_4f36th of unk_4f32 boxes): copies of the
    // main sprite on the boxes not at x below -990, or with bit 3 a
    // vertex strip in unk_39f8 up to the first such box; bit 1 narrows
    // it and bit 2 fades it towards the end. Returns 1.
    static int DrawEnemies(EnemyManager *mgr, int from, int to);
    // A draw callback: DrawEnemies for lists 0 and 1.
    static int OnDraw(EnemyManager *mgr);
    // The other draw callback: lists 2 and 3.
    static int OnDraw2(EnemyManager *mgr);
    // Loads the enemy sprites into slots 0xf and 0x10 (either path may
    // be NULL), picks dropCounter and dropIndex at random, ends the spell
    // card and hides AsciiText's unk_b7c sprites at -999. Returns 0, or
    // -1 if loading fails.
    static int AddedCallback(EnemyManager *mgr);
    // Frees the enemy sprites and hides AsciiText's unk_b7c sprites at
    // -999 again. Returns 0.
    static int DeletedCallback(EnemyManager *mgr);
    // Initializes, remembers the stage's two enemy sprite files and adds
    // the update element (priority 10) and the draw elements (5 and 7).
    // Returns 0, or -1 if adding fails.
    static int RegisterChain(char *anmPath, char *anmPath2);
    // Removes the three chain elements.
    static void RemoveChain();
    // The update callback, th06's OnUpdate, reworked. Outside dialogue,
    // adds 100 to the subrank every 2400 - 240 * lives frames and counts
    // GameManager::progressFrames; runs the timelines; then for each
    // active enemy: its script (again after a life or timer callback),
    // movement, trail, leaving the play area, contact with the player,
    // damage (bonus points for it; cut by stage for shot type 0, by
    // spell cards and by unk_4f38), the homing targets, death (with th06's
    // death modes and drops, and a boss clearing the bullets and enemies
    // for a bonus), the hit flash in color2, the boss bar and markers,
    // and the draw lists. Returns 4 when a checksum check every 200
    // frames fails, else 1.
    static int OnUpdate(EnemyManager *mgr);

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
    // The active enemies, counted by OnUpdate.
    int enemyCount;
    unsigned char pad_9545c0[0x8];
    // th06's spellCardActive; other code reaches it as g_SpellCardActive.
    int spellCardActive;
    unsigned char pad_9545cc[0x10];
    // While a spell card runs, a bomb's damage counts (divided by 2.5)
    // only while it is set, as th06's spellCardBombed.
    int spellCardBombed;
    Timer spellCardBonusTimer;
    int unusedInt9545ec;
    // Set by script opcode 147.
    int scriptArg147;
    EnemyTimeline timelines[16];
    Timer frameTimer;
    Enemy *drawListHeads[4];
};
ASSERT_SIZE(EnemyManager, 0x954710);
extern EnemyManager g_EnemyManager;
// The items the enemies without one of their own drop in turn, every
// third, as th06's.
extern BYTE g_EnemyItemDrops[0x20];
// Its statically allocated chain elements.
extern ChainElem g_EnemyManagerCalcChain;
extern ChainElem g_EnemyManagerDrawChain;
extern ChainElem g_EnemyManagerDrawChain2;

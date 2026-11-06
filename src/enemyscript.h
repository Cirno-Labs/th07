// The enemy scripts (ECL), as th06's.
// Included by unsorted.h where these declarations were; it relies on what
// unsorted.h declares before it.

#pragma once

// One argument of a script instruction.
union EnemyScriptArg
{
    int i;
    float f;
};

// One instruction of an enemy script, th06's with a mask of the
// arguments that are variables.
struct EnemyScriptInstr
{
    // The frame it runs on.
    int time;
    short opcode;
    // The size of the instruction, arguments included.
    short size;
    BYTE pad_8;
    BYTE difficultyMask;
    // Bit n: argument n names a variable.
    WORD paramMask;
    EnemyScriptArg args[1];
};

// The head of an enemy script file: the subroutine count, then the
// offsets of sixteen timelines, then those of the subroutines.
struct EnemyScriptHeader
{
    short subCount;
    short timelineCount;
    void *timelines[16];
};

// The arguments of the spawning opcodes (92 and 93), as th06's.
struct EnemySpawnArgs
{
    int sub;
    D3DXVECTOR3 pos;
    int life;
    int item;
    int score;
};

struct EnemyTimelineEntry;

// th06's ScriptStarPattern keeps its star here; in th07 no code reads
// or writes the 0x160 bytes. It ends with eight vectors, whose (empty)
// constructors the original's static initializer runs.
struct EnemyScriptsStar
{
    unsigned char pad_0[0x100];
    D3DXVECTOR3 ends[8];
};

// The enemy scripts (ECL), at 0x1347938, as th06's EnemyScripts.
class EnemyScripts
{
public:
    // Loads the script file at `path` and turns its offsets into
    // pointers, as th06's Load. Returns 0, or -1 if loading fails.
    int Load(char *path);
    // Starts subroutine `sub` in `ctx`, from its first instruction with
    // both timers at 0, as th06's StartSub. Returns 0.
    int StartSub(EnemyScriptState *ctx, short sub);
    // Runs `enemy`'s script for a frame, as th06's RunEnemy, grown to
    // 161 opcodes: first a pending interrupt, or the timed call that
    // opcode 144 set up; then the instructions due this frame unless
    // the wait of opcode 45 holds them; then the movement (modes 1 to 3,
    // with six easings), the shooting, the turning sprite scripts, the
    // function of opcode 122, the interpolations of opcode 27, and boss
    // 0's spell card bonus. Returns -1 once the script ends, else 0.
    int RunEnemy(Enemy *enemy);
    // Frees the script file, as th06's Release.
    void Release();
    // Timeline `i` of the file. Inline: EnemyManager::OnUpdate keeps
    // what it returns in a temporary.
    EnemyTimelineEntry *GetTimeline(int i)
    {
        return (EnemyTimelineEntry *)this->file->timelines[i];
    }

    EnemyScriptHeader *file;
    void **subs;
    EnemyScriptsStar star;
    // The scripts' shared variables, 10037 to 10040 and 10041 to 10044.
    int globalInts[4];
    float globalFloats[4];
};
extern EnemyScripts g_EnemyScripts;

// The value of script operand `id` for `enemy`: a variable (floats
// truncated) or another value the scripts read by number from 10000;
// any other number is its own value.
int GetIntOperand(Enemy *enemy, int id);
// The int variable script operand `operand` names, as th06's
// ResolveOperand, or `operand` itself if it names none or if `bit` is
// not negative and clear in `mask`.
int *ResolveOperand(Enemy *enemy, int *operand, WORD mask, int bit);
// GetIntOperand for float operands: the value of script operand `id`
// for `enemy` as a float, with random floats for 10055, 10056 and
// 10060.
float GetFloatOperand(Enemy *enemy, float id);
// ResolveOperand for float variables.
float *ResolveFloatOperand(Enemy *enemy, float *operand, WORD mask, int bit);
// Starts a timed move at an angle (argument 2, normalized), as th06's
// ScriptMoveAngle: moveDelta is argument 3 times argument 0, the
// duration; argument 1 picks the easing. Mirrored with flags.mirrored.
void ScriptMoveAngle(Enemy *enemy, EnemyScriptInstr *instr);
// Starts a timed move to arguments 2 to 4, as th06's ScriptMovePos,
// with ScriptMoveAngle's duration, easing and mirroring; stops the
// velocity.
void ScriptMovePos(Enemy *enemy, EnemyScriptInstr *instr);
// Sets the float variable argument 8 names to argument 4 eased
// linearly towards argument 5 by `t`, 0 to 1.
void ScriptMoveLinear(Enemy *enemy, EnemyScriptInstr *instr, float t);
// ScriptMoveLinear along a cubic Hermite curve from argument 4 to argument
// 5, with tangents arguments 6 and 7.
void ScriptMoveHermite(Enemy *enemy, EnemyScriptInstr *instr, float t);
// Starts a spell card, as th06's RunEnemy did: shows the name
// (arguments 1 on, xor 0xaa) with sprite argument 0, clears the
// bullets, starts the stage's spell card background, sets the bonus
// from g_SpellCardBonuses[index] (the high word of argument 0), resets
// the rank values, and starts effect 0x19 in reserved slot 1 as
// effects[24], with its scale and then 0.125 twice in scaleTween and
// timerCallbackTime in its interpFrame[4] timer. Unless bit 3 of
// g_GameManager.stateFlags is set, counts the attempt in the card's
// Catk (by shotIndex and in all), first clearing its counts if their
// checksum (checksum, with the name's) fails.
void StartSpellCard(Enemy *enemy, EnemyScriptInstr *instr);
// Ends the spell card, as th06's RunEnemy did: hides the name; if it
// was active (1), clears the bullets and the other enemies for bonuses
// and, if it was captured, scores its bonus and, unless bit 3 of
// g_GameManager.stateFlags is set, keeps the best bonus and counts the
// capture in its Catk as StartSpellCard does the attempt. Then releases
// the bosses' effects[24] and plays sound 0xf. Always resets the
// stage's spellCardState.
void EndSpellCard(Enemy *enemy, EnemyScriptInstr *instr);

// The functions of the table at 0x49f158, th06's g_EnemyScriptFuncs,
// in its order.

// Puts `enemy` where boss argument 1 is, with its velocity and angle,
// and sets flags.isWarpingToBoss.
void WarpToBoss(Enemy *enemy, EnemyScriptInstr *instr);
// Shakes and flashes the screen, and starts the flying bullets with
// sprites that no call has turned yet (only color 8 for argument 1 = 1,
// color 4 for 2) turning by a small random angle a frame, its sign by
// color, slowed to 0.3.
void ShakeAndSpinBullets(Enemy *enemy, EnemyScriptInstr *instr);
// Each flying color 2 bullet with a sprite within a range of the enemy
// (128, 192, 256 or 999 by argument 1; 0 also shakes and flashes the
// screen) fires two color 6 bullets and is removed (FreeBullet).
void SpawnFansFromNearbyBullets(Enemy *enemy, EnemyScriptInstr *instr);
// Does nothing.
void NoOp(Enemy *enemy, EnemyScriptInstr *instr);
// Finds the first flying bullet with a sprite at least 60 high, keeps
// its position in floats[0] and [1] (floats[0] is -999 if none),
// starts effect 2 there and removes the bullet.
void ReplaceTallBulletWithEffect(Enemy *enemy, EnemyScriptInstr *instr);
// Copies boss 0's position into moveStart, and its orbitRadius and
// orbitAngularVelocity; then switches on its ints2[0] with no cases.
void CopyBossMoveStart(Enemy *enemy, EnemyScriptInstr *instr);
// Each flying bullet with a sprite of color 6, 15 or 2 (argument 1 = 0,
// 1 or 2) fires three fans back the way it came and is removed.
void FireBackwardFans(Enemy *enemy, EnemyScriptInstr *instr);
// For the even or odd lasers by callbackTimer, those not ending: the
// flying bullets with sprites inside one count scriptFuncState down; at 0 they
// slow by 0.1 (above 0.5), turn square to the laser on the side their
// velocity points to, take typeSprites[5]'s sprites, and wait 10 more.
void UpdateLaserEdgeBullets(Enemy *enemy, EnemyScriptInstr *instr);
// For every third laser by callbackTimer, those not ending: the flying
// bullets with sprites inside one, 1.5 times its width, that it has not
// deflected yet (scriptFuncState neither li + 1 nor negative) slow to a random
// 0.7 to 1 (0.8 to 1.2 from difficulty 2) of their speed, turn square
// to the laser on the side their velocity points to, and take
// typeSprites[5]'s sprites; scriptFuncState becomes -1, or li + 1 from
// difficulty 2.
void DeflectBulletsOffLasers(Enemy *enemy, EnemyScriptInstr *instr);
// Shakes the screen for 0x50 frames and starts the type 30 effects
// falling (StartPetalsFalling).
void ShakeAndStartFallingPetals(Enemy *enemy, EnemyScriptInstr *instr);
// Slows the game to 1 / argument 1 (App::gameSpeed), sends interrupt 2 to
// the stage's first two dialogueSprites sprites, and scales the velocity of
// every bullet by it, keeping each sprite in baseSpriteIndex and
// showing sprites 0x260 to 0x26f as 0x26f.
void StartSlowdown(Enemy *enemy, EnemyScriptInstr *instr);
// Undoes StartSlowdown: scales the bullets' velocities back up, restores
// sprites 0x260 to 0x26f from baseSpriteIndex, sets bit 5 of
// App::stateFlags if 1 / argument 1 is below 1, and puts the game speed
// and the stage sprites' interrupts back to 1.
void EndSlowdown(Enemy *enemy, EnemyScriptInstr *instr);
// Flashes the screen 0x50cfcfff for 8 frames. Each bullet with a sprite
// over 48 high within 64 (difficulty 0 and 1) or 48 of the enemy in y
// bursts into 10, 18, 22 or 25 bullets (by difficulty) around it at
// speed 0.1 with a flag 0x20 command, of three random sprite and color
// pairs, at a random angle in a 3 pi / 2 arc from -pi / 2 (argument 1
// = 0) or from pi / 4, every other one with flag 2, and is removed.
void BurstBulletsByDifficulty(Enemy *enemy, EnemyScriptInstr *instr);
// Flashes the screen 0x50cfcfff for 0x10 frames; each bullet turned
// once (scriptFuncState 1) gets a flag 0x10 command 0 for 0x5a frames with its
// angle to the player, clears command 1's flag and is marked 2.
void AimTurnedBulletsAtPlayer(Enemy *enemy, EnemyScriptInstr *instr);
// Flashes the screen 0xd0cfcfff for argument 1 frames.
void FlashScreen(Enemy *enemy, EnemyScriptInstr *instr);
// Flashes the screen 0x50cfcfff for 8 frames. Each bullet with a sprite
// over 48 high within 128 (difficulty 2) or 180 of the enemy in y
// bursts into 15 bullets around it at speed 0.1 with a flag 0x20
// command, of three random sprite and color pairs, at a random angle in
// a 3 pi / 2 arc from -pi / 2 (argument 1 = 0) or from pi / 4, and is
// removed.
void BurstBulletsFixedCount(Enemy *enemy, EnemyScriptInstr *instr);
// Fades the music out over 3 seconds.
void ScriptFadeOutMusic(Enemy *enemy, EnemyScriptInstr *instr);
// Plays music slot 2 (App::PlayMidi) and, if that returns nonzero,
// bgm/th07_13b.mid.
void ScriptPlayMusic(Enemy *enemy, EnemyScriptInstr *instr);
// Each bullet not yet turned (scriptFuncState 0) with y between the enemy's
// and 352 and x within 16 of it starts turning by pi / 60 a frame
// (-pi / 60 for even indices) for 0xa0 frames, slowing by its speed /
// 180, and is marked turned (1).
void TurnNearbyBullets(Enemy *enemy, EnemyScriptInstr *instr);
// Each bullet not yet turned (scriptFuncState 0) with a sprite from 0x278 to
// 0x27f fires five color 6 bullets (aim mode 1, pi / 8 apart) back the
// way it came, at speed floats[1].
void FireBackFromUnturned(Enemy *enemy, EnemyScriptInstr *instr);
// Flashes the screen 0x80cfcfff for 0xc frames. Each bullet not yet
// turned (scriptFuncState 0) with sprite 0x27c is replaced by an enemy running
// the next sub (life 1, item -2, score 10) with a copy of this enemy's
// variables, floats[0] the bullet's angle and floats[7] an angle from
// -pi up in steps of pi / 4; bullets with sprites 0x278 to 0x27f are
// removed.
void ReplaceBulletsWithEnemies(Enemy *enemy, EnemyScriptInstr *instr);
// Counts in ints[0] the bullets with sprite 0x27c not yet turned
// (scriptFuncState 0).
void CountUnturnedBullets(Enemy *enemy, EnemyScriptInstr *instr);
// Unless callbackTimer is a multiple of 3: each bullet without behavior
// 0x40 above y 320 with a sprite over 60 high fires bullets at a random
// angle (aim mode 3, sound 0x19), color 6 from color 1 bullets, 2 from
// others; when callbackTimer is even two of sprite 3 at 0.8, when odd
// one of sprite 1 at 1.2 with a flag 0x80 command (0x3c frames, 3.1).
void FireAimedBulletPattern1(Enemy *enemy, EnemyScriptInstr *instr);
// Unless callbackTimer is 2 modulo 3: each bullet without behavior 0x40
// above y 320 with a sprite over 60 high fires one bullet at a random
// angle (aim mode 3, sound 0x19), color 10 from color 2 bullets, 13 from
// others; sprite 3 at 0.8 on a multiple of 3, otherwise sprite 1 at
// 1.2 with a flag 0x80 command (0x28 frames, 2.9).
void FireAimedBulletPattern2(Enemy *enemy, EnemyScriptInstr *instr);
// Whether `point`, turned about `origin` by the angle with sine
// `sinAngle` and cosine `cosAngle`, lies in the `size` box centred on
// `center` (x and y only).
int PointInRotatedBox(D3DXVECTOR3 *point, D3DXVECTOR3 *center, D3DXVECTOR3 *size, D3DXVECTOR3 *origin,
               float sinAngle, float cosAngle);

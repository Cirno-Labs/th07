// The file with EnemyScripts, th06's EnemyScripts, from 0x40e420, as
// th06's enemyscript.cpp.

#include <math.h>
#include <string.h>

#include "ascii.h"
#include "bullet.h"
#include "effect.h"
#include "enemy.h"
#include "item.h"
#include "player.h"
#include "screeneffect.h"
#include "unsorted.h"

// The game's data, as the original defines it here. The DLL build uses
// the original's copies instead.
#ifdef TH07_EXE
EnemyScriptFunc g_EnemyScriptFuncs[24] = {
    WarpToBoss,
    ShakeAndSpinBullets,
    SpawnFansFromNearbyBullets,
    NoOp,
    ReplaceTallBulletWithEffect,
    CopyBossMoveStart,
    FireBackwardFans,
    UpdateLaserEdgeBullets,
    DeflectBulletsOffLasers,
    ShakeAndStartFallingPetals,
    StartSlowdown,
    EndSlowdown,
    BurstBulletsByDifficulty,
    TurnNearbyBullets,
    AimTurnedBulletsAtPlayer,
    FlashScreen,
    FireBackFromUnturned,
    ReplaceBulletsWithEnemies,
    CountUnturnedBullets,
    ScriptFadeOutMusic,
    ScriptPlayMusic,
    BurstBulletsFixedCount,
    FireAimedBulletPattern1,
    FireAimedBulletPattern2,
};
int g_SpellCardBonuses[141] = {
    2000000, 2000000, 2200000, 2200000, 2400000, 2400000, 2400000, 2400000,
    2400000, 2400000, 2600000, 2600000, 2600000, 2600000, 2600000, 2600000,
    2600000, 2600000, 2600000, 2600000, 2600000, 2600000, 2600000, 2600000,
    2600000, 2600000, 3000000, 3000000, 3000000, 3000000, 3000000, 3000000,
    3000000, 3000000, 3000000, 3000000, 3000000, 3000000, 3000000, 3000000,
    3000000, 3000000, 3000000, 3000000, 3500000, 3500000, 3500000, 3500000,
    3500000, 3500000, 3500000, 3500000, 3500000, 3500000, 3500000, 3500000,
    3500000, 3500000, 3500000, 3500000, 3500000, 3500000, 3500000, 3500000,
    3500000, 3500000, 3500000, 3500000, 4000000, 4000000, 4000000, 4000000,
    4000000, 4000000, 4000000, 4000000, 4000000, 4000000, 4000000, 4000000,
    4000000, 4000000, 4000000, 4000000, 4000000, 4000000, 4000000, 4000000,
    5000000, 5000000, 5000000, 5000000, 5000000, 5000000, 5000000, 5000000,
    5000000, 5000000, 5000000, 5000000, 5000000, 5000000, 5000000, 5000000,
    5000000, 5000000, 5000000, 5000000, 5000000, 5000000, 5000000, 5000000,
    3000000, 3000000, 3000000, 3000000, 6000000, 6000000, 7000000, 7000000,
    7000000, 7000000, 7000000, 7000000, 7000000, 7000000, 4000000, 7000000,
    7000000, 7000000, 8000000, 8000000, 8000000, 8000000, 8000000, 8000000,
    8000000, 8000000, 4000000, 8000000, 4000000,
};
EnemyScriptInterp g_EnemyScriptInterps[8] = {
    ScriptMoveLinear,
    ScriptMoveLinear,
    ScriptMoveLinear,
    ScriptMoveLinear,
    ScriptMoveLinear,
    ScriptMoveLinear,
    ScriptMoveLinear,
    ScriptMoveHermite,
};
// Constructed, never used.
ChainElem g_ChainElem1347ac0;
EnemyScripts g_EnemyScripts;
#endif // TH07_EXE

// Argument `n` of `instr`, or the value of the variable it names.
#define INT_ARG(n) \
    ((instr->paramMask & (1 << (n))) ? GetIntOperand(enemy, instr->args[n].i) : instr->args[n].i)
#define FLOAT_ARG(n) \
    ((instr->paramMask & (1 << (n))) ? GetFloatOperand(enemy, instr->args[n].f) : instr->args[n].f)

int EnemyScripts::Load(char *path)
{
    int i;

    this->file = (EnemyScriptHeader *)LoadFileData(path, 0);
    if (this->file == NULL)
    {
        LogPrintf(&g_Log, "敵データの読み込みに失敗しました、データが壊れてるか失われています\r\n");
        return -1;
    }
    for (i = 0; i < 16; i++)
    {
        this->file->timelines[i] = (char *)this->file->timelines[i] + (int)this->file;
    }
    this->subs = (void **)((char *)this->file + sizeof(EnemyScriptHeader));
    for (i = 0; i < this->file->subCount; i++)
    {
        this->subs[i] = (char *)this->subs[i] + (int)this->file;
    }
    return 0;
}

void EnemyScripts::Release()
{
    if (this->file != NULL)
    {
        MemFree(this->file);
    }
    this->file = NULL;
}

int EnemyScripts::StartSub(EnemyScriptState *ctx, short sub)
{
    ctx->code = this->subs[sub];
    ctx->timer.Set(0);
    ctx->waitTimer.Set(0);
    ctx->sub = sub;
    return 0;
}

int GetIntOperand(Enemy *enemy, int id)
{
    switch (id)
    {
    case 10000:
        return enemy->ctx.vars.ints[0];
    case 10001:
        return enemy->ctx.vars.ints[1];
    case 10002:
        return enemy->ctx.vars.ints[2];
    case 10003:
        return enemy->ctx.vars.ints[3];
    case 10029:
        return enemy->ctx.vars.ints3[0];
    case 10030:
        return enemy->ctx.vars.ints3[1];
    case 10031:
        return enemy->ctx.vars.ints3[2];
    case 10032:
        return enemy->ctx.vars.ints3[3];
    case 10012:
        return enemy->ctx.vars.ints2[0];
    case 10013:
        return enemy->ctx.vars.ints2[1];
    case 10014:
        return enemy->ctx.vars.ints2[2];
    case 10015:
        return enemy->ctx.vars.ints2[3];
    case 10016:
        return g_GameManager.difficulty;
    case 10017:
        return g_GameManager.rank;
    case 10025:
        return enemy->callbackTimer.current;
    case 10027:
        return enemy->life;
    case 10028:
        return g_GameManager.shotIndex;
    case 10072:
        return enemy->ctx.vars.floats2[0];
    case 10073:
        return enemy->ctx.vars.floats2[1];
    case 10004:
        return enemy->ctx.vars.floats[0];
    case 10005:
        return enemy->ctx.vars.floats[1];
    case 10006:
        return enemy->ctx.vars.floats[2];
    case 10007:
        return enemy->ctx.vars.floats[3];
    case 10008:
        return enemy->ctx.vars.floats[4];
    case 10009:
        return enemy->ctx.vars.floats[5];
    case 10010:
        return enemy->ctx.vars.floats[6];
    case 10011:
        return enemy->ctx.vars.floats[7];
    case 10033:
        return enemy->ctx.vars.floats3[0];
    case 10034:
        return enemy->ctx.vars.floats3[1];
    case 10035:
        return enemy->ctx.vars.floats3[2];
    case 10036:
        return enemy->ctx.vars.floats3[3];
    case 10037:
        return g_EnemyScripts.globalInts[0];
    case 10038:
        return g_EnemyScripts.globalInts[1];
    case 10039:
        return g_EnemyScripts.globalInts[2];
    case 10040:
        return g_EnemyScripts.globalInts[3];
    case 10041:
        return g_EnemyScripts.globalFloats[0];
    case 10042:
        return g_EnemyScripts.globalFloats[1];
    case 10043:
        return g_EnemyScripts.globalFloats[2];
    case 10044:
        return g_EnemyScripts.globalFloats[3];
    case 10018:
        return enemy->pos.x;
    case 10019:
        return enemy->pos.y;
    case 10020:
        return enemy->pos.z;
    case 10021:
        return g_Player.position.x;
    case 10022:
        return g_Player.position.y;
    case 10023:
        return g_Player.position.z;
    case 10050:
        return enemy->moveStart.x;
    case 10051:
        return enemy->moveStart.y;
    case 10052:
        return enemy->moveStart.z;
    case 10063:
        return enemy->posDelta.x;
    case 10064:
        return enemy->posDelta.y;
    case 10065:
        return enemy->posDelta.z;
    case 10066:
        return enemy->lifeCallbackLife[0];
    case 10067:
        return enemy->lifeCallbackLife[1];
    case 10068:
        return enemy->lifeCallbackLife[2];
    case 10069:
        return enemy->lifeCallbackLife[3];
    case 10045:
        return enemy->angle;
    case 10046:
        return enemy->angularVelocity;
    case 10047:
        return enemy->speed;
    case 10048:
        return enemy->acceleration;
    case 10049:
        return enemy->orbitRadius;
    case 10053:
        return enemy->orbitAngle;
    case 10054:
        return enemy->orbitAngularVelocity;
    case 10055:
        return g_Rng.GetRandomU32();
    case 10056:
        return g_Rng.GetRandomU32InRange(enemy->ctx.vars.ints3[0]) + enemy->ctx.vars.ints3[1];
    case 10061:
        return enemy->lastDamage;
    case 10062:
        return enemy->bossIndex;
    case 10070:
        return enemy->item;
    case 10071:
        return enemy->score;
    case 10024:
        return g_Player.AngleFrom(&enemy->pos);
    case 10026:
        return D3DXVec3Length(&(g_Player.position - enemy->pos));
    }
    return id;
}

int *ResolveOperand(Enemy *enemy, int *operand, WORD mask, int bit)
{
    if (bit >= 0 && (mask & (1 << bit)) == 0)
    {
        return operand;
    }
    switch (*operand)
    {
    case 10000:
        return &enemy->ctx.vars.ints[0];
    case 10001:
        return &enemy->ctx.vars.ints[1];
    case 10002:
        return &enemy->ctx.vars.ints[2];
    case 10003:
        return &enemy->ctx.vars.ints[3];
    case 10029:
        return &enemy->ctx.vars.ints3[0];
    case 10030:
        return &enemy->ctx.vars.ints3[1];
    case 10031:
        return &enemy->ctx.vars.ints3[2];
    case 10032:
        return &enemy->ctx.vars.ints3[3];
    case 10012:
        return &enemy->ctx.vars.ints2[0];
    case 10013:
        return &enemy->ctx.vars.ints2[1];
    case 10014:
        return &enemy->ctx.vars.ints2[2];
    case 10015:
        return &enemy->ctx.vars.ints2[3];
    case 10016:
        return &g_GameManager.difficulty;
    case 10017:
        return &g_GameManager.rank;
    case 10025:
        return &enemy->callbackTimer.current;
    case 10027:
        return &enemy->life;
    case 10070:
        return &enemy->item;
    case 10071:
        return &enemy->score;
    case 10037:
        return &g_EnemyScripts.globalInts[0];
    case 10038:
        return &g_EnemyScripts.globalInts[1];
    case 10039:
        return &g_EnemyScripts.globalInts[2];
    case 10040:
        return &g_EnemyScripts.globalInts[3];
    }
    return operand;
}

float GetFloatOperand(Enemy *enemy, float id)
{
    switch ((int)id)
    {
    case 10000:
        return enemy->ctx.vars.ints[0];
    case 10001:
        return enemy->ctx.vars.ints[1];
    case 10002:
        return enemy->ctx.vars.ints[2];
    case 10003:
        return enemy->ctx.vars.ints[3];
    case 10029:
        return enemy->ctx.vars.ints3[0];
    case 10030:
        return enemy->ctx.vars.ints3[1];
    case 10031:
        return enemy->ctx.vars.ints3[2];
    case 10032:
        return enemy->ctx.vars.ints3[3];
    case 10012:
        return enemy->ctx.vars.ints2[0];
    case 10013:
        return enemy->ctx.vars.ints2[1];
    case 10014:
        return enemy->ctx.vars.ints2[2];
    case 10015:
        return enemy->ctx.vars.ints2[3];
    case 10016:
        return g_GameManager.difficulty;
    case 10017:
        return g_GameManager.rank;
    case 10025:
        return enemy->callbackTimer.current;
    case 10027:
        return enemy->life;
    case 10028:
        return g_GameManager.shotIndex;
    case 10070:
        return enemy->item;
    case 10071:
        return enemy->score;
    case 10037:
        return g_EnemyScripts.globalInts[0];
    case 10038:
        return g_EnemyScripts.globalInts[1];
    case 10039:
        return g_EnemyScripts.globalInts[2];
    case 10040:
        return g_EnemyScripts.globalInts[3];
    case 10041:
        return g_EnemyScripts.globalFloats[0];
    case 10042:
        return g_EnemyScripts.globalFloats[1];
    case 10043:
        return g_EnemyScripts.globalFloats[2];
    case 10044:
        return g_EnemyScripts.globalFloats[3];
    case 10004:
        return enemy->ctx.vars.floats[0];
    case 10005:
        return enemy->ctx.vars.floats[1];
    case 10006:
        return enemy->ctx.vars.floats[2];
    case 10007:
        return enemy->ctx.vars.floats[3];
    case 10008:
        return enemy->ctx.vars.floats[4];
    case 10009:
        return enemy->ctx.vars.floats[5];
    case 10010:
        return enemy->ctx.vars.floats[6];
    case 10011:
        return enemy->ctx.vars.floats[7];
    case 10033:
        return enemy->ctx.vars.floats3[0];
    case 10034:
        return enemy->ctx.vars.floats3[1];
    case 10035:
        return enemy->ctx.vars.floats3[2];
    case 10036:
        return enemy->ctx.vars.floats3[3];
    case 10018:
        return enemy->pos.x;
    case 10019:
        return enemy->pos.y;
    case 10020:
        return enemy->pos.z;
    case 10021:
        return g_Player.position.x;
    case 10022:
        return g_Player.position.y;
    case 10023:
        return g_Player.position.z;
    case 10072:
        return enemy->ctx.vars.floats2[0];
    case 10073:
        return enemy->ctx.vars.floats2[1];
    case 10050:
        return enemy->moveStart.x;
    case 10051:
        return enemy->moveStart.y;
    case 10052:
        return enemy->moveStart.z;
    case 10057:
        return enemy->moveDelta.x;
    case 10058:
        return enemy->moveDelta.y;
    case 10059:
        return enemy->moveDelta.z;
    case 10063:
        return enemy->posDelta.x;
    case 10064:
        return enemy->posDelta.y;
    case 10065:
        return enemy->posDelta.z;
    case 10066:
        return enemy->lifeCallbackLife[0];
    case 10067:
        return enemy->lifeCallbackLife[1];
    case 10068:
        return enemy->lifeCallbackLife[2];
    case 10069:
        return enemy->lifeCallbackLife[3];
    case 10024:
        return g_Player.AngleFrom(&enemy->pos);
    case 10045:
        return enemy->angle;
    case 10046:
        return enemy->angularVelocity;
    case 10047:
        return enemy->speed;
    case 10048:
        return enemy->acceleration;
    case 10049:
        return enemy->orbitRadius;
    case 10053:
        return enemy->orbitAngle;
    case 10054:
        return enemy->orbitAngularVelocity;
    case 10055:
        return g_Rng.GetRandomF32ZeroToOne();
    case 10056:
        return g_Rng.GetRandomF32InRange(enemy->ctx.vars.floats3[0]) + enemy->ctx.vars.floats3[1];
    case 10060:
        return g_Rng.GetRandomF32ZeroToOne() * 6.2831855f - 3.1415927f;
    case 10062:
        return enemy->bossIndex;
    case 10061:
        return enemy->lastDamage;
    case 10026:
        return D3DXVec3Length(&(g_Player.position - enemy->pos));
    }
    return id;
}

float *ResolveFloatOperand(Enemy *enemy, float *operand, WORD mask, int bit)
{
    if (bit >= 0 && (mask & (1 << bit)) == 0)
    {
        return operand;
    }
    switch ((int)*operand)
    {
    case 10004:
        return &enemy->ctx.vars.floats[0];
    case 10005:
        return &enemy->ctx.vars.floats[1];
    case 10006:
        return &enemy->ctx.vars.floats[2];
    case 10007:
        return &enemy->ctx.vars.floats[3];
    case 10008:
        return &enemy->ctx.vars.floats[4];
    case 10009:
        return &enemy->ctx.vars.floats[5];
    case 10010:
        return &enemy->ctx.vars.floats[6];
    case 10011:
        return &enemy->ctx.vars.floats[7];
    case 10033:
        return &enemy->ctx.vars.floats3[0];
    case 10034:
        return &enemy->ctx.vars.floats3[1];
    case 10035:
        return &enemy->ctx.vars.floats3[2];
    case 10036:
        return &enemy->ctx.vars.floats3[3];
    case 10018:
        return &enemy->pos.x;
    case 10019:
        return &enemy->pos.y;
    case 10020:
        return &enemy->pos.z;
    case 10021:
        return &g_Player.position.x;
    case 10022:
        return &g_Player.position.y;
    case 10023:
        return &g_Player.position.z;
    case 10072:
        return &enemy->ctx.vars.floats2[0];
    case 10073:
        return &enemy->ctx.vars.floats2[1];
    case 10041:
        return &g_EnemyScripts.globalFloats[0];
    case 10042:
        return &g_EnemyScripts.globalFloats[1];
    case 10043:
        return &g_EnemyScripts.globalFloats[2];
    case 10044:
        return &g_EnemyScripts.globalFloats[3];
    case 10050:
        return &enemy->moveStart.x;
    case 10051:
        return &enemy->moveStart.y;
    case 10052:
        return &enemy->moveStart.z;
    case 10057:
        return &enemy->moveDelta.x;
    case 10058:
        return &enemy->moveDelta.y;
    case 10059:
        return &enemy->moveDelta.z;
    case 10045:
        return &enemy->angle;
    case 10046:
        return &enemy->angularVelocity;
    case 10047:
        return &enemy->speed;
    case 10048:
        return &enemy->acceleration;
    case 10049:
        return &enemy->orbitRadius;
    case 10053:
        return &enemy->orbitAngle;
    case 10054:
        return &enemy->orbitAngularVelocity;
    }
    return operand;
}

void ScriptMoveAngle(Enemy *enemy, EnemyScriptInstr *instr)
{
    float angle;
    int duration;

    angle = AddNormalizeAngle(FLOAT_ARG(2), 0.0f);
    enemy->moveDelta.x = cosf(angle) * FLOAT_ARG(3) * INT_ARG(0);
    enemy->moveDelta.y = sinf(angle) * FLOAT_ARG(3) * INT_ARG(0);
    enemy->moveDelta.z = 0.0f;
    enemy->moveStart = enemy->pos;
    if (instr->paramMask & 1)
    {
        duration = GetIntOperand(enemy, instr->args[0].i);
    }
    else
    {
        duration = instr->args[0].i;
    }
    enemy->moveDuration = duration;
    enemy->moveTimer.Set(duration);
    enemy->flags.moveEasing = INT_ARG(1);
    enemy->flags.moveMode = 2;
    if (enemy->flags.mirrored)
    {
        enemy->moveDelta.x = -enemy->moveDelta.x;
    }
}

void ScriptMovePos(Enemy *enemy, EnemyScriptInstr *instr)
{
    D3DXVECTOR3 target;

    target.x = FLOAT_ARG(2);
    target.y = FLOAT_ARG(3);
    target.z = FLOAT_ARG(4);
    enemy->moveDelta = target - enemy->pos;
    enemy->moveStart = enemy->pos;
    enemy->moveTimer.Set(enemy->moveDuration = INT_ARG(0));
    enemy->flags.moveEasing = INT_ARG(1);
    enemy->flags.moveMode = 2;
    enemy->velocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    if (enemy->flags.mirrored)
    {
        enemy->moveDelta.x = -enemy->moveDelta.x;
    }
}

void ScriptMoveLinear(Enemy *enemy, EnemyScriptInstr *instr, float t)
{
    float start;
    float end;

    start = GetFloatOperand(enemy, instr->args[4].f);
    end = GetFloatOperand(enemy, instr->args[5].f);
    *ResolveFloatOperand(enemy, &instr->args[8].f, 0, -1) = (end - start) * t + start;
}

void ScriptMoveHermite(Enemy *enemy, EnemyScriptInstr *instr, float t)
{
    float start;
    float hStart;
    float weightIn;
    float end;
    float slopeIn;
    float wEnd;
    float tangentOut;
    float outWeight;

    start = GetFloatOperand(enemy, instr->args[4].f);
    end = GetFloatOperand(enemy, instr->args[5].f);
    slopeIn = GetFloatOperand(enemy, instr->args[6].f);
    tangentOut = GetFloatOperand(enemy, instr->args[7].f);
    hStart = (t - 1.0f) * (t - 1.0f) * (2.0f * t + 1.0f);
    wEnd = t * t * (3.0f - 2.0f * t);
    weightIn = (1.0f - t) * (1.0f - t) * t;
    outWeight = (t - 1.0f) * t * t;
    *ResolveFloatOperand(enemy, &instr->args[8].f, 0, -1) =
        hStart * start + wEnd * end + weightIn * slopeIn + outWeight * tangentOut;
}

// A spell card's name in its instruction.
struct SpellCardName
{
    BYTE text[0x30];
};

void StartSpellCard(Enemy *enemy, EnemyScriptInstr *instr)
{
    int sum2;
    int sum;
    int len;
    Catk *rec;
    SpellCardName cardName;
    int i;

    cardName = *(SpellCardName *)&instr->args[1];
    for (i = 0; i < ARRAY_SIZE(cardName.text); i++)
    {
        cardName.text[i] ^= 0xaa;
    }
    g_Gui.ShowSpellCardName(*(short *)&instr->args[0], (char *)cardName.text);
    g_BulletManager.ClearAll(1);
    g_Stage.spellCardState = 1;
    g_Stage.spellCardFrames = 0;
    for (i = 0; i < g_Stage.dialogueSpriteCount; i++)
    {
        g_SpriteRenderer->StartScript(&g_Stage.dialogueSprites[i], i + g_Stage.dialogueScriptOffset + 0x2dc);
    }
    g_EnemyManager.spellCardActive = 1;
    g_EnemyManager.spellCardCapture = 1;
    g_EnemyManager.spellCardIndex = *((WORD *)&instr->args[0] + 1);
    g_EnemyManager.spellCardBonus = g_SpellCardBonuses[g_EnemyManager.spellCardIndex];
    g_EnemyManager.spellCardGrazeBonus = 0;
    g_EnemyManager.spellCardBonusStep = g_EnemyManager.spellCardBonus / (enemy->timerCallbackTime / 60 + 10);
    g_EnemyManager.spellCardBonusTimer.Set(0);
    enemy->rankSpeedLow = -0.5f;
    enemy->rankSpeedHigh = 0.5f;
    enemy->rankCount1Low = 0;
    enemy->rankCount1High = 0;
    enemy->rankCount2Low = 0;
    enemy->rankCount2High = 0;
    enemy->effects[24] = g_EffectManager.SpawnReservedEffect(0x19, &enemy->pos, 1, 1, 0xffffffff);
    enemy->effects[24]->sprite.interpDuration[4].Set(0);
    enemy->effects[24]->sprite.interpFrame[4].Set(enemy->timerCallbackTime);
    enemy->effects[24]->sprite.scaleInterpEasing = 0;
    *(D3DXVECTOR2 *)&enemy->effects[24]->sprite.scaleTween = *(D3DXVECTOR2 *)&enemy->effects[24]->sprite.scaleY;
    enemy->effects[24]->sprite.scaleTween.z = 0.125f;
    enemy->effects[24]->sprite.colorTween.x = 0.125f;
    enemy->effects[24]->pos = enemy->pos;
    enemy->flags.hidden = 0;
    if (!g_GameManager.isInReplay)
    {
        rec = &g_GameManager.catks[g_EnemyManager.spellCardIndex];
        sum = 0;
        strcpy(rec->name, (char *)cardName.text);
        len = strlen(rec->name);
        while (len > 0)
        {
            len--;
            sum += rec->name[len];
        }
        sum2 = sum;
        for (len = 0; len < 7; len++)
        {
            sum += rec->captures[len];
            sum += rec->attempts[len];
            sum += rec->maxBonus[len];
        }
        if (rec->checksum != (BYTE)sum)
        {
            for (len = 0; len < 7; len++)
            {
                rec->captures[len] = 0;
                rec->attempts[len] = 0;
                rec->maxBonus[len] = 0;
            }
        }
        if (rec->attempts[g_GameManager.shotIndex] < 9999)
        {
            rec->attempts[g_GameManager.shotIndex]++;
        }
        if (rec->attempts[6] < 9999)
        {
            rec->attempts[6]++;
        }
        for (len = 0; len < 7; len++)
        {
            sum2 += rec->captures[len];
            sum2 += rec->attempts[len];
            sum2 += rec->maxBonus[len];
        }
        rec->checksum = sum2;
    }
}

void EndSpellCard(Enemy *enemy, EnemyScriptInstr *instr)
{
    int bossIdx;
    int sum2;
    int sum;
    int shot;
    int len;
    Catk *rec;
    int bonus;

    if (g_EnemyManager.spellCardActive != 0)
    {
        g_Gui.EndSpellCardName();
        if (g_EnemyManager.spellCardActive == 1)
        {
            bonus = g_BulletManager.ClearAllWithBonus(8000, 1);
            bonus = g_EnemyManager.ClearEnemiesForBonus(8000, bonus);
            if (bonus != 0)
            {
                g_GameManager.AddScore(bonus);
                g_Gui.ShowBonusPopup(bonus);
            }
            if (g_EnemyManager.spellCardCapture != 0)
            {
                rec = &g_GameManager.catks[g_EnemyManager.spellCardIndex];
                bonus = g_EnemyManager.spellCardBonus + g_EnemyManager.spellCardGrazeBonus;
                g_Gui.ShowSpellCardBonus(bonus);
                g_GameManager.AddScore(bonus);
                if (!g_GameManager.isInReplay)
                {
                    sum = 0;
                    len = strlen(rec->name);
                    while (len > 0)
                    {
                        len--;
                        sum += rec->name[len];
                    }
                    sum2 = sum;
                    for (len = 0; len < 7; len++)
                    {
                        sum += rec->captures[len];
                        sum += rec->attempts[len];
                        sum += rec->maxBonus[len];
                    }
                    if (rec->checksum != (BYTE)sum)
                    {
                        for (len = 0; len < 7; len++)
                        {
                            rec->captures[len] = 0;
                            rec->attempts[len] = 0;
                            rec->maxBonus[len] = 0;
                        }
                    }
                    shot = g_GameManager.shotIndex;
                    if (rec->maxBonus[shot] < bonus)
                    {
                        rec->maxBonus[shot] = bonus;
                    }
                    if (rec->maxBonus[6] < bonus)
                    {
                        rec->maxBonus[6] = bonus;
                    }
                    if (rec->captures[shot] < 9999)
                    {
                        rec->captures[shot]++;
                    }
                    if (rec->captures[6] < 9999)
                    {
                        rec->captures[6]++;
                    }
                    for (len = 0; len < 7; len++)
                    {
                        sum2 += rec->captures[len];
                        sum2 += rec->attempts[len];
                        sum2 += rec->maxBonus[len];
                    }
                    rec->checksum = sum2;
                }
                g_GameManager.stats->spellCardsCaptured++;
            }
        }
        g_EnemyManager.spellCardActive = 0;
        for (bossIdx = 0; bossIdx < 8; bossIdx++)
        {
            if (g_EnemyManager.bosses[bossIdx] != NULL && g_EnemyManager.bosses[bossIdx]->effects[24] != NULL)
            {
                g_EnemyManager.bosses[bossIdx]->effects[24]->active = 0;
                g_EnemyManager.bosses[bossIdx]->effects[24] = NULL;
            }
        }
        g_SoundPlayer.QueueSound(0xf, 0);
    }
    g_Stage.spellCardState = 0;
}

// Moves `lo` towards `hi` with the rank: lo at rank 0, hi at rank 32, as
// th06's.
inline int RankLerp(int lo, int hi)
{
    return g_GameManager.rank * (hi - lo) / 32 + lo;
}

inline float RankLerpF(float lo, float hi)
{
    return g_GameManager.rank * (hi - lo) / 32.0f + lo;
}

// The variable argument `n` of `instr` names, through ResolveOperand.
#define INT_VAR(n) (*ResolveOperand(enemy, &instr->args[n].i, instr->paramMask, (n)))
#define FLOAT_VAR(n) (*ResolveFloatOperand(enemy, &instr->args[n].f, instr->paramMask, (n)))
// INT_ARG and FLOAT_ARG for a value `v` whose variable bit is `bit`.
// `v` must stay unparenthesized: a parenthesized float is loaded onto
// the FPU instead of being copied through an integer register.
#define INT_OP(v, bit) ((instr->paramMask & (bit)) ? GetIntOperand(enemy, v) : v)
#define FLOAT_OP(v, bit) ((instr->paramMask & (bit)) ? GetFloatOperand(enemy, v) : v)

int EnemyScripts::RunEnemy(Enemy *enemy)
{
    int value;
    EnemyScriptInstr *instr;

restart:
    instr = (EnemyScriptInstr *)enemy->ctx.code;
    if (enemy->pendingInterrupt >= 0)
    {
        goto interrupt;
    }
    if (enemy->periodicSubIndex >= 0)
    {
        enemy->periodicSubTimer.Tick();
        if (enemy->periodicSubTimer.AsFrames() >= enemy->periodicSubInterval.AsFrames())
        {
            enemy->periodicSubTimer.Set(0);
            enemy->savedCtx[enemy->stackDepth] = enemy->ctx;
            enemy->ctx.vars = enemy->periodicSubVars;
            g_EnemyScripts.StartSub(&enemy->ctx, enemy->periodicSubIndex);
            if (enemy->stackDepth < 15)
            {
                enemy->stackDepth++;
            }
            instr = (EnemyScriptInstr *)enemy->ctx.code;
            enemy->ctx.timedCallActive = 1;
        }
    }
loop:
    if (enemy->ctx.waitTimer.AsFrames() > 0)
    {
        enemy->ctx.waitTimer.Sub(1);
        enemy->ctx.timer.Sub(1);
        goto done;
    }
    if (enemy->ctx.timer == instr->time)
    {
        if ((instr->difficultyMask & g_GameManager.difficultyMask) == 0)
        {
            goto next;
        }
        switch (instr->opcode)
        {
        case 1:
            return -1;
        case 45:
            enemy->ctx.waitTimer.Set(INT_ARG(0));
            break;
        case 3:
            (*ResolveOperand(enemy, &instr->args[2].i, instr->paramMask, 2))--;
            if (INT_ARG(2) <= 0)
            {
                break;
            }
        case 2:
        jump:
            enemy->ctx.timer.current = instr->args[0].i;
            instr = (EnemyScriptInstr *)((char *)instr + instr->args[1].i);
            goto loop;
        case 4:
            INT_VAR(0) = INT_ARG(1);
            break;
        case 5:
            FLOAT_VAR(0) = FLOAT_ARG(1);
            break;
        case 40:
            FLOAT_VAR(0) = AddNormalizeAngle(FLOAT_ARG(0), 0.0f);
            break;
        case 6:
            INT_VAR(0) = g_Rng.GetRandomU32InRange(INT_ARG(1));
            break;
        case 7:
            INT_VAR(0) = g_Rng.GetRandomU32InRange(INT_ARG(1)) + INT_ARG(2);
            break;
        case 8:
            FLOAT_VAR(0) = g_Rng.GetRandomF32InRange(FLOAT_ARG(1));
            break;
        case 9:
            FLOAT_VAR(0) = g_Rng.GetRandomF32InRange(FLOAT_ARG(1)) + FLOAT_ARG(2);
            break;
        case 10:
            INT_VAR(0) = (g_Rng.GetRandomU16() & 1 ? 1 : -1) * INT_ARG(1);
            break;
        case 11:
            FLOAT_VAR(0) = (g_Rng.GetRandomU16() & 1 ? 1.0f : -1.0f) * FLOAT_ARG(1);
            break;
        case 17:
            INT_VAR(0)++;
            break;
        case 18:
            INT_VAR(0)--;
            break;
        case 43:
            INT_VAR(0) = (instr->paramMask & 2) ? GetIntOperand(g_EnemyManager.bosses[INT_ARG(2)], instr->args[1].i)
                                                : instr->args[1].i;
            break;
        case 44:
            FLOAT_VAR(0) = (instr->paramMask & 2)
                               ? GetFloatOperand(g_EnemyManager.bosses[INT_ARG(2)], instr->args[1].f)
                               : instr->args[1].f;
            break;
        case 12:
            INT_VAR(0) = INT_ARG(1) + INT_ARG(2);
            break;
        case 19:
            FLOAT_VAR(0) = FLOAT_ARG(1) + FLOAT_ARG(2);
            break;
        case 13:
            INT_VAR(0) = INT_ARG(1) - INT_ARG(2);
            break;
        case 20:
            FLOAT_VAR(0) = FLOAT_ARG(1) - FLOAT_ARG(2);
            break;
        case 14:
            INT_VAR(0) = INT_ARG(1) * INT_ARG(2);
            break;
        case 21:
            FLOAT_VAR(0) = FLOAT_ARG(1) * FLOAT_ARG(2);
            break;
        case 15:
            INT_VAR(0) = INT_ARG(1) / INT_ARG(2);
            break;
        case 22:
            FLOAT_VAR(0) = FLOAT_ARG(1) / FLOAT_ARG(2);
            break;
        case 16:
            INT_VAR(0) = INT_ARG(1) % INT_ARG(2);
            break;
        case 23:
            FLOAT_VAR(0) = fmodf(FLOAT_ARG(1), FLOAT_ARG(2));
            break;
        case 24:
            FLOAT_VAR(0) = sinf(FLOAT_ARG(1));
            break;
        case 25:
            FLOAT_VAR(0) = cosf(FLOAT_ARG(1));
            break;
        case 26:
            FLOAT_VAR(0) = atan2f(FLOAT_ARG(4) - FLOAT_ARG(2), FLOAT_ARG(3) - FLOAT_ARG(1));
            break;
        case 159:
        {
            float scale;

            scale = FLOAT_ARG(1) - FLOAT_ARG(2);
            FLOAT_VAR(0) = scale * FLOAT_ARG(3) + FLOAT_ARG(2);
            break;
        }
        case 27:
        {
            int i;
            EnemyScriptLoop *loop;

            for (loop = enemy->ctx.loops, i = 0; i < 8; i++, loop++)
            {
                if (loop->func != NULL && loop->target != instr->args[0].f)
                {
                    continue;
                }
                loop->timer.Set(0);
                loop->target = instr->args[0].f;
                loop->duration = INT_ARG(1);
                loop->type = INT_ARG(2);
                loop->mode = INT_ARG(3);
                loop->func = g_EnemyScriptInterps[loop->type];
                loop->args[0] = FLOAT_ARG(4);
                loop->args[1] = FLOAT_ARG(5);
                loop->args[2] = FLOAT_ARG(6);
                loop->args[3] = FLOAT_ARG(7);
                break;
            }
            break;
        }
        case 28:
            if (INT_ARG(0) == INT_ARG(1))
            {
                goto jumpIf;
            }
            break;
        case 29:
            if (FLOAT_ARG(0) == FLOAT_ARG(1))
            {
                goto jumpIf;
            }
            break;
        case 30:
            if (INT_ARG(0) != INT_ARG(1))
            {
                goto jumpIf;
            }
            break;
        case 31:
            if (FLOAT_ARG(0) != FLOAT_ARG(1))
            {
                goto jumpIf;
            }
            break;
        case 32:
            if (INT_ARG(0) < INT_ARG(1))
            {
                goto jumpIf;
            }
            break;
        case 33:
            if (FLOAT_ARG(0) < FLOAT_ARG(1))
            {
                goto jumpIf;
            }
            break;
        case 34:
            if (INT_ARG(0) <= INT_ARG(1))
            {
                goto jumpIf;
            }
            break;
        case 35:
            if (FLOAT_ARG(0) <= FLOAT_ARG(1))
            {
                goto jumpIf;
            }
            break;
        case 36:
            if (INT_ARG(0) > INT_ARG(1))
            {
                goto jumpIf;
            }
            break;
        case 37:
            if (FLOAT_ARG(0) > FLOAT_ARG(1))
            {
                goto jumpIf;
            }
            break;
        case 38:
            if (INT_ARG(0) >= INT_ARG(1))
            {
                goto jumpIf;
            }
            break;
        case 39:
            if (FLOAT_ARG(0) >= FLOAT_ARG(1))
            {
                goto jumpIf;
            }
            break;
        jumpIf:
            enemy->ctx.timer.current = instr->args[2].i;
            instr = (EnemyScriptInstr *)((char *)instr + instr->args[3].i);
            goto loop;
        case 41:
            value = instr->args[0].i;
            enemy->ctx.code = (char *)instr + instr->size;
            if (!enemy->flags.disableCallStack)
            {
                enemy->savedCtx[enemy->stackDepth] = enemy->ctx;
            }
            g_EnemyScripts.StartSub(&enemy->ctx, value);
            memcpy(enemy->ctx.vars.ints3, g_EnemyScripts.globalInts, sizeof(int) * 8);
            if (!enemy->flags.disableCallStack && enemy->stackDepth < 15)
            {
                enemy->stackDepth++;
            }
            goto restart;
        case 42:
            if (enemy->flags.disableCallStack)
            {
                DebugPrint("error : no Stack Ret\r\n");
            }
            enemy->stackDepth--;
            if (enemy->ctx.timedCallActive)
            {
                enemy->periodicSubVars = enemy->ctx.vars;
                enemy->ctx.timedCallActive = 0;
            }
            enemy->ctx = enemy->savedCtx[enemy->stackDepth];
            goto restart;
        case 95:
            g_SpriteRenderer->StartScript(&enemy->sprite, INT_ARG(0) + 0x900);
            break;
        case 97:
            if (INT_ARG(0) >= 2)
            {
                DebugPrint("error : sub anim overflow\r\n");
            }
            if (INT_ARG(1) >= 0)
            {
                g_SpriteRenderer->StartScript(&enemy->sprites[INT_ARG(0)], INT_ARG(1) + 0x900);
            }
            else
            {
                enemy->sprites[INT_ARG(0)].scriptIndex = -1;
            }
            break;
        case 46:
            enemy->pos.x = FLOAT_ARG(0);
            enemy->pos.y = FLOAT_ARG(1);
            enemy->pos.z = FLOAT_ARG(2);
            enemy->ClampPos();
            break;
        case 47:
            enemy->velocity.x = FLOAT_ARG(0);
            enemy->velocity.y = FLOAT_ARG(1);
            enemy->velocity.z = FLOAT_ARG(2);
            enemy->angle = atan2f(enemy->velocity.y, enemy->velocity.x);
            enemy->flags.moveMode = 0;
            break;
        case 48:
            enemy->angularVelocity = FLOAT_ARG(0);
            enemy->flags.moveMode = 1;
            break;
        case 53:
            enemy->angle = g_Player.AngleFrom(&enemy->pos) + FLOAT_ARG(0);
            enemy->speed = FLOAT_ARG(1);
            enemy->flags.moveMode = 1;
            break;
        case 49:
            enemy->speed = FLOAT_ARG(0);
            enemy->flags.moveMode = 1;
            break;
        case 50:
            enemy->acceleration = FLOAT_ARG(0);
            enemy->flags.moveMode = 1;
            break;
        case 59:
            enemy->flags.moveMode = 1;
            enemy->moveTimer.Set(enemy->moveDuration = INT_ARG(0));
            break;
        case 60:
            enemy->flags.moveMode = 3;
            enemy->moveTimer.Set(enemy->moveDuration = INT_ARG(0));
            break;
        case 61:
            enemy->flags.moveMode = 2;
            enemy->moveTimer.Set(enemy->moveDuration = INT_ARG(0));
            break;
        case 64:
        case 65:
        case 66:
        case 67:
        case 68:
        case 69:
        case 70:
        case 71:
        case 72:
        {
            EnemyScriptArg *bulletArgs;
            BulletShooter *bs;

            if (enemy->life <= 0)
            {
                break;
            }
            bulletArgs = instr->args;
            bs = &enemy->shooter;
            value = *(short *)&bulletArgs[0];
            bs->sprite = INT_OP(value, 1);
            bs->aimMode = instr->opcode - 64;
            bs->count1 = INT_OP(bulletArgs[1].i, 4);
            bs->count2 = INT_OP(bulletArgs[2].i, 8);
            bs->pos = enemy->pos + enemy->shootOffset;
            bs->angle1 = FLOAT_OP(bulletArgs[5].f, 0x40);
            bs->speed1 = FLOAT_OP(bulletArgs[3].f, 0x10);
            bs->angle2 = FLOAT_OP(bulletArgs[6].f, 0x80);
            bs->speed2 = FLOAT_OP(bulletArgs[4].f, 0x20);
            if (!g_EnemyManager.spellCardActive)
            {
                bs->count1 += RankLerp(enemy->rankCount1Low, enemy->rankCount1High);
                if (bs->count1 <= 0)
                {
                    bs->count1 = 1;
                }
                bs->count2 += RankLerp(enemy->rankCount2Low, enemy->rankCount2High);
                if (bs->count2 <= 0)
                {
                    bs->count2 = 1;
                }
                if (bs->speed1 != 0.0f)
                {
                    bs->speed1 += RankLerpF(enemy->rankSpeedLow, enemy->rankSpeedHigh);
                    if (bs->speed1 < 0.3f)
                    {
                        bs->speed1 = 0.3f;
                    }
                }
                bs->speed2 += RankLerpF(enemy->rankSpeedLow, enemy->rankSpeedHigh) / 2.0f;
                if (bs->speed2 < 0.3f)
                {
                    bs->speed2 = 0.3f;
                }
            }
            bs->unusedAfterFireC2 = 0;
            bs->flags = bulletArgs[7].i;
            value = ((short *)bulletArgs)[1];
            bs->color = INT_OP(value, 2);
            if (!enemy->flags.holdFire)
            {
                g_BulletManager.FirePattern(bs);
            }
            break;
        }
        case 79:
        {
            BulletCommand *cmd;

            cmd = &enemy->shooter.commands[INT_ARG(0)];
            cmd->flag = INT_ARG(1);
            cmd->concurrentFlag = INT_ARG(2);
            cmd->frames = INT_ARG(3);
            cmd->maxTurns = INT_ARG(4);
            cmd->arg0 = FLOAT_ARG(5);
            cmd->arg1 = FLOAT_ARG(6);
            break;
        }
        case 98:
            enemy->deathEffect = *(char *)instr->args;
            enemy->deathEffectIndex = ((BYTE *)instr->args)[1];
            enemy->deathEffectVariant = ((BYTE *)instr->args)[2];
            break;
        case 73:
            enemy->shootInterval = INT_ARG(0);
            if (enemy->shootInterval != 0)
            {
                enemy->shootInterval += RankLerp(enemy->shootInterval / 5, -enemy->shootInterval / 5);
                enemy->shootTimer.Set(0);
            }
            break;
        case 74:
            enemy->shootInterval = INT_ARG(0);
            if (enemy->shootInterval != 0)
            {
                enemy->shootInterval += RankLerp(enemy->shootInterval / 5, -enemy->shootInterval / 5);
                enemy->shootTimer.Set(g_Rng.GetRandomU32InRange(enemy->shootInterval));
            }
            break;
        case 75:
            enemy->flags.holdFire = 1;
            break;
        case 76:
            enemy->flags.holdFire = 0;
            break;
        case 77:
            enemy->shooter.pos = enemy->pos + enemy->shootOffset;
            g_BulletManager.FirePattern(&enemy->shooter);
            break;
        case 78:
            enemy->shootOffset.x = FLOAT_ARG(0);
            enemy->shootOffset.y = FLOAT_ARG(1);
            enemy->shootOffset.z = FLOAT_ARG(2);
            break;
        case 82:
        case 83:
        {
            EnemyScriptArg *laserArgs;
            BulletShooter *shooter;

            laserArgs = instr->args;
            shooter = &enemy->laserShooter;
            shooter->pos = enemy->pos + enemy->shootOffset;
            shooter->sprite = *(short *)&laserArgs[0];
            shooter->color = INT_OP(((short *)laserArgs)[1], 2);
            shooter->angle1 = FLOAT_OP(laserArgs[1].f, 4);
            shooter->speed1 = FLOAT_OP(laserArgs[2].f, 8);
            shooter->exFloats[0] = FLOAT_OP(laserArgs[3].f, 0x10);
            shooter->exFloats[1] = FLOAT_OP(laserArgs[4].f, 0x20);
            shooter->exFloats[2] = FLOAT_OP(laserArgs[5].f, 0x40);
            shooter->exFloats[3] = laserArgs[6].f;
            shooter->exInts[0] = laserArgs[7].i;
            shooter->exInts[1] = laserArgs[8].i;
            shooter->exInts[2] = laserArgs[9].i;
            shooter->exInts[3] = laserArgs[10].i;
            shooter->laserEndHitFrames = laserArgs[11].i;
            shooter->flags = laserArgs[12].i;
            if (instr->opcode == 83)
            {
                shooter->aimMode = 0;
            }
            else
            {
                shooter->aimMode = 1;
            }
            enemy->lasers[enemy->laserIndex] = g_BulletManager.SpawnLaser(shooter);
            break;
        }
        case 84:
            enemy->laserIndex = INT_ARG(0);
            break;
        case 85:
            value = INT_ARG(0);
            if (enemy->lasers[value] != NULL)
            {
                enemy->lasers[value]->angle = AddNormalizeAngle(enemy->lasers[value]->angle, FLOAT_ARG(1));
            }
            break;
        case 152:
            value = INT_ARG(0);
            if (enemy->lasers[value] != NULL)
            {
                enemy->lasers[value]->angle = FLOAT_ARG(1);
            }
            break;
        case 86:
            value = INT_ARG(0);
            if (enemy->lasers[value] != NULL)
            {
                enemy->lasers[value]->angle = g_Player.AngleFrom(&enemy->lasers[value]->pos) + FLOAT_ARG(1);
            }
            break;
        case 87:
            value = INT_ARG(0);
            if (enemy->lasers[value] != NULL)
            {
                enemy->lasers[value]->pos.x = FLOAT_ARG(1) + enemy->pos.x;
                enemy->lasers[value]->pos.y = FLOAT_ARG(2) + enemy->pos.y;
                enemy->lasers[value]->pos.z = FLOAT_ARG(3) + enemy->pos.z;
            }
            break;
        case 156:
            value = INT_ARG(0);
            if (enemy->lasers[value] != NULL)
            {
                enemy->lasers[value]->drawDelayFlag = INT_ARG(1);
            }
            break;
        case 88:
            value = INT_ARG(0);
            if (enemy->lasers[value] != NULL && enemy->lasers[value]->active)
            {
                enemy->ctx.compareResult = 0;
            }
            else
            {
                enemy->ctx.compareResult = 1;
            }
            break;
        case 89:
            value = INT_ARG(0);
            if (enemy->lasers[value] != NULL && enemy->lasers[value]->active && enemy->lasers[value]->state < 2)
            {
                enemy->lasers[value]->state = 2;
                enemy->lasers[value]->timer.Set(0);
                enemy->lasers[value]->width = enemy->lasers[value]->baseWidth;
            }
            break;
        case 134:
        {
            int i;

            for (i = 0; i < 0x20; i++)
            {
                enemy->lasers[i] = NULL;
            }
            break;
        }
        case 157:
            value = INT_ARG(0);
            if (enemy->lasers[value] != NULL)
            {
                enemy->lasers[value]->maxLength = FLOAT_ARG(1);
            }
            break;
        case 158:
            value = INT_ARG(0);
            if (enemy->lasers[value] != NULL)
            {
                enemy->lasers[value]->startOffset = FLOAT_ARG(1);
                enemy->lasers[value]->endOffset = FLOAT_ARG(2);
            }
            break;
        case 147:
            g_EnemyManager.scriptArg147 = INT_ARG(0);
            break;
        case 99:
            if (INT_ARG(0) >= 0)
            {
                g_EnemyManager.bosses[INT_ARG(0)] = enemy;
                g_Gui.bossPresent = 1;
                g_Gui.bossHealth = 1.0f;
                enemy->flags.isBoss = 1;
                enemy->bossIndex = INT_ARG(0);
                g_AsciiText.SetNearbyLabelInterrupt(enemy->bossIndex, 1);
            }
            else
            {
                if (enemy->bossIndex < 4)
                {
                    g_Gui.bossPresent = 0;
                }
                g_EnemyManager.bosses[enemy->bossIndex] = NULL;
                enemy->flags.isBoss = 0;
                g_AsciiText.SetNearbyLabelInterrupt(enemy->bossIndex, 2);
                ReleaseEnemyEffects(enemy);
            }
            break;
        case 100:
        {
            EnemyScriptArg *effectArgs;

            effectArgs = instr->args;
            enemy->effects[enemy->effectCount] =
                g_EffectManager.SpawnEffect(0xd, &enemy->pos, 1, g_EffectColors[effectArgs[0].i]);
            enemy->effects[enemy->effectCount]->axis = *(D3DXVECTOR3 *)&effectArgs[1];
            enemy->effectRadius = effectArgs[4].f;
            enemy->effectCount++;
            break;
        }
        case 54:
            if (INT_ARG(0) <= 0)
            {
                enemy->angle = AddNormalizeAngle(FLOAT_ARG(2), 0.0f);
                enemy->speed = FLOAT_ARG(3);
                enemy->flags.moveMode = 1;
                enemy->moveTimer.Set(enemy->moveDuration = INT_ARG(0));
            }
            else
            {
                ScriptMoveAngle(enemy, instr);
            }
            break;
        case 55:
            ScriptMovePos(enemy, instr);
            break;
        case 56:
            enemy->moveTimer.Set(enemy->moveDuration = INT_ARG(0));
            enemy->moveStart.x = FLOAT_ARG(1);
            enemy->moveStart.y = FLOAT_ARG(2);
            enemy->moveStart.z = FLOAT_ARG(3);
            enemy->orbitAngle = FLOAT_ARG(4);
            enemy->orbitAngularVelocity = FLOAT_ARG(5);
            enemy->orbitRadius = FLOAT_ARG(6);
            enemy->orbitRadiusVelocity = FLOAT_ARG(7);
            enemy->flags.moveMode = 3;
            break;
        case 57:
            enemy->orbitRadius = FLOAT_ARG(0);
            enemy->orbitRadiusVelocity = FLOAT_ARG(1);
            break;
        case 58:
            enemy->orbitAngle = FLOAT_ARG(0);
            enemy->orbitAngularVelocity = FLOAT_ARG(1);
            break;
        case 62:
            enemy->lowerBound[0] = FLOAT_ARG(0);
            enemy->lowerBound[1] = FLOAT_ARG(1);
            enemy->upperBound[0] = FLOAT_ARG(2);
            enemy->upperBound[1] = FLOAT_ARG(3);
            enemy->flags.clampPos = 1;
            break;
        case 63:
            enemy->flags.clampPos = 0;
            break;
        case 51:
            FLOAT_VAR(0) = g_Rng.GetRandomF32InRange(FLOAT_ARG(2) - FLOAT_ARG(1)) + FLOAT_ARG(1);
            break;
        case 52:
        {
            float angle;

            if (g_Player.position.x < enemy->pos.x)
            {
                angle = AddNormalizeAngle(g_Rng.GetRandomF32InRange(1.5707964f) + 2.3561945f, 0.0f);
            }
            else
            {
                angle = g_Rng.GetRandomF32InRange(1.5707964f) - 0.7853982f;
            }
            if (enemy->lowerBound[0] + 96.0f > enemy->pos.x)
            {
                if (angle > 1.5707964f)
                {
                    angle = 3.1415927f - angle;
                }
                else if (angle < -1.5707964f)
                {
                    angle = -3.1415927f - angle;
                }
            }
            if (enemy->upperBound[0] - 96.0f < enemy->pos.x)
            {
                if (angle < 1.5707964f && angle >= 0.0f)
                {
                    angle = 3.1415927f - enemy->angle;
                }
                else if (angle > -1.5707964f && angle <= 0.0f)
                {
                    angle = -3.1415927f - angle;
                }
            }
            if (enemy->lowerBound[1] + 48.0f > enemy->pos.y && angle < 0.0f)
            {
                angle = -angle;
            }
            if (enemy->upperBound[1] - 48.0f < enemy->pos.y && angle > 0.0f)
            {
                angle = -angle;
            }
            FLOAT_VAR(0) = angle;
            break;
        }
        case 96:
            enemy->turnStill = ((short *)instr->args)[0];
            enemy->turnStillFromLeft = ((short *)instr->args)[1];
            enemy->turnStillFromRight = ((short *)instr->args)[2];
            enemy->turnLeft = ((short *)instr->args)[3];
            enemy->turnRight = ((short *)instr->args)[4];
            enemy->spriteDirection = 0xff;
            break;
        case 101:
            enemy->hitboxSize.x = FLOAT_ARG(0);
            enemy->hitboxSize.y = FLOAT_ARG(1);
            enemy->hitboxSize.z = FLOAT_ARG(2);
            break;
        case 153:
            enemy->extraHitboxSize.x = FLOAT_ARG(0);
            enemy->extraHitboxSize.y = FLOAT_ARG(1);
            enemy->extraHitboxSize.z = FLOAT_ARG(2);
            break;
        case 102:
            enemy->flags.contactDamage = instr->args[0].i;
            break;
        case 103:
            enemy->flags.takesDamage = instr->args[0].i;
            break;
        case 104:
            enemy->flags.hitByShots = instr->args[0].i;
            break;
        case 105:
            g_SoundPlayer.QueueSound(INT_ARG(0), 0);
            break;
        case 106:
            enemy->flags.deathMode = instr->args[0].i;
            break;
        case 107:
            enemy->deathSub = *(BYTE *)instr->args;
            break;
        case 108:
            enemy->interrupts[INT_ARG(1)] = INT_ARG(0);
            break;
        case 109:
            enemy->pendingInterrupt = INT_ARG(0);
        interrupt:
            enemy->ctx.code = (char *)instr + instr->size;
            if (!enemy->flags.disableCallStack)
            {
                enemy->savedCtx[enemy->stackDepth] = enemy->ctx;
            }
            g_EnemyScripts.StartSub(&enemy->ctx, enemy->interrupts[enemy->pendingInterrupt]);
            if (enemy->stackDepth < 15)
            {
                enemy->stackDepth++;
            }
            enemy->pendingInterrupt = -1;
            goto restart;
        case 110:
        {
            int i;

            enemy->life = enemy->maxLife = INT_ARG(0);
            if (!enemy->bossIndex && enemy->flags.isBoss)
            {
                for (i = 0; i < 8; i++)
                {
                    g_Gui.sectionEnds[i] = 0.0f;
                    g_Gui.sectionStarts[i] = 0.0f;
                }
            }
            break;
        }
        case 139:
        {
            int idx;

            idx = INT_ARG(0);
            g_Gui.SetBossSection(idx, INT_ARG(1) / (float)enemy->maxLife,
                                           INT_ARG(2) / (float)enemy->maxLife);
            g_Gui.SetBossSectionColor(idx, INT_ARG(3));
            break;
        }
        case 90:
            StartSpellCard(enemy, instr);
            break;
        case 91:
            EndSpellCard(enemy, instr);
            break;
        case 111:
            enemy->callbackTimer.Set(INT_ARG(0));
            break;
        case 112:
            enemy->lifeCallbackLife[0] = INT_ARG(0);
            break;
        case 113:
            enemy->lifeCallbackSub[0] = INT_ARG(0);
            break;
        case 148:
            enemy->lifeCallbackLife[INT_ARG(0)] = INT_ARG(1);
            enemy->lifeCallbackSub[INT_ARG(0)] = INT_ARG(2);
            break;
        case 114:
            enemy->timerCallbackTime = INT_ARG(0);
            enemy->callbackTimer.Set(0);
            break;
        case 115:
            enemy->timerCallbackSub = INT_ARG(0);
            break;
        case 144:
            enemy->periodicSubInterval.Set(INT_ARG(0));
            enemy->periodicSubIndex = INT_ARG(1);
            enemy->periodicSubTimer.Set(0);
            enemy->periodicSubVars = enemy->ctx.vars;
            break;
        case 116:
            enemy->flags.shootable = instr->args[0].i;
            break;
        case 117:
            g_EffectManager.SpawnEffect(INT_ARG(0), &enemy->pos, INT_ARG(1), INT_VAR(2));
            break;
        case 118:
        {
            D3DXVECTOR3 velocity;

            velocity.x = FLOAT_ARG(3);
            velocity.y = FLOAT_ARG(4);
            velocity.z = FLOAT_ARG(5);
            g_EffectManager.SpawnEffectWithVelocity(INT_ARG(0), &enemy->pos, &velocity, INT_ARG(1), INT_VAR(2));
            break;
        }
        case 119:
        {
            int count;
            int i;
            D3DXVECTOR3 pos;

            count = INT_ARG(0);
            for (i = 0; i < count; i++)
            {
                pos = enemy->pos;
                pos[0] += g_Rng.GetRandomF32InRange(128.0f) - 64.0f;
                pos[1] += g_Rng.GetRandomF32InRange(128.0f) - 64.0f;
                if ((int)g_GameManager.stats->power < 0x80)
                {
                    g_ItemManager.SpawnItem(&pos, i == 0 ? 2 : 0, 0);
                }
                else
                {
                    g_ItemManager.SpawnItem(&pos, 1, 0);
                }
            }
            break;
        }
        case 154:
        {
            int count;
            int i;
            D3DXVECTOR3 pos;

            count = INT_ARG(0);
            for (i = 0; i < count; i++)
            {
                pos = enemy->pos;
                pos[0] += g_Rng.GetRandomF32InRange(128.0f) - 64.0f;
                pos[1] += g_Rng.GetRandomF32InRange(128.0f) - 64.0f;
                g_ItemManager.SpawnItem(&pos, 1, 0);
            }
            break;
        }
        case 120:
            enemy->flags.rotateSprite = instr->args[0].i;
            break;
        case 121:
            g_EnemyScriptFuncs[INT_ARG(0)](enemy, instr);
            break;
        case 122:
            if (INT_ARG(0) >= 0)
            {
                enemy->ctx.func = g_EnemyScriptFuncs[INT_ARG(0)];
                enemy->ctx.funcInstr = instr;
            }
            else
            {
                enemy->ctx.func = NULL;
            }
            break;
        case 123:
            enemy->ctx.timer += INT_ARG(0);
            break;
        case 124:
            g_ItemManager.SpawnItem(&enemy->pos, INT_ARG(0), 0);
            break;
        case 125:
            g_Stage.waitLabel = INT_ARG(0);
            break;
        case 126:
            g_Gui.SetBossLives(INT_ARG(0));
            g_GameManager.progressFrames += 1800;
            break;
        case 92:
        {
            Enemy *child;
            EnemySpawnArgs spawn;
            D3DXVECTOR3 pos;

            if (enemy->life > 0)
            {
                spawn = *(EnemySpawnArgs *)instr->args;
                pos.x = FLOAT_OP(spawn.pos.x, 2);
                pos.y = FLOAT_OP(spawn.pos.y, 4);
                pos.z = FLOAT_OP(spawn.pos.z, 8);
                child = g_EnemyManager.SpawnWithVars(spawn.sub, &pos, INT_ARG(4), INT_ARG(5), INT_ARG(6),
                                                        &enemy->ctx.vars);
            }
            break;
        }
        case 93:
        {
            Enemy *child;
            EnemySpawnArgs spawn;
            D3DXVECTOR3 pos;

            if (enemy->life > 0)
            {
                spawn = *(EnemySpawnArgs *)instr->args;
                pos.x = FLOAT_OP(spawn.pos.x, 2);
                pos.y = FLOAT_OP(spawn.pos.y, 4);
                pos.z = FLOAT_OP(spawn.pos.z, 8);
                pos += enemy->pos;
                child = g_EnemyManager.SpawnWithVars(spawn.sub, &pos, INT_ARG(4), INT_ARG(5), INT_ARG(6),
                                                         &enemy->ctx.vars);
            }
            break;
        }
        case 94:
            g_EnemyManager.ClearEnemiesForBonus(8000, 0);
            break;
        case 128:
            enemy->sprite.pendingInterrupt = INT_ARG(0);
            break;
        case 129:
            enemy->sprites[instr->args[0].i].pendingInterrupt = instr->args[1].i;
            break;
        case 80:
            g_BulletManager.ClearAll(1);
            break;
        case 81:
            if (INT_ARG(0) >= 0)
            {
                enemy->shooter.sound = INT_ARG(0);
                enemy->shooter.flags |= 0x200;
            }
            else
            {
                enemy->shooter.flags &= ~0x200;
            }
            enemy->shooter.bulletSound = INT_ARG(1);
            break;
        case 130:
            enemy->flags.disableCallStack = instr->args[0].i;
            break;
        case 131:
            enemy->rankSpeedLow = FLOAT_ARG(0);
            enemy->rankSpeedHigh = FLOAT_ARG(1);
            enemy->rankCount1Low = INT_ARG(2);
            enemy->rankCount1High = INT_ARG(3);
            enemy->rankCount2Low = INT_ARG(4);
            enemy->rankCount2High = INT_ARG(5);
            break;
        case 132:
            enemy->flags.hideFollowEffects = instr->args[0].i;
            break;
        case 133:
            enemy->timerCallbackSub = enemy->deathSub;
            enemy->callbackTimer.Set(0);
            break;
        case 135:
            enemy->flags.keepSpellCard = instr->args[0].i;
            break;
        case 136:
            enemy->flags.grazeable = instr->args[0].i;
            enemy->drawListIndex = 2;
            break;
        case 137:
            enemy->flags.staysOffscreen = instr->args[0].i;
            break;
        case 138:
            enemy->trailFlags = instr->args[0].i;
            enemy->trailSegmentCount = INT_ARG(1);
            enemy->trailContactSegments = INT_ARG(2);
            enemy->trailSegmentStep = INT_ARG(3);
            if (enemy->trailFlags & 8)
            {
                g_SpriteRenderer->FillSpriteStripVertices(&enemy->sprite, (VertexTex1DiffuseXyzrwh *)enemy->spriteStripVertices,
                                                        enemy->trailSegmentCount / enemy->trailSegmentStep * 2);
            }
            break;
        case 140:
            g_EffectManager.colorScale[0] = FLOAT_ARG(0);
            g_EffectManager.colorScale[1] = FLOAT_ARG(1);
            g_EffectManager.colorScale[2] = FLOAT_ARG(2);
            g_EffectManager.colorScale[3] = FLOAT_ARG(3);
            break;
        case 142:
            enemy->invulnTimer.Set(INT_ARG(0));
            break;
        case 143:
            g_BulletManager.ConvertBulletsToItems(&enemy->pos, FLOAT_ARG(0));
            break;
        case 145:
            if (g_EnemyManager.bosses[INT_ARG(0)] != NULL)
            {
                g_EnemyManager.bosses[INT_ARG(0)]->pendingInterrupt = INT_ARG(1);
            }
            break;
        case 146:
            g_BulletManager.ClearAll(0);
            break;
        case 149:
            enemy->flags.hidden = INT_ARG(0);
            if (!enemy->flags.hidden)
            {
                enemy->effects[24]->pos.x = FLOAT_ARG(1);
                enemy->effects[24]->pos.y = FLOAT_ARG(2);
                enemy->effects[24]->pos.z = FLOAT_ARG(3);
            }
            break;
        case 150:
            enemy->sprite.rotationZ = FLOAT_ARG(0);
            break;
        case 151:
            *ResolveFloatOperand(enemy, &instr->args[1].f, instr->paramMask, 1) = sinf(FLOAT_ARG(2)) * FLOAT_ARG(3);
            FLOAT_VAR(0) = cosf(FLOAT_ARG(2)) * FLOAT_ARG(3);
            break;
        case 155:
            if ((g_Player.position.x < enemy->pos.x && enemy->pos.x > 96.0f) || enemy->pos.x > 288.0f)
            {
                FLOAT_VAR(0) = AddNormalizeAngle(g_Rng.GetRandomF32InRange(1.5707964f) + 2.3561945f, 0.0f);
            }
            else
            {
                FLOAT_VAR(0) = g_Rng.GetRandomF32InRange(1.5707964f) - 0.7853982f;
            }
            break;
        case 160:
            g_GameManager.AddBonusPoints(INT_ARG(0));
            break;
        case 161:
            enemy->flags.haltedByBomb = INT_ARG(0);
            break;
        }
    next:
        instr = (EnemyScriptInstr *)((char *)instr + instr->size);
        goto loop;
    }
done:
    switch (enemy->flags.moveMode)
    {
    case 3:
    {
        // Unused, but they take their slots.
        float radius;
        Vector2 polar;

        enemy->orbitAngle = AddNormalizeAngle(enemy->orbitAngle, g_App.gameSpeed * enemy->orbitAngularVelocity);
        enemy->orbitRadius = g_App.gameSpeed * enemy->orbitRadiusVelocity + enemy->orbitRadius;
        polar.SetPolar(enemy->orbitAngle, enemy->orbitRadius);
        enemy->velocity.x = polar.x + enemy->moveStart.x - enemy->pos.x;
        enemy->velocity.y = polar.y + enemy->moveStart.y - enemy->pos.y;
        enemy->angle = atan2f(enemy->velocity.y, enemy->velocity.x);
        if (enemy->moveDuration > 0)
        {
            enemy->moveTimer.Sub(1);
            if (enemy->moveTimer <= 0)
            {
                enemy->flags.moveMode = 0;
            }
        }
        break;
    }
    case 1:
    {
        // Unused, as th06's vec.
        D3DXVECTOR3 vec;

        enemy->angle = AddNormalizeAngle(enemy->angle, g_App.gameSpeed * enemy->angularVelocity);
        enemy->speed = g_App.gameSpeed * enemy->acceleration + enemy->speed;
        ((Vector2 *)&enemy->velocity)->SetPolar(enemy->angle, enemy->speed);
        enemy->velocity.z = 0.0f;
        if (enemy->moveDuration > 0)
        {
            enemy->moveTimer.Sub(1);
            if (enemy->moveTimer <= 0)
            {
                enemy->flags.moveMode = 0;
            }
        }
        break;
    }
    case 2:
    {
        float t;

        enemy->moveTimer.Sub(1);
        t = 1.0f - enemy->moveTimer.AsFloat() / enemy->moveDuration;
        if (t < 0.0f)
        {
            t = 0.0f;
        }
        switch (enemy->flags.moveEasing)
        {
        case 1:
            t = t * t;
            break;
        case 2:
            t = t * t * t;
            break;
        case 3:
            t = t * t * t * t;
            break;
        case 4:
            t = (t = 1.0f - t) * t;
            t = 1.0f - t;
            break;
        case 5:
            t = (t = 1.0f - t) * t * t;
            t = 1.0f - t;
            break;
        case 6:
            t = (t = 1.0f - t) * t * t * t;
            t = 1.0f - t;
            break;
        }
        enemy->velocity = t * enemy->moveDelta + enemy->moveStart - enemy->pos;
        if (enemy->flags.mirrored)
        {
            enemy->velocity.x = -enemy->velocity.x;
        }
        enemy->angle = atan2f(enemy->velocity.y, enemy->velocity.x);
        if (enemy->moveTimer <= 0)
        {
            enemy->flags.moveMode = 0;
            enemy->pos = enemy->moveStart + enemy->moveDelta;
            enemy->velocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        }
        break;
    }
    }
    if (enemy->life > 0)
    {
        if (enemy->shootInterval > 0)
        {
            enemy->shootTimer.Tick();
            if (enemy->shootTimer >= enemy->shootInterval)
            {
                enemy->shooter.pos = enemy->pos + enemy->shootOffset;
                g_BulletManager.FirePattern(&enemy->shooter);
                enemy->shootTimer.Set(0);
            }
        }
        if (enemy->turnLeft >= 0)
        {
            int dir;

            dir = 0;
            if (!enemy->flags.mirrored)
            {
                if (enemy->velocity.x < -0.01f)
                {
                    dir = 1;
                }
                else if (enemy->velocity.x > 0.01f)
                {
                    dir = 2;
                }
            }
            else
            {
                if (enemy->velocity.x < -0.01f)
                {
                    dir = 2;
                }
                else if (enemy->velocity.x > 0.01f)
                {
                    dir = 1;
                }
            }
            if (enemy->spriteDirection != dir)
            {
                switch (dir)
                {
                case 0:
                    if (enemy->spriteDirection == 0xff)
                    {
                        g_SpriteRenderer->StartScript(&enemy->sprite, enemy->turnStill + 0x900);
                    }
                    else if (enemy->spriteDirection == 1)
                    {
                        g_SpriteRenderer->StartScript(&enemy->sprite, enemy->turnStillFromLeft + 0x900);
                    }
                    else
                    {
                        g_SpriteRenderer->StartScript(&enemy->sprite, enemy->turnStillFromRight + 0x900);
                    }
                    break;
                case 1:
                    g_SpriteRenderer->StartScript(&enemy->sprite, enemy->turnLeft + 0x900);
                    break;
                case 2:
                    g_SpriteRenderer->StartScript(&enemy->sprite, enemy->turnRight + 0x900);
                    break;
                }
                enemy->spriteDirection = dir;
            }
        }
        if (enemy->ctx.func != NULL)
        {
            enemy->ctx.func(enemy, enemy->ctx.funcInstr);
        }
        {
            int i;
            BOOL moved;
            float time;
            EnemyScriptLoop *l;
            D3DXVECTOR3 savedPos;

            moved = FALSE;
            l = enemy->ctx.loops;
            savedPos = enemy->pos;
            for (i = 0; i < 8; i++, l++)
            {
                if (l->func != NULL)
                {
                    l->timer.Tick();
                    if (l->timer >= l->duration)
                    {
                        l->timer.Set(l->duration);
                    }
                    time = l->timer.AsFloat() / l->duration;
                    switch (l->mode)
                    {
                    case 1:
                        time = time * time;
                        break;
                    case 2:
                        time = time * time * time;
                        break;
                    case 3:
                        time = time * time * time * time;
                        break;
                    case 4:
                        time = (time = 1.0f - time) * time;
                        time = 1.0f - time;
                        break;
                    case 5:
                        time = (time = 1.0f - time) * time * time;
                        time = 1.0f - time;
                        break;
                    case 6:
                        time = (time = 1.0f - time) * time * time * time;
                        time = 1.0f - time;
                        break;
                    }
                    l->func(enemy, (EnemyScriptInstr *)l, time);
                    if (l->timer >= l->duration)
                    {
                        l->func = NULL;
                    }
                    // A position variable: the move becomes a velocity below.
                    if (l->target == 10018.0f || l->target == 10019.0f || l->target == 10020.0f)
                    {
                        moved = TRUE;
                    }
                }
            }
            if (moved)
            {
                enemy->velocity.x = enemy->pos.x - savedPos.x;
                enemy->velocity.y = enemy->pos.y - savedPos.y;
                enemy->angle = atan2f(enemy->velocity.y, enemy->velocity.x);
                enemy->pos = savedPos;
            }
        }
    }
    enemy->ctx.code = instr;
    enemy->ctx.timer.Tick();
    if (enemy->flags.isBoss && !enemy->bossIndex && g_EnemyManager.spellCardActive && g_EnemyManager.spellCardCapture)
    {
        if (!enemy->flags.keepSpellCard)
        {
            g_EnemyManager.spellCardBonus = g_SpellCardBonuses[g_EnemyManager.spellCardIndex] -
                               g_EnemyManager.spellCardBonusStep * g_EnemyManager.spellCardBonusTimer.AsFloat() / 60.0f;
            g_EnemyManager.spellCardBonus -= g_EnemyManager.spellCardBonus % 10;
        }
        g_EnemyManager.spellCardBonusTimer.Tick();
    }
    if (enemy->flags.isBoss && g_GameManager.stage >= 7)
    {
        if (g_Player.bomb.running && g_EnemyManager.spellCardActive && g_EnemyManager.spellCardIndex >= 0x76)
        {
            enemy->flags.hiddenByBomb = 1;
            enemy->hiddenByBombDelay = 1;
        }
        else if (enemy->hiddenByBombDelay > 0)
        {
            enemy->hiddenByBombDelay--;
        }
        else
        {
            enemy->flags.hiddenByBomb = 0;
        }
    }
    return 0;
}

#ifdef TH07_EXE
#include <new.h>
// Unused: pulls libcmt's new_mode.obj, see UnusedDeleteNothrow (bomb.cpp).
int UnusedSetNewMode(int mode)
{
    return _set_new_mode(mode);
}
#endif // TH07_EXE

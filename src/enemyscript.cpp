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
    ctx->unk_80.Set(0);
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
        return g_Unknown626270.difficulty;
    case 10017:
        return g_Unknown626270.unk_9634;
    case 10025:
        return enemy->callbackTimer.current;
    case 10027:
        return enemy->life;
    case 10028:
        return g_Unknown626270.unk_93d7;
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
        return g_Unknown1347938.globalInts[0];
    case 10038:
        return g_Unknown1347938.globalInts[1];
    case 10039:
        return g_Unknown1347938.globalInts[2];
    case 10040:
        return g_Unknown1347938.globalInts[3];
    case 10041:
        return g_Unknown1347938.globalFloats[0];
    case 10042:
        return g_Unknown1347938.globalFloats[1];
    case 10043:
        return g_Unknown1347938.globalFloats[2];
    case 10044:
        return g_Unknown1347938.globalFloats[3];
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
        return enemy->unk_2b6c;
    case 10053:
        return enemy->unk_2b5c;
    case 10054:
        return enemy->unk_2b60;
    case 10055:
        return g_Rng.GetRandomU32();
    case 10056:
        return g_Rng.GetRandomU32InRange(enemy->ctx.vars.ints3[0]) + enemy->ctx.vars.ints3[1];
    case 10061:
        return enemy->unk_2e4c;
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
        return &g_Unknown626270.difficulty;
    case 10017:
        return &g_Unknown626270.unk_9634;
    case 10025:
        return &enemy->callbackTimer.current;
    case 10027:
        return &enemy->life;
    case 10070:
        return &enemy->item;
    case 10071:
        return &enemy->score;
    case 10037:
        return &g_Unknown1347938.globalInts[0];
    case 10038:
        return &g_Unknown1347938.globalInts[1];
    case 10039:
        return &g_Unknown1347938.globalInts[2];
    case 10040:
        return &g_Unknown1347938.globalInts[3];
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
        return g_Unknown626270.difficulty;
    case 10017:
        return g_Unknown626270.unk_9634;
    case 10025:
        return enemy->callbackTimer.current;
    case 10027:
        return enemy->life;
    case 10028:
        return g_Unknown626270.unk_93d7;
    case 10070:
        return enemy->item;
    case 10071:
        return enemy->score;
    case 10037:
        return g_Unknown1347938.globalInts[0];
    case 10038:
        return g_Unknown1347938.globalInts[1];
    case 10039:
        return g_Unknown1347938.globalInts[2];
    case 10040:
        return g_Unknown1347938.globalInts[3];
    case 10041:
        return g_Unknown1347938.globalFloats[0];
    case 10042:
        return g_Unknown1347938.globalFloats[1];
    case 10043:
        return g_Unknown1347938.globalFloats[2];
    case 10044:
        return g_Unknown1347938.globalFloats[3];
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
        return enemy->unk_2b6c;
    case 10053:
        return enemy->unk_2b5c;
    case 10054:
        return enemy->unk_2b60;
    case 10055:
        return g_Rng.GetRandomF32ZeroToOne();
    case 10056:
        return g_Rng.GetRandomF32InRange(enemy->ctx.vars.floats3[0]) + enemy->ctx.vars.floats3[1];
    case 10060:
        return g_Rng.GetRandomF32ZeroToOne() * 6.2831855f - 3.1415927f;
    case 10062:
        return enemy->bossIndex;
    case 10061:
        return enemy->unk_2e4c;
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
        return &g_Unknown1347938.globalFloats[0];
    case 10042:
        return &g_Unknown1347938.globalFloats[1];
    case 10043:
        return &g_Unknown1347938.globalFloats[2];
    case 10044:
        return &g_Unknown1347938.globalFloats[3];
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
        return &enemy->unk_2b6c;
    case 10053:
        return &enemy->unk_2b5c;
    case 10054:
        return &enemy->unk_2b60;
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
    g_Unknown49fbf0.ShowSpellCardName(*(short *)&instr->args[0], (char *)cardName.text);
    g_BulletManager.ClearAll(1);
    g_Unknown1347b00.spellCardState = 1;
    g_Unknown1347b00.spellCardFrames = 0;
    for (i = 0; i < g_Unknown1347b00.unk_520; i++)
    {
        g_SpriteEngine->StartScript(&g_Unknown1347b00.unk_528[i], i + g_Unknown1347b00.unk_524 + 0x2dc);
    }
    g_Unknown12fe0c8 = 1;
    g_Unknown12fe0c4 = 1;
    g_Unknown12fe0d8 = *((WORD *)&instr->args[0] + 1);
    g_Unknown12fe0cc = g_SpellCardBonuses[g_Unknown12fe0d8];
    g_Unknown12fe0d0 = 0;
    g_Unknown12fe0d4 = g_Unknown12fe0cc / (enemy->timerCallbackTime / 60 + 10);
    g_Unknown9a9b00.unk_9545e0.Set(0);
    enemy->rankSpeedLow = -0.5f;
    enemy->rankSpeedHigh = 0.5f;
    enemy->rankCount1Low = 0;
    enemy->rankCount1High = 0;
    enemy->rankCount2Low = 0;
    enemy->rankCount2High = 0;
    enemy->effects[24] = g_EffectManager.SpawnReservedEffect(0x19, &enemy->pos, 1, 1, 0xffffffff);
    enemy->effects[24]->sprite.unk_48[4].Set(0);
    enemy->effects[24]->sprite.unk_84[4].Set(enemy->timerCallbackTime);
    enemy->effects[24]->sprite.unk_c4 = 0;
    *(D3DXVECTOR2 *)&enemy->effects[24]->sprite.unk_218 = *(D3DXVECTOR2 *)&enemy->effects[24]->sprite.scaleY;
    enemy->effects[24]->sprite.unk_218.z = 0.125f;
    enemy->effects[24]->sprite.unk_224.x = 0.125f;
    enemy->effects[24]->pos = enemy->pos;
    enemy->flags.unk_19 = 0;
    if (!g_Unknown626270.unk_93d8Bit3)
    {
        rec = &g_Unknown626270.catks[g_Unknown12fe0d8];
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
        if (rec->unk_2a != (BYTE)sum)
        {
            for (len = 0; len < 7; len++)
            {
                rec->captures[len] = 0;
                rec->attempts[len] = 0;
                rec->maxBonus[len] = 0;
            }
        }
        if (rec->attempts[g_Unknown626270.unk_93d7] < 9999)
        {
            rec->attempts[g_Unknown626270.unk_93d7]++;
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
        rec->unk_2a = sum2;
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

    if (g_Unknown12fe0c8 != 0)
    {
        g_Unknown49fbf0.EndSpellCardName();
        if (g_Unknown12fe0c8 == 1)
        {
            bonus = g_BulletManager.ClearAllWithBonus(8000, 1);
            bonus = g_Unknown9a9b00.ClearEnemiesForBonus(8000, bonus);
            if (bonus != 0)
            {
                g_Unknown626270.AddScore(bonus);
                g_Unknown49fbf0.ShowBonusPopup(bonus);
            }
            if (g_Unknown12fe0c4 != 0)
            {
                rec = &g_Unknown626270.catks[g_Unknown12fe0d8];
                bonus = g_Unknown12fe0cc + g_Unknown12fe0d0;
                g_Unknown49fbf0.ShowSpellCardBonus(bonus);
                g_Unknown626270.AddScore(bonus);
                if (!g_Unknown626270.unk_93d8Bit3)
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
                    if (rec->unk_2a != (BYTE)sum)
                    {
                        for (len = 0; len < 7; len++)
                        {
                            rec->captures[len] = 0;
                            rec->attempts[len] = 0;
                            rec->maxBonus[len] = 0;
                        }
                    }
                    shot = g_Unknown626270.unk_93d7;
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
                    rec->unk_2a = sum2;
                }
                g_Unknown626270.stats->spellCardsCaptured++;
            }
        }
        g_Unknown12fe0c8 = 0;
        for (bossIdx = 0; bossIdx < 8; bossIdx++)
        {
            if (g_Unknown9a9b00.bosses[bossIdx] != NULL && g_Unknown9a9b00.bosses[bossIdx]->effects[24] != NULL)
            {
                g_Unknown9a9b00.bosses[bossIdx]->effects[24]->active = 0;
                g_Unknown9a9b00.bosses[bossIdx]->effects[24] = NULL;
            }
        }
        g_SoundPlayer.QueueSound(0xf, 0);
    }
    g_Unknown1347b00.spellCardState = 0;
}

// Moves `lo` towards `hi` with the rank: lo at rank 0, hi at rank 32, as
// th06's.
inline int RankLerp(int lo, int hi)
{
    return g_Unknown626270.unk_9634 * (hi - lo) / 32 + lo;
}

inline float RankLerpF(float lo, float hi)
{
    return g_Unknown626270.unk_9634 * (hi - lo) / 32.0f + lo;
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
    if (enemy->unk_2ee4 >= 0)
    {
        enemy->unk_2f5c.Tick();
        if (enemy->unk_2f5c.AsFrames() >= enemy->unk_2f50.AsFrames())
        {
            enemy->unk_2f5c.Set(0);
            enemy->savedCtx[enemy->stackDepth] = enemy->ctx;
            enemy->ctx.vars = enemy->unk_2ee8;
            g_Unknown1347938.StartSub(&enemy->ctx, enemy->unk_2ee4);
            if (enemy->stackDepth < 15)
            {
                enemy->stackDepth++;
            }
            instr = (EnemyScriptInstr *)enemy->ctx.code;
            enemy->ctx.unk_210 = 1;
        }
    }
loop:
    if (enemy->ctx.unk_80.AsFrames() > 0)
    {
        enemy->ctx.unk_80.Sub(1);
        enemy->ctx.timer.Sub(1);
        goto done;
    }
    if (enemy->ctx.timer == instr->time)
    {
        if ((instr->unk_9 & g_Unknown626270.unk_14) == 0)
        {
            goto next;
        }
        switch (instr->opcode)
        {
        case 1:
            return -1;
        case 45:
            enemy->ctx.unk_80.Set(INT_ARG(0));
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
            INT_VAR(0) = (instr->paramMask & 2) ? GetIntOperand(g_Unknown9a9b00.bosses[INT_ARG(2)], instr->args[1].i)
                                                : instr->args[1].i;
            break;
        case 44:
            FLOAT_VAR(0) = (instr->paramMask & 2)
                               ? GetFloatOperand(g_Unknown9a9b00.bosses[INT_ARG(2)], instr->args[1].f)
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

            for (loop = enemy->ctx.unk_8c, i = 0; i < 8; i++, loop++)
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
            if (!enemy->flags.unk_15)
            {
                enemy->savedCtx[enemy->stackDepth] = enemy->ctx;
            }
            g_Unknown1347938.StartSub(&enemy->ctx, value);
            memcpy(enemy->ctx.vars.ints3, g_Unknown1347938.globalInts, sizeof(int) * 8);
            if (!enemy->flags.unk_15 && enemy->stackDepth < 15)
            {
                enemy->stackDepth++;
            }
            goto restart;
        case 42:
            if (enemy->flags.unk_15)
            {
                DebugPrint("error : no Stack Ret\r\n");
            }
            enemy->stackDepth--;
            if (enemy->ctx.unk_210)
            {
                enemy->unk_2ee8 = enemy->ctx.vars;
                enemy->ctx.unk_210 = 0;
            }
            enemy->ctx = enemy->savedCtx[enemy->stackDepth];
            goto restart;
        case 95:
            g_SpriteEngine->StartScript(&enemy->sprite, INT_ARG(0) + 0x900);
            break;
        case 97:
            if (INT_ARG(0) >= 2)
            {
                DebugPrint("error : sub anim overflow\r\n");
            }
            if (INT_ARG(1) >= 0)
            {
                g_SpriteEngine->StartScript(&enemy->sprites[INT_ARG(0)], INT_ARG(1) + 0x900);
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
            if (!g_Unknown12fe0c8)
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
            bs->unk_c2 = 0;
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
            cmd->unk_14 = INT_ARG(2);
            cmd->unk_8 = INT_ARG(3);
            cmd->unk_c = INT_ARG(4);
            cmd->unk_0 = FLOAT_ARG(5);
            cmd->unk_4 = FLOAT_ARG(6);
            break;
        }
        case 98:
            enemy->deathEffect = *(char *)instr->args;
            enemy->unk_2e15 = ((BYTE *)instr->args)[1];
            enemy->unk_2e16 = ((BYTE *)instr->args)[2];
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
                enemy->lasers[value]->unk_4e9 = INT_ARG(1);
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
                enemy->lasers[value]->width = enemy->lasers[value]->unk_4b8;
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
            g_Unknown9a9b00.unk_9545f0 = INT_ARG(0);
            break;
        case 99:
            if (INT_ARG(0) >= 0)
            {
                g_Unknown9a9b00.bosses[INT_ARG(0)] = enemy;
                g_Unknown49fbf0.bossPresent = 1;
                g_Unknown49fbf0.bossHealth = 1.0f;
                enemy->flags.isBoss = 1;
                enemy->bossIndex = INT_ARG(0);
                g_AsciiText.SetUnkB7cInterrupt(enemy->bossIndex, 1);
            }
            else
            {
                if (enemy->bossIndex < 4)
                {
                    g_Unknown49fbf0.bossPresent = 0;
                }
                g_Unknown9a9b00.bosses[enemy->bossIndex] = NULL;
                enemy->flags.isBoss = 0;
                g_AsciiText.SetUnkB7cInterrupt(enemy->bossIndex, 2);
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
            enemy->unk_2b5c = FLOAT_ARG(4);
            enemy->unk_2b60 = FLOAT_ARG(5);
            enemy->unk_2b6c = FLOAT_ARG(6);
            enemy->unk_2b70 = FLOAT_ARG(7);
            enemy->flags.moveMode = 3;
            break;
        case 57:
            enemy->unk_2b6c = FLOAT_ARG(0);
            enemy->unk_2b70 = FLOAT_ARG(1);
            break;
        case 58:
            enemy->unk_2b5c = FLOAT_ARG(0);
            enemy->unk_2b60 = FLOAT_ARG(1);
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
            enemy->unk_2b48.x = FLOAT_ARG(0);
            enemy->unk_2b48.y = FLOAT_ARG(1);
            enemy->unk_2b48.z = FLOAT_ARG(2);
            break;
        case 102:
            enemy->flags.unk_9 = instr->args[0].i;
            break;
        case 103:
            enemy->flags.unk_a = instr->args[0].i;
            break;
        case 104:
            enemy->flags.unk_c = instr->args[0].i;
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
            if (!enemy->flags.unk_15)
            {
                enemy->savedCtx[enemy->stackDepth] = enemy->ctx;
            }
            g_Unknown1347938.StartSub(&enemy->ctx, enemy->interrupts[enemy->pendingInterrupt]);
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
                    g_Unknown49fbf0.unk_54[i] = 0.0f;
                    g_Unknown49fbf0.unk_34[i] = 0.0f;
                }
            }
            break;
        }
        case 139:
        {
            int idx;

            idx = INT_ARG(0);
            g_Unknown49fbf0.SetBossSection(idx, INT_ARG(1) / (float)enemy->maxLife,
                                           INT_ARG(2) / (float)enemy->maxLife);
            g_Unknown49fbf0.SetBossSectionColor(idx, INT_ARG(3));
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
            enemy->unk_2f50.Set(INT_ARG(0));
            enemy->unk_2ee4 = INT_ARG(1);
            enemy->unk_2f5c.Set(0);
            enemy->unk_2ee8 = enemy->ctx.vars;
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
                if ((int)g_Unknown626270.stats->power < 0x80)
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
            g_Unknown1347b00.unk_50f4 = INT_ARG(0);
            break;
        case 126:
            g_Unknown49fbf0.SetBossLives(INT_ARG(0));
            g_Unknown626270.progressFrames += 1800;
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
                child = g_Unknown9a9b00.SpawnWithVars(spawn.sub, &pos, INT_ARG(4), INT_ARG(5), INT_ARG(6),
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
                child = g_Unknown9a9b00.SpawnWithVars(spawn.sub, &pos, INT_ARG(4), INT_ARG(5), INT_ARG(6),
                                                         &enemy->ctx.vars);
            }
            break;
        }
        case 94:
            g_Unknown9a9b00.ClearEnemiesForBonus(8000, 0);
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
            enemy->shooter.unk_cc = INT_ARG(1);
            break;
        case 130:
            enemy->flags.unk_15 = instr->args[0].i;
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
            enemy->flags.unk_b = instr->args[0].i;
            break;
        case 133:
            enemy->timerCallbackSub = enemy->deathSub;
            enemy->callbackTimer.Set(0);
            break;
        case 135:
            enemy->flags.keepSpellCard = instr->args[0].i;
            break;
        case 136:
            enemy->flags.unk_d = instr->args[0].i;
            enemy->unk_2e2f = 2;
            break;
        case 137:
            enemy->flags.unk_17 = instr->args[0].i;
            break;
        case 138:
            enemy->unk_4f30 = instr->args[0].i;
            enemy->unk_4f32 = INT_ARG(1);
            enemy->unk_4f34 = INT_ARG(2);
            enemy->unk_4f36 = INT_ARG(3);
            if (enemy->unk_4f30 & 8)
            {
                g_SpriteEngine->FillSpriteStripVertices(&enemy->sprite, (VertexTex1DiffuseXyzrwh *)enemy->unk_39f8,
                                                        enemy->unk_4f32 / enemy->unk_4f36 * 2);
            }
            break;
        case 140:
            g_EffectManager.colorScale[0] = FLOAT_ARG(0);
            g_EffectManager.colorScale[1] = FLOAT_ARG(1);
            g_EffectManager.colorScale[2] = FLOAT_ARG(2);
            g_EffectManager.colorScale[3] = FLOAT_ARG(3);
            break;
        case 142:
            enemy->unk_4f38.Set(INT_ARG(0));
            break;
        case 143:
            g_BulletManager.ConvertBulletsToItems(&enemy->pos, FLOAT_ARG(0));
            break;
        case 145:
            if (g_Unknown9a9b00.bosses[INT_ARG(0)] != NULL)
            {
                g_Unknown9a9b00.bosses[INT_ARG(0)]->pendingInterrupt = INT_ARG(1);
            }
            break;
        case 146:
            g_BulletManager.ClearAll(0);
            break;
        case 149:
            enemy->flags.unk_19 = INT_ARG(0);
            if (!enemy->flags.unk_19)
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
            g_Unknown626270.AddBonusPoints(INT_ARG(0));
            break;
        case 161:
            enemy->flags.unk_1b = INT_ARG(0);
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

        enemy->unk_2b5c = AddNormalizeAngle(enemy->unk_2b5c, g_App.gameSpeed * enemy->unk_2b60);
        enemy->unk_2b6c = g_App.gameSpeed * enemy->unk_2b70 + enemy->unk_2b6c;
        polar.SetPolar(enemy->unk_2b5c, enemy->unk_2b6c);
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
                        g_SpriteEngine->StartScript(&enemy->sprite, enemy->turnStill + 0x900);
                    }
                    else if (enemy->spriteDirection == 1)
                    {
                        g_SpriteEngine->StartScript(&enemy->sprite, enemy->turnStillFromLeft + 0x900);
                    }
                    else
                    {
                        g_SpriteEngine->StartScript(&enemy->sprite, enemy->turnStillFromRight + 0x900);
                    }
                    break;
                case 1:
                    g_SpriteEngine->StartScript(&enemy->sprite, enemy->turnLeft + 0x900);
                    break;
                case 2:
                    g_SpriteEngine->StartScript(&enemy->sprite, enemy->turnRight + 0x900);
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
            l = enemy->ctx.unk_8c;
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
    if (enemy->flags.isBoss && !enemy->bossIndex && g_Unknown12fe0c8 && g_Unknown12fe0c4)
    {
        if (!enemy->flags.keepSpellCard)
        {
            g_Unknown12fe0cc = g_SpellCardBonuses[g_Unknown12fe0d8] -
                               g_Unknown12fe0d4 * g_Unknown9a9b00.unk_9545e0.AsFloat() / 60.0f;
            g_Unknown12fe0cc -= g_Unknown12fe0cc % 10;
        }
        g_Unknown9a9b00.unk_9545e0.Tick();
    }
    if (enemy->flags.isBoss && g_Unknown626270.stage >= 7)
    {
        if (g_Player.bomb.running && g_Unknown12fe0c8 && g_Unknown12fe0d8 >= 0x76)
        {
            enemy->flags.unk_1a = 1;
            enemy->unk_2e2c = 1;
        }
        else if (enemy->unk_2e2c > 0)
        {
            enemy->unk_2e2c--;
        }
        else
        {
            enemy->flags.unk_1a = 0;
        }
    }
    return 0;
}

void WarpToBoss(Enemy *enemy, EnemyScriptInstr *instr)
{
    int boss;

    boss = instr->args[1].i;
    enemy->pos = g_Unknown9a9b00.bosses[boss]->pos;
    enemy->velocity = g_Unknown9a9b00.bosses[boss]->velocity;
    enemy->angle = g_Unknown9a9b00.bosses[boss]->angle;
    enemy->flags.unk_18 = 1;
}

void ShakeAndSpinBullets(Enemy *enemy, EnemyScriptInstr *instr)
{
    int i;
    Bullet *curBullet;
    float angle;

    curBullet = g_BulletManager.bullets;
    ScreenEffect::StartEffect(1, 0x1e, 0xc, 0, 0);
    ScreenEffect::StartEffect(3, 4, 3, 0x80ffcfcf, 0);
    for (i = 0; i < 0x400; i++, curBullet++)
    {
        if (curBullet->state == 0 || curBullet->state == 5)
        {
            continue;
        }
        if (curBullet->sprites.sprite0.spriteData != NULL && curBullet->unk_c08 == 0)
        {
            if (instr->args[1].i == 1 && curBullet->color != 8)
            {
                continue;
            }
            if (instr->args[1].i == 2 && curBullet->color != 4)
            {
                continue;
            }
            if (curBullet->color == 2)
            {
                angle = -3.1415927f / (g_Rng.GetRandomF32ZeroToOne() * 60.0f + 180.0f);
            }
            else if (curBullet->color == 6)
            {
                angle = 3.1415927f / (g_Rng.GetRandomF32ZeroToOne() * 60.0f + 180.0f);
            }
            else if (curBullet->color == 8)
            {
                angle = 3.1415927f / (g_Rng.GetRandomF32ZeroToOne() * 60.0f + 180.0f);
            }
            else if (curBullet->color == 4)
            {
                angle = -3.1415927f / (g_Rng.GetRandomF32ZeroToOne() * 60.0f + 180.0f);
            }
            curBullet->speed = 0.3f;
            memset(curBullet->commands, 0, sizeof(curBullet->commands));
            if (g_Unknown626270.difficulty < 3)
            {
                curBullet->SetTurnCommand(0, 0, 60, angle, 1.0f / 60.0f);
            }
            else
            {
                curBullet->SetTurnCommand(0, 0, 240, angle, 1.0f / 190.0f);
            }
            curBullet->unk_c08 = 1;
        }
    }
}

void SpawnFansFromNearbyBullets(Enemy *enemy, EnemyScriptInstr *instr)
{
    int variant;
    Bullet *bullet = g_BulletManager.bullets;
    float dist;
    float range;
    int i;
    BulletShooter shooter;

    variant = instr->args[1].i;
    switch (variant)
    {
    case 0:
        ScreenEffect::StartEffect(1, 0x20, 0xc, 0, 0);
        ScreenEffect::StartEffect(3, 4, 1, 0x80cfcfff, 0);
        range = 128.0f;
        break;
    case 1:
        range = 192.0f;
        break;
    case 2:
        range = 256.0f;
        break;
    case 3:
        range = 999.0f;
        break;
    }
    for (i = 0; i < 0x400; i++, bullet++)
    {
        if (bullet->state == 0 || bullet->state == 5)
        {
            continue;
        }
        if (bullet->sprites.sprite0.spriteData != NULL && bullet->color == 2)
        {
            dist = sqrtf((enemy->pos.x - bullet->pos.x) * (enemy->pos.x - bullet->pos.x) +
                         (enemy->pos.y - bullet->pos.y) * (enemy->pos.y - bullet->pos.y));
            if (dist < range)
            {
                shooter.pos = bullet->pos;
                shooter.sprite = 0;
                shooter.color = 6;
                shooter.angle1 = 0.0f;
                shooter.angle2 = -3.1415927f;
                shooter.speed1 = 0.7f;
                shooter.count1 = 2;
                shooter.count2 = 1;
                shooter.flags = 2;
                shooter.aimMode = 6;
                shooter.SetShooterAccelerateCommand(0, 0, 180, g_Rng.GetRandomF32InRange(0.005f) + 0.013f, 1.5707964f);
                g_BulletManager.FirePattern(&shooter);
                bullet->FreeBullet();
            }
        }
    }
}

void NoOp(Enemy *enemy, EnemyScriptInstr *instr)
{
}

void ReplaceTallBulletWithEffect(Enemy *enemy, EnemyScriptInstr *instr)
{
    int variant;
    Bullet *bullet = g_BulletManager.bullets;
    int i;
    BulletShooter shooter;

    variant = instr->args[1].i;
    enemy->ctx.vars.floats[0] = -999.0f;
    for (i = 0; i < 0x400; i++, bullet++)
    {
        if (bullet->state == 0 || bullet->state == 5)
        {
            continue;
        }
        if (bullet->sprites.sprite0.spriteData->height >= 60.0f)
        {
            enemy->ctx.vars.floats[0] = bullet->pos.x;
            enemy->ctx.vars.floats[1] = bullet->pos.y;
            g_EffectManager.SpawnEffect(2, &bullet->pos, 1, 0xffffffff);
            bullet->FreeBullet();
            break;
        }
    }
}

void CopyBossMoveStart(Enemy *enemy, EnemyScriptInstr *instr)
{
    Enemy *boss;

    boss = g_Unknown9a9b00.bosses[0];
    enemy->moveStart = boss->pos;
    enemy->unk_2b6c = boss->unk_2b6c;
    enemy->unk_2b60 = boss->unk_2b60;
    switch (boss->ctx.vars.ints2[0])
    {
    }
}

void FireBackwardFans(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    int i;
    BulletShooter shooter;

    for (i = 0; i < 0x400; i++, bullet++)
    {
        if (bullet->state == 0 || bullet->state == 5)
        {
            continue;
        }
        if (bullet->sprites.sprite0.spriteData != NULL &&
            ((instr->args[1].i == 0 && bullet->color == 6) || (instr->args[1].i == 1 && bullet->color == 15) ||
             (instr->args[1].i == 2 && bullet->color == 2)))
        {
            shooter.pos = bullet->pos;
            shooter.sprite = 6;
            shooter.color = 15;
            shooter.angle1 = AddNormalizeAngle(bullet->angle, 3.1415927f);
            shooter.angle2 = 0.5235988f;
            shooter.speed1 = bullet->speed * 1.1f;
            if (g_Unknown626270.difficulty < 3)
            {
                shooter.count1 = 4;
            }
            else
            {
                shooter.count1 = 2;
                shooter.angle2 = 1.5707964f;
            }
            shooter.count2 = 1;
            shooter.flags = 2;
            shooter.SetHiddenCommand(0, 0, 0x82);
            if (instr->args[1].i == 0)
            {
                shooter.flags = 0x2002;
            }
            else if (instr->args[1].i == 1)
            {
                if (g_Unknown626270.difficulty != 3)
                {
                    shooter.flags = 2;
                }
                else
                {
                    shooter.flags = 0x2002;
                }
                shooter.color = 2;
            }
            else if (instr->args[1].i == 2)
            {
                shooter.flags = 2;
                shooter.color = 10;
            }
            shooter.aimMode = 1;
            g_BulletManager.FirePattern(&shooter);
            shooter.angle2 = 1.0471976f;
            if (instr->args[1].i == 0)
            {
                shooter.flags = 0x2000;
            }
            else if (instr->args[1].i == 1)
            {
                if (g_Unknown626270.difficulty != 3)
                {
                    shooter.flags = 0;
                }
                else
                {
                    shooter.flags = 0x2000;
                }
            }
            else
            {
                shooter.flags = 0;
            }
            shooter.speed1 = bullet->speed * 0.7f;
            shooter.count1 = 2;
            g_BulletManager.FirePattern(&shooter);
            shooter.speed1 = bullet->speed * 0.85f;
            shooter.count1 = 1;
            g_BulletManager.FirePattern(&shooter);
            bullet->FreeBullet();
        }
    }
}

int PointInRotatedBox(D3DXVECTOR3 *point, D3DXVECTOR3 *center, D3DXVECTOR3 *size, D3DXVECTOR3 *origin,
               float sinAngle, float cosAngle)
{
    D3DXVECTOR3 low;
    D3DXVECTOR3 hi;
    D3DXVECTOR3 rotated;

    low = *point - *origin;
    hi.x = low.x * cosAngle + low.y * sinAngle;
    hi.y = low.y * cosAngle - low.x * sinAngle;
    rotated = hi + *origin;
    low = *center - *size / 2.0f;
    hi = *size / 2.0f + *center;
    if (rotated.x > hi.x || rotated.x < low.x || rotated.y > hi.y || rotated.y < low.y)
    {
        return 0;
    }
    return 1;
}

void UpdateLaserEdgeBullets(Enemy *enemy, EnemyScriptInstr *instr)
{
    D3DXVECTOR3 middle;
    float c;
    int li;
    Laser *l;
    Bullet *blt;
    D3DXVECTOR3 size;
    float proj;
    float s;
    int n;

    l = g_BulletManager.lasers;
    for (li = 0; li < 0x40; li++, l++)
    {
        if (l->active == 0)
        {
            continue;
        }
        if (enemy->callbackTimer.current % 2 != li)
        {
            continue;
        }
        if (l->state < 2)
        {
            size.y = l->width;
            size.x = l->endOffset - l->startOffset;
            middle.x = (l->endOffset - l->startOffset) / 2.0f + l->startOffset + l->pos.x;
            middle.y = l->pos.y;
            SinCos(&s, &c, l->angle);
            blt = g_BulletManager.bullets;
            for (n = 0; n < 0x400; n++, blt++)
            {
                if (blt->state == 0 || blt->state == 5)
                {
                    continue;
                }
                if (blt->sprites.sprite0.spriteData != NULL && PointInRotatedBox(&blt->pos, &middle, &size, &l->pos, s, c))
                {
                    if (blt->unk_c08 > 0)
                    {
                        blt->unk_c08--;
                    }
                    if (blt->unk_c08 == 0)
                    {
                        if (blt->speed > 0.5f)
                        {
                            blt->speed -= 0.1f;
                        }
                        proj = c * blt->velocity.y + s * blt->velocity.x;
                        if (proj >= 0.0f)
                        {
                            blt->angle = AddNormalizeAngle(l->angle, 1.5707964f);
                        }
                        else
                        {
                            blt->angle = AddNormalizeAngle(l->angle, -1.5707964f);
                        }
                        ((Vector2 *)&blt->velocity)->SetPolar(blt->angle, g_App.gameSpeed * blt->speed);
                        blt->unk_c08 = 10;
                        blt->sprites = g_BulletManager.typeSprites[5];
                        g_SpriteEngine->SetSprite(&blt->sprites.sprite0, blt->sprites.sprite0.spriteIndex + blt->color);
                    }
                }
            }
        }
    }
}

void DeflectBulletsOffLasers(Enemy *enemy, EnemyScriptInstr *instr)
{
    D3DXVECTOR3 middle;
    float c;
    int li;
    Laser *l;
    float nx;
    Bullet *blt;
    D3DXVECTOR3 size;
    float proj;
    float s;
    int n;
    float ny;

    l = g_BulletManager.lasers;
    for (li = 0; li < 0x40; li++, l++)
    {
        if (l->active == 0)
        {
            continue;
        }
        if (enemy->callbackTimer % 3 != li % 3)
        {
            continue;
        }
        if (l->state < 2)
        {
            size.y = l->width * 1.5f;
            size.x = l->endOffset - l->startOffset;
            middle.x = (l->endOffset - l->startOffset) / 2.0f + l->startOffset + l->pos.x;
            middle.y = l->pos.y;
            SinCos(&s, &c, l->angle);
            nx = -s;
            ny = c;
            blt = g_BulletManager.bullets;
            for (n = 0; n < 0x400; n++, blt++)
            {
                if (blt->state == 0 || blt->state == 5)
                {
                    continue;
                }
                if (blt->sprites.sprite0.spriteData != NULL && blt->unk_c08 != li + 1 && blt->unk_c08 >= 0 &&
                    PointInRotatedBox(&blt->pos, &middle, &size, &l->pos, s, c))
                {
                    if (g_Unknown626270.difficulty < 2)
                    {
                        blt->speed *= g_Rng.GetRandomF32ZeroToOne() * 0.3f + 0.7f;
                    }
                    else
                    {
                        blt->speed *= g_Rng.GetRandomF32ZeroToOne() * 0.4f + 0.8f;
                    }
                    proj = nx * blt->velocity.x + ny * blt->velocity.y;
                    if (proj >= 0.0f)
                    {
                        blt->velocity.x = nx;
                        blt->velocity.y = ny;
                    }
                    else
                    {
                        blt->velocity.x = -nx;
                        blt->velocity.y = -ny;
                    }
                    blt->sprites = g_BulletManager.typeSprites[5];
                    g_SpriteEngine->SetSprite(&blt->sprites.sprite0, blt->sprites.sprite0.spriteIndex + blt->color);
                    blt->angle = atan2f(blt->velocity.y, blt->velocity.x);
                    ((Vector2 *)&blt->velocity)->SetPolar(blt->angle, blt->speed);
                    if (g_Unknown626270.difficulty < 2)
                    {
                        blt->unk_c08 = -1;
                    }
                    else
                    {
                        blt->unk_c08 = li + 1;
                    }
                }
            }
        }
    }
}

void ShakeAndStartFallingPetals(Enemy *enemy, EnemyScriptInstr *instr)
{
    ScreenEffect::StartEffect(1, 0x50, 8, 0, 0);
    StartPetalsFalling();
}

void StartSlowdown(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet;
    int i;

    g_App.gameSpeed = 1.0f / instr->args[1].i;
    g_Unknown1347b00.unk_528[0].pendingInterrupt = 2;
    g_Unknown1347b00.unk_528[1].pendingInterrupt = 2;
    bullet = g_BulletManager.bullets;
    for (i = 0; i < 0x400; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        bullet->velocity *= g_App.gameSpeed;
        bullet->sprites.sprite0.baseSpriteIndex = bullet->sprites.sprite0.spriteIndex;
        if (bullet->sprites.sprite0.spriteIndex >= 0x260 && bullet->sprites.sprite0.spriteIndex <= 0x26f)
        {
            g_SpriteEngine->SetSprite(&bullet->sprites.sprite0, 0x26f);
        }
    }
}

void EndSlowdown(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    float scale = 1.0f / g_App.gameSpeed;
    int i;

    for (i = 0; i < 0x400; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        bullet->velocity *= scale;
        if (bullet->sprites.sprite0.spriteIndex >= 0x260 && bullet->sprites.sprite0.spriteIndex <= 0x26f)
        {
            g_SpriteEngine->SetSprite(&bullet->sprites.sprite0, bullet->sprites.sprite0.baseSpriteIndex);
        }
    }
    g_App.gameSpeed = 1.0f / instr->args[1].i;
    if (g_App.gameSpeed < 1.0f)
    {
        g_App.stateFlags |= 0x20;
    }
    g_App.gameSpeed = 1.0f;
    g_Unknown1347b00.unk_528[0].pendingInterrupt = 1;
    g_Unknown1347b00.unk_528[1].pendingInterrupt = 1;
}

void BurstBulletsByDifficulty(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    int i;
    int count;
    BulletShooter shooter;
    int j;

    ScreenEffect::StartEffect(3, 8, 1, 0x50cfcfff, 0);
    count = g_Unknown626270.difficulty == 0   ? 10
            : g_Unknown626270.difficulty == 1 ? 18
            : g_Unknown626270.difficulty == 2 ? 22
                                              : 25;
    for (i = 0; i < 0x400; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if ((g_Unknown626270.difficulty < 2 && bullet->sprites.sprite0.spriteData->height > 48.0f &&
             enemy->pos.y - 64.0f < bullet->pos.y && enemy->pos.y + 64.0f > bullet->pos.y) ||
            (g_Unknown626270.difficulty >= 2 && bullet->sprites.sprite0.spriteData->height > 48.0f &&
             enemy->pos.y - 48.0f < bullet->pos.y && enemy->pos.y + 48.0f > bullet->pos.y))
        {
            for (j = 0; j < count; j++)
            {
                shooter.pos = bullet->pos;
                shooter.pos.x += g_Rng.GetRandomF32ZeroToOne() * 32.0f - 16.0f;
                shooter.pos.y += g_Rng.GetRandomF32ZeroToOne() * 32.0f - 16.0f;
                switch (g_Rng.GetRandomU16InRange(3))
                {
                case 0:
                    shooter.sprite = 0;
                    shooter.color = 2;
                    break;
                case 1:
                    shooter.sprite = 3;
                    shooter.color = 2;
                    break;
                case 2:
                    shooter.sprite = 7;
                    shooter.color = 1;
                    break;
                }
                if (instr->args[1].i == 0)
                {
                    shooter.angle1 = g_Rng.GetRandomF32ZeroToOne() * 4.712389f - 1.5707964f;
                }
                else
                {
                    shooter.angle1 = AddNormalizeAngle(g_Rng.GetRandomF32ZeroToOne() * 4.712389f, 0.7853982f);
                }
                shooter.speed1 = 0.1f;
                shooter.count1 = 1;
                shooter.count2 = 1;
                shooter.flags = (j & 1) ? 2 : 0;
                shooter.aimMode = 1;
                shooter.SetShooterTurnCommand(0, 0, 100, 0.0f, g_Rng.GetRandomF32ZeroToOne() * 0.008f + 0.01f);
                g_BulletManager.FirePattern(&shooter);
            }
            bullet->FreeBullet();
        }
    }
}

void AimTurnedBulletsAtPlayer(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    int i;

    ScreenEffect::StartEffect(3, 0x10, 1, 0x50cfcfff, 0);
    for (i = 0; i < 0x400; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if (bullet->unk_c08 == 1)
        {
            bullet->SetAccelerateCommand(0, 0, 0x5a, 0.026666667f, g_Player.AngleFrom(&bullet->pos));
            bullet->ClearCommandFlag(1);
            bullet->unk_c08 = 2;
        }
    }
}

void FlashScreen(Enemy *enemy, EnemyScriptInstr *instr)
{
    ScreenEffect::StartEffect(3, instr->args[1].i, 1, 0xd0cfcfff, 0);
}

void BurstBulletsFixedCount(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    int i;
    BulletShooter shooter;
    float w;
    int j;

    w = g_Unknown626270.difficulty == 2 ? 128.0f : 180.0f;
    ScreenEffect::StartEffect(3, 8, 1, 0x50cfcfff, 0);
    for (i = 0; i < 0x400; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if (bullet->sprites.sprite0.spriteData->height > 48.0f && enemy->pos.y - w < bullet->pos.y &&
            w + enemy->pos.y > bullet->pos.y)
        {
            for (j = 0; j < 15; j++)
            {
                shooter.pos = bullet->pos;
                shooter.pos.x += g_Rng.GetRandomF32ZeroToOne() * 32.0f - 16.0f;
                shooter.pos.y += g_Rng.GetRandomF32ZeroToOne() * 32.0f - 16.0f;
                switch (g_Rng.GetRandomU16InRange(3))
                {
                case 0:
                    shooter.sprite = 0;
                    shooter.color = 4;
                    break;
                case 1:
                    shooter.sprite = 3;
                    shooter.color = 4;
                    break;
                case 2:
                    shooter.sprite = 7;
                    shooter.color = 2;
                    break;
                }
                if (instr->args[1].i == 0)
                {
                    shooter.angle1 = g_Rng.GetRandomF32ZeroToOne() * 4.712389f - 1.5707964f;
                }
                else
                {
                    shooter.angle1 = AddNormalizeAngle(g_Rng.GetRandomF32ZeroToOne() * 4.712389f, 0.7853982f);
                }
                shooter.speed1 = 0.1f;
                shooter.count1 = 1;
                shooter.count2 = 1;
                shooter.flags = 0;
                shooter.aimMode = 1;
                shooter.SetShooterTurnCommand(0, 0, 100, 0.0f, g_Rng.GetRandomF32ZeroToOne() * 0.008f + 0.01f);
                g_BulletManager.FirePattern(&shooter);
            }
            bullet->FreeBullet();
        }
    }
}

void ScriptFadeOutMusic(Enemy *enemy, EnemyScriptInstr *instr)
{
    g_App.FadeOutMusic(3.0f);
}

void ScriptPlayMusic(Enemy *enemy, EnemyScriptInstr *instr)
{
    if (g_App.PlayMidi(2))
    {
        g_App.PlayMusic("bgm/th07_13b.mid");
    }
}

void TurnNearbyBullets(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    int i;

    for (i = 0; i < 0x400; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if (bullet->unk_c08 == 0 && bullet->pos.y > enemy->pos.y && bullet->pos.y < 352.0f &&
            enemy->pos.x - 16.0f < bullet->pos.x && enemy->pos.x + 16.0f > bullet->pos.x)
        {
            bullet->SetTurnCommand(0, 0, 0xa0, (i & 1) ? 0.05235988f : -0.05235988f, -bullet->speed / 180.0f);
            bullet->unk_c08 = 1;
        }
    }
}

void FireBackFromUnturned(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    BulletShooter pattern;
    int i;

    for (i = 0; i < 0x400; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if (bullet->unk_c08 == 0 && bullet->sprites.sprite0.spriteIndex >= 0x278 &&
            bullet->sprites.sprite0.spriteIndex <= 0x27f)
        {
            pattern.pos = bullet->pos;
            pattern.sprite = 0;
            pattern.color = 6;
            pattern.angle1 = AddNormalizeAngle(bullet->angle, 3.1415927f);
            pattern.angle2 = 0.3926991f;
            pattern.speed1 = enemy->ctx.vars.floats[1];
            pattern.count1 = 5;
            pattern.count2 = 1;
            pattern.flags = 2;
            pattern.aimMode = 1;
            g_BulletManager.FirePattern(&pattern);
        }
    }
}

void ReplaceBulletsWithEnemies(Enemy *enemy, EnemyScriptInstr *instr)
{
    EnemyScriptVars childVars;
    Bullet *blt = g_BulletManager.bullets;
    int i;
    float theta;

    childVars = enemy->ctx.vars;
    theta = -3.1415927f;

    ScreenEffect::StartEffect(3, 0xc, 1, 0x80cfcfff, 0);
    for (i = 0; i < 0x400; i++, blt++)
    {
        if (blt->state == 0)
        {
            continue;
        }
        if (blt->unk_c08 == 0 && blt->sprites.sprite0.spriteIndex == 0x27c)
        {
            childVars.floats[0] = blt->angle;
            childVars.floats[7] = theta;
            theta += 0.7853982f;
            g_Unknown9a9b00.SpawnWithVars(enemy->ctx.sub + 1, &blt->pos, 1, -2, 10, &childVars);
            blt->FreeBullet();
        }
        else if (blt->sprites.sprite0.spriteIndex >= 0x278 && blt->sprites.sprite0.spriteIndex <= 0x27f)
        {
            blt->FreeBullet();
        }
    }
}

void CountUnturnedBullets(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    int i;

    enemy->ctx.vars.ints[0] = 0;
    for (i = 0; i < 0x400; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if (bullet->unk_c08 == 0 && bullet->sprites.sprite0.spriteIndex == 0x27c)
        {
            enemy->ctx.vars.ints[0]++;
        }
    }
}

void FireAimedBulletPattern1(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    BulletShooter pattern;
    int variant;
    int i;

    if (enemy->callbackTimer.current % 3 == 0)
    {
        return;
    }
    variant = enemy->callbackTimer.current % 2;
    for (i = 0; i < 0x400; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if ((bullet->activeFlags & 0x40) == 0 && bullet->pos.y < 320.0f &&
            bullet->sprites.sprite0.spriteData->height > 60.0f)
        {
            pattern.pos = bullet->pos;
            if (variant != 0)
            {
                pattern.sprite = 1;
            }
            else
            {
                pattern.sprite = 3;
            }
            pattern.color = bullet->color == 1 ? 6 : 2;
            pattern.angle1 = g_Rng.GetRandomF32ZeroToOne() * 6.2831855f - 3.1415927f;
            pattern.angle2 = -3.1415927f;
            if (variant != 0)
            {
                pattern.speed1 = 1.2f;
            }
            else
            {
                pattern.speed1 = 0.8f;
            }
            if (variant != 0)
            {
                pattern.count1 = 1;
            }
            else
            {
                pattern.count1 = 2;
            }
            pattern.count2 = 1;
            pattern.flags = 0x208;
            pattern.aimMode = 3;
            pattern.unk_cc = 0x19;
            if (variant != 0)
            {
                pattern.SetStopTurnCommand(0, 0, 0x3c, 1, 0.0f, 3.1f);
            }
            g_BulletManager.FirePattern(&pattern);
        }
    }
}

void FireAimedBulletPattern2(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    BulletShooter pattern;
    int variant;
    int i;

    if (enemy->callbackTimer.current % 3 == 2)
    {
        return;
    }
    variant = enemy->callbackTimer.current % 3;
    for (i = 0; i < 0x400; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if ((bullet->activeFlags & 0x40) == 0 && bullet->pos.y < 320.0f &&
            bullet->sprites.sprite0.spriteData->height > 60.0f)
        {
            pattern.pos = bullet->pos;
            if (variant != 0)
            {
                pattern.sprite = 1;
            }
            else
            {
                pattern.sprite = 3;
            }
            pattern.color = bullet->color == 2 ? 10 : 13;
            pattern.angle1 = g_Rng.GetRandomF32ZeroToOne() * 6.2831855f - 3.1415927f;
            pattern.angle2 = -3.1415927f;
            if (variant != 0)
            {
                pattern.speed1 = 1.2f;
            }
            else
            {
                pattern.speed1 = 0.8f;
            }
            if (variant != 0)
            {
                pattern.count1 = 1;
            }
            else
            {
                pattern.count1 = 1;
            }
            pattern.count2 = 1;
            pattern.flags = 0x208;
            pattern.aimMode = 3;
            pattern.unk_cc = 0x19;
            if (variant != 0)
            {
                pattern.SetStopTurnCommand(0, 0, 0x28, 1, 0.0f, 2.9f);
            }
            g_BulletManager.FirePattern(&pattern);
        }
    }
}

// As in bullet.cpp: defined after its callers, so they call the copy.
inline void Bullet::FreeBullet()
{
    this->state = 0;
    this->timer.Set(0);
    this->grazeDelayTimer.Set(0);
}

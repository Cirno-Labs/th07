// The file with Unknown1347938, th06's EnemyScripts, from 0x40e420, as
// th06's enemyscript.cpp.

#include <math.h>
#include <string.h>

#include "ascii.h"
#include "bullet.h"
#include "effect.h"
#include "enemy.h"
#include "player.h"
#include "screeneffect.h"
#include "unsorted.h"

// Argument `n` of `instr`, or the value of the variable it names.
#define INT_ARG(n) \
    ((instr->paramMask & (1 << (n))) ? GetIntOperand(enemy, instr->args[n].i) : instr->args[n].i)
#define FLOAT_ARG(n) \
    ((instr->paramMask & (1 << (n))) ? GetFloatOperand(enemy, instr->args[n].f) : instr->args[n].f)

int Unknown1347938::Load(char *path)
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

void Unknown1347938::Release()
{
    if (this->file != NULL)
    {
        MemFree(this->file);
    }
    this->file = NULL;
}

int Unknown1347938::StartSub(EnemyScriptState *ctx, short sub)
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

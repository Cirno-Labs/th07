// The file with Unknown1347938, th06's EnemyScripts, from 0x40e420, as
// th06's enemyscript.cpp.

#include <math.h>

#include "enemy.h"
#include "player.h"
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

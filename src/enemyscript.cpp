// The file with Unknown1347938, th06's EnemyScripts, from 0x40e420, as
// th06's enemyscript.cpp.

#include "enemy.h"
#include "unsorted.h"

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

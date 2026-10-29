// cflags: /Os
// The /Os file with Unknown626270, probably th07's GameManager, from
// 0x42d560 to the Chain code at 0x42fa60.

#include <string.h>

#include "unsorted.h"

Unknown626270::Unknown626270()
{
    memset(this, 0, sizeof(Unknown626270));
    this->arena[0] = 32.0f;
    this->arena[1] = 16.0f;
    this->arena[2] = 384.0f;
    this->arena[3] = 448.0f;
    this->demoIndex = 2;
    this->unk_9624 = 1;
}

void Unknown626270::AddLives(int lives)
{
    if (this->Func404fe0())
    {
        memset(&g_App, 0xff, 0x2d0);
    }
    this->stats->lives += lives;
    this->Func4012b0();
}

void Unknown626270::AddBombs(int bombs)
{
    if (this->Func404fe0())
    {
        memset(&g_App, 0xff, 0x2d0);
    }
    this->stats->bombs += bombs;
    this->Func4012b0();
}

void Unknown626270::SetPlayArea()
{
    this->arena[0] = 32.0f;
    this->arena[1] = 16.0f;
    this->arena[2] = 384.0f;
    this->arena[3] = 448.0f;
    this->unk_9604[0] = 8.0f;
    this->unk_9604[1] = 16.0f;
    this->unk_9604[2] = 368.0f;
    this->unk_9604[3] = 416.0f;
}

BOOL Unknown626270::IsInPlayArea(float x, float y, float width, float height)
{
    if (x + width / 2.0f < 0.0f)
    {
        return FALSE;
    }
    if (x - width / 2.0f > 384.0f)
    {
        return FALSE;
    }
    if (y + height / 2.0f < 0.0f)
    {
        return FALSE;
    }
    if (y - height / 2.0f > 448.0f)
    {
        return FALSE;
    }
    return TRUE;
}

int ChecksumBytes(BYTE *data, int size)
{
    int i;
    int sum;

    sum = 0;
    for (i = 0; i < size; i++, data++)
    {
        sum += *data;
        g_Unknown626270.stats->unk_ac += g_Unknown626270.stats->unk_b4[2];
    }
    return sum;
}

int Unknown626270::ComputeChecksum()
{
    int sum;

    sum = ChecksumBytes((BYTE *)g_Unknown626270.stats->unk_34, (BYTE *)&this->stats->unk_ac - (BYTE *)this->stats->unk_34);
    sum += ChecksumBytes((BYTE *)g_Unknown626270.stats->unk_b4, sizeof(g_Unknown626270.stats->unk_b4));
    sum += ChecksumBytes((BYTE *)g_Unknown626270.unk_4, 0x38);
    sum += ChecksumBytes((BYTE *)&g_App.controllerMapping, 0x38);
    return sum;
}

void Unknown626270::AddLifeOrBomb()
{
    if ((int)this->stats->lives < 8)
    {
        this->AddLives(1);
        g_SoundPlayer.QueueSound(0x1c, 0);
        this->AddSubRank(200);
        g_Unknown49fbf0.livesChanged = 2;
    }
    else if ((int)this->stats->bombs < 8)
    {
        this->AddBombs(1);
        g_SoundPlayer.QueueSound(0x1c, 0);
        this->AddSubRank(200);
        g_Unknown49fbf0.bombsChanged = 2;
    }
}

int Unknown626270::OnDraw(Unknown626270 *mgr)
{
    if (mgr->unk_93dc)
    {
        mgr->unk_93dc = 2;
    }
    return 1;
}

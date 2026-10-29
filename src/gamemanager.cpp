// cflags: /Os
// The /Os file with Unknown626270, probably th07's GameManager, from
// 0x42d560 to the Chain code at 0x42fa60.

#include <string.h>

#include "unsorted.h"

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

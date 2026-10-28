// The code from 0x43b5c0: helpers out of line, then what looks like
// th06's player.cpp (0x43b7f0 constructs the static at 0x4bdad8).

#include "player.h"
#include "unsorted.h"

void Unknown626270::RerollLifeChecksumKeys()
{
    this->stats->unk_34[0] = g_Rng.GetRandomU32InRange(100000) + 6543;
    this->stats->unk_34[1] = g_Rng.GetRandomU32InRange(100000) + 6543;
    this->stats->unk_34[2] = g_Rng.GetRandomU32InRange(100000) + 6543;
    this->stats->unk_34[3] = g_Rng.GetRandomU32InRange(100000) + 6543;
    this->stats->unk_34[4] = g_Rng.GetRandomU32InRange(100000) + 6543;
    this->stats->unk_70[0] = g_Rng.GetRandomF32ZeroToOne() * 100000.0f + 6543.0f;
    this->stats->unk_70[1] = g_Rng.GetRandomF32ZeroToOne() * 100000.0f + 6543.0f;
    this->stats->unk_70[2] = g_Rng.GetRandomF32ZeroToOne() * 100000.0f + 6543.0f;
}

void Unknown626270::AddDeaths(int deaths)
{
    if (this->Func404fe0())
    {
        memset(&g_App, 0xff, 0x2d0);
    }
    this->stats->deaths += deaths;
    this->Func4012b0();
}

void Unknown626270::AddBombsUsed(int bombs)
{
    if (this->Func404fe0())
    {
        memset(&g_App, 0xff, 0x2d0);
    }
    this->stats->bombsUsed += bombs;
    this->Func4012b0();
}

Player::Player()
{
}

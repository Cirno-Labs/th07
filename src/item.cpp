// The items, and the helpers out of line before them.

#include "unsorted.h"

void Unknown4325c0::SetPolar(float angle, float radius)
{
    __asm
    {
        mov eax, this
        fld angle
        fsincos
        fmul radius
        fstp [eax]
        fmul radius
        fstp [eax+4]
    }
}

void Unknown626270::AddPower(int power)
{
    if (this->Func404fe0())
    {
        memset(&g_App, 0xff, 0x2d0);
    }
    this->stats->power += power;
    this->Func4012b0();
}

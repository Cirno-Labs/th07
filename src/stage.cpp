// The file with Unknown1347b00, th06's Stage, from 0x404fe0, as th06's
// stage.cpp.

#include "unsorted.h"

void Unknown1347b00::SetTintColor(D3DCOLOR color)
{
    ColorBytes c;

    if (this->unk_52ac.a == 0)
    {
        this->unk_52ac.color = color;
    }
    else
    {
        c.color = color;
        this->unk_52ac.r = (DWORD)(c.r + this->unk_52ac.r) >> 1;
        this->unk_52ac.g = (DWORD)(c.g + this->unk_52ac.g) >> 1;
        this->unk_52ac.b = (DWORD)(c.b + this->unk_52ac.b) >> 1;
        this->unk_52ac.a = (DWORD)(c.a + this->unk_52ac.a) >> 1;
    }
}

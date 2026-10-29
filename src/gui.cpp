// cflags: /Os
// The /Os file with Unknown49fbf0, th06's Gui, from 0x427b21 to
// Unknown626270 at 0x42d560.

#include "unsorted.h"

void Unknown49fbf0::EndBombName()
{
    this->impl->bombName.pendingInterrupt = 1;
    this->impl->bombNameBar.SetPendingInterrupt(2);
}

void Unknown49fbf0::EndSpellCardName()
{
    this->impl->spellName.pendingInterrupt = 1;
    this->impl->spellNameBar.SetPendingInterrupt(2);
    this->impl->unk_7774.SetPendingInterrupt(2);
}

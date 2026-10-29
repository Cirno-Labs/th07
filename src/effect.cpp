// The file with EffectManager, from 0x41a210, as th06's effect.cpp.

#include <string.h>

#include "effect.h"
#include "unsorted.h"

void EffectManager::Reset()
{
    memset(this, 0, sizeof(EffectManager));
}

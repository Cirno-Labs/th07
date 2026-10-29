// The file with EffectManager, from 0x41a210, as th06's effect.cpp.

#include <string.h>

#include "effect.h"
#include "unsorted.h"

EffectManager::EffectManager()
{
    this->Reset();
    this->colorScale[0] = 1.0f;
    this->colorScale[1] = 1.0f;
    this->colorScale[2] = 1.0f;
    this->colorScale[3] = 1.0f;
}

void EffectManager::Reset()
{
    memset(this, 0, sizeof(EffectManager));
}

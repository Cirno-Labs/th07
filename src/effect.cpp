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

Effect *EffectManager::SpawnEffect(int type, D3DXVECTOR3 *pos, int count, D3DCOLOR color)
{
    Effect *effect;
    int tries;

    effect = &this->effects[this->next];
    for (tries = 0; tries < 400; tries++)
    {
        this->next++;
        if (this->next >= 400)
        {
            this->next = 0;
        }
        if (effect->active)
        {
            if (this->next == 0)
            {
                effect = this->effects;
            }
            else
            {
                effect++;
            }
            continue;
        }
        effect->drawList = 0;
        effect->active = 1;
        effect->type = type;
        effect->pos = *pos;
        g_SpriteEngine->StartScript(&effect->sprite, g_EffectInfos[type].script);
        effect->sprite.flags |= 0x1000;
        effect->sprite.color = color;
        effect->update = g_EffectInfos[type].update;
        effect->timer.Set(0);
        effect->fadeOut = 0;
        effect->fadeFrames = 0;
        effect->velocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        if (g_EffectInfos[type].init != NULL && g_EffectInfos[type].init(effect) != 0)
        {
            effect->active = 0;
        }
        count--;
        if (count == 0)
        {
            break;
        }
        if (this->next == 0)
        {
            effect = this->effects;
        }
        else
        {
            effect++;
        }
    }
    return tries >= 400 ? &this->effects[408] : effect;
}

Effect *EffectManager::SpawnEffectWithVelocity(int type, D3DXVECTOR3 *pos, D3DXVECTOR3 *velocity, int count, D3DCOLOR color)
{
    Effect *effect;
    int tries;

    effect = &this->effects[this->next];
    for (tries = 0; tries < 400; tries++)
    {
        this->next++;
        if (this->next >= 400)
        {
            this->next = 0;
        }
        if (effect->active)
        {
            if (this->next == 0)
            {
                effect = this->effects;
            }
            else
            {
                effect++;
            }
            continue;
        }
        effect->drawList = 0;
        effect->active = 1;
        effect->type = type;
        effect->pos = *pos;
        g_SpriteEngine->StartScript(&effect->sprite, g_EffectInfos[type].script);
        effect->sprite.color = color;
        effect->update = g_EffectInfos[type].update;
        effect->timer.Set(0);
        effect->fadeOut = 0;
        effect->fadeFrames = 0;
        effect->velocity = *velocity;
        if (g_EffectInfos[type].init != NULL && g_EffectInfos[type].init(effect) != 0)
        {
            effect->active = 0;
        }
        count--;
        if (count == 0)
        {
            break;
        }
        if (this->next == 0)
        {
            effect = this->effects;
        }
        else
        {
            effect++;
        }
    }
    return tries >= 400 ? &this->effects[408] : effect;
}

Effect *EffectManager::SpawnReservedEffect(int type, D3DXVECTOR3 *pos, int slot, int count, D3DCOLOR color)
{
    Effect *effect;

    effect = &this->effects[slot + 400];
    effect->drawList = 0;
    effect->active = 1;
    effect->type = type;
    effect->pos = *pos;
    g_SpriteEngine->StartScript(&effect->sprite, g_EffectInfos[type].script);
    effect->sprite.flags |= 0x1000;
    effect->sprite.color = color;
    effect->update = g_EffectInfos[type].update;
    effect->timer.Set(0);
    effect->fadeOut = 0;
    effect->fadeFrames = 0;
    effect->velocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    if (g_EffectInfos[type].init != NULL && g_EffectInfos[type].init(effect) != 0)
    {
        effect->active = 0;
    }
    return effect;
}

int EffectManager::OnUpdate(EffectManager *mgr)
{
    Effect *eff = mgr->effects;
    int i;

    mgr->activeCount = 0;
    mgr->drawListTails[0] = &mgr->drawList0;
    mgr->drawListTails[1] = &mgr->drawList1;
    mgr->drawListTails[2] = &mgr->drawList2;
    mgr->drawListTails[3] = &mgr->drawList3;
    mgr->drawList0.next = NULL;
    mgr->drawList1.next = NULL;
    mgr->drawList2.next = NULL;
    mgr->drawList3.next = NULL;
    for (i = 0; i < 0x198; i++, eff++)
    {
        if (!eff->active)
        {
            continue;
        }
        mgr->activeCount++;
        if (eff->update != NULL && eff->update(eff) != 1)
        {
            eff->active = 0;
            continue;
        }
        if (g_SpriteEngine->Step(&eff->sprite) != 0)
        {
            eff->active = 0;
            continue;
        }
        eff->timer.Tick();
        eff->next = NULL;
        if (eff->drawList == 1 || eff->drawList == 3)
        {
            mgr->drawListTails[1]->next = eff;
            mgr->drawListTails[1] = eff;
        }
        else if (eff->drawList == 0)
        {
            if (eff->sprite.flags >> 4 & 1)
            {
                mgr->drawListTails[3]->next = eff;
                mgr->drawListTails[3] = eff;
            }
            else
            {
                mgr->drawListTails[0]->next = eff;
                mgr->drawListTails[0] = eff;
            }
        }
        else
        {
            mgr->drawListTails[2]->next = eff;
            mgr->drawListTails[2] = eff;
        }
    }
    mgr->frames++;
    if (mgr->frames % 300 == 100 && g_Unknown626270.Func404fe0())
    {
        return 4;
    }
    return 1;
}

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

int EffectInitDrift(Effect *effect)
{
    effect->step.x = (g_Rng.GetRandomF32ZeroToOne() * 256.0f - 128.0f) / 12.0f;
    effect->step.y = (g_Rng.GetRandomF32ZeroToOne() * 256.0f - 128.0f) / 12.0f;
    effect->step.z = 0.0f;
    effect->acceleration = -effect->step / 19.0f;
    effect->step *= g_App.unk_178;
    effect->acceleration *= g_App.unk_178;
    return 0;
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

int EffectManager::OnDraw(EffectManager *mgr)
{
    Effect *eff;

    eff = mgr->drawList0.next;
    while (eff != NULL)
    {
        eff->sprite.pos = eff->pos;
        eff->sprite.pos.x = g_Unknown626270.arena[0] + eff->sprite.pos.x;
        eff->sprite.pos.y = g_Unknown626270.arena[1] + eff->sprite.pos.y;
        g_SpriteEngine->Draw(&eff->sprite);
        eff = eff->next;
    }
    eff = mgr->drawList2.next;
    while (eff != NULL)
    {
        eff->sprite.pos = eff->pos;
        g_SpriteEngine->DrawProjected(&eff->sprite);
        eff = eff->next;
    }
    eff = mgr->drawList3.next;
    while (eff != NULL)
    {
        eff->sprite.pos = eff->pos;
        eff->sprite.pos.x = g_Unknown626270.arena[0] + eff->sprite.pos.x;
        eff->sprite.pos.y = g_Unknown626270.arena[1] + eff->sprite.pos.y;
        g_SpriteEngine->Draw(&eff->sprite);
        eff = eff->next;
    }
    return 1;
}

int EffectManager::OnDraw2()
{
    Effect *eff = this->drawList1.next;
    int n = 0;
    float r;
    float g;
    float b;
    float a;
    int t;

    if (g_App.unk_13c == 0)
    {
        return 1;
    }
    while (eff != NULL)
    {
        n++;
        if (g_App.unk_13c == 1 && (n & 1) != 0)
        {
            return 1;
        }
        if (eff->type == 20)
        {
            r = eff->sprite.unk_rgb[2];
            g = eff->sprite.unk_rgb[1];
            b = eff->sprite.unk_rgb[0];
            a = eff->sprite.alpha;
            t = r * this->colorScale[0];
            eff->sprite.unk_rgb[2] = t > 0xff ? 0xff : t;
            t = g * this->colorScale[1];
            eff->sprite.unk_rgb[1] = t > 0xff ? 0xff : t;
            t = b * this->colorScale[2];
            eff->sprite.unk_rgb[0] = t > 0xff ? 0xff : t;
            t = a * this->colorScale[3];
            eff->sprite.alpha = t > 0xff ? 0xff : t;
        }
        eff->sprite.pos = eff->pos;
        if (eff->drawList == 1)
        {
            g_SpriteEngine->DrawProjected(&eff->sprite);
        }
        else
        {
            g_SpriteEngine->DrawProjected3D(&eff->sprite);
        }
        if (eff->type == 20)
        {
            eff->sprite.unk_rgb[2] = r;
            eff->sprite.unk_rgb[1] = g;
            eff->sprite.unk_rgb[0] = b;
            eff->sprite.alpha = a;
        }
        eff = eff->next;
    }
    return 1;
}

int EffectManager::AddedCallback(EffectManager *mgr)
{
    mgr->Reset();
    g_Unknown1348024 = 0;
    switch (g_Unknown626270.stage)
    {
    case 0:
    case 1:
        g_Unknown1348020 = 1;
        if (g_SpriteEngine->LoadAnm(0x11, "data/eff01.anm", 0x2dc) != 0)
        {
            return -1;
        }
        break;
    case 2:
        g_Unknown1348020 = 1;
        if (g_SpriteEngine->LoadAnm(0x11, "data/eff02.anm", 0x2dc) != 0)
        {
            return -1;
        }
        break;
    case 3:
        g_Unknown1348020 = 1;
        if (g_SpriteEngine->LoadAnm(0x11, "data/eff03.anm", 0x2dc) != 0)
        {
            return -1;
        }
        break;
    case 4:
        g_Unknown1348020 = 2;
        if (g_SpriteEngine->LoadAnm(0x11, "data/eff04.anm", 0x2dc) != 0)
        {
            return -1;
        }
        if (g_SpriteEngine->LoadAnm(0x12, "data/eff04b.anm", 0x2dd) != 0)
        {
            return -1;
        }
        break;
    case 5:
        g_Unknown1348020 = 2;
        if (g_SpriteEngine->LoadAnm(0x11, "data/eff05.anm", 0x2dc) != 0)
        {
            return -1;
        }
        break;
    case 6:
        g_Unknown1348020 = 2;
        if (g_SpriteEngine->LoadAnm(0x11, "data/eff05.anm", 0x2dc) != 0)
        {
            return -1;
        }
        if (g_SpriteEngine->LoadAnm(0x13, "data/eff06.anm", 0x2de) != 0)
        {
            return -1;
        }
        break;
    case 7:
        g_Unknown1348020 = 1;
        if (g_SpriteEngine->LoadAnm(0x11, "data/eff02.anm", 0x2dc) != 0)
        {
            return -1;
        }
        if (g_SpriteEngine->LoadAnm(0x12, "data/eff07.anm", 0x2dd) != 0)
        {
            return -1;
        }
        break;
    case 8:
        g_Unknown1348020 = 2;
        if (g_SpriteEngine->LoadAnm(0x11, "data/eff07.anm", 0x2dc) != 0)
        {
            return -1;
        }
        if (g_SpriteEngine->LoadAnm(0x13, "data/eff08.anm", 0x2de) != 0)
        {
            return -1;
        }
        break;
    }
    return 0;
}

int EffectManager::DeletedCallback(EffectManager *mgr)
{
    g_SpriteEngine->ReleaseAnm(0x11);
    g_SpriteEngine->ReleaseAnm(0x12);
    g_SpriteEngine->ReleaseAnm(0x13);
    g_SpriteEngine->ReleaseAnm(0x14);
    return 0;
}

int EffectManager::RegisterChain()
{
    EffectManager *mgr = &g_EffectManager;

    mgr->Reset();
    g_EffectCalcChain.SetCallback((ChainCallback)EffectManager::OnUpdate);
    g_EffectCalcChain.addedCallback = (ChainCallback)EffectManager::AddedCallback;
    g_EffectCalcChain.deletedCallback = (ChainCallback)EffectManager::DeletedCallback;
    g_EffectCalcChain.arg = mgr;
    if (g_Chain.AddToCalcChain(&g_EffectCalcChain, 11) != 0)
    {
        return -1;
    }
    g_EffectDrawChain.SetCallback((ChainCallback)EffectManager::OnDraw);
    g_EffectDrawChain.arg = mgr;
    g_Chain.AddToDrawChain(&g_EffectDrawChain, 9);
    return 0;
}

void EffectManager::RemoveChain()
{
    g_Chain.Remove(&g_EffectCalcChain);
    g_Chain.Remove(&g_EffectDrawChain);
}

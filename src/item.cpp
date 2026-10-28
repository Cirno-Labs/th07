// The items, and the helpers out of line before them.

#include "item.h"
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

Item *ItemManager::SpawnItem(D3DXVECTOR3 *pos, int type, int state)
{
    int i;
    Item *item;

    item = &this->items[this->nextIndex];
    if ((int)g_Unknown626270.stats->power >= 128 && (type == 0 || type == 2))
    {
        type = 7;
    }
    for (i = 0; i < 0x44c; i++)
    {
        this->nextIndex++;
        if (item->active)
        {
            if (this->nextIndex >= 0x44c)
            {
                this->nextIndex = 0;
                item = this->items;
            }
            else
            {
                item++;
            }
            continue;
        }
        if (this->nextIndex >= 0x44c)
        {
            this->nextIndex = 0;
        }
        item->active = 1;
        item->pos = *pos;
        item->velocity.x = 0.0f;
        item->velocity.y = -2.2f;
        item->velocity.z = 0.0f;
        item->type = type;
        item->state = state;
        item->timer.Set(0);
        if (state == 2)
        {
            item->target.x = g_Rng.GetRandomF32ZeroToOne() * 288.0f + 48.0f;
            item->target.y = g_Rng.GetRandomF32ZeroToOne() * 192.0f - 64.0f;
            item->target.z = 0.0f;
            item->velocity = item->pos;
        }
        else if (state == 3)
        {
            item->state = 1;
        }
        else if (state == 4)
        {
            item->state = 0;
        }
        g_SpriteEngine->StartScript(&item->sprite, type + 0x2c4);
        item->sprite.color = 0xffffffff;
        item->sprite.flags |= 0x1000;
        item->unk_280 = 0;
        item->onScreen = 1;
        break;
    }
    return i < 0x44c ? item : &this->items[0x44c];
}

void ItemManager::CollectAll()
{
    Item *entry = this->items;
    int i;

    for (i = 0; i < 0x44c; i++, entry++)
    {
        if (!entry->active)
        {
            continue;
        }
        entry->state = 1;
        entry->velocity = D3DXVECTOR3(0.0f, -0.5f, 0.0f);
    }
}

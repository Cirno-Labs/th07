// The items, and the helpers out of line before them.

#include "ascii.h"
#include "bullet.h"
#include "effect.h"
#include "item.h"
#include "player.h"
#include "replay.h"
#include "unsorted.h"

void Vector2::SetPolar(float angle, float radius)
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

void GameManager::AddPower(int power)
{
    if (this->ChecksumMismatch())
    {
        memset(&g_App, 0xff, 0x2d0);
    }
    this->stats->power += power;
    this->RerollChecksumKeys();
}

ItemManager::ItemManager()
{
}

Item *ItemManager::SpawnItem(D3DXVECTOR3 *pos, int type, int state)
{
    int i;
    Item *item;

    item = &this->items[this->nextIndex];
    if ((int)g_GameManager.stats->power >= 128 && (type == 0 || type == 2))
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
        item->countsFullValue = 0;
        item->onScreen = 1;
        break;
    }
    return i < 0x44c ? item : &this->items[0x44c];
}

void ItemManager::UpdateItems()
{
    int i;
    float weight;
    int amount;
    float angle;
    // Whether any item was collected this frame.
    BOOL any;
    Item *entry = this->items;
    D3DXVECTOR3 catchSize(g_Player.shotType->itemCatchSize,
                          g_Player.shotType->itemCatchSize, 16.0f);

    any = FALSE;
    this->activeCount = 0;
    this->drawListTail = &this->drawList;
    this->drawList.next = NULL;
    for (i = 0; i < 0x44c; i++, entry++)
    {
        if (!entry->active)
        {
            continue;
        }
        this->activeCount++;
        if (entry->state == 2)
        {
            if (entry->timer < 60)
            {
                weight = entry->timer.AsFloat() / 60.0f;
                entry->pos = weight * entry->target + entry->velocity * (1.0f - weight);
                goto touch;
            }
            if (entry->timer == 60)
            {
                entry->velocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
                entry->state = 0;
            }
            goto move;
        }
        else
        {
            if (entry->state == 1 ||
                (((int)g_GameManager.stats->power >= 128.0 ||
                  g_GameManager.difficulty >= 4) &&
                 g_Player.position.y < g_Player.shotType->itemAutoCollectLine) ||
                g_Player.bombState == 1)
            {
                if (g_Player.playerState != 1)
                {
                    angle = g_Player.AngleFrom(&entry->pos);
                    ((Vector2 *)&entry->velocity)
                        ->SetPolar(angle, g_Player.shotType->itemPullSpeed);
                    entry->state = 1;
                    if (g_Player.bombState == 1)
                    {
                        entry->countsFullValue = 1;
                    }
                }
                else
                {
                    entry->velocity.y = -0.5f;
                    entry->state = 0;
                }
            }
            else
            {
                entry->velocity.x = 0.0f;
                entry->velocity.z = 0.0f;
                if (entry->velocity.y < -2.2f)
                {
                    entry->velocity.y = -2.2f;
                }
            }
        move:
            entry->pos += entry->velocity * g_App.gameSpeed;
            if (g_GameManager.arena[3] + 16.0f <= entry->pos.y)
            {
                entry->active = 0;
                g_GameManager.SubSubRank(3);
                continue;
            }
            if (entry->velocity.y < 3.0f)
            {
                entry->velocity.y += 0.03f * g_App.gameSpeed;
            }
            else
            {
                entry->velocity.y = 3.0f;
            }
        }
    touch:
        if (g_Player.TouchesGrazeBox(&entry->pos, &catchSize))
        {
            g_ReplayManager->frameEvents |= 0x40;
            switch (entry->type)
            {
            case 0:
                if ((int)g_GameManager.stats->power >= 128)
                {
                    g_GameManager.powerItemCombo++;
                    if (g_GameManager.powerItemCombo >= ARRAY_SIZE(g_PowerItemBonus))
                    {
                        g_GameManager.powerItemCombo = ARRAY_SIZE(g_PowerItemBonus) - 1;
                    }
                    amount = g_PowerItemBonus[g_GameManager.powerItemCombo];
                    g_GameManager.AddScore(amount);
                    g_AsciiText.AddPopup(&entry->pos, amount,
                                         amount >= 12800 ? 0xffffff00 : 0xffffffff);
                }
                else
                {
                    int prevLevel;
                    int level;

                    level = 0;
                    while ((int)g_GameManager.stats->power >= g_PowerLevels[level])
                    {
                        level++;
                    }
                    prevLevel = level;
                    g_GameManager.powerItemCombo = 0;
                    g_GameManager.AddPower(1);
                    if ((int)g_GameManager.stats->power >= 128)
                    {
                        g_GameManager.stats->power = 128.0f;
                        g_GameManager.RerollChecksumKeys();
                        if (!g_SpellCardActive)
                        {
                            g_BulletManager.ClearAll(1);
                        }
                        g_Gui.ShowFullPowerPopup(0, 1);
                        this->ConvertPowerItems(i);
                    }
                    g_GameManager.AddScore(10);
                    g_Gui.powerChanged = 2;
                    while ((int)g_GameManager.stats->power >= g_PowerLevels[level])
                    {
                        level++;
                    }
                    if (level != prevLevel)
                    {
                        g_AsciiText.AddPopup(&entry->pos, -1, 0xffffc0a0);
                        g_SoundPlayer.QueueSound(0x1f, 0);
                    }
                    else
                    {
                        g_AsciiText.AddPopup(&entry->pos, 10, 0xffffffff);
                    }
                }
                g_GameManager.AddSubRank(1);
                break;
            case 1:
                amount = (entry->pos.y < g_Player.shotType->itemAutoCollectLine ? 1 : 0)
                             ? 50000
                             : 50000 - (int)(entry->pos.y - g_Player.shotType->itemAutoCollectLine) * 100;
                if (entry->countsFullValue == 1)
                {
                    amount = 50000;
                }
                if (amount >= 50000)
                {
                    if (g_GameManager.bonusPoints - g_GameManager.stats->grazeCount > 50000)
                    {
                        amount = g_GameManager.bonusPoints - g_GameManager.stats->grazeCount;
                    }
                }
                else if (g_GameManager.bonusPoints - g_GameManager.stats->grazeCount > 50000)
                {
                    amount += (g_GameManager.bonusPoints - g_GameManager.stats->grazeCount - 50000) / 5;
                }
                amount -= amount % 10;
                g_AsciiText.AddPopup(&entry->pos, amount,
                                     entry->pos.y < g_Player.shotType->itemAutoCollectLine || entry->countsFullValue == 1
                                         ? 0xffffff00
                                         : 0xffffffff);
                g_GameManager.AddScore(amount);
                g_GameManager.stats->stageClearPoints++;
                g_GameManager.stats->pointItemsTotal++;
                g_Gui.frameItemsHideTimer = 2;
                if (entry->pos.y < 128.0f)
                {
                    g_GameManager.AddSubRank(10);
                }
                else
                {
                    g_GameManager.AddSubRank(3);
                }
                if (g_GameManager.stats->checksumKeyStage >= 0)
                {
                    for (;;)
                    {
                        if (g_GameManager.difficulty < 4)
                        {
                            if (g_GameManager.stats->checksumKeyStage < 3)
                            {
                                g_GameManager.stats->checksumKeyPoints = g_GameManager.stats->checksumKeyStage * 75 + 50;
                            }
                            else if (g_GameManager.stats->checksumKeyStage < 5)
                            {
                                g_GameManager.stats->checksumKeyPoints = (g_GameManager.stats->checksumKeyStage - 3) * 150 + 300;
                            }
                            else
                            {
                                g_GameManager.stats->checksumKeyPoints = (g_GameManager.stats->checksumKeyStage - 5) * 200 + 800;
                            }
                        }
                        else if (g_GameManager.stats->checksumKeyStage == 0)
                        {
                            g_GameManager.stats->checksumKeyPoints = 200;
                        }
                        else if (g_GameManager.stats->checksumKeyStage == 1)
                        {
                            g_GameManager.stats->checksumKeyPoints = 500;
                        }
                        else
                        {
                            g_GameManager.stats->checksumKeyPoints = (g_GameManager.stats->checksumKeyStage - 2) * 500 + 800;
                        }
                        if (g_GameManager.stats->pointItemsTotal >= g_GameManager.stats->checksumKeyPoints)
                        {
                            g_GameManager.AddLifeOrBomb();
                            g_GameManager.stats->checksumKeyStage++;
                            continue;
                        }
                        break;
                    }
                }
                break;
            case 2:
                if ((int)g_GameManager.stats->power >= 128)
                {
                    g_AsciiText.AddPopup(&entry->pos, amount,
                                         amount >= 1000 ? 0xffffff00 : 0xffffffff);
                }
                else
                {
                    int prevLevel;
                    int level;

                    level = 0;
                    while ((int)g_GameManager.stats->power >= g_PowerLevels[level])
                    {
                        level++;
                    }
                    prevLevel = level;
                    g_GameManager.AddPower(8);
                    if ((int)g_GameManager.stats->power >= 128)
                    {
                        g_GameManager.stats->power = 128.0f;
                        g_GameManager.RerollChecksumKeys();
                        if (!g_SpellCardActive)
                        {
                            g_BulletManager.ClearAll(1);
                        }
                        g_Gui.ShowFullPowerPopup(0, 1);
                        this->ConvertPowerItems(i);
                    }
                    g_Gui.powerChanged = 2;
                    g_GameManager.AddScore(10);
                    while ((int)g_GameManager.stats->power >= g_PowerLevels[level])
                    {
                        level++;
                    }
                    if (level != prevLevel)
                    {
                        g_AsciiText.AddPopup(&entry->pos, -1, 0xffffc0a0);
                        g_SoundPlayer.QueueSound(0x1f, 0);
                    }
                    else
                    {
                        g_AsciiText.AddPopup(&entry->pos, 10, 0xffffffff);
                    }
                }
                break;
            case 3:
                if ((int)g_GameManager.stats->bombs < 8)
                {
                    g_GameManager.AddBombs(1);
                    g_Gui.bombsChanged = 2;
                }
                g_GameManager.AddSubRank(5);
                break;
            case 5:
                g_GameManager.AddLifeOrBomb();
                break;
            case 4:
                if ((int)g_GameManager.stats->power < 128)
                {
                    g_BulletManager.ClearAll(1);
                    g_Gui.ShowFullPowerPopup(0, 1);
                    g_SoundPlayer.QueueSound(0x1f, 0);
                    g_AsciiText.AddPopup(&entry->pos, -1, 0xffffc0a0);
                    this->ConvertPowerItems(i);
                }
                g_GameManager.stats->power = 128.0f;
                g_GameManager.RerollChecksumKeys();
                g_GameManager.AddScore(1000);
                g_AsciiText.AddPopup(&entry->pos, 1000, 0xffffffff);
                g_Gui.powerChanged = 2;
                break;
            case 6:
                if (!g_Player.bombJustStarted)
                {
                    amount = g_GameManager.stats->grazeTotal / 40 * 10 + 300;
                    if (amount <= 0)
                    {
                        amount = 10;
                    }
                }
                else
                {
                    amount = 100;
                }
                g_AsciiText.AddPopup2(&entry->pos, amount, 0xffffffff);
                g_GameManager.AddScore(amount);
                if (!g_Player.bomb.running)
                {
                    g_GameManager.AddBonusPoints(20);
                }
                else if ((i & 1) == 0)
                {
                    g_GameManager.AddBonusPoints(10);
                }
                else
                {
                    g_GameManager.AddBonusPointsNoTrack(10);
                }
                break;
            case 8:
                g_GameManager.AddBonusPoints(30);
                g_GameManager.AddBonusPointsNoTrack(70);
                break;
            case 7:
                if ((g_GameManager.bonusPoints >= g_GameManager.bonusPointsCap) != 0)
                {
                    amount = (entry->pos.y < g_Player.shotType->itemAutoCollectLine || entry->countsFullValue ? 1 : 0)
                                 ? 50000
                                 : 50000 - (int)(entry->pos.y - g_Player.shotType->itemAutoCollectLine) * 100;
                    amount -= amount % 10;
                    g_AsciiText.AddPopup(&entry->pos, amount,
                                         entry->pos.y < g_Player.shotType->itemAutoCollectLine || entry->countsFullValue
                                             ? 0xffffff00
                                             : 0xffffffff);
                    g_GameManager.AddScore(amount);
                }
                amount = 1000;
                amount += g_GameManager.stats->spellCardsCaptured * 100;
                if ((g_GameManager.bonusPoints >= g_GameManager.bonusPointsCap) == 0)
                {
                    g_AsciiText.AddPopup(&entry->pos, amount, 0xffff4040);
                }
                g_GameManager.AddBonusPoints(amount);
                break;
            case 9:
                amount = g_GameManager.stats->grazeTotal / 40 * 10 + 300;
                if (amount <= 0)
                {
                    amount = 10;
                }
                if ((g_GameManager.bonusPoints >= g_GameManager.bonusPointsCap) != 0)
                {
                    g_AsciiText.AddPopup(&entry->pos, amount, 0xffffffff);
                }
                g_GameManager.AddScore(amount);
                amount = 100;
                if ((g_GameManager.bonusPoints >= g_GameManager.bonusPointsCap) == 0)
                {
                    g_AsciiText.AddPopup(&entry->pos, amount, 0xffff4040);
                }
                g_GameManager.AddBonusPoints(amount);
                break;
            }
            entry->active = 0;
            any = TRUE;
            continue;
        }
        entry->timer.Tick();
        if (entry->sprite.scriptPos != NULL)
        {
            g_SpriteEngine->Step(&entry->sprite);
        }
        this->drawListTail->next = entry;
        entry->next = NULL;
        this->drawListTail = entry;
    }
    if (any)
    {
        g_SoundPlayer.QueueSound(0x15, 0);
    }
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

void ItemManager::ConvertPowerItems(int except)
{
    Item *entry = this->items;
    int i;

    for (i = 0; i < 0x44c; i++, entry++)
    {
        if (!entry->active || i == except)
        {
            continue;
        }
        if (entry->type == 0 || entry->type == 2)
        {
            if (entry->velocity.y > -0.5f)
            {
                entry->velocity.x = 0.0f;
                entry->velocity.y = -0.5f;
                entry->velocity.z = 0.0f;
            }
            g_EffectManager.SpawnEffect(0, &entry->pos, 1, 0xffffffff);
            entry->type = 7;
            g_SpriteEngine->StartScript(&entry->sprite, 0x2cb);
        }
    }
}

void ItemManager::RecycleCollectedItems()
{
    Item *entry = this->items;
    int i;

    for (i = 0; i < 0x44c; i++, entry++)
    {
        if (entry->active != 1)
        {
            continue;
        }
        if (entry->state == 1)
        {
            entry->state = 0;
            entry->velocity.x = 0.0f;
            entry->velocity.y = -0.9f;
            entry->velocity.z = 0.0f;
        }
    }
}

void ItemManager::DrawItems()
{
    Item *entry = this->drawList.next;
    int alpha;

    while (entry != NULL)
    {
        entry->sprite.pos.x = g_GameManager.arena[0] + entry->pos.x;
        entry->sprite.pos.y = g_GameManager.arena[1] + entry->pos.y;
        entry->sprite.pos.z = 0.01f;
        if (entry->pos.y < -8.0f)
        {
            entry->sprite.pos.y = 8.0f + g_GameManager.arena[1];
            if (entry->onScreen)
            {
                g_SpriteEngine->SetSprite(&entry->sprite, entry->type + 0x2b6);
                entry->onScreen = 0;
                entry->sprite.flags |= 0x1000;
            }
            alpha = 255 - (int)((8.0f - entry->pos.y) * 255.0f / 128.0f);
            if (alpha < 64)
            {
                alpha = 64;
            }
            entry->sprite.color = entry->sprite.color & 0xffffff | alpha << 24;
        }
        else if (!entry->onScreen)
        {
            g_SpriteEngine->SetSprite(&entry->sprite, entry->type + 0x2ac);
            entry->onScreen = 1;
            entry->sprite.color = 0xffffffff;
            entry->sprite.flags |= 0x1000;
        }
        g_SpriteEngine->Draw(&entry->sprite);
        entry = entry->next;
    }
}

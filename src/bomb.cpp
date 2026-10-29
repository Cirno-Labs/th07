// The file with the player's bombs, from 0x4083f0, as th06's bomb.cpp.

#include "effect.h"
#include "item.h"
#include "player.h"
#include "screeneffect.h"
#include "sprite.h"
#include "unsorted.h"

void DimPlayAreaForBomb(Player *player)
{
    ColorBytes color;

    color.a = 0x80;
    if (player->bomb.timer < 60)
    {
        color.b = 0x80 - player->bomb.timer.AsFrames() * 80 / 60;
        color.g = color.b;
        color.r = color.g;
    }
    else if (player->bomb.timer >= player->bomb.duration - 60)
    {
        color.b = 0x80 - (player->bomb.duration - player->bomb.timer.AsFrames()) * 80 / 60;
        color.g = color.b;
        color.r = color.g;
    }
    else
    {
        color.b = 0x30;
        color.g = color.b;
        color.r = color.g;
    }
    g_SpriteEngine->ResetUnk0();
    g_Unknown1347b00.SetTintColor(color.color);
    g_Unknown1347b00.unk_52b0 = 1;
}

void SpawnBombAura(Player *player)
{
    Effect *effect;

    if (player->unk_b7e68 != NULL)
    {
        player->unk_b7e68->active = 0;
    }
    effect = g_EffectManager.SpawnReservedEffect(0x19, &player->position, 0, 1, 0xffffffff);
    effect->sprite.unk_48[4].Set(0);
    effect->sprite.unk_84[4] = player->stateTimer;
    effect->sprite.unk_c4 = 0;
    *(D3DXVECTOR2 *)&effect->sprite.unk_218 = *(D3DXVECTOR2 *)&effect->sprite.scaleY;
    effect->sprite.unk_218.z = 0.0625f;
    effect->sprite.unk_224.x = 0.0625f;
    effect->sprite.unk_c8 = player->stateTimer.AsFrames();
    effect->sprite.unk_14 *= -1.0f;
    effect->sprite.unk_rgb[2] = 0xff;
    effect->sprite.unk_rgb[1] = 0x40;
    effect->sprite.unk_rgb[0] = 0x40;
    player->unk_b7e68 = effect;
}

void SetBombDrain(Player *player, int minimum, float share)
{
    int drain;

    drain = (g_Unknown626270.unk_961c - g_Unknown626270.stats->unk_88) * share;
    switch (g_Unknown626270.difficulty)
    {
    case 2:
        drain /= 2;
        break;
    case 3:
        drain /= 4;
        break;
    case 4:
    case 5:
        drain /= 3;
        break;
    }
    drain /= player->bomb.duration;
    drain -= drain % 10;
    minimum /= player->bomb.duration;
    minimum -= minimum % 10;
    player->bomb.unk_c = drain < minimum ? minimum : drain;
}

void UpdateReimuABomb(Player *player)
{
    PlayerBomb *bomb = &player->bomb;
    float ang;
    PlayerBombPart *orb;
    int i;
    Sprite *sprite;
    int k;
    D3DXVECTOR3 unused;

    if (player->bomb.timer >= player->bomb.duration)
    {
        g_Unknown49fbf0.EndBombName();
        player->bomb.running = 0;
        return;
    }
    if (bomb->timer.HasTicked() && bomb->timer == 0)
    {
        g_Unknown49fbf0.ShowBombName(0x4a1, "霊符「夢想封印　散」");
        bomb->duration = 140;
        player->stateTimer.Set(200);
        SpawnBombAura(player);
        for (i = 0; i < 0x20; i++)
        {
            bomb->parts[i].state = 0;
        }
        g_ItemManager.CollectAll();
        g_EffectManager.SpawnEffect(0xc, &player->position, 1, 0xff4040ff);
        player->AddHitboxAreaCircle(&player->position, 32.0f, 8.0f, 0x10, 6);
        bomb->startPos = player->position;
        SetBombDrain(player, 4000, 0.2f);
    }
    if (bomb->timer.HasTicked() && bomb->timer >= 8 && bomb->timer < 80 && bomb->timer.AsFrames() % 6 == 0)
    {
        i = (bomb->timer.AsFrames() - 8) / 6;
        orb = &bomb->parts[i];
        orb->state = 1;
        orb->speed = 15.0f;
        orb->pos = player->position;
        if (bomb->startPos.x < 192.0f)
        {
            ang = i * 6.2831855f / 8.0f - 1.5707964f;
        }
        else
        {
            ang = -i * 6.2831855f / 8.0f - 1.5707964f;
        }
        orb->angle = AddNormalizeAngle(ang, 0.0f);
        orb->unk_4 = 0;
        player->unk_9dc[i].unk_1c = 0;
        for (sprite = orb->sprites, k = 0; k < 4; k++, sprite++)
        {
            g_SpriteEngine->StartScriptAtOrigin(sprite, k + 0x485);
        }
        g_SoundPlayer.QueueSound(0xd, 0);
    }
    player->playerState = 3;
    for (i = 0, orb = bomb->parts; i < 8; i++, orb++)
    {
        if (orb->state == 0)
        {
            continue;
        }
        if (orb->state == 1)
        {
            orb->speed -= 0.4f * g_App.unk_178;
            ((Vector2 *)&orb->velocity)->SetPolar(orb->angle, orb->speed);
            if (orb->speed < -10.0f)
            {
                g_EffectManager.SpawnEffect(6, &orb->pos, 8, 0xffffffff);
                g_EffectManager.SpawnEffect(0xc, &orb->pos, 1, 0xff4040ff);
                orb->state = 2;
                orb->sprites[0].pendingInterrupt = 1;
                orb->sprites[1].pendingInterrupt = 1;
                orb->sprites[2].pendingInterrupt = 1;
                orb->sprites[3].pendingInterrupt = 1;
                player->unk_9dc[i].pos = orb->pos;
                player->unk_9dc[i].size.x = 256.0f;
                player->unk_9dc[i].size.y = 256.0f;
                player->unk_9dc[i].unk_18 = 400;
                player->AddHitboxAreaCircle(&orb->pos, 64.0f, 4.2666669f, 0x1e, 6);
                orb->velocity.x = 0.0f;
                orb->velocity.y = 0.0f;
                orb->velocity.z = 0.0f;
                g_SoundPlayer.QueueSound(0xf, 0);
                ScreenEffect::StartEffect(1, 0x10, 8, 0, 0);
            }
            if (bomb->timer.HasTicked())
            {
                player->unk_9dc[i].size.x = 48.0f;
                player->unk_9dc[i].size.y = 48.0f;
                player->unk_9dc[i].pos = orb->pos;
                player->unk_9dc[i].unk_18 = 8;
                player->AddHitboxAreaCircle(&orb->pos, 128.0f, 0.0f, 0, 6);
            }
        }
        else if (orb->state != 0 && bomb->timer.HasTicked())
        {
            player->unk_9dc[i].pos = bomb->parts[i].pos;
            player->unk_9dc[i].size.x = 256.0f;
            player->unk_9dc[i].size.y = 256.0f;
            player->unk_9dc[i].unk_18 = 2;
            orb->unk_4++;
            if (orb->unk_4 >= 30)
            {
                orb->state = 0;
            }
        }
        orb->pos.x += g_App.unk_178 * orb->velocity.x;
        orb->pos.y += g_App.unk_178 * orb->velocity.y;
        g_SpriteEngine->Step(&orb->sprites[0]);
        g_SpriteEngine->Step(&orb->sprites[1]);
        g_SpriteEngine->Step(&orb->sprites[2]);
        g_SpriteEngine->Step(&orb->sprites[3]);
    }
    bomb->timer.Tick();
}

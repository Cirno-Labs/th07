// The file with the player's bombs, from 0x4083f0, as th06's bomb.cpp.

#include <math.h>

#include "bullet.h"
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
    g_SpriteRenderer->ResetColorModulation();
    g_Stage.SetTintColor(color.color);
    g_Stage.keepTintColor = 1;
}

void SpawnBombAura(Player *player)
{
    Effect *effect;

    if (player->invulnerabilityEffect != NULL)
    {
        player->invulnerabilityEffect->active = 0;
    }
    effect = g_EffectManager.SpawnReservedEffect(0x19, &player->position, 0, 1, 0xffffffff);
    effect->sprite.interpDuration[4].Set(0);
    effect->sprite.interpFrame[4] = player->stateTimer;
    effect->sprite.scaleInterpEasing = 0;
    *(D3DXVECTOR2 *)&effect->sprite.scaleTween = *(D3DXVECTOR2 *)&effect->sprite.scaleY;
    effect->sprite.scaleTween.z = 0.0625f;
    effect->sprite.colorTween.x = 0.0625f;
    effect->sprite.scriptVar0 = player->stateTimer.AsFrames();
    effect->sprite.rotationSpeedZ *= -1.0f;
    effect->sprite.colorRgb[2] = 0xff;
    effect->sprite.colorRgb[1] = 0x40;
    effect->sprite.colorRgb[0] = 0x40;
    player->invulnerabilityEffect = effect;
}

void SetBombDrain(Player *player, int minimum, float share)
{
    int drain;

    drain = (g_GameManager.bonusPoints - g_GameManager.stats->grazeCount) * share;
    switch (g_GameManager.difficulty)
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
    player->bomb.drainPerFrame = drain < minimum ? minimum : drain;
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
        g_Gui.EndBombName();
        player->bomb.running = 0;
        return;
    }
    if (bomb->timer.HasTicked() && bomb->timer == 0)
    {
        g_Gui.ShowBombName(0x4a1, "霊符「夢想封印　散」");
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
        orb->burstTimer = 0;
        player->bombHitboxes[i].damageDealt = 0;
        for (sprite = orb->sprites, k = 0; k < 4; k++, sprite++)
        {
            g_SpriteRenderer->StartScriptAtOrigin(sprite, k + 0x485);
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
            orb->speed -= 0.4f * g_App.gameSpeed;
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
                player->bombHitboxes[i].pos = orb->pos;
                player->bombHitboxes[i].size.x = 256.0f;
                player->bombHitboxes[i].size.y = 256.0f;
                player->bombHitboxes[i].damage = 400;
                player->AddHitboxAreaCircle(&orb->pos, 64.0f, 4.2666669f, 0x1e, 6);
                orb->velocity.x = 0.0f;
                orb->velocity.y = 0.0f;
                orb->velocity.z = 0.0f;
                g_SoundPlayer.QueueSound(0xf, 0);
                ScreenEffect::StartEffect(1, 0x10, 8, 0, 0);
            }
            if (bomb->timer.HasTicked())
            {
                player->bombHitboxes[i].size.x = 48.0f;
                player->bombHitboxes[i].size.y = 48.0f;
                player->bombHitboxes[i].pos = orb->pos;
                player->bombHitboxes[i].damage = 8;
                player->AddHitboxAreaCircle(&orb->pos, 128.0f, 0.0f, 0, 6);
            }
        }
        else if (orb->state != 0 && bomb->timer.HasTicked())
        {
            player->bombHitboxes[i].pos = bomb->parts[i].pos;
            player->bombHitboxes[i].size.x = 256.0f;
            player->bombHitboxes[i].size.y = 256.0f;
            player->bombHitboxes[i].damage = 2;
            orb->burstTimer++;
            if (orb->burstTimer >= 30)
            {
                orb->state = 0;
            }
        }
        orb->pos.x += g_App.gameSpeed * orb->velocity.x;
        orb->pos.y += g_App.gameSpeed * orb->velocity.y;
        g_SpriteRenderer->Step(&orb->sprites[0]);
        g_SpriteRenderer->Step(&orb->sprites[1]);
        g_SpriteRenderer->Step(&orb->sprites[2]);
        g_SpriteRenderer->Step(&orb->sprites[3]);
    }
    bomb->timer.Tick();
}

// Places curSprite at orb's position plus its offset in the play area,
// draws it and moves on to the next.
#define DRAW_BOMB_SPRITE()                                              \
    curSprite->pos = orb->pos + curSprite->posOffset;                   \
    curSprite->pos.x = g_GameManager.arena[0] + curSprite->pos.x;     \
    curSprite->pos.y = g_GameManager.arena[1] + curSprite->pos.y;     \
    curSprite->pos.z = 0.0f;                                            \
    g_SpriteRenderer->DrawUnrotated(curSprite);                           \
    curSprite++;

void DrawReimuABomb(Player *player)
{
    Sprite *curSprite;
    PlayerBombPart *orb;
    int i;

    DimPlayAreaForBomb(player);
    for (i = 0, orb = player->bomb.parts; i < 8; i++, orb++)
    {
        if (orb->state == 0)
        {
            continue;
        }
        curSprite = orb->sprites;
        DRAW_BOMB_SPRITE();
        DRAW_BOMB_SPRITE();
        DRAW_BOMB_SPRITE();
        DRAW_BOMB_SPRITE();
    }
}

void UpdateReimuABombFocused(Player *player)
{
    PlayerBomb *bombs = &player->bomb;
    float x;
    float y;
    PlayerBombPart *orb;
    int i;
    Sprite *spr;
    float len;
    D3DXVECTOR3 aim;
    int k;

    if (bombs->timer >= bombs->duration)
    {
        g_Gui.EndBombName();
        bombs->running = 0;
        player->moveScaleY = 1.0f;
        player->moveScaleX = 1.0f;
        return;
    }
    if (bombs->timer.HasTicked() && bombs->timer == 0)
    {
        g_Gui.ShowBombName(0x4a1, "霊符「夢想封印　集」");
        bombs->duration = 300;
        player->stateTimer.Set(360);
        SpawnBombAura(player);
        for (i = 0; i < 8; i++)
        {
            bombs->parts[i].state = 0;
        }
        g_ItemManager.CollectAll();
        g_EffectManager.SpawnEffect(0xc, &player->position, 1, 0xff4040ff);
        player->AddHitboxAreaCircle(&player->position, 32.0f, 8.0f, 0x10, 6);
        SetBombDrain(player, 5000, 0.22f);
        player->moveScaleY = 0.6f;
        player->moveScaleX = 0.6f;
    }
    if (bombs->timer >= 60 && bombs->timer < 180)
    {
        if (bombs->timer.AsFrames() % 16 == 0)
        {
            i = (bombs->timer.AsFrames() - 60) / 16;
            orb = &bombs->parts[i];
            if (i != 0)
            {
                orb->state = 1;
                orb->burstTimer = 0;
                orb->homingSpeed = 8.0f;
                orb->pos = player->position;
                x = g_Rng.GetRandomF32ZeroToOne() * 6.2831855f - 3.1415927f;
                ((Vector2 *)&orb->velocity)->SetPolar(x, orb->homingSpeed);
                player->bombHitboxes[i].damageDealt = 0;
                for (spr = orb->sprites, k = 0; k < 4; k++, spr++)
                {
                    g_SpriteRenderer->StartScriptAtOrigin(spr, k + 0x485);
                }
                g_SoundPlayer.QueueSound(0xd, 0);
            }
        }
    }
    player->playerState = 3;
    for (i = 0, orb = bombs->parts; i < 8; i++, orb++)
    {
        if (orb->state == 0)
        {
            continue;
        }
        if (orb->state == 1)
        {
            if (bombs->timer.HasTicked())
            {
                if (player->orbHomingTarget.x > -100.0f)
                {
                    aim = player->orbHomingTarget;
                }
                else
                {
                    aim = player->position;
                }
                x = aim.x - orb->pos.x;
                y = aim.y - orb->pos.y;
                len = sqrtf(x * x + y * y) / (orb->homingSpeed / 8.0f);
                if (len < 1.0f)
                {
                    len = 1.0f;
                }
                x = x / len + orb->velocity.x;
                y = y / len + orb->velocity.y;
                len = sqrtf(x * x + y * y);
                orb->homingSpeed = (len > 10.0f) ? 10.0f : len;
                if (orb->homingSpeed < 1.0f)
                {
                    orb->homingSpeed = 1.0f;
                }
                orb->velocity.x = x * orb->homingSpeed / len;
                orb->velocity.y = y * orb->homingSpeed / len;
                player->bombHitboxes[i].size.x = 48.0f;
                player->bombHitboxes[i].size.y = 48.0f;
                player->bombHitboxes[i].pos = orb->pos;
                player->bombHitboxes[i].damage = 8;
                player->AddHitboxAreaCircle(&orb->pos, 128.0f, 0.0f, 0, 6);
                if (player->bombHitboxes[i].damageDealt >= 100 || bombs->timer >= bombs->duration - 30)
                {
                    g_EffectManager.SpawnEffect(6, &orb->pos, 8, 0xffffffff);
                    g_EffectManager.SpawnEffect(0xc, &orb->pos, 1, 0xff4040ff);
                    orb->state = 2;
                    orb->sprites[0].pendingInterrupt = 1;
                    orb->sprites[1].pendingInterrupt = 1;
                    orb->sprites[2].pendingInterrupt = 1;
                    orb->sprites[3].pendingInterrupt = 1;
                    player->bombHitboxes[i].size.x = 256.0f;
                    player->bombHitboxes[i].size.y = 256.0f;
                    player->bombHitboxes[i].damage = 400;
                    player->AddHitboxAreaCircle(&orb->pos, 32.0f, 6.6666665f, 0xf, 6);
                    D3DXVECTOR3 velocity = orb->velocity / 8.0f;
                    g_SoundPlayer.QueueSound(0xf, 0);
                    ScreenEffect::StartEffect(1, 0x10, 8, 0, 0);
                }
            }
        }
        else if (orb->state != 0 && bombs->timer.HasTicked())
        {
            orb->burstTimer++;
            if (orb->burstTimer >= 30)
            {
                orb->state = 0;
            }
        }
        orb->pos.x += g_App.gameSpeed * orb->velocity.x;
        orb->pos.y += g_App.gameSpeed * orb->velocity.y;
        g_SpriteRenderer->Step(&orb->sprites[0]);
        g_SpriteRenderer->Step(&orb->sprites[1]);
        g_SpriteRenderer->Step(&orb->sprites[2]);
        g_SpriteRenderer->Step(&orb->sprites[3]);
    }
    bombs->timer.Tick();
}

// The same with th06's own macro's stores, for part i of the bomb.
#define DRAW_BOMB_PART_SPRITE()                                         \
    curSprite->pos = player->bomb.parts[i].pos + curSprite->posOffset;  \
    curSprite->pos[0] += g_GameManager.arena[0];                      \
    curSprite->pos[1] += g_GameManager.arena[1];                      \
    curSprite->pos.z = 0.0f;                                            \
    g_SpriteRenderer->DrawUnrotated(curSprite);                           \
    curSprite++;

void DrawReimuABombFocused(Player *player)
{
    Sprite *curSprite;
    int i;

    DimPlayAreaForBomb(player);
    for (i = 0; i < 8; i++)
    {
        if (player->bomb.parts[i].state == 0)
        {
            continue;
        }
        curSprite = player->bomb.parts[i].sprites;
        DRAW_BOMB_PART_SPRITE();
        DRAW_BOMB_PART_SPRITE();
        DRAW_BOMB_PART_SPRITE();
        DRAW_BOMB_PART_SPRITE();
    }
}

void UpdateReimuBBomb(Player *player)
{
    int i;
    Sprite *sprite;
    D3DXVECTOR3 unused;
    PlayerArea *area[4];

    if (player->bomb.timer >= player->bomb.duration)
    {
        g_Gui.EndBombName();
        player->bomb.running = 0;
        return;
    }
    if (player->bomb.timer.HasTicked() && player->bomb.timer == 0)
    {
        g_ItemManager.CollectAll();
        g_Gui.ShowBombName(0x4a1, "夢符「封魔陣」");
        player->bomb.duration = 140;
        player->stateTimer.Set(200);
        SpawnBombAura(player);
        for (i = 0; i < 4; i++)
        {
            sprite = player->bomb.parts[i].sprites;
            g_SpriteRenderer->StartScriptAtOrigin(sprite, i + 0x489);
        }
        g_SoundPlayer.QueueSound(6, 0);
        player->bomb.parts[0].pos.x = player->position.x;
        player->bomb.parts[0].pos.y = 224.0f;
        player->bomb.parts[0].pos.z = 0.42f;
        player->bomb.parts[1].pos.x = 192.0f;
        player->bomb.parts[1].pos.y = player->position.y;
        player->bomb.parts[1].pos.z = 0.415f;
        player->bomb.parts[2].pos.x = player->position.x;
        player->bomb.parts[2].pos.y = 224.0f;
        player->bomb.parts[2].pos.z = 0.41f;
        player->bomb.parts[3].pos.x = 192.0f;
        player->bomb.parts[3].pos.y = player->position.y;
        player->bomb.parts[3].pos.z = 0.405f;
        ScreenEffect::StartEffect(1, 60, 2, 6, 0);
        SetBombDrain(player, 3000, 0.17f);
    }
    else
    {
        if (player->bomb.timer == 60)
        {
            ScreenEffect::StartEffect(1, 80, 20, 0, 0);
        }
        area[0] = player->AddHitboxArea(&player->position, 62.0f, 448.0f, 6);
        area[1] = player->AddHitboxArea(&player->position, 384.0f, 62.0f, 6);
        area[2] = player->AddHitboxArea(&player->position, 62.0f, 448.0f, 6);
        area[3] = player->AddHitboxArea(&player->position, 384.0f, 62.0f, 6);
        for (i = 0; i < 4; i++)
        {
            g_SpriteRenderer->Step(player->bomb.parts[i].sprites);
            if (player->bomb.timer.HasTicked() && player->bomb.timer.AsFrames() % 2 != 0)
            {
                area[i]->x = player->bomb.parts[i].pos.x + player->bomb.parts[i].sprites[0].posOffset.x;
                area[i]->y = player->bomb.parts[i].pos.y + player->bomb.parts[i].sprites[0].posOffset.y;
                player->bombHitboxes[i].size.x = area[i]->width;
                player->bombHitboxes[i].size.y = area[i]->height;
                player->bombHitboxes[i].pos = player->bomb.parts[i].pos + player->bomb.parts[i].sprites[0].posOffset;
                player->bombHitboxes[i].damage = 16;
            }
        }
    }
    player->playerState = 3;
    player->bomb.timer.Tick();
}

void DrawReimuBBomb(Player *player)
{
    Sprite *curSprite;
    int i;

    DimPlayAreaForBomb(player);
    for (i = 0; i < 4; i++)
    {
        curSprite = player->bomb.parts[i].sprites;
        curSprite->pos = player->bomb.parts[i].pos + curSprite->posOffset;
        curSprite->pos.x = g_GameManager.arena[0] + curSprite->pos.x;
        curSprite->pos.y = g_GameManager.arena[1] + curSprite->pos.y;
        curSprite->pos.z = 0.0f;
        g_SpriteRenderer->Draw(curSprite);
    }
}

void UpdateReimuBBombFocused(Player *player)
{
    int i;
    Sprite *sprite;
    D3DXVECTOR3 unused;

    if (player->bomb.timer >= player->bomb.duration)
    {
        g_Gui.EndBombName();
        player->bomb.running = 0;
        player->moveScaleY = 1.0f;
        player->moveScaleX = 1.0f;
        return;
    }
    if (player->bomb.timer.HasTicked() && player->bomb.timer == 0)
    {
        g_ItemManager.CollectAll();
        g_Gui.ShowBombName(0x4a1, "夢符「二重結界」");
        player->bomb.duration = 190;
        player->stateTimer.Set(250);
        SpawnBombAura(player);
        for (sprite = player->bomb.parts[0].sprites, i = 0; i < 3; i++, sprite++)
        {
            g_SpriteRenderer->StartScriptAtOrigin(sprite, i + 0x48d);
        }
        g_SoundPlayer.QueueSound(6, 0);
        ScreenEffect::StartEffect(1, 60, 2, 6, 0);
        player->bomb.startPos = player->position;
        SetBombDrain(player, 3000, 0.17f);
        player->moveScaleY = 0.4f;
        player->moveScaleX = 0.4f;
        player->AddHitboxAreaCircle(&player->position, 192.0f, 0.384f, 210, 6);
    }
    else
    {
        if (player->bomb.timer == 60)
        {
            ScreenEffect::StartEffect(1, 80, 20, 0, 0);
        }
        g_SpriteRenderer->Step(&player->bomb.parts[0].sprites[0]);
        g_SpriteRenderer->Step(&player->bomb.parts[0].sprites[1]);
        g_SpriteRenderer->Step(&player->bomb.parts[0].sprites[2]);
        player->bombHitboxes[0].size.x = 256.0f;
        player->bombHitboxes[0].size.y = 256.0f;
        player->bombHitboxes[0].pos = player->bomb.startPos + player->bomb.parts[0].sprites[0].posOffset;
        player->bombHitboxes[0].damage = 18;
    }
    player->playerState = 3;
    player->bomb.timer.Tick();
}

void DrawReimuBBombFocused(Player *player)
{
    Sprite *curSprite;
    int i;

    DimPlayAreaForBomb(player);
    for (i = 0; i < 3; i++)
    {
        curSprite = &player->bomb.parts[0].sprites[i];
        curSprite->pos = player->bomb.startPos + curSprite->posOffset;
        curSprite->pos.x = g_GameManager.arena[0] + curSprite->pos.x;
        curSprite->pos.y = g_GameManager.arena[1] + curSprite->pos.y;
        curSprite->pos.z = 0.0f;
        g_SpriteRenderer->Draw(curSprite);
    }
}

void UpdateMarisaABomb(Player *player)
{
    int i;
    Sprite *sprite;
    D3DXVECTOR3 unused;
    float theta;

    if (player->bomb.timer >= player->bomb.duration)
    {
        g_Gui.EndBombName();
        player->bomb.running = 0;
        return;
    }
    if (player->bomb.timer.HasTicked() && player->bomb.timer == 0)
    {
        g_ItemManager.CollectAll();
        g_Gui.ShowBombName(0x4a3, "魔符「スターダストレヴァリエ」");
        player->bomb.duration = 200;
        player->stateTimer.Set(250);
        SpawnBombAura(player);
        for (i = 0; i < 8; i++, sprite++)
        {
            sprite = player->bomb.parts[i].sprites;
            g_SpriteRenderer->StartScriptAtOrigin(sprite, i % 3 + 0x405);
            player->bomb.parts[i].pos = player->position;
            theta = (float)i * 6.2831855f / 8.0f;
            ((Vector2 *)&player->bomb.parts[i].velocity)->SetPolar(theta, 2.0f);
            player->bomb.parts[i].velocity.z = 0.0f;
        }
        g_SoundPlayer.QueueSound(6, 0);
        ScreenEffect::StartEffect(1, 120, 4, 1, 0);
        SetBombDrain(player, 8000, 0.3f);
    }
    else
    {
        for (i = 0; i < 8; i++)
        {
            player->bomb.parts[i].pos += player->bomb.parts[i].velocity * g_App.gameSpeed;
            if (player->bomb.timer.HasTicked() && player->bomb.timer.AsFrames() % 3 != 0)
            {
                player->AddHitboxAreaCircle(&player->bomb.parts[i].pos, 96.0f, 0.0f, 0, 6);
                player->bombHitboxes[i].size.x = 128.0f;
                player->bombHitboxes[i].size.y = 128.0f;
                player->bombHitboxes[i].pos = player->bomb.parts[i].pos;
                player->bombHitboxes[i].damage = 8;
            }
            g_SpriteRenderer->Step(player->bomb.parts[i].sprites);
        }
    }
    player->playerState = 3;
    player->bomb.timer.Tick();
}

void DrawMarisaABomb(Player *player)
{
    Sprite *curSprite;
    int i;

    DimPlayAreaForBomb(player);
    for (i = 0; i < 8; i++)
    {
        curSprite = player->bomb.parts[i].sprites;
        curSprite->pos = player->bomb.parts[i].pos;
        curSprite->pos.x = g_GameManager.arena[0] + curSprite->pos.x;
        curSprite->pos.y = g_GameManager.arena[1] + curSprite->pos.y;
        curSprite->pos.z = 0.0f;
        curSprite->scaleY = 3.2f;
        curSprite->scaleX = 3.2f;
        g_SpriteRenderer->Draw(curSprite);
        curSprite->pos -= player->bomb.parts[i].velocity * 6.0f;
        curSprite->pos.x += -32.0f;
        curSprite->pos.y += -32.0f;
        curSprite->pos.z = 0.0f;
        curSprite->scaleY = 2.2f;
        curSprite->scaleX = 2.2f;
        g_SpriteRenderer->Draw(curSprite);
        curSprite->pos -= player->bomb.parts[i].velocity * 2.0f;
        curSprite->pos.x += 64.0f;
        curSprite->pos.y += 64.0f;
        curSprite->pos.z = 0.0f;
        curSprite->pos -= player->bomb.parts[i].velocity * 2.0f;
        curSprite->pos.x += -32.0f;
        curSprite->pos.y += -32.0f;
        curSprite->pos.z = 0.0f;
        curSprite->scaleY = 1.0f;
        curSprite->scaleX = 1.0f;
        g_SpriteRenderer->Draw(curSprite);
        curSprite++;
    }
}

void UpdateMarisaABombFocused(Player *player)
{
    PlayerBombPart *orb;
    int i;
    Sprite *sprite;
    int k;
    D3DXVECTOR3 unused;
    float theta;

    if (player->bomb.timer >= player->bomb.duration)
    {
        g_Gui.EndBombName();
        player->bomb.running = 0;
        player->moveScaleY = 1.0f;
        player->moveScaleX = 1.0f;
        return;
    }
    if (player->bomb.timer.HasTicked() && player->bomb.timer == 0)
    {
        g_ItemManager.CollectAll();
        g_Gui.ShowBombName(0x4a2, "魔符「ミルキーウェイ」");
        player->bomb.duration = 260;
        player->stateTimer.Set(310);
        SpawnBombAura(player);
        for (i = 0; i < 24; i++)
        {
            player->bomb.parts[i].state = 0;
        }
        SetBombDrain(player, 9000, 0.33f);
        player->moveScaleY = 0.4f;
        player->moveScaleX = 0.4f;
    }
    if (player->bomb.timer.HasTicked() && player->bomb.timer.current % 6 == 0)
    {
        i = player->bomb.timer.AsFrames() / 6;
        if (i < 24)
        {
            sprite = player->bomb.parts[i].sprites;
            g_SpriteRenderer->StartScriptAtOrigin(sprite, i % 3 + 0x405);
            player->bomb.parts[i].pos = player->position;
            for (k = 0; k < 8; k++)
            {
                player->bomb.parts[i].trail[k] = player->position;
            }
            player->bomb.parts[i].state = 1;
            theta = g_Rng.GetRandomF32ZeroToOne() * 0.3926991f - 0.19634955f - 1.5707964f;
            ((Vector2 *)&player->bomb.parts[i].velocity)->SetPolar(theta, -5.0f);
            player->bomb.parts[i].velocity.z = 0.0f;
            theta = g_Rng.GetRandomF32ZeroToOne() * 0.3926991f - 0.19634955f - 1.5707964f;
            ((Vector2 *)&player->bomb.parts[i].acceleration)->SetPolar(theta, 0.24f);
            player->bomb.parts[i].acceleration.z = 0.0f;
            g_SoundPlayer.QueueSound(7, 0);
            ScreenEffect::StartEffect(1, 120, 4, 1, 0);
            player->bombHitboxes[i].damageDealt = 0;
        }
    }
    for (orb = player->bomb.parts, i = 0; i < 24; i++, orb++)
    {
        if (orb->state == 0)
        {
            continue;
        }
        for (k = 7; k > 0; k--)
        {
            orb->trail[k] = orb->trail[k - 1];
        }
        orb->trail[0] = orb->pos;
        orb->pos += orb->velocity * g_App.gameSpeed;
        orb->velocity += orb->acceleration * g_App.gameSpeed;
        if (orb->pos.y < -256.0f)
        {
            orb->state = 0;
        }
        player->AddHitboxAreaCircle(&orb->pos, 96.0f, 0.0f, 0, 6);
        if (player->bombHitboxes[i].damageDealt < 80)
        {
            player->bombHitboxes[i].size.x = 128.0f;
            player->bombHitboxes[i].size.y = 128.0f;
            player->bombHitboxes[i].pos = orb->pos;
            player->bombHitboxes[i].damage = 12;
        }
        g_SpriteRenderer->Step(orb->sprites);
    }
    player->playerState = 3;
    player->bomb.timer.Tick();
}

void DrawMarisaABombFocused(Player *player)
{
    Sprite *curSprite;
    PlayerBombPart *orb;
    int i;

    DimPlayAreaForBomb(player);
    for (orb = player->bomb.parts, i = 0; i < 24; i++, orb++)
    {
        if (player->bomb.parts[i].state == 0)
        {
            continue;
        }
        curSprite = orb->sprites;
        curSprite->pos = orb->pos;
        curSprite->pos.x = g_GameManager.arena[0] + curSprite->pos.x;
        curSprite->pos.y = g_GameManager.arena[1] + curSprite->pos.y;
        curSprite->pos.z = 0.3f;
        curSprite->scaleY = 3.2f;
        curSprite->scaleX = 3.2f;
        g_SpriteRenderer->Draw(curSprite);
        curSprite->pos = orb->trail[3];
        curSprite->pos.x = g_GameManager.arena[0] + curSprite->pos.x;
        curSprite->pos.y = g_GameManager.arena[1] + curSprite->pos.y;
        curSprite->pos.z = 0.32f;
        curSprite->scaleY = 2.2f;
        curSprite->scaleX = 2.2f;
        g_SpriteRenderer->Draw(curSprite);
        curSprite->pos = orb->trail[7];
        curSprite->pos.x = g_GameManager.arena[0] + curSprite->pos.x;
        curSprite->pos.y = g_GameManager.arena[1] + curSprite->pos.y;
        curSprite->pos.z = 0.34f;
        curSprite->scaleY = 1.3f;
        curSprite->scaleX = 1.3f;
        g_SpriteRenderer->Draw(curSprite);
        curSprite++;
    }
}

void UpdateMarisaBBomb(Player *player)
{
    int i;
    PlayerBombPart *laser;
    PlayerBombHit *hit;
    float len;
    int k;
    D3DXVECTOR3 unused;
    float theta;

    if (player->bomb.timer >= player->bomb.duration)
    {
        g_Gui.EndBombName();
        player->bomb.running = 0;
        player->moveScaleY = 1.0f;
        player->moveScaleX = 1.0f;
        return;
    }
    if (player->bomb.timer.HasTicked() && player->bomb.timer == 0)
    {
        g_ItemManager.CollectAll();
        player->bomb.startPos = player->position;
        g_Gui.ShowBombName(0x4a1, "恋符「ノンディレクショナルレーザー」");
        player->bomb.duration = 300;
        player->stateTimer.Set(300);
        SpawnBombAura(player);
        for (laser = player->bomb.parts, i = 0; i < 3; i++, laser++)
        {
            g_SpriteRenderer->StartScriptAtOrigin(laser->sprites, i + 0x40c);
            laser->pos = player->position;
            laser->laserAngle = i * 6.2831855f / 3.0f + -1.5707964f;
        }
        g_SoundPlayer.QueueSound(0x13, 0);
        player->moveScaleY = 0.4f;
        player->moveScaleX = 0.4f;
        SetBombDrain(player, 8000, 0.35f);
    }
    else
    {
        for (laser = player->bomb.parts, hit = player->bombHitboxes, i = 0; i < 3; i++, laser++)
        {
            if (player->bomb.startPos.x < 192.0f)
            {
                laser->laserAngle = AddNormalizeAngle(laser->laserAngle, player->bomb.timer.AsFloat() * 3.1415927f / 30.0f / player->bomb.duration);
            }
            else
            {
                laser->laserAngle = AddNormalizeAngle(laser->laserAngle, player->bomb.timer.AsFloat() * -3.1415927f / 30.0f / player->bomb.duration);
            }
            theta = laser->laserAngle;
            laser->sprites[0].pos = player->position;
            laser->sprites[0].pos.x += cosf(theta) * laser->sprites[0].spriteData->height * laser->sprites[0].scaleX / 2.0f;
            laser->sprites[0].pos.y += sinf(theta) * laser->sprites[0].spriteData->height * laser->sprites[0].scaleX / 2.0f;
            len = 32.0f;
            for (k = 0; k < 6; k++, hit++)
            {
                hit->pos = player->position;
                hit->pos.x += cosf(theta) * len;
                hit->pos.y += sinf(theta) * len;
                hit->size.x = 128.0f;
                hit->size.y = 128.0f;
                hit->damage = 10;
                player->AddHitboxAreaCircle(&hit->pos, 64.0f, 0.0f, 0, 6);
                len += laser->sprites[0].spriteData->height * laser->sprites[0].scaleX / 5.0f;
            }
            g_SpriteRenderer->Step(laser->sprites);
        }
        if (player->bomb.timer == 20)
        {
            ScreenEffect::StartEffect(1, 60, 1, 7, 0);
        }
        else if (player->bomb.timer == 80)
        {
            ScreenEffect::StartEffect(1, 100, 24, 0, 0);
        }
    }
    player->playerState = 3;
    player->bomb.timer.Tick();
}

void DrawMarisaBBomb(Player *player)
{
    Sprite *curSprite;
    int i;
    float theta;

    DimPlayAreaForBomb(player);
    for (i = 0; i < 3; i++)
    {
        curSprite = player->bomb.parts[i].sprites;
        theta = player->bomb.parts[i].laserAngle;
        curSprite->pos = player->position;
        curSprite->pos.x += cosf(theta) * curSprite->spriteData->height * curSprite->scaleX / 2.0f;
        curSprite->pos.y += sinf(theta) * curSprite->spriteData->height * curSprite->scaleX / 2.0f;
        curSprite->SetRotationZ(AddNormalizeAngle(theta, 1.5707964f));
        curSprite->pos.x = g_GameManager.arena[0] + curSprite->pos.x;
        curSprite->pos.y = g_GameManager.arena[1] + curSprite->pos.y;
        curSprite->pos.z = 0.0f;
        g_SpriteRenderer->Draw(curSprite);
        curSprite++;
    }
}

void UpdateMarisaBBombFocused(Player *player)
{
    int i;
    Sprite *sprite;
    D3DXVECTOR3 unused;

    if (player->bomb.timer >= player->bomb.duration)
    {
        g_Gui.EndBombName();
        player->bomb.running = 0;
        player->moveScaleY = 1.0f;
        player->moveScaleX = 1.0f;
        return;
    }
    if (player->bomb.timer.HasTicked() && player->bomb.timer == 0)
    {
        g_ItemManager.CollectAll();
        g_Gui.ShowBombName(0x4a2, "恋符「マスタースパーク」");
        player->bomb.duration = 340;
        player->stateTimer.Set(390);
        SpawnBombAura(player);
        for (sprite = player->bomb.parts[0].sprites, i = 0; i < 4; i++, sprite++)
        {
            g_SpriteRenderer->StartScriptAtOrigin(sprite, i + 0x408);
            player->bomb.parts[i].pos = player->position;
        }
        g_SoundPlayer.QueueSound(0x13, 0);
        player->moveScaleY = 0.2f;
        player->moveScaleX = 0.2f;
        SetBombDrain(player, 10000, 0.41f);
    }
    else
    {
        if (player->bomb.timer == 60)
        {
            ScreenEffect::StartEffect(1, 60, 1, 7, 0);
        }
        else if (player->bomb.timer == 120)
        {
            ScreenEffect::StartEffect(1, 200, 24, 0, 0);
        }
        if (player->bomb.timer.HasTicked() && player->bomb.timer.AsFrames() % 4 != 0)
        {
            player->bombHitboxes[0].size.x = 384.0f;
            player->bombHitboxes[0].size.y = player->position.y;
            player->bombHitboxes[0].pos.x = 192.0f;
            player->bombHitboxes[0].pos.y = player->position.y / 2.0f;
            player->bombHitboxes[0].damage = 23;
            player->AddHitboxArea(&player->bombHitboxes[0].pos, 384.0f, player->position.y, 6);
        }
        g_SpriteRenderer->Step(&player->bomb.parts[0].sprites[0]);
        g_SpriteRenderer->Step(&player->bomb.parts[0].sprites[1]);
        g_SpriteRenderer->Step(&player->bomb.parts[0].sprites[2]);
        g_SpriteRenderer->Step(&player->bomb.parts[0].sprites[3]);
    }
    player->playerState = 3;
    player->bomb.timer.Tick();
}

void DrawMarisaBBombFocused(Player *player)
{
    Sprite *curSprite;
    int i;
    float angle;

    DimPlayAreaForBomb(player);
    curSprite = player->bomb.parts[0].sprites;
    for (i = 0; i < 4; i++)
    {
        angle = 0.62831855f * (float)i / 3.0f - 3.1415927f + 1.2566371f;
        curSprite->pos = player->position;
        curSprite->pos.x += cosf(angle) * curSprite->spriteData->height * curSprite->scaleX / 2.0f;
        curSprite->pos.y += sinf(angle) * curSprite->spriteData->height * curSprite->scaleX / 2.0f;
        curSprite->SetRotationZ(AddNormalizeAngle(angle, 1.5707964f));
        curSprite->pos.x = g_GameManager.arena[0] + curSprite->pos.x;
        curSprite->pos.y = g_GameManager.arena[1] + curSprite->pos.y;
        curSprite->pos.z = 0.0f;
        g_SpriteRenderer->Draw(curSprite);
        curSprite++;
    }
}

void UpdateSakuyaABomb(Player *player)
{
    int i;
    Sprite *spr;
    PlayerBombPart *knife;
    D3DXVECTOR3 tmp;
    float angle;
    int remaining;

    if (player->bomb.timer >= player->bomb.duration)
    {
        g_Gui.EndBombName();
        player->bomb.running = 0;
        return;
    }
    if (player->bomb.timer.HasTickedTo(0))
    {
        g_ItemManager.CollectAll();
        g_Gui.ShowBombName(0x4a1, "幻符「インディスクリミネイト」");
        player->bomb.duration = 160;
        player->stateTimer.Set(210);
        SpawnBombAura(player);
        player->bomb.startPos = player->position;
        for (knife = player->bomb.parts, i = 0; i < 0x60; i++, knife++)
        {
            knife->state = 0;
        }
        SetBombDrain(player, 6000, 0.28f);
        player->bomb.parts[0].effect = g_EffectManager.SpawnEffect(0x15, &player->position, 1, 0xffffffff);
        g_SoundPlayer.QueueSound(5, 0);
    }
    if (player->bomb.timer >= 60)
    {
        remaining = 5;
        for (knife = player->bomb.parts, i = 0; i < 0x60; i++, knife++)
        {
            if (knife->state == 0)
            {
                if (player->bomb.timer <= 120 && remaining != 0)
                {
                    knife->state = 1;
                    spr = knife->sprites;
                    g_SpriteRenderer->StartScriptAtOrigin(spr, (i & 1) + 0x405);
                    angle = g_Rng.GetRandomF32ZeroToOne() * 6.2831855f - 3.1415927f;
                    knife->angle = angle;
                    knife->speed = g_Rng.GetRandomF32ZeroToOne() * 6.0f + 5.5f;
                    knife->speedStep = g_Rng.GetRandomF32ZeroToOne() * 0.1f + 0.1f;
                    knife->acceleration.x = g_Rng.GetRandomF32ZeroToOne() * 0.06283186f - 0.031415927f;
                    knife->velocity.x = cosf(knife->angle) * 24.0f;
                    knife->velocity.y = sinf(knife->angle) * 24.0f;
                    knife->pos = player->bomb.startPos + knife->velocity;
                    knife->velocity.z = 0.0f;
                    player->bombHitboxes[i].damageDealt = 0;
                    remaining--;
                }
                continue;
            }
            else
            {
                knife->angle = AddNormalizeAngle(knife->angle, knife->acceleration.x);
                knife->speed += knife->speedStep;
                knife->velocity.x = cosf(knife->angle) * knife->speed;
                knife->velocity.y = sinf(knife->angle) * knife->speed;
                if (player->bombHitboxes[i].damageDealt < 30)
                {
                    knife->pos += knife->velocity * g_App.gameSpeed;
                    player->AddHitboxAreaCircle(&knife->pos, 32.0f, 0.0f, 0, 6);
                    player->bombHitboxes[i].size.x = 24.0f;
                    player->bombHitboxes[i].size.y = 24.0f;
                    player->bombHitboxes[i].pos = knife->pos;
                    player->bombHitboxes[i].damage = 10;
                }
                else if (player->bombHitboxes[i].damageDealt < 999)
                {
                    g_SpriteRenderer->StartScriptAtOrigin(knife->sprites, 0x460);
                    player->bombHitboxes[i].damageDealt = 999;
                }
                if (!g_GameManager.IsInPlayArea(knife->pos.x, knife->pos.y, 64.0f, 64.0f))
                {
                    knife->state = 0;
                }
                g_SpriteRenderer->Step(knife->sprites);
            }
        }
    }
    player->playerState = 3;
    player->bomb.timer.Tick();
}

void DrawSakuyaABomb(Player *player)
{
    int i;
    PlayerBombPart *knife;

    DimPlayAreaForBomb(player);
    for (knife = player->bomb.parts, i = 0; i < 0x60; i++, knife++)
    {
        if (knife->state == 0)
        {
            continue;
        }
        knife->sprites[0].SetRotationZ(AddNormalizeAngle(knife->angle, 1.5707964f));
        knife->sprites[0].pos = knife->pos;
        knife->sprites[0].pos.z = 0.0f;
        g_SpriteRenderer->Draw(knife->sprites);
    }
}

void UpdateSakuyaABombFocused(Player *player)
{
    int i;
    Sprite *spr;
    PlayerBombPart *knife;
    D3DXVECTOR3 tmp;
    float angle;

    if (player->bomb.timer >= player->bomb.duration)
    {
        g_Gui.EndBombName();
        player->bomb.running = 0;
        player->moveScaleY = 1.0f;
        player->moveScaleX = 1.0f;
        return;
    }
    if (player->bomb.timer.HasTickedTo(0))
    {
        g_ItemManager.CollectAll();
        g_Gui.ShowBombName(0x4a1, "幻符「殺人ドール」");
        player->bomb.duration = 250;
        player->stateTimer.Set(290);
        SpawnBombAura(player);
        for (knife = player->bomb.parts, i = 0; i < 0x60; i++, knife++)
        {
            knife->state = 0;
        }
        SetBombDrain(player, 6500, 0.29f);
        player->moveScaleY = 0.3f;
        player->moveScaleX = 0.3f;
        player->bomb.parts[0].effect = g_EffectManager.SpawnEffect(0x15, &player->position, 1, 0xffffffff);
        g_SoundPlayer.QueueSound(5, 0);
    }
    if (player->bomb.timer >= 0 && player->bomb.timer <= 60)
    {
        player->bomb.parts[0].effect->pos = player->position;
    }
    if (player->bomb.timer >= 20 && player->bomb.timer < 116)
    {
        for (knife = player->bomb.parts, i = 0; i < 0x60; i++, knife++)
        {
            if (!player->bomb.timer.HasTickedTo(i % 48 * 2 + 20))
            {
                continue;
            }
            knife->state = 1;
            spr = knife->sprites;
            g_SpriteRenderer->StartScriptAtOrigin(spr, (i & 1) + 0x407);
            angle = i * 6.2831855f / 96.0f - 3.1415927f;
            knife->angle = angle;
            knife->speed = g_Rng.GetRandomF32ZeroToOne() * 1.0f + 0.5f;
            knife->speedStep = g_Rng.GetRandomF32ZeroToOne() * 0.1f + 0.03f;
            knife->acceleration.x = g_Rng.GetRandomU16InRange(1) ? 0.15707964f : -0.15707964f;
            knife->velocity.x = cosf(knife->angle) * 24.0f;
            knife->velocity.y = sinf(knife->angle) * 24.0f;
            knife->pos = player->position + knife->velocity;
            knife->timer.Set(0);
            knife->velocity.z = 0.0f;
            player->bombHitboxes[i].damageDealt = 0;
        }
        g_SoundPlayer.QueueSound(6, 0);
        ScreenEffect::StartEffect(1, 120, 4, 1, 0);
    }
    for (knife = player->bomb.parts, i = 0; i < 0x60; i++, knife++)
    {
        if (knife->state == 0)
        {
            continue;
        }
        if (knife->timer.AsFrames() < 30 || knife->timer.AsFrames() >= 70)
        {
            if (knife->timer.HasTickedTo(70))
            {
                if (player->orbHomingTarget.x > -100.0f)
                {
                    knife->angle = AddNormalizeAngle(atan2f(player->orbHomingTarget.y - knife->pos.y, player->orbHomingTarget.x - knife->pos.x), 0.0f);
                }
                knife->speed = 14.0f;
            }
            knife->speed += knife->speedStep;
            knife->velocity.x = cosf(knife->angle) * knife->speed;
            knife->velocity.y = sinf(knife->angle) * knife->speed;
        }
        else
        {
            knife->angle = AddNormalizeAngle(knife->angle, knife->acceleration.x);
            knife->velocity.x = 0.0f;
            knife->velocity.y = 0.0f;
        }
        if (player->bombHitboxes[i].damageDealt == 0)
        {
            knife->pos += knife->velocity * g_App.gameSpeed;
            player->AddHitboxAreaCircle(&player->bomb.parts[i].pos, 32.0f, 0.0f, 0, 6);
            player->bombHitboxes[i].size.x = 24.0f;
            player->bombHitboxes[i].size.y = 24.0f;
            player->bombHitboxes[i].pos = knife->pos;
            player->bombHitboxes[i].damage = 22;
        }
        else if (player->bombHitboxes[i].damageDealt < 999)
        {
            g_SpriteRenderer->StartScriptAtOrigin(knife->sprites, 0x460);
            player->bombHitboxes[i].damageDealt = 999;
            g_EffectManager.SpawnEffect(0, &player->bomb.parts[i].pos, 1, 0xffff80ff);
        }
        g_SpriteRenderer->Step(knife->sprites);
        knife->timer.Tick();
    }
    player->playerState = 3;
    player->bomb.timer.Tick();
}

void DrawSakuyaABombFocused(Player *player)
{
    int i;
    PlayerBombPart *knife;

    DimPlayAreaForBomb(player);
    for (knife = player->bomb.parts, i = 0; i < 0x60; i++, knife++)
    {
        if (knife->state == 0)
        {
            continue;
        }
        knife->sprites[0].SetRotationZ(AddNormalizeAngle(knife->angle, 1.5707964f));
        knife->sprites[0].pos = knife->pos;
        knife->sprites[0].pos.x = g_GameManager.arena[0] + knife->sprites[0].pos.x;
        knife->sprites[0].pos.y = g_GameManager.arena[1] + knife->sprites[0].pos.y;
        knife->sprites[0].pos.z = 0.0f;
        g_SpriteRenderer->Draw(knife->sprites);
    }
}

void UpdateSakuyaBBomb(Player *player)
{
    int i;
    Sprite *spr;
    PlayerBombPart *knife;
    D3DXVECTOR3 tmp;

    if (player->bomb.timer >= player->bomb.duration)
    {
        g_Gui.EndBombName();
        player->bomb.running = 0;
        player->moveScaleY = 1.0f;
        player->moveScaleX = 1.0f;
        player->AddHitboxAreaCircle(&player->position, 800.0f, 0.0f, 0, 6);
        return;
    }
    if (player->bomb.timer.HasTickedTo(0))
    {
        g_ItemManager.CollectAll();
        g_Gui.ShowBombName(0x4a3, "時符「パーフェクトスクウェア」");
        player->bomb.duration = 160;
        player->stateTimer.Set(260);
        SpawnBombAura(player);
        for (knife = player->bomb.parts, i = 0; i < 4; i++, knife++)
        {
            knife->state = 0;
        }
        g_SoundPlayer.QueueSound(0x13, 0);
        SetBombDrain(player, 5500, 0.26f);
        player->moveScaleY = 2.0f;
        player->moveScaleX = 2.0f;
        g_BulletManager.StopAllBullets();
    }
    if (player->bomb.timer.HasTickedTo(60))
    {
        g_BulletManager.StopAllBullets();
    }
    if (player->bomb.timer.HasTickedTo(120))
    {
        g_BulletManager.StopAllBullets();
    }
    if (player->bomb.timer.HasTickedTo(30))
    {
        for (knife = player->bomb.parts, i = 0; i < 4; i++, knife++)
        {
            knife->state = 1;
            spr = knife->sprites;
            g_SpriteRenderer->StartScriptAtOrigin(spr, i + 0x409);
            spr->pos.x = 192.0f + ((i & 1) ? 128.0f : -128.0f);
            spr->pos.y = 224.0f + ((i / 2) ? 128.0f : -128.0f);
            spr->pos.z = 0.49f;
            knife->velocity.z = 0.0f;
        }
    }
    if (player->bomb.timer >= 30 && player->bomb.timer.HasTicked() && player->bomb.timer.current % 4 == 0)
    {
        player->bombHitboxes[0].pos.x = 192.0f;
        player->bombHitboxes[0].pos.y = 224.0f;
        player->bombHitboxes[0].size.x = 352.0f;
        player->bombHitboxes[0].size.y = 416.0f;
        player->bombHitboxes[0].damage = 3;
    }
    if (player->bomb.timer.HasTickedTo(40))
    {
        ScreenEffect::StartEffect(1, 60, 1, 7, 0);
    }
    if (player->bomb.timer.HasTickedTo(100))
    {
        ScreenEffect::StartEffect(1, 70, 24, 0, 0);
    }
    for (knife = player->bomb.parts, i = 0; i < 4; i++, knife++)
    {
        if (knife->state == 0)
        {
            continue;
        }
        g_SpriteRenderer->Step(knife->sprites);
    }
    player->playerState = 3;
    player->bomb.timer.Tick();
}

void DrawSakuyaBBomb(Player *player)
{
    int i;
    PlayerBombPart *knife;

    DimPlayAreaForBomb(player);
    for (knife = player->bomb.parts, i = 0; i < 4; i++, knife++)
    {
        if (knife->state == 0)
        {
            continue;
        }
        knife->sprites[0].pos.x = g_GameManager.arena[0] + knife->sprites[0].pos.x;
        knife->sprites[0].pos.y = g_GameManager.arena[1] + knife->sprites[0].pos.y;
        knife->sprites[0].pos.z = 0.0f;
        g_SpriteRenderer->Draw(knife->sprites);
        knife->sprites[0].pos.x -= g_GameManager.arena[0];
        knife->sprites[0].pos.y -= g_GameManager.arena[1];
    }
}

void UpdateSakuyaBBombFocused(Player *player)
{
    int i;
    Sprite *spr;
    PlayerBombPart *knife;
    D3DXVECTOR3 tmp;
    int k;

    if (player->bomb.timer >= player->bomb.duration)
    {
        g_Gui.EndBombName();
        player->bomb.running = 0;
        player->moveScaleY = 1.0f;
        player->moveScaleX = 1.0f;
        player->AddHitboxAreaCircle(&player->position, 800.0f, 0.0f, 0, 6);
        player->hitAreas[0].x = 192.0f;
        player->hitAreas[0].y = 224.0f;
        player->hitAreas[0].width = 448.0f;
        player->hitAreas[0].height = 512.0f;
        return;
    }
    if (player->bomb.timer.HasTickedTo(0))
    {
        g_ItemManager.CollectAll();
        g_Gui.ShowBombName(0x4a3, "時符「プライベートスクウェア」");
        player->bomb.duration = 300;
        player->stateTimer.Set(420);
        SpawnBombAura(player);
        player->bombJustStarted = 0;
        for (knife = player->bomb.parts, i = 0; i < 2; i++, knife++)
        {
            knife->state = 1;
            spr = knife->sprites;
            g_SpriteRenderer->StartScriptAtOrigin(spr, i + 0x40d);
            knife->pos = player->position;
            for (k = 31; k >= 0; k--)
            {
                knife->trail[k] = knife->pos;
            }
            knife->velocity.x = 0.0f;
            knife->velocity.y = 0.0f;
            knife->velocity.z = 0.0f;
            knife->acceleration.x = 0.0f;
            knife->acceleration.y = -0.008f;
            knife->acceleration.z = 0.0f;
        }
        player->moveScaleY = 1.5f;
        player->moveScaleX = 1.5f;
        g_SoundPlayer.QueueSound(0x13, 0);
        SetBombDrain(player, 6000, 0.29f);
    }
    player->AddHitboxAreaCircle(&player->bomb.parts[0].pos, 96.0f, 0.0f, 0, 6);
    player->bombHitboxes[0].pos.x = player->bomb.parts[0].pos.x;
    player->bombHitboxes[0].pos.y = player->bomb.parts[0].pos.y;
    player->bombHitboxes[0].size.x = 160.0f;
    player->bombHitboxes[0].size.y = 160.0f;
    player->bombHitboxes[0].damage = 1;
    if (player->bomb.timer.HasTickedTo(40))
    {
        ScreenEffect::StartEffect(1, 60, 1, 7, 0);
        g_BulletManager.StopAllBullets();
    }
    if (player->bomb.timer.HasTickedTo(100))
    {
        g_BulletManager.StopAllBullets();
        ScreenEffect::StartEffect(1, 70, 24, 0, 0);
    }
    for (knife = player->bomb.parts, i = 0; i < 2; i++, knife++)
    {
        if (knife->state == 0)
        {
            continue;
        }
        knife->acceleration = (player->position - knife->pos) / 1700.0f * g_App.gameSpeed;
        knife->velocity += knife->acceleration * g_App.gameSpeed;
        for (k = 31; k > 0; k--)
        {
            knife->trail[k] = knife->trail[k - 1];
        }
        knife->trail[0] = knife->pos;
        knife->pos += knife->velocity * g_App.gameSpeed;
        g_SpriteRenderer->Step(knife->sprites);
    }
    player->playerState = 3;
    player->bomb.timer.Tick();
}

void DrawSakuyaBBombFocused(Player *player)
{
    int alpha;
    int i;
    PlayerBombPart *knife;
    int k;

    DimPlayAreaForBomb(player);
    for (knife = player->bomb.parts, i = 0; i < 2; i++, knife++)
    {
        if (knife->state == 0)
        {
            continue;
        }
        alpha = knife->sprites[0].alpha;
        knife->sprites[0].pos = knife->pos;
        knife->sprites[0].pos.x = g_GameManager.arena[0] + knife->sprites[0].pos.x;
        knife->sprites[0].pos.y = g_GameManager.arena[1] + knife->sprites[0].pos.y;
        knife->sprites[0].pos.z = 0.0f;
        g_SpriteRenderer->Draw(knife->sprites);
        for (k = 3; k < 32; k += 4)
        {
            knife->sprites[0].pos = knife->trail[k];
            knife->sprites[0].alpha = alpha - alpha * k / 32;
            knife->sprites[0].pos.x = g_GameManager.arena[0] + knife->sprites[0].pos.x;
            knife->sprites[0].pos.y = g_GameManager.arena[1] + knife->sprites[0].pos.y;
            knife->sprites[0].pos.z = 0.0f;
            g_SpriteRenderer->Draw(knife->sprites);
        }
        knife->sprites[0].alpha = alpha;
    }
}

#ifdef TH07_EXE
#include <new>
// Unused, like UnusedSetNewMode (enemyscript.cpp), UnusedDeleteArray
// (midi.cpp) and UnusedNewArray (screeneffect.cpp): the Rich header
// counts four more plain C++ objects than libcmt's C++ members we load,
// so discarded code in the original pulled four more, ones without data
// (no .data/.rdata byte tells them apart). Which four, and from where,
// is a guess: these keep the original's Utc13_CPP count.
void UnusedDeleteNothrow(void *p, const std::nothrow_t &nothrow)
{
    operator delete(p, nothrow);
}
#endif // TH07_EXE

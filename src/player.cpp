// The code from 0x43b5c0: helpers out of line, then what looks like
// th06's player.cpp (0x43b7f0 constructs the static at 0x4bdad8).

#include <math.h>

#include "ascii.h"
#include "effect.h"
#include "player.h"
#include "replay.h"
#include "unsorted.h"

void Unknown626270::RerollLifeChecksumKeys()
{
    this->stats->unk_34[0] = g_Rng.GetRandomU32InRange(100000) + 6543;
    this->stats->unk_34[1] = g_Rng.GetRandomU32InRange(100000) + 6543;
    this->stats->unk_34[2] = g_Rng.GetRandomU32InRange(100000) + 6543;
    this->stats->unk_34[3] = g_Rng.GetRandomU32InRange(100000) + 6543;
    this->stats->unk_34[4] = g_Rng.GetRandomU32InRange(100000) + 6543;
    this->stats->unk_70[0] = g_Rng.GetRandomF32ZeroToOne() * 100000.0f + 6543.0f;
    this->stats->unk_70[1] = g_Rng.GetRandomF32ZeroToOne() * 100000.0f + 6543.0f;
    this->stats->unk_70[2] = g_Rng.GetRandomF32ZeroToOne() * 100000.0f + 6543.0f;
}

void Unknown626270::AddDeaths(int deaths)
{
    if (this->Func404fe0())
    {
        memset(&g_App, 0xff, 0x2d0);
    }
    this->stats->deaths += deaths;
    this->Func4012b0();
}

void Unknown626270::AddBombsUsed(int bombs)
{
    if (this->Func404fe0())
    {
        memset(&g_App, 0xff, 0x2d0);
    }
    this->stats->bombsUsed += bombs;
    this->Func4012b0();
}

Player::Player()
{
}

void FireShot(Player *player, PlayerBullet *bullet, ShotData *sd)
{
    if (sd->option == 0)
    {
        bullet->pos = player->position;
    }
    else
    {
        bullet->pos = player->optionPos[sd->option - 1];
    }
    bullet->pos[0] += sd->offsetX;
    bullet->pos[1] += sd->offsetY;
    bullet->pos.z = 0.495f;
    bullet->size.x = sd->sizeX;
    bullet->size.y = sd->sizeY;
    bullet->size.z = 1.0f;
    bullet->angle = sd->angle;
    bullet->speed = sd->speed;
    bullet->velocity.x = cosf(sd->angle) * sd->speed;
    bullet->velocity.y = sinf(sd->angle) * sd->speed;
    bullet->timer.Set(0);
    bullet->type = sd->type;
    bullet->damage = sd->damage;
    if (sd->sound >= 0)
    {
        g_SoundPlayer.QueueSound(sd->sound, 0);
    }
    g_SpriteEngine->StartScript(&bullet->sprite, sd->script);
}

int TryFireShot(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    if (frame % sd->interval == sd->phase)
    {
        FireShot(player, bullet, sd);
        return 1;
    }
    return 0;
}

int FireLaser(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    int laser = sd->phase;

    if (player->lasers[laser].bullet != NULL)
    {
        if (player->laserShots[laser] != sd)
        {
            player->lasers[laser].bullet->sprite.pendingInterrupt = 1;
            player->lasers[laser].bullet = NULL;
        }
        return 0;
    }
    if (player->unk_240a != 1)
    {
        return 0;
    }
    player->lasers[laser].timer.Set(sd->interval);
    player->lasers[laser].bullet = bullet;
    bullet->laserIndex = laser;
    bullet->option = sd->option;
    bullet->velocity.z = sd->offsetX;
    bullet->laserOffsetY = sd->offsetY;
    FireShot(player, bullet, sd);
    player->laserShots[laser] = sd;
    return 1;
}

int FireTrailLaser(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    int laser = sd->phase;
    int j;

    if (player->lasers[laser].bullet != NULL)
    {
        if (player->laserShots[laser] != sd)
        {
            player->lasers[laser].bullet->sprite.pendingInterrupt = 1;
            player->lasers[laser].bullet = NULL;
        }
        return 0;
    }
    if (player->unk_240a != 3)
    {
        return 0;
    }
    player->lasers[laser].timer.Set(999);
    player->lasers[laser].bullet = bullet;
    bullet->laserIndex = laser;
    bullet->option = sd->option;
    bullet->velocity.z = sd->offsetX;
    bullet->laserOffsetY = sd->offsetY;
    bullet->unk_352 = sd->interval;
    FireShot(player, bullet, sd);
    for (j = 15; j >= 0; j--)
    {
        bullet->unk_258[j].x = -999.0f;
    }
    bullet->pos.x = -999.0f;
    player->laserShots[laser] = sd;
    return 1;
}

int FireHomingShot(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    float diffY;
    float diffX;
    float angle;
    float speed;

    if (frame % sd->interval == sd->phase)
    {
        FireShot(player, bullet, sd);
        if (player->homingTarget.x > -100.0f)
        {
            diffX = player->homingTarget.x - bullet->pos.x;
            diffY = player->homingTarget.y - bullet->pos.y;
            angle = AddNormalizeAngle(atan2f(diffY, diffX), sd->angle + 1.5707964f);
            speed = sd->speed * 1.5f;
            ((Unknown4325c0 *)&bullet->velocity)->SetPolar(angle, speed);
            bullet->angle = angle;
        }
        return 1;
    }
    return 0;
}

int FireAimedShot(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    float angle;
    float speed;

    if (frame % sd->interval == sd->phase)
    {
        FireShot(player, bullet, sd);
        angle = AddNormalizeAngle(player->unk_b7e58, sd->angle + 1.5707964f);
        speed = sd->speed;
        ((Unknown4325c0 *)&bullet->velocity)->SetPolar(angle, speed);
        bullet->angle = angle;
        return 1;
    }
    return 0;
}

int UpdateHomingOrb(Player *player, PlayerBullet *bullet)
{
    float x;
    float y;
    float len;

    if (bullet->state == 1)
    {
        if (player->unk_2428.x > -100.0f && bullet->timer.AsFrames() < 40 && bullet->timer.HasTicked())
        {
            x = player->unk_2428.x - bullet->pos.x;
            y = player->unk_2428.y - bullet->pos.y;
            len = sqrtf(x * x + y * y) / (bullet->speed / 4.0f);
            if (len < 1.0f)
            {
                len = 1.0f;
            }
            x = x / len + bullet->velocity.x;
            y = y / len + bullet->velocity.y;
            len = sqrtf(x * x + y * y);
            bullet->speed = (len > 10.0f) ? 10.0f : len;
            if (bullet->speed < 1.0f)
            {
                bullet->speed = 1.0f;
            }
            bullet->velocity.x = x * bullet->speed / len;
            bullet->velocity.y = y * bullet->speed / len;
        }
        else if (bullet->speed < 10.0f)
        {
            bullet->speed += 0.33333334f;
            x = bullet->velocity.x;
            y = bullet->velocity.y;
            len = sqrtf(x * x + y * y);
            bullet->velocity.x = x * bullet->speed / len;
            bullet->velocity.y = y * bullet->speed / len;
        }
    }
    return 0;
}

int UpdateHomingOrbFast(Player *player, PlayerBullet *bullet)
{
    float x;
    float y;
    float len;

    if (bullet->state == 1)
    {
        if (player->unk_2428.x > -100.0f && bullet->timer.AsFrames() < 40 && bullet->timer.HasTicked())
        {
            x = player->unk_2428.x - bullet->pos.x;
            y = player->unk_2428.y - bullet->pos.y;
            len = sqrtf(x * x + y * y) / (bullet->speed / 4.0f);
            if (len < 1.0f)
            {
                len = 1.0f;
            }
            x = x / len + bullet->velocity.x;
            y = y / len + bullet->velocity.y;
            len = sqrtf(x * x + y * y);
            bullet->speed = (len > 18.0f) ? 18.0f : len;
            if (bullet->speed < 1.0f)
            {
                bullet->speed = 1.0f;
            }
            bullet->velocity.x = x * bullet->speed / len;
            bullet->velocity.y = y * bullet->speed / len;
        }
        else if (bullet->speed < 18.0f)
        {
            bullet->speed += 0.6f;
            x = bullet->velocity.x;
            y = bullet->velocity.y;
            len = sqrtf(x * x + y * y);
            bullet->velocity.x = x * bullet->speed / len;
            bullet->velocity.y = y * bullet->speed / len;
        }
    }
    return 0;
}

int UpdateAcceleratingBullet(Player *player, PlayerBullet *bullet)
{
    if (bullet->state == 1)
    {
        bullet->velocity.y -= g_Rng.GetRandomF32ZeroToOne() * 0.1f + 0.27f;
    }
    return 0;
}

int UpdateLaserBullet(Player *player, PlayerBullet *bullet)
{
    if (player->lasers[bullet->laserIndex].bullet != bullet && (bullet->sprite.flags >> 13 & 1))
    {
        bullet->sprite.pendingInterrupt = 1;
    }
    if ((g_Unknown49fbf0.Func42ad66() || player->bomb.running) &&
        player->lasers[bullet->laserIndex].timer.AsFrames() > 20)
    {
        player->lasers[bullet->laserIndex].timer.Set(20);
    }
    if (player->lasers[bullet->laserIndex].timer <= 0)
    {
        player->lasers[bullet->laserIndex].timer.Set(0);
        player->lasers[bullet->laserIndex].bullet = NULL;
        bullet->state = 0;
        return 1;
    }
    if (player->lasers[bullet->laserIndex].timer <= 70 && (bullet->sprite.flags >> 13 & 1))
    {
        bullet->sprite.pendingInterrupt = 1;
    }
    bullet->pos = player->optionPos[bullet->option - 1];
    bullet->pos.x += bullet->velocity.z;
    bullet->pos.z = 0.44f;
    if (player->playerState == 2)
    {
        return 1;
    }
    bullet->sprite.scaleX = bullet->pos.y / 14.0f;
    bullet->size.y = bullet->pos.y;
    bullet->pos.y /= 2.0f;
    return 0;
}

int UpdateTrailLaserBullet(Player *player, PlayerBullet *bullet)
{
    int i;

    if (player->lasers[bullet->laserIndex].bullet != bullet && (bullet->sprite.flags >> 13 & 1))
    {
        bullet->sprite.pendingInterrupt = 1;
    }
    if ((g_Unknown49fbf0.Func42ad66() || player->bomb.running) &&
        player->lasers[bullet->laserIndex].timer.AsFrames() > 20)
    {
        player->lasers[bullet->laserIndex].timer.Set(20);
    }
    if (player->lasers[bullet->laserIndex].timer <= 0)
    {
        player->lasers[bullet->laserIndex].timer.Set(0);
        bullet->state = 0;
        player->lasers[bullet->laserIndex].bullet = NULL;
        return 1;
    }
    if (player->lasers[bullet->laserIndex].timer <= 70 && (bullet->sprite.flags >> 13 & 1))
    {
        bullet->sprite.pendingInterrupt = 1;
    }
    for (i = 0; i < bullet->unk_352; i++)
    {
        if (bullet->unk_258[i].x >= -900.0f)
        {
            player->unk_9dc[i + 0x60].pos = bullet->unk_258[i];
            player->unk_9dc[i + 0x60].unk_18 = 1;
            player->unk_9dc[i + 0x60].size = bullet->size;
        }
    }
    for (i = 15; i > 0; i--)
    {
        bullet->unk_258[i] = bullet->unk_258[i - 1];
    }
    bullet->unk_258[0] = bullet->pos;
    if (player->playerState == 2)
    {
        return 1;
    }
    bullet->pos = player->position;
    bullet->pos.x += bullet->velocity.z;
    bullet->pos.z = 0.44f;
    bullet->sprite.scaleX = (bullet->pos.y + 64.0f) / 14.0f;
    bullet->size.y = player->position.y + 64.0f;
    bullet->pos.y = bullet->pos.y / 2.0f - 32.0f;
    return 0;
}

int DrawTrailLaserBullet(Player *player, PlayerBullet *bullet)
{
    int alpha = bullet->sprite.alpha;
    int i;

    for (i = 0; i < bullet->unk_352; i++)
    {
        if (bullet->unk_258[i].x == -999.0f)
        {
            break;
        }
        bullet->sprite.pos.x = bullet->unk_258[i].x;
        bullet->sprite.pos.y = bullet->unk_258[i].y;
        bullet->sprite.pos.z = bullet->unk_258[i].z;
        bullet->sprite.alpha = alpha - alpha * i / bullet->unk_352;
        bullet->sprite.pos[0] += g_Unknown626270.arena[0];
        bullet->sprite.pos[1] += g_Unknown626270.arena[1];
        g_SpriteEngine->Draw(&bullet->sprite);
    }
    bullet->sprite.alpha = alpha;
    return 0;
}

int UpdateBombShot(Player *player, PlayerBullet *bullet, D3DXVECTOR3 *pos)
{
    float angle;

    if (bullet->state == 2)
    {
        if (bullet->timer.AsFrames() % 2 != 0)
        {
            return 1;
        }
        bullet->damage /= 3;
        if (bullet->damage == 0)
        {
            bullet->damage = 1;
        }
        bullet->velocity.x *= 0.88f;
        bullet->velocity.y *= 0.88f;
    }
    else
    {
        angle = g_Rng.GetRandomF32ZeroToOne() * 1.5707964f - 2.3561945f;
        switch (bullet->sprite.scriptIndex)
        {
        case 0x441:
            bullet->size.x = 32.0f;
            bullet->size.y = 32.0f;
            ((Unknown4325c0 *)&bullet->velocity)->SetPolar(angle, 4.0f);
            break;
        case 0x442:
            bullet->size.x = 42.0f;
            bullet->size.y = 42.0f;
            ((Unknown4325c0 *)&bullet->velocity)->SetPolar(angle, 4.0f);
            break;
        case 0x443:
            bullet->size.x = 48.0f;
            bullet->size.y = 48.0f;
            ((Unknown4325c0 *)&bullet->velocity)->SetPolar(angle, 4.0f);
            break;
        case 0x444:
            bullet->size.x = 56.0f;
            bullet->size.y = 56.0f;
            ((Unknown4325c0 *)&bullet->velocity)->SetPolar(angle, 4.0f);
            break;
        case 0x445:
            bullet->size.x = 48.0f;
            bullet->size.y = 48.0f;
            ((Unknown4325c0 *)&bullet->velocity)->SetPolar(angle, 6.0f);
            break;
        case 0x446:
            bullet->size.x = 64.0f;
            bullet->size.y = 64.0f;
            ((Unknown4325c0 *)&bullet->velocity)->SetPolar(angle, 6.0f);
            break;
        case 0x447:
            bullet->size.x = 80.0f;
            bullet->size.y = 80.0f;
            ((Unknown4325c0 *)&bullet->velocity)->SetPolar(angle, 6.0f);
            break;
        case 0x448:
            bullet->size.x = 96.0f;
            bullet->size.y = 96.0f;
            ((Unknown4325c0 *)&bullet->velocity)->SetPolar(angle, 6.0f);
            break;
        }
    }
    if (bullet->timer.AsFrames() % 6 == 0)
    {
        g_EffectManager.SpawnEffect(5, pos, 1, 0xffffffff);
    }
    return 0;
}

int BurstHitEffect(Player *player, PlayerBullet *bullet, D3DXVECTOR3 *pos)
{
    D3DXVECTOR3 effectPos;

    player->hitCounter++;
    if (player->hitCounter % 8 == 0)
    {
        effectPos = *pos;
        effectPos.x = bullet->pos.x;
        g_EffectManager.SpawnEffect(5, &effectPos, 1, 0xffffffff);
    }
    return 0;
}

void FireShots(Player *player, int frame)
{
    ShotData *shot;
    int ret;
    PlayerBullet *curBullet;
    ShotTable *shotTable;
    int i;

    shotTable = player->focused == 0 ? player->shotType->tables : player->shotTypeFocused->tables;
    while ((int)g_Unknown626270.stats->power >= shotTable->maxPower)
    {
        shotTable++;
    }
    shot = shotTable->shots;
    curBullet = player->bullets;
    for (i = 0; i < 0x60; i++, curBullet++)
    {
        if (curBullet->state != 0)
        {
            continue;
        }
    again:
        if (shot->fire != NULL)
        {
            ret = shot->fire(player, curBullet, frame, shot);
        }
        else
        {
            ret = TryFireShot(player, curBullet, frame, shot);
        }
        if (ret == 1)
        {
            curBullet->sprite.flags |= 0x1000;
            curBullet->state = 1;
            curBullet->shot = shot;
            curBullet->update = curBullet->shot->update;
            curBullet->draw = curBullet->shot->draw;
            curBullet->hit = curBullet->shot->hit;
        }
        shot++;
        if (shot->interval < 0)
        {
            return;
        }
        if (ret == 0)
        {
            goto again;
        }
    }
}

void UpdateBullets(Player *player)
{
    int i;
    PlayerBullet *bullet;

    if (player->unk_240a != 3 && player->lasers[2].bullet != NULL)
    {
        player->lasers[2].bullet->state = 0;
        player->lasers[2].bullet = NULL;
    }
    if (player->unk_240a != 1)
    {
        if (player->lasers[0].bullet != NULL)
        {
            player->lasers[0].bullet->sprite.pendingInterrupt = 1;
            player->lasers[0].bullet = NULL;
        }
        if (player->lasers[1].bullet != NULL)
        {
            player->lasers[1].bullet->sprite.pendingInterrupt = 1;
            player->lasers[1].bullet = NULL;
        }
    }
    if (player->playerState == 2)
    {
        for (i = 0; i < 3; i++)
        {
            if (player->lasers[i].bullet != NULL)
            {
                player->lasers[i].bullet->state = 0;
                player->lasers[i].bullet = NULL;
            }
        }
    }
    for (i = 0; i < 3; i++)
    {
        if (player->lasers[i].bullet == NULL)
        {
            continue;
        }
        if (player->lasers[i].timer.AsFrames() > 0 && player->lasers[i].timer.AsFrames() < 999)
        {
            player->lasers[i].timer.Sub(1);
        }
        if (player->fireTimer.AsFrames() < 0 && player->lasers[i].timer.AsFrames() > 50)
        {
            player->lasers[i].timer.Set(50);
        }
        if (player->lasers[i].timer.AsFrames() == 0)
        {
            player->lasers[i].bullet = NULL;
        }
    }
    bullet = player->bullets;
    for (i = 0; i < 0x60; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if (bullet->update != NULL && bullet->update(player, bullet))
        {
            bullet->state = 0;
            continue;
        }
        bullet->pos[0] += g_App.unk_178 * bullet->velocity.x;
        bullet->pos[1] += g_App.unk_178 * bullet->velocity.y;
        if (bullet->type != 4 && bullet->type != 5 &&
            !g_Unknown626270.IsInPlayArea(bullet->pos.x, bullet->pos.y, bullet->sprite.spriteData->width,
                                          bullet->sprite.spriteData->height))
        {
            bullet->state = 0;
        }
        if (g_SpriteEngine->Step(&bullet->sprite))
        {
            bullet->state = 0;
        }
        bullet->timer.Tick();
    }
}

void DrawBullets(Player *player)
{
    PlayerBullet *bullet = player->bullets;
    int i;

    for (i = 0; i < 0x60; i++, bullet++)
    {
        if (bullet->state != 1)
        {
            continue;
        }
        if (bullet->sprite.autoRotate != 0)
        {
            bullet->sprite.SetRotationZ(AddNormalizeAngle(bullet->angle, 1.5707964f));
        }
        bullet->sprite.pos.x = g_Unknown626270.arena[0] + bullet->pos.x;
        bullet->sprite.pos.y = g_Unknown626270.arena[1] + bullet->pos.y;
        bullet->sprite.pos.z = 0.4f;
        g_SpriteEngine->Draw(&bullet->sprite);
        if (bullet->draw != NULL)
        {
            bullet->draw(player, bullet);
        }
    }
}

void DrawHitBullets(Player *player)
{
    PlayerBullet *bullet = player->bullets;
    int i;

    for (i = 0; i < 0x60; i++, bullet++)
    {
        if (bullet->state != 2)
        {
            continue;
        }
        if (bullet->sprite.autoRotate != 0)
        {
            bullet->sprite.SetRotationZ(AddNormalizeAngle(bullet->angle, 1.5707964f));
        }
        bullet->sprite.pos.x = g_Unknown626270.arena[0] + bullet->pos.x;
        bullet->sprite.pos.y = g_Unknown626270.arena[1] + bullet->pos.y;
        bullet->sprite.pos.z = 0.4f;
        g_SpriteEngine->Draw(&bullet->sprite);
    }
}

int UpdateFiring(Player *player)
{
    if (player->fireTimer.AsFrames() < 0)
    {
        return 0;
    }
    if (player->fireTimer.HasTicked() &&
        (!g_Player.bomb.running || g_Unknown626270.character != 1 || g_Unknown626270.shotType != 1))
    {
        FireShots(player, player->fireTimer.AsFrames());
    }
    player->fireTimer.Tick();
    if (player->fireTimer.AsFrames() >= 30 || player->playerState == 2 || player->playerState == 1)
    {
        player->fireTimer.Set(-1);
    }
    return 0;
}

void Player::StartFiring(Player *player)
{
    if (player->fireTimer.AsFrames() < 0)
    {
        player->fireTimer.Set(0);
    }
}

int Player::CheckAreaHit(D3DXVECTOR3 *pos, D3DXVECTOR3 *size)
{
    int idx;
    D3DXVECTOR3 start;
    D3DXVECTOR3 end;
    PlayerArea *area;
    D3DXVECTOR3 areaMax;
    // The point's offset from a circle's centre.
    D3DXVECTOR2 rel;
    D3DXVECTOR3 areaMin;

    area = this->unk_17dc;
    start.x = pos->x - size->x / 2.0f;
    start.y = pos->y - size->y / 2.0f;
    end.x = size->x / 2.0f + pos->x;
    end.y = size->y / 2.0f + pos->y;
    for (idx = 0; idx < 0x60; idx++, area++)
    {
        if (area->width != 0.0f)
        {
            areaMin.x = area->x - area->width / 2.0f;
            areaMin.y = area->y - area->height / 2.0f;
            areaMax.x = area->width / 2.0f + area->x;
            areaMax.y = area->height / 2.0f + area->y;
            if (!(areaMin.x > end.x || areaMax.x < start.x || areaMin.y > end.y || areaMax.y < start.y))
            {
                this->unk_2404 = area->unk_1c;
                return 2;
            }
        }
        else if (area->unk_10 != 0.0)
        {
            rel.x = pos->x - area->x;
            rel.y = pos->y - area->y;
            if (rel.x * rel.x + rel.y * rel.y < area->unk_10 * area->unk_10)
            {
                this->unk_2404 = area->unk_1c;
                return 2;
            }
        }
        else
        {
            continue;
        }
    }
    return 0;
}

int Player::CheckHit(D3DXVECTOR3 *pos, D3DXVECTOR3 *size)
{
    D3DXVECTOR3 start;
    D3DXVECTOR3 end;

    this->unk_2404 = 6;
    if (this->CheckAreaHit(pos, size))
    {
        return 2;
    }
    start.x = pos->x - size->x / 2.0f;
    start.y = pos->y - size->y / 2.0f;
    end.x = size->x / 2.0f + pos->x;
    end.y = size->y / 2.0f + pos->y;
    if (this->hitboxMin.x > end.x || this->hitboxMin.y > end.y || this->hitboxMax.x < start.x ||
        this->hitboxMax.y < start.y)
    {
        return 0;
    }
    g_ReplayManager->unk_d6 |= 2;
    if (this->playerState == 4)
    {
        g_Player.Func441bd0(0);
        return 1;
    }
    if (this->playerState != 0)
    {
        return 1;
    }
    g_Unknown626270.RerollLifeChecksumKeys();
    this->Die();
    return 1;
}

int Player::CheckGraze(D3DXVECTOR3 *pos, D3DXVECTOR3 *size)
{
    D3DXVECTOR3 start;
    D3DXVECTOR3 end;

    this->unk_2404 = 6;
    if (this->CheckAreaHit(pos, size))
    {
        return 2;
    }
    start.x = pos->x - size->x / 2.0f - 20.0f;
    start.y = pos->y - size->y / 2.0f - 20.0f;
    end.x = size->x / 2.0f + pos->x + 20.0f;
    end.y = size->y / 2.0f + pos->y + 20.0f;
    if (this->playerState == 2 || this->playerState == 1)
    {
        return 0;
    }
    if (this->grazeMin.x > end.x || this->grazeMax.x < start.x || this->grazeMin.y > end.y ||
        this->grazeMax.y < start.y)
    {
        return 0;
    }
    this->Graze(pos);
    return 1;
}

int Player::TouchesGrazeBox(D3DXVECTOR3 *pos, D3DXVECTOR3 *size)
{
    D3DXVECTOR3 start;
    D3DXVECTOR3 end;

    if (this->playerState != 0 && this->playerState != 3 && this->playerState != 4)
    {
        return 0;
    }
    start = *pos - *size / 2.0f;
    end = *size / 2.0f + *pos;
    if (this->unk_978.x > end.x || this->unk_984.x < start.x || this->unk_978.y > end.y ||
        this->unk_984.y < start.y)
    {
        return 0;
    }
    return 1;
}

int Player::CheckLaserHit(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, D3DXVECTOR3 *origin, float angle, int canGraze)
{
    D3DXVECTOR3 boxMin;
    D3DXVECTOR3 start;
    D3DXVECTOR3 end;
    D3DXVECTOR3 boxMax;

    // The hitbox, turned into the laser's frame.
    start = this->position - *origin;
    Rotate2D(&end, &start, angle);
    end.z = 0.0f;
    start = end + *origin;
    boxMin = start - this->hitboxSize;
    boxMax = start + this->hitboxSize;
    start = *pos - *size / 2.0f;
    end = *size / 2.0f + *pos;
    if (!(boxMin.x > end.x || boxMax.x < start.x || boxMin.y > end.y || boxMax.y < start.y))
    {
        goto hit;
    }
    if (!canGraze)
    {
        return 0;
    }
    start.x -= 48.0f;
    start.y -= 48.0f;
    end.x += 48.0f;
    end.y += 48.0f;
    if (boxMin.x > end.x || boxMax.x < start.x || boxMin.y > end.y || boxMax.y < start.y)
    {
        return 0;
    }
    if (this->playerState == 2 || this->playerState == 1)
    {
        return 0;
    }
    this->Graze(&this->position);
    return 2;

hit:
    g_ReplayManager->unk_d6 |= 2;
    if (this->playerState == 4)
    {
        g_Player.Func441bd0(0);
        return 1;
    }
    if (this->playerState != 0)
    {
        return 0;
    }
    g_Unknown626270.RerollLifeChecksumKeys();
    this->Die();
    return 1;
}

void Player::Graze(D3DXVECTOR3 *pos)
{
    D3DXVECTOR3 mid;

    if (!g_Player.bomb.running)
    {
        if (g_Unknown626270.stats->graze < 9999)
        {
            g_Unknown626270.stats->graze++;
        }
        if (g_Unknown626270.stats->grazeTotal < 999999)
        {
            g_Unknown626270.stats->grazeTotal++;
        }
    }
    mid = (this->position + *pos) / 2.0f;
    if (this->unk_240d == 1)
    {
        if (this->focused)
        {
            g_EffectManager.SpawnEffect(8, &mid, 1, 0xffffffff);
        }
        else
        {
            g_EffectManager.SpawnEffect(8, &mid, 3, 0xffff8080);
        }
    }
    else
    {
        g_EffectManager.SpawnEffect(8, &mid, 1, 0xffffffff);
    }
    g_Unknown626270.AddSubRank(6);
    g_Unknown49fbf0.grazeChanged = 2;
    g_SoundPlayer.QueueSound(0x1e, 0);
    g_Unknown12fe0d0 += 2500 + (g_Unknown626270.unk_961c - g_Unknown626270.stats->unk_88) / 1500 * 20;
    g_Unknown626270.AddScore(2000);
    if (this->unk_240d == 1)
    {
        if (this->focused)
        {
            g_Unknown626270.Func42f789(30);
            g_Unknown626270.Func42f736(30);
        }
        else
        {
            g_Unknown626270.Func42f789(80);
            g_Unknown626270.Func42f736(80);
        }
    }
}

void Player::Die()
{
    g_Unknown626270.Func4012b0();
    g_EffectManager.Func41c610(0xc, &this->position, 3, 1, 0xff4040ff);
    g_EffectManager.SpawnEffect(6, &this->position, 16, 0xffffffff);
    this->playerState = 2;
    this->stateTimer.Set(0);
    g_SoundPlayer.QueueSound(4, 0);
}

void UpdateHitboxAreas(Player *player)
{
    int i;
    PlayerArea *area;

    for (i = 0; i < 0x70; i++)
    {
        player->unk_9dc[i].size.x = 0.0f;
    }
    area = player->unk_17dc;
    for (i = 0; i < 0x60; i++, area++)
    {
        if (area->unk_18 <= 0)
        {
            area->unk_10 = 0.0f;
            area->width = 0.0f;
        }
        else
        {
            area->unk_18--;
            area->unk_10 += area->unk_14;
        }
    }
}

PlayerArea *Player::AddHitboxArea(D3DXVECTOR3 *pos, float width, float height, int unk_1c)
{
    PlayerArea *area = this->unk_17dc;
    int i;

    for (i = 0; i < 0x5f; i++, area++)
    {
        if (area->width == 0.0f && area->unk_10 == 0.0f)
        {
            break;
        }
    }
    area->x = pos->x;
    area->y = pos->y;
    area->width = width;
    area->height = height;
    area->unk_18 = 0;
    area->unk_1c = unk_1c;
    return area;
}

PlayerArea *Player::AddHitboxAreaCircle(D3DXVECTOR3 *pos, float unk_10, float unk_14, int unk_18, int unk_1c)
{
    PlayerArea *area = this->unk_17dc;
    int i;

    for (i = 0; i < 0x5f; i++, area++)
    {
        if (area->width == 0.0f && area->unk_10 == 0.0f)
        {
            break;
        }
    }
    area->x = pos->x;
    area->y = pos->y;
    area->unk_10 = unk_10;
    area->unk_14 = unk_14;
    area->unk_18 = unk_18;
    area->unk_1c = unk_1c;
    return area;
}

void ResetHomingAndText(Player *player)
{
    player->unk_2428 = D3DXVECTOR3(-999.0f, -999.0f, 0.0f);
    player->homingTarget = D3DXVECTOR3(-999.0f, -999.0f, 0.0f);
    player->unk_2440 = 0;
    if (player->position.y >= 400.0f)
    {
        if (g_AsciiText.GetUnk74d4() != 2 && player->position.x < 160.0f)
        {
            g_AsciiText.SetUnk74d4(2);
        }
        else if (g_AsciiText.GetUnk74d4() == 2 && player->position.x > 160.0f)
        {
            g_AsciiText.SetUnk74d4(3);
        }
    }
    else if (g_AsciiText.GetUnk74d4() == 2)
    {
        g_AsciiText.SetUnk74d4(3);
    }
}

int Player::OnUpdate(Player *player)
{
    if (g_Unknown626270.isFrozen)
    {
        return 1;
    }
    UpdateHitboxAreas(player);
    Func4409f0(player);
    if (player->playerState == 2)
    {
        if (Func440cf0(player))
        {
            goto respawn;
        }
    }
    else if (player->playerState == 1)
    {
    respawn:
        Func4411c0(player);
    }
    Func441330(player);
    if (player->playerState != 2 && player->playerState != 1)
    {
        Func43ee50(player);
    }
    g_SpriteEngine->Step(&player->sprite);
    if (player->unk_240a)
    {
        g_SpriteEngine->Step(&player->optionSprites[0]);
        g_SpriteEngine->Step(&player->optionSprites[1]);
    }
    UpdateBullets(player);
    UpdateFiring(player);
    ResetHomingAndText(player);
    return 1;
}

int Player::OnDraw2(Player *player)
{
    DrawHitBullets(player);
    return 1;
}

float Player::AngleFrom(D3DXVECTOR3 *pos)
{
    float x;
    float y;

    x = this->position.x - pos->x;
    y = this->position.y - pos->y;
    if (y == 0.0f && x == 0.0f)
    {
        return 1.5707964f;
    }
    return atan2f(y, x);
}

int Player::RegisterChain(int unk)
{
    Player *player = &g_Player;

    memset(player, 0, sizeof(Player));
    player->stateTimer.Set(0);
    player->unk_2409 = unk;
    player->calcChain = g_Chain.CreateElem((ChainCallback)Player::OnUpdate);
    player->drawChain = g_Chain.CreateElem((ChainCallback)Player::OnDraw);
    player->drawChain2 = g_Chain.CreateElem((ChainCallback)Player::OnDraw2);
    player->calcChain->arg = player;
    player->drawChain->arg = player;
    player->drawChain2->arg = player;
    player->calcChain->addedCallback = (ChainCallback)Player::AddedCallback;
    player->calcChain->deletedCallback = (ChainCallback)Player::DeletedCallback;
    if (g_Chain.AddToCalcChain(player->calcChain, 8) != 0)
    {
        return -1;
    }
    g_Chain.AddToDrawChain(player->drawChain, 6);
    g_Chain.AddToDrawChain(player->drawChain2, 8);
    return 0;
}

int Player::DeletedCallback(Player *player)
{
    if ((g_App.scene != 3 && g_App.scene != 0xb && g_App.scene != 0xc) != 0)
    {
        g_SpriteEngine->ReleaseAnm(10);
        g_AsciiText.unk_498.pendingInterrupt = 99;
        g_AsciiText.unk_74d4 = 99;
        g_AsciiText.InterruptUnkB7c(0);
        g_AsciiText.InterruptUnkB7c(1);
        g_AsciiText.InterruptUnkB7c(2);
    }
    if (g_Player.shotType != NULL)
    {
        MemFree(g_Player.shotType);
        g_Player.shotType = NULL;
    }
    if (g_Player.shotTypeFocused != NULL)
    {
        MemFree(g_Player.shotTypeFocused);
        g_Player.shotTypeFocused = NULL;
    }
    return 0;
}

void Player::RemoveChain()
{
    g_Chain.Remove(g_Player.calcChain);
    g_Player.calcChain = NULL;
    g_Chain.Remove(g_Player.drawChain);
    g_Player.drawChain = NULL;
    g_Chain.Remove(g_Player.drawChain2);
    g_Player.drawChain2 = NULL;
}

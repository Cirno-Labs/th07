// The code from 0x43b5c0: helpers out of line, then what looks like
// th06's player.cpp (0x43b7f0 constructs the static at 0x4bdad8).

#include <math.h>

#include "effect.h"
#include "player.h"
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
    if (player->unk_2408 == 2)
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
    if (player->unk_2408 == 2)
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

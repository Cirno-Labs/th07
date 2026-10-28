// The code from 0x43b5c0: helpers out of line, then what looks like
// th06's player.cpp (0x43b7f0 constructs the static at 0x4bdad8).

#include <math.h>

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

// The code from 0x43b5c0: helpers out of line, then what looks like
// th06's player.cpp (0x43b7f0 constructs the static at 0x4bdad8).

#include <math.h>

#include "ascii.h"
#include "bullet.h"
#include "effect.h"
#include "item.h"
#include "player.h"
#include "replay.h"
#include "unsorted.h"

void GameManager::RerollLifeChecksumKeys()
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

void GameManager::AddDeaths(int deaths)
{
    if (this->ChecksumMismatch())
    {
        memset(&g_App, 0xff, 0x2d0);
    }
    this->stats->deaths += deaths;
    this->RerollChecksumKeys();
}

void GameManager::AddBombsUsed(int bombs)
{
    if (this->ChecksumMismatch())
    {
        memset(&g_App, 0xff, 0x2d0);
    }
    this->stats->bombsUsed += bombs;
    this->RerollChecksumKeys();
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

// The corners of a `size` box centred on `pos`.
inline void GetBox(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, D3DXVECTOR3 &min, D3DXVECTOR3 &max)
{
    min.x = pos->x - size->x * 0.5f;
    min.y = pos->y - size->y * 0.5f;
    max.x = size->x * 0.5f + pos->x;
    max.y = size->y * 0.5f + pos->y;
}

int Player::CalcDamage(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, int *hitByBomb)
{
    PlayerBullet *cur;
    int idx;
    // The enemy's box, and the box of the bullet or bomb hitbox tested.
    D3DXVECTOR3 eMin;
    D3DXVECTOR3 eMax;
    D3DXVECTOR3 targetLo;
    D3DXVECTOR3 targetHi;
    int damage;

    damage = 0;
    if (!this->stateTimer.HasTicked())
    {
        return 0;
    }
    GetBox(pos, size, eMin, eMax);
    cur = this->bullets;
    if (hitByBomb != NULL)
    {
        *hitByBomb = 0;
    }
    for (idx = 0; idx < 0x60; idx++, cur++)
    {
        if (cur->state == 0 || (cur->state != 1 && cur->type != 3))
        {
            continue;
        }
        GetBox(&cur->pos, &cur->size, targetLo, targetHi);
        if (targetLo.y > eMax.y || targetLo.x > eMax.x || targetHi.y < eMin.y || targetHi.x < eMin.x)
        {
            continue;
        }
        if ((cur->type == 4 || cur->type == 5) && cur->timer.current % 2 != 0)
        {
            continue;
        }
        if (cur->hit != NULL && cur->hit(this, cur, pos))
        {
            continue;
        }
        if (!this->bomb.running)
        {
            damage += cur->damage;
        }
        else
        {
            damage += (cur->damage / 3 != 0) ? cur->damage / 3 : 1;
        }
        if (cur->type != 4 && cur->type != 5)
        {
            if (cur->state == 1)
            {
                g_SpriteEngine->StartScript(&cur->sprite, cur->sprite.scriptIndex + 0x20);
                g_EffectManager.SpawnEffect(5, &cur->pos, 1, 0xffffffff);
                cur->pos.z = 0.1f;
            }
            cur->state = 2;
            if (cur->type != 3)
            {
                cur->velocity.x /= 8.0f;
                cur->velocity.y /= 8.0f;
            }
        }
    }
    for (idx = 0; idx < 0x70; idx++)
    {
        if (this->unk_9dc[idx].size.x <= 0.0f)
        {
            continue;
        }
        targetLo = this->unk_9dc[idx].pos - this->unk_9dc[idx].size / 2.0f;
        targetHi = this->unk_9dc[idx].size / 2.0f + this->unk_9dc[idx].pos;
        if (targetLo.x > eMax.x || targetHi.x < eMin.x || targetLo.y > eMax.y || targetHi.y < eMin.y)
        {
            continue;
        }
        damage += this->unk_9dc[idx].unk_18;
        this->unk_9dc[idx].unk_1c += this->unk_9dc[idx].unk_18;
        this->hitCounter++;
        if (this->hitCounter % 4 == 0)
        {
            if (idx < 0x60)
            {
                g_EffectManager.SpawnEffect(3, pos, 1, 0xffffffff);
            }
            else
            {
                g_EffectManager.SpawnEffect(5, pos, 1, 0xffffffff);
            }
        }
        if (this->bomb.running && hitByBomb != NULL)
        {
            *hitByBomb = 1;
        }
    }
    return damage;
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
        g_Player.StopBombState(0);
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
        g_Player.StopBombState(0);
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
            g_Unknown626270.IncreaseBonusCap(30);
            g_Unknown626270.AddBonusPointsCapped(30);
        }
        else
        {
            g_Unknown626270.IncreaseBonusCap(80);
            g_Unknown626270.AddBonusPointsCapped(80);
        }
    }
}

void Player::Die()
{
    g_Unknown626270.RerollChecksumKeys();
    g_EffectManager.SpawnReservedEffect(0xc, &this->position, 3, 1, 0xff4040ff);
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

void UpdateBombState(Player *player)
{
    if (player->unk_240d && !player->bomb.running && (g_Unknown4b9e50 & 2))
    {
        player->StopBombState(1);
        player->unk_23dc = 0;
        g_ItemManager.CollectAll();
        return;
    }
    if (player->unk_240d == 2)
    {
        StartBombState(player);
    }
    if (player->unk_23fc != 0)
    {
        player->unk_23fc--;
    }
    if (player->bomb.running)
    {
        if (player->bomb.timer.HasTicked())
        {
            g_Unknown626270.SubUnk961c(player->bomb.unk_c);
            g_Unknown49fbf0.unk_4_8 = 2;
        }
        if (!player->bomb.focused)
        {
            player->bomb.update(player);
        }
        else
        {
            player->bomb.updateFocused(player);
        }
        return;
    }
    if (!g_Unknown626270.ChecksumMismatch() && !g_Unknown49fbf0.Func42ad66() && player->deathbombFrames != 0 &&
        (int)g_Unknown626270.stats->bombs > 0 && player->unk_23fc == 0 && (g_Unknown4b9e50 & 2))
    {
        g_ReplayManager->unk_d6 |= 1;
        g_Unknown626270.AddBombsUsed(1);
        g_Unknown626270.AddBombs(-1);
        g_Unknown49fbf0.bombsChanged = 2;
        player->bomb.focused = player->focused;
        player->bomb.running = 1;
        player->unk_23dc = 1;
        player->bomb.timer.Set(0);
        player->bomb.duration = 999;
        if (!player->bomb.focused)
        {
            player->bomb.update(player);
        }
        else
        {
            player->bomb.updateFocused(player);
        }
        g_Unknown12fe0cc = 0;
        g_Unknown12fe0c4 = 0;
        g_Unknown626270.SubSubRank(200);
        g_Unknown12fe0dc = g_Unknown12fe0c8;
        player->deathbombFrames += 6;
        if (player->deathbombFrames > g_Player.shotType->deathbombFrames)
        {
            player->deathbombFrames = g_Player.shotType->deathbombFrames;
        }
    }
    else
    {
        player->unk_23dc = 0;
    }
}

int UpdateDyingState(Player *player)
{
    int bonus;
    float progress;

    if (player->deathbombFrames != 0)
    {
        if (player->unk_240d == 1)
        {
            player->StopBombState(0);
            return 0;
        }
        player->deathbombFrames--;
        if (player->deathbombFrames == 0)
        {
            g_ReplayManager->unk_d6 |= 4;
            g_Unknown626270.unk_93d4 = 0;
            g_Unknown12fe0cc = 0;
            g_Unknown12fe0c4 = 0;
            g_Unknown626270.AddDeaths(1);
            if ((int)g_Unknown626270.stats->lives > 0)
            {
                if ((int)g_Unknown626270.stats->power <= 16)
                {
                    g_Unknown626270.stats->power = 0;
                    g_Unknown626270.RerollChecksumKeys();
                }
                else
                {
                    g_Unknown626270.AddPower(-16);
                }
                g_ItemManager.SpawnItem(&player->position, 2, 2);
                g_ItemManager.SpawnItem(&player->position, 0, 2);
                g_ItemManager.SpawnItem(&player->position, 0, 2);
                g_ItemManager.SpawnItem(&player->position, 0, 2);
                g_ItemManager.SpawnItem(&player->position, 0, 2);
                g_ItemManager.SpawnItem(&player->position, 0, 2);
                g_Unknown49fbf0.powerChanged = 2;
                bonus = (g_Unknown626270.unk_961c - g_Unknown626270.stats->unk_88) * g_Player.shotType->unk_1c;
                if (g_Unknown626270.character != 2)
                {
                    if (bonus > 100000)
                    {
                        bonus = 100000;
                    }
                }
                else if (bonus > 60000)
                {
                    bonus = 60000;
                }
                bonus -= bonus % 10;
                g_Unknown626270.unk_961c -= bonus;
                g_Unknown49fbf0.unk_4_8 = 2;
                g_ItemManager.RecycleCollectedItems();
            }
            else
            {
                g_Unknown626270.stats->power = 0;
                g_Unknown626270.RerollChecksumKeys();
                g_ItemManager.SpawnItem(&player->position, 4, 2);
                g_ItemManager.SpawnItem(&player->position, 4, 2);
                g_ItemManager.SpawnItem(&player->position, 4, 2);
                g_ItemManager.SpawnItem(&player->position, 4, 2);
                g_ItemManager.SpawnItem(&player->position, 4, 2);
                g_Unknown49fbf0.powerChanged = 2;
            }
            g_Unknown626270.SubSubRank(1600);
        }
    }
    else
    {
        progress = player->stateTimer.AsFloat() / 30.0f;
        player->sprite.scaleX = 3.0f * progress + 1.0f;
        player->sprite.scaleY = 1.0f - 1.0f * progress;
        player->sprite.color = (int)(255.0f - player->stateTimer.AsFloat() * 255.0f / 30.0f) << 24 | 0xffffff;
        player->sprite.flags |= 0x10;
        player->velocityX = 0.0f;
        player->velocityY = 0.0f;
        if (player->stateTimer.AsFrames() >= 30)
        {
            player->playerState = 1;
            player->position.x = g_Unknown626270.arena[2] / 2.0f;
            player->position.y = g_Unknown626270.arena[3] - 64.0f;
            player->position.z = 0.2f;
            player->stateTimer.Set(0);
            player->sprite.scaleY = 3.0f;
            player->sprite.scaleX = 3.0f;
            g_SpriteEngine->StartScript(&player->sprite, 0x400);
            if ((int)g_Unknown626270.stats->lives <= 0)
            {
                g_Unknown626270.isInContinueMenu = 1;
            }
            else
            {
                g_Unknown626270.AddLives(-1);
                g_Unknown49fbf0.livesChanged = 2;
                g_Unknown626270.SetBombs(g_Player.shotType->bombs);
                g_Unknown49fbf0.bombsChanged = 2;
                return 1;
            }
        }
    }
    return 0;
}

void UpdateRespawningState(Player *player)
{
    float progress;

    player->respawnClearFrames = 60;
    progress = 1.0f - player->stateTimer.AsFloat() / 30.0f;
    player->sprite.scaleX = 2.0f * progress + 1.0f;
    player->sprite.scaleY = 1.0f - 1.0f * progress;
    player->sprite.flags |= 0x10;
    player->moveScaleY = 1.0f;
    player->moveScaleX = 1.0f;
    player->sprite.color = player->stateTimer.AsFrames() * 255 / 30 << 24 | 0xffffff;
    player->deathbombFrames = 0;
    if (player->stateTimer.AsFrames() >= 30)
    {
        player->playerState = 3;
        player->sprite.scaleY = 1.0f;
        player->sprite.scaleX = 1.0f;
        player->sprite.color = 0xffffffff;
        player->sprite.flags &= ~0x10;
        player->stateTimer.Set(240);
        player->deathbombFrames = g_Player.shotType->deathbombFrames;
    }
}

void UpdateInvulnerability(Player *player)
{
    ColorBytes color;

    if (player->respawnClearFrames != 0)
    {
        player->respawnClearFrames--;
        g_BulletManager.ClearAll(0);
    }
    if (player->playerState == 3)
    {
        if (player->unk_b7e68 != NULL)
        {
            player->unk_b7e68->pos = player->position;
        }
        player->stateTimer.Sub(1);
        if (player->stateTimer.AsFrames() <= 0)
        {
            if (player->unk_b7e68 != NULL)
            {
                player->unk_b7e68->active = 0;
                player->unk_b7e68 = NULL;
            }
            player->playerState = 0;
            player->stateTimer.Set(0);
            player->sprite.color = 0xffffffff;
        }
        else if (player->stateTimer.AsFrames() % 8 < 2)
        {
            player->sprite.color = 0xff404040;
        }
        else
        {
            player->sprite.color = 0xffffffff;
        }
    }
    else if (player->playerState == 4)
    {
        if (player->unk_b7e6c != NULL)
        {
            player->unk_b7e6c->pos = player->position;
        }
        g_Unknown626270.unk_9620 = player->stateTimer.AsFrames() * 50000 / player->unk_16a0c.AsFrames();
        if (g_Unknown626270.unk_9620 < 0)
        {
            g_Unknown626270.unk_9620 = 0;
        }
        g_Unknown626270.unk_9620 += g_Unknown626270.stats->unk_88;
        player->stateTimer.Sub(1);
        if (player->stateTimer.AsFrames() <= 0)
        {
            player->sprite.color = 0xffffffff;
            player->EndBombState();
        }
        else
        {
            if (player->stateTimer.AsFrames() % 4 < 2)
            {
                player->sprite.color = 0xffff0000;
            }
            else
            {
                player->sprite.color = 0xffffffff;
            }
            color.a = 0x80;
            if (g_Player.stateTimer >= 510)
            {
                color.b = 0x80 - (540 - g_Player.stateTimer.AsFrames()) * 80 / 30;
                color.g = color.b;
                color.r = color.g;
            }
            else if (g_Player.stateTimer < 30)
            {
                color.b = 0x80 - g_Player.stateTimer.AsFrames() * 80 / 30;
                color.g = color.b;
                color.r = color.g;
            }
            else
            {
                color.b = 0x30;
                color.g = color.b;
                color.r = color.g;
            }
            g_Unknown1347b00.SetTintColor(color.color);
        }
    }
    else
    {
        player->stateTimer.Tick();
    }
}

void StartBombState(Player *player)
{
    Effect *effect;

    if (player->bomb.running || g_Unknown49fbf0.Func42ad66())
    {
        player->unk_240d = 2;
        return;
    }
    switch (player->playerState)
    {
    case 1:
    case 3:
        player->unk_240d = 2;
        return;
    case 2:
        if (player->deathbombFrames != 0)
        {
            player->StopBombState(0);
            return;
        }
        player->unk_240d = 2;
        return;
    }
    player->stateTimer.Set(540);
    player->unk_16a0c = player->stateTimer;
    player->unk_240d = 1;
    player->playerState = 4;
    if (player->unk_b7e6c != NULL)
    {
        player->unk_b7e6c->active = 0;
    }
    if (player->unk_b7e68 != NULL)
    {
        player->unk_b7e68->active = 0;
        player->unk_b7e68 = NULL;
    }
    effect = g_EffectManager.SpawnReservedEffect(0x1c, &player->position, 4, 1, 0xffffffff);
    effect->sprite.unk_48[4].Set(0);
    effect->sprite.unk_84[4].Set(player->stateTimer.AsFrames());
    effect->sprite.unk_c4 = 0;
    effect->sprite.unk_218.y = 1.0f;
    effect->sprite.unk_218.x = 1.0f;
    effect->sprite.unk_218.z = 0.25f;
    effect->sprite.unk_224.x = 0.25f;
    effect->sprite.unk_c8 = player->stateTimer.AsFrames();
    effect->sprite.unk_14 *= -1.0f;
    player->unk_b7e6c = effect;
    g_Unknown49fbf0.Func427c81(0, 2);
    g_SoundPlayer.QueueSound(0x20, 0);
    g_SoundPlayer.QueueSound(0x24, 0);
    g_ReplayManager->unk_d6 |= 8;
}

void Player::EndBombState()
{
    int bonus;

    g_Unknown626270.IncreaseBonusCap(10000);
    g_Unknown626270.AddBonusPointsCapped(10000);
    bonus = g_Unknown626270.unk_961c - g_Unknown626270.stats->unk_88;
    bonus *= 10;
    g_Unknown626270.AddScore(bonus);
    g_Unknown49fbf0.Func427c81(bonus, 4);
    g_Unknown626270.unk_9620 = g_Unknown626270.stats->unk_88;
    g_SoundPlayer.QueueSound(0x21, 0);
    if (this->playerState == 1)
    {
        this->sprite.scaleY = 1.0f;
        this->sprite.scaleX = 1.0f;
        this->sprite.color = 0xffffffff;
        this->sprite.flags &= ~0x10;
        this->stateTimer.Set(240);
        this->deathbombFrames = g_Player.shotType->deathbombFrames;
    }
    this->playerState = 3;
    this->stateTimer.Set(40);
    this->unk_23fc = 40;
    this->unk_240d = 0;
    if (this->unk_b7e6c != NULL)
    {
        this->unk_b7e6c->active = 0;
        this->unk_b7e6c = NULL;
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

void Player::StopBombState(int unk)
{
    Effect *eff;
    int i;
    float angle;

    if (this->unk_b7e6c != NULL)
    {
        this->unk_b7e6c->active = 0;
        this->unk_b7e6c = NULL;
    }
    eff = g_EffectManager.SpawnReservedEffect(0x1c, &this->position, 4, 1, 0xffffffff);
    eff->sprite.unk_48[4].Set(0);
    eff->sprite.unk_84[4].Set(30);
    eff->sprite.unk_c4 = 0;
    eff->sprite.unk_218.x = 0.0625f;
    eff->sprite.unk_218.y = 0.0625f;
    eff->sprite.unk_218.z = 1.3f;
    eff->sprite.unk_224.x = 1.3f;
    eff->sprite.unk_48[2].Set(0);
    eff->sprite.unk_84[2].Set(30);
    eff->sprite.unk_c2 = 1;
    // Bytes 0x22b and 0x22f, probably the alphas of two colors over
    // unk_224.y and .z (the vector type keeps Sprite's inline budget).
    ((BYTE *)&eff->sprite.unk_224.y)[3] = eff->sprite.alpha;
    ((BYTE *)&eff->sprite.unk_224.z)[3] = 0;
    eff->sprite.unk_c8 = 30;
    this->unk_b7e6c = eff;
    g_Unknown12fe0cc = 0;
    g_Unknown12fe0c4 = 0;
    this->unk_240d = 0;
    this->playerState = 3;
    this->stateTimer.Set(40);
    this->unk_23fc = 40;
    g_Unknown626270.unk_9620 = g_Unknown626270.stats->unk_88;
    this->AddHitboxAreaCircle(&this->position, 32.0f, 16.0f, 0x32, 8);
    angle = -D3DX_PI;
    for (i = 0; i < 0x20; i++, angle += D3DX_PI / 16.0f)
    {
        eff = g_EffectManager.SpawnEffect(0x1d, &this->position, 1, 0xffffffff);
        eff->axis.x = cosf(angle);
        eff->axis.y = sinf(angle);
    }
    g_SoundPlayer.QueueSound(7, 0);
    g_SoundPlayer.QueueSound(0x21, 0);
    g_ReplayManager->unk_d6 |= 0x10;
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
    UpdateBombState(player);
    if (player->playerState == 2)
    {
        if (UpdateDyingState(player))
        {
            goto respawn;
        }
    }
    else if (player->playerState == 1)
    {
    respawn:
        UpdateRespawningState(player);
    }
    UpdateInvulnerability(player);
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

int Player::OnDraw(Player *player)
{
    ColorBytes color;

    DrawBullets(player);
    if (player->bomb.running)
    {
        if (!player->bomb.focused)
        {
            player->bomb.draw(player);
        }
        else
        {
            player->bomb.drawFocused(player);
        }
    }
    if (!g_Unknown626270.isInContinueMenu)
    {
        player->sprite.pos.x = g_Unknown626270.arena[0] + player->position.x;
        player->sprite.pos.y = g_Unknown626270.arena[1] + player->position.y;
        player->sprite.pos.z = 0.0f;
        g_SpriteEngine->DrawUnrotated(&player->sprite);
        if (player->unk_240a && (player->playerState == 0 || player->playerState == 4 || player->playerState == 3))
        {
            player->optionSprites[0].pos.x = g_Unknown626270.arena[0] + player->optionPos[0].x;
            player->optionSprites[0].pos.y = g_Unknown626270.arena[1] + player->optionPos[0].y;
            player->optionSprites[0].pos.z = 0.0f;
            player->optionSprites[1].pos.x = g_Unknown626270.arena[0] + player->optionPos[1].x;
            player->optionSprites[1].pos.y = g_Unknown626270.arena[1] + player->optionPos[1].y;
            player->optionSprites[1].pos.z = 0.0f;
            g_SpriteEngine->Draw(&player->optionSprites[0]);
            g_SpriteEngine->Draw(&player->optionSprites[1]);
        }
    }
    if (player->playerState == 4 && player->stateTimer.AsFrames() > 0)
    {
        if (player->stateTimer.AsFrames() % 4 < 2)
        {
            player->sprite.color = 0xffff0000;
        }
        else
        {
            player->sprite.color = 0xffffffff;
        }
        color.a = 0x80;
        if (g_Player.stateTimer >= 510)
        {
            color.b = 0x80 - (540 - g_Player.stateTimer.AsFrames()) * 80 / 30;
            color.g = color.b;
            color.r = color.g;
        }
        else if (g_Player.stateTimer < 30)
        {
            color.b = 0x80 - g_Player.stateTimer.AsFrames() * 80 / 30;
            color.g = color.b;
            color.r = color.g;
        }
        else
        {
            color.b = 0x30;
            color.g = color.b;
            color.r = color.g;
        }
        g_Unknown1347b00.SetTintColor(color.color);
    }
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

int Player::AddedCallback(Player *player)
{
    int i;
    PlayerBullet *bullet;

    if (LoadShotTypeFile(&player->shotType, g_ShotFiles[g_Unknown626270.unk_93d7]) != 0)
    {
        return -1;
    }
    if (LoadShotTypeFile(&player->shotTypeFocused, g_ShotFilesFocused[g_Unknown626270.unk_93d7]) != 0)
    {
        return -1;
    }
    if ((g_App.scene != 3 && g_App.scene != 0xb && g_App.scene != 0xc) != 0)
    {
        switch (g_Unknown626270.character)
        {
        case 0:
            if (g_SpriteEngine->LoadAnm(10, "data/player00.anm", 0x400) != 0)
            {
                return -1;
            }
            break;
        case 1:
            if (g_SpriteEngine->LoadAnm(10, "data/player01.anm", 0x400) != 0)
            {
                return -1;
            }
            break;
        case 2:
            if (g_SpriteEngine->LoadAnm(10, "data/player02.anm", 0x400) != 0)
            {
                return -1;
            }
            break;
        }
    }
    g_SpriteEngine->StartScript(&player->sprite, 0x400);
    player->position.x = g_Unknown626270.arena[2] / 2.0f;
    player->position.y = g_Unknown626270.arena[3] - 64.0f;
    player->position.z = 0.49f;
    player->optionPos[0].z = 0.49f;
    player->optionPos[1].z = 0.49f;
    // 0x80 entries: past unk_9dc's 0x70, into unk_17dc.
    for (i = 0; i < 0x80; i++)
    {
        player->unk_9dc[i].size.x = 0.0f;
    }
    player->hitboxSize.y = g_Player.shotType->hitboxSize / 2.0f;
    player->hitboxSize.x = player->hitboxSize.y;
    player->hitboxSize.z = 5.0f;
    player->grazeSize.y = g_Player.shotType->grazeSize / 2.0f;
    player->grazeSize.x = player->grazeSize.y;
    player->grazeSize.z = 5.0f;
    player->unk_9a8.x = 12.0f;
    player->unk_9a8.y = 12.0f;
    player->unk_9a8.z = 5.0f;
    player->unk_241c = 0;
    player->playerState = 1;
    player->stateTimer.Set(120);
    player->unk_240a = 1;
    g_SpriteEngine->StartScript(&player->optionSprites[0], 0x480);
    g_SpriteEngine->StartScript(&player->optionSprites[1], 0x481);
    bullet = player->bullets;
    for (i = 0; i < 0x60; i++, bullet++)
    {
        bullet->state = 0;
    }
    player->fireTimer.Set(-1);
    player->bomb.update = g_Bombs[g_Unknown626270.unk_93d7].update;
    player->bomb.draw = g_Bombs[g_Unknown626270.unk_93d7].draw;
    player->bomb.updateFocused = g_Bombs[g_Unknown626270.unk_93d7].updateFocused;
    player->bomb.drawFocused = g_Bombs[g_Unknown626270.unk_93d7].drawFocused;
    player->bomb.running = 0;
    player->unk_b7e58 = -(D3DX_PI / 2.0f);
    player->moveScaleY = 1.0f;
    player->moveScaleX = 1.0f;
    player->deathbombFrames = g_Player.shotType->deathbombFrames;
    if ((g_App.scene != 3 && g_App.scene != 0xb && g_App.scene != 0xc) != 0)
    {
        g_AsciiText.unk_498.pendingInterrupt = 1;
        g_AsciiText.unk_74d4 = 1;
    }
    g_AsciiText.SetUnkB7cInterrupt(0, 2);
    g_AsciiText.SetUnkB7cInterrupt(1, 2);
    g_AsciiText.SetUnkB7cInterrupt(2, 2);
    if (g_Unknown626270.unk_9620 >= g_Unknown626270.stats->unk_88 + 50000)
    {
        g_Unknown626270.unk_9620 = g_Unknown626270.stats->unk_88 + 50000;
        StartBombState(&g_Player);
    }
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

int LoadShotTypeFile(ShotTypeData **out, char *path)
{
    int i;
    ShotData *shot;

    *out = (ShotTypeData *)LoadFileData(path, 0);
    if (*out == NULL)
    {
        return -1;
    }
    for (i = 0; i < (*out)->tableCount; i++)
    {
        (*out)->tables[i].shots = (ShotData *)((int)(*out)->tables[i].shots + (int)*out);
        shot = (*out)->tables[i].shots;
        while (shot->interval >= 0)
        {
            shot->fire = g_ShotFireFuncs[(int)shot->fire];
            shot->update = g_ShotUpdateFuncs[(int)shot->update];
            shot->draw = g_ShotDrawFuncs[(int)shot->draw];
            shot->hit = g_ShotHitFuncs[(int)shot->hit];
            shot++;
        }
    }
    return 0;
}

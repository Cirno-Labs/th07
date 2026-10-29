// The file with BulletManager, from 0x4232e0, as th06's bullet.cpp.

#include <math.h>
#include <string.h>

#include "ascii.h"
#include "bullet.h"
#include "item.h"
#include "player.h"
#include "unsorted.h"

BulletManager::BulletManager()
{
    this->Reset();
}

void ShowSpawnSprite(Sprite *dst, Sprite *src, Bullet *bullet, int color)
{
    if (dst->spriteIndex != src->spriteIndex + color)
    {
        if (bullet->sprites.sprite0.spriteData->height <= 16.0f)
        {
            g_SpriteEngine->SetSprite(dst, src->spriteIndex + g_BulletColorOffsets16[color]);
        }
        else if (bullet->sprites.sprite0.spriteData->height <= 32.0f)
        {
            g_SpriteEngine->SetSprite(dst, src->spriteIndex + g_BulletColorOffsets32[color]);
        }
        else
        {
            g_SpriteEngine->SetSprite(dst, src->spriteIndex + color);
        }
    }
}

void BulletManager::Reset()
{
    memset(this, 0, sizeof(BulletManager));
    this->nextBullet = this->bullets;
    this->bullets[0x400].state = 6;
    this->unk_37a160 = 6;
}

int BulletManager::DeletedCallback(BulletManager *mgr)
{
    if ((g_App.scene != 3 && g_App.scene != 0xb && g_App.scene != 0xc) != 0)
    {
        g_SpriteEngine->ReleaseAnm(0xb);
        g_SpriteEngine->ReleaseAnm(0xc);
        g_SpriteEngine->ReleaseAnm(0xd);
        g_SpriteEngine->ReleaseAnm(0xe);
    }
    return 0;
}

int BulletManager::RegisterChain(char *anmPath)
{
    BulletManager *mgr = &g_BulletManager;

    g_EffectColors = g_EffectColorsFull;
    mgr->Reset();
    mgr->anmPath = anmPath;
    g_BulletCalcChain.SetCallback((ChainCallback)BulletManager::OnUpdate);
    g_BulletCalcChain.addedCallback = (ChainCallback)BulletManager::AddedCallback;
    g_BulletCalcChain.deletedCallback = (ChainCallback)BulletManager::DeletedCallback;
    g_BulletCalcChain.arg = mgr;
    if (g_Chain.AddToCalcChain(&g_BulletCalcChain, 0xc) != 0)
    {
        return -1;
    }
    g_BulletDrawChain.SetCallback((ChainCallback)BulletManager::OnDraw);
    g_BulletDrawChain.arg = mgr;
    g_Chain.AddToDrawChain(&g_BulletDrawChain, 10);
    return 0;
}

void BulletManager::RemoveChain()
{
    g_Chain.Remove(&g_BulletCalcChain);
    g_Chain.Remove(&g_BulletDrawChain);
    memset(&g_BulletManager, 0, sizeof(BulletManager));
}

void BulletManager::StopAllBullets()
{
    Bullet *bullet;
    int i;

    bullet = g_BulletManager.bullets;
    for (i = 0; i < 0x400; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        bullet->velocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        bullet->unk_ba4 = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        bullet->unk_bb8 = 0.0f;
        bullet->unk_bb4 = 0.0f;
        bullet->speed = 0.0f;
        bullet->color = 0;
        g_SpriteEngine->SetSprite(&bullet->sprites.sprite0, bullet->sprites.sprite0.baseSpriteIndex + bullet->color);
    }
}

BulletCommand *Bullet::RestartCommand(int idx, int unk_14, DWORD flag)
{
    BulletCommand *cmd = &this->commands[idx];

    cmd->flag = flag;
    cmd->unk_14 = unk_14;
    this->flags |= flag;
    this->commandIndex = 0;
    return cmd;
}

BulletCommand *BulletShooter::StartCommand(int idx, int unk_14, DWORD flag)
{
    BulletCommand *cmd = &this->commands[idx];

    cmd->flag = flag;
    cmd->unk_14 = unk_14;
    this->flags |= flag;
    return cmd;
}

void Bullet::SetTurnCommand(int idx, int unk_14, int unk_8, float unk_4, float unk_0)
{
    BulletCommand *cmd = this->RestartCommand(idx, unk_14, 0x20);

    cmd->unk_8 = unk_8;
    cmd->unk_0 = unk_0;
    cmd->unk_4 = unk_4;
}

void Bullet::SetAccelerateCommand(int idx, int unk_14, int unk_8, float unk_0, float unk_4)
{
    BulletCommand *cmd = this->RestartCommand(idx, unk_14, 0x10);

    cmd->unk_8 = unk_8;
    cmd->unk_0 = unk_0;
    cmd->unk_4 = unk_4;
}

void BulletShooter::SetShooterTurnCommand(int idx, int unk_14, int unk_8, float unk_4, float unk_0)
{
    BulletCommand *cmd = this->StartCommand(idx, unk_14, 0x20);

    cmd->unk_8 = unk_8;
    cmd->unk_0 = unk_0;
    cmd->unk_4 = unk_4;
}

void BulletShooter::SetStopTurnCommand(int idx, int unk_14, int unk_8, int unk_c, float unk_0, float unk_4)
{
    BulletCommand *cmd = this->StartCommand(idx, unk_14, 0x80);

    cmd->unk_8 = unk_8;
    cmd->unk_c = unk_c;
    cmd->unk_0 = unk_0;
    cmd->unk_4 = unk_4;
}

void BulletShooter::SetShooterAccelerateCommand(int idx, int unk_14, int unk_8, float unk_0, float unk_4)
{
    BulletCommand *cmd = this->StartCommand(idx, unk_14, 0x10);

    cmd->unk_8 = unk_8;
    cmd->unk_0 = unk_0;
    cmd->unk_4 = unk_4;
}

void BulletShooter::SetHiddenCommand(int idx, int unk_14, int unk_8)
{
    BulletCommand *cmd = this->StartCommand(idx, unk_14, 0x2000);

    cmd->unk_8 = unk_8;
}

int BulletManager::FirePattern(BulletShooter *shooter)
{
    int col;
    float playerAngle;
    int row;

    if (g_BulletManager.bulletCount >= 0x400)
    {
        return 0;
    }
    shooter->typeSprites = &this->typeSprites[shooter->sprite];
    playerAngle = g_Player.AngleFrom(&shooter->pos);
    for (row = 0; row < shooter->count2; row++)
    {
        for (col = 0; col < shooter->count1; col++)
        {
            if (this->SpawnBullet(shooter, col, row, playerAngle))
            {
                goto done;
            }
        }
    }
done:
    if (shooter->flags & 0x200)
    {
        g_SoundPlayer.QueueSound(shooter->sound, 0);
    }
    return 0;
}

Laser *BulletManager::SpawnLaser(BulletShooter *shooter)
{
    int i;
    Laser *laser;

    laser = this->lasers;
    if (this->unk_37a12c != 0 && !(shooter->flags & 4))
    {
        return laser;
    }
    for (i = 0; i < 0x40; i++, laser++)
    {
        if (laser->active != 0)
        {
            continue;
        }
        g_SpriteEngine->StartScript(&laser->sprite0, shooter->sprite + 0x20a);
        g_SpriteEngine->SetSprite(&laser->sprite0, laser->sprite0.spriteIndex + shooter->color);
        g_SpriteEngine->InitSprite(&laser->sprite1, g_BulletColorOffsets16[shooter->color] + 0x292);
        laser->sprite1.flags |= 0x10;
        laser->pos = shooter->pos;
        laser->color = shooter->color;
        laser->active = 1;
        laser->angle = shooter->angle1;
        if (shooter->aimMode == 0)
        {
            laser->angle += g_Player.AngleFrom(&shooter->pos);
        }
        laser->flags = shooter->flags;
        laser->timer.Set(0);
        laser->startOffset = shooter->exFloats[0];
        laser->endOffset = shooter->exFloats[1];
        laser->maxLength = shooter->exFloats[2];
        laser->width = shooter->exFloats[3];
        laser->speed = shooter->speed1;
        laser->startFrames = shooter->exInts[0];
        laser->durationFrames = shooter->exInts[1];
        laser->endFrames = shooter->exInts[2];
        laser->hitStartFrame = shooter->exInts[3];
        laser->endHitFrames = shooter->laserEndHitFrames;
        laser->unk_4e9 = 0;
        if (laser->startFrames == 0)
        {
            laser->state = 1;
        }
        else
        {
            laser->state = 0;
        }
        break;
    }
    return laser;
}

void BulletManager::ClearAll(int mode)
{
    D3DXVECTOR3 p;
    int i;
    float s;
    Bullet *curBullet;
    Laser *beam;
    float c;
    float t;

    curBullet = g_BulletManager.bullets;
    for (i = 0; i < 0x400; i++, curBullet++)
    {
        if (curBullet->state == 0 || curBullet->state == 5)
        {
            continue;
        }
        if (mode != 0 && mode < 9)
        {
            if (mode < 3)
            {
                g_ItemManager.SpawnItem(&curBullet->pos, this->unk_37a160, mode);
            }
            else
            {
                g_ItemManager.SpawnItem(&curBullet->pos, 8, 1);
            }
            memset(curBullet, 0, sizeof(Bullet));
        }
        else
        {
            curBullet->state = 5;
        }
    }
    beam = this->lasers;
    for (i = 0; i < 0x40; i++, beam++)
    {
        if (!beam->active)
        {
            continue;
        }
        if ((beam->flags & 4) && mode != 10)
        {
            continue;
        }
        if (beam->state < 2)
        {
            beam->state = 2;
            beam->timer.Set(0);
            beam->width = beam->unk_4b8;
            if (mode != 0 && mode < 9)
            {
                t = beam->startOffset;
                SinCos(&s, &c, beam->angle);
                while (beam->endOffset > t)
                {
                    p.x = c * t + beam->pos.x;
                    p.y = s * t + beam->pos.y;
                    p.z = 0.0f;
                    if (mode < 3)
                    {
                        g_ItemManager.SpawnItem(&p, this->unk_37a160, mode);
                    }
                    else
                    {
                        g_ItemManager.SpawnItem(&p, 8, 1);
                    }
                    t += 32.0f;
                }
            }
        }
        beam->endHitFrames = 0;
    }
    this->unk_37a12c = 10;
}

int BulletManager::ClearAllWithBonus(int maxBonus, int toItems)
{
    int score;
    int totalBonus;
    int numCleared;
    int i;
    float s;
    Bullet *curBullet;
    D3DXVECTOR3 spot;
    Laser *beam;
    float c;
    float t;

    totalBonus = 0;
    score = 2000;
    numCleared = 0;
    curBullet = g_BulletManager.bullets;
    for (i = 0; i < 0x400; i++, curBullet++)
    {
        if (curBullet->state == 0)
        {
            continue;
        }
        g_ItemManager.SpawnItem(&curBullet->pos, this->unk_37a160, 1);
        g_AsciiText.AddPopup(&curBullet->pos, score, score >= maxBonus ? 0xffffff00 : 0xffffffff);
        totalBonus += score;
        numCleared++;
        score += 20;
        if (score > maxBonus)
        {
            score = maxBonus;
        }
        curBullet->state = 5;
    }
    beam = this->lasers;
    for (i = 0; i < 0x40; i++, beam++)
    {
        if (!beam->active)
        {
            continue;
        }
        if (beam->state < 2)
        {
            beam->state = 2;
            beam->timer.Set(0);
            beam->width = beam->unk_4b8;
            if (toItems)
            {
                g_ItemManager.SpawnItem(&beam->pos, this->unk_37a160, 1);
                t = beam->startOffset;
                SinCos(&s, &c, beam->angle);
                while (beam->endOffset > t)
                {
                    spot.x = c * t + beam->pos.x;
                    spot.y = s * t + beam->pos.y;
                    spot.z = 0.0f;
                    g_ItemManager.SpawnItem(&spot, this->unk_37a160, 1);
                    t += 32.0f;
                }
            }
        }
        beam->endHitFrames = 0;
    }
    this->unk_37a12c = 10;
    return totalBonus;
}

void BulletManager::ConvertBulletsToItems(D3DXVECTOR3 *pos, float radius)
{
    Bullet *bullet;
    int i;
    D3DXVECTOR3 v;

    bullet = g_BulletManager.bullets;
    radius = radius * radius;
    for (i = 0; i < 0x400; i++, bullet++)
    {
        if (bullet->state == 0 || bullet->state == 5)
        {
            continue;
        }
        v = bullet->pos - *pos;
        if (D3DXVec3LengthSq(&v) > radius)
        {
            continue;
        }
        g_ItemManager.SpawnItem(&bullet->pos, 6, 1);
        memset(bullet, 0, sizeof(Bullet));
    }
}

void SlowBulletStart(Bullet *bullet)
{
    float speed;

    if (bullet->ex[0].timer <= 16)
    {
        speed = 5.0f - bullet->ex[0].timer.AsFloat() * 5.0f / 16.0f;
        ((Unknown4325c0 *)&bullet->velocity)->SetPolar(bullet->angle, (speed + bullet->speed) * g_App.unk_178);
    }
    else
    {
        bullet->activeFlags ^= 1;
    }
    bullet->ex[0].timer.Tick();
}

void AccelerateAlongVelocity(Bullet *bullet)
{
    if (bullet->ex[1].timer >= bullet->ex[1].duration)
    {
        bullet->activeFlags &= ~0x10;
    }
    else
    {
        bullet->velocity += bullet->ex[1].accel * g_App.unk_178;
        if (fabsf(bullet->velocity.x) > 0.0001f || fabsf(bullet->velocity.y) > 0.0001f)
        {
            bullet->angle = atan2f(bullet->velocity.y, bullet->velocity.x);
        }
    }
    bullet->ex[1].timer.Tick();
}

void TurnAndAccelerate(Bullet *bullet)
{
    if (bullet->ex[2].timer >= bullet->ex[2].duration)
    {
        bullet->activeFlags &= ~0x20;
    }
    else
    {
        bullet->angle = AddNormalizeAngle(bullet->angle, g_App.unk_178 * bullet->ex[2].unk_10);
        bullet->speed = g_App.unk_178 * bullet->ex[2].unk_c + bullet->speed;
        ((Unknown4325c0 *)&bullet->velocity)->SetPolar(bullet->angle, g_App.unk_178 * bullet->speed);
    }
    bullet->ex[2].timer.Tick();
}

void StopTurnAndRestart(Bullet *bullet)
{
    float speed;

    if (bullet->ex[3].timer >= bullet->ex[3].duration)
    {
        if (bullet->sound >= 0)
        {
            g_SoundPlayer.QueueSound(bullet->sound, 0);
        }
        bullet->ex[3].unk_28++;
        if (bullet->ex[3].unk_28 >= bullet->ex[3].unk_24)
        {
            bullet->activeFlags &= ~0x40;
        }
        bullet->angle += bullet->ex[3].unk_10;
        bullet->speed = bullet->ex[3].unk_c;
        speed = bullet->speed;
        bullet->ex[3].timer.Set(0);
    }
    else
    {
        speed = bullet->speed - bullet->ex[3].timer.AsFloat() * bullet->speed / bullet->ex[3].duration;
    }
    ((Unknown4325c0 *)&bullet->velocity)->SetPolar(bullet->angle, speed * g_App.unk_178);
    bullet->ex[3].timer.Tick();
}

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

BulletTypeSprites::BulletTypeSprites()
{
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

// Copies `src` over `dst` unless `dst` already runs the same script and
// `src` has not started one.
inline void CopyBulletSprite(Sprite *dst, Sprite *src)
{
    if (dst->scriptIndex != src->scriptIndex || src->scriptPos != NULL)
    {
        *dst = *src;
    }
}

inline float BulletShooter::RandomAngle()
{
    float range = this->angle1 - this->angle2;
    return g_Rng.GetRandomF32ZeroToOne() * range + this->angle2;
}

inline float BulletShooter::RandomSpeed()
{
    float range = this->speed1 - this->speed2;
    return g_Rng.GetRandomF32ZeroToOne() * range + this->speed2;
}

int BulletManager::SpawnBullet(BulletShooter *shooter, int col, int row, float aimAngle)
{
    float v;
    int i = 0;
    Bullet *cur;
    float angle;
    // Set and never read.
    int bulletFlags;

    cur = this->nextBullet;
    for (i = 0; i < 0x400; i++)
    {
        if (cur->state == 0)
        {
            break;
        }
        cur++;
        if (cur->state == 6)
        {
            cur = this->bullets;
        }
    }
    if (i >= 0x400)
    {
        return 1;
    }
    angle = 0.0f;
    if (shooter->count2 > 1)
    {
        v = shooter->speed1 - (shooter->speed1 - shooter->speed2) * row / shooter->count2;
    }
    else
    {
        v = shooter->speed1;
    }
    switch (shooter->aimMode)
    {
    case 0:
    case 1:
        if (shooter->count1 & 1)
        {
            angle += (col + 1) / 2 * shooter->angle2;
        }
        else
        {
            angle += col / 2 * shooter->angle2 + shooter->angle2 * 0.5f;
        }
        if (col & 1)
        {
            angle *= -1.0f;
        }
        if (shooter->aimMode == 0)
        {
            angle += aimAngle;
        }
        angle += shooter->angle1;
        break;
    case 2:
        angle += aimAngle;
    case 3:
        angle += col * 6.2831855f / shooter->count1;
        angle += row * shooter->angle2 + shooter->angle1;
        break;
    case 4:
        angle += aimAngle;
    case 5:
        angle += 3.1415927f / shooter->count1;
        angle += col * 6.2831855f / shooter->count1;
        angle += shooter->angle1;
        break;
    case 6:
        angle = shooter->RandomAngle();
        break;
    case 7:
        v = shooter->RandomSpeed();
        angle += col * 6.2831855f / shooter->count1;
        angle += row * shooter->angle2 + shooter->angle1;
        break;
    case 8:
        angle = shooter->RandomAngle();
        v = shooter->RandomSpeed();
        break;
    }
    cur->state = 1;
    cur->unk_c00 = 1;
    cur->grazed = 0;
    cur->timer.Set(0);
    cur->unk_bd4.Set(0);
    cur->speed = v;
    cur->angle = AddNormalizeAngle(angle, 0.0f);
    cur->pos = shooter->pos;
    cur->pos.z = 0.1f;
    ((Vector2 *)&cur->velocity)->SetPolar(angle, v * g_App.gameSpeed);
    cur->activeFlags = shooter->flags;
    cur->color = shooter->color;
    cur->unk_c08 = 0;
    CopyBulletSprite(&cur->sprites.sprite0, &shooter->typeSprites->sprite0);
    CopyBulletSprite(&cur->sprites.sprite4, &shooter->typeSprites->sprite4);
    cur->sprites.hitboxSize = shooter->typeSprites->hitboxSize;
    cur->sprites.unk_b88 = shooter->typeSprites->unk_b88;
    cur->sprites.height = shooter->typeSprites->height;
    cur->sprites.unk_b8a = shooter->typeSprites->unk_b8a;
    cur->sound = shooter->unk_cc;
    cur->unk_bf0 = 0;
    if (cur->sprites.sprite0.spriteIndex != shooter->typeSprites->sprite0.spriteIndex + shooter->color)
    {
        g_SpriteEngine->SetSprite(&cur->sprites.sprite0, shooter->typeSprites->sprite0.spriteIndex + shooter->color);
    }
    if (cur->sprites.sprite4.spriteIndex != shooter->typeSprites->sprite4.spriteIndex + shooter->color)
    {
        if (cur->sprites.sprite0.spriteData->height <= 16.0f)
        {
            g_SpriteEngine->SetSprite(&cur->sprites.sprite4, shooter->typeSprites->sprite4.spriteIndex + g_BulletColorOffsets16[shooter->color]);
        }
        else if (cur->sprites.sprite0.spriteData->height <= 32.0f)
        {
            g_SpriteEngine->SetSprite(&cur->sprites.sprite4, shooter->typeSprites->sprite4.spriteIndex + g_BulletColorOffsets32[shooter->color]);
        }
        else
        {
            g_SpriteEngine->SetSprite(&cur->sprites.sprite4, shooter->typeSprites->sprite4.spriteIndex + shooter->color);
        }
    }
    bulletFlags = shooter->flags;
    if (shooter->flags & 2)
    {
        CopyBulletSprite(&cur->sprites.sprite1, &shooter->typeSprites->sprite1);
        ShowSpawnSprite(&cur->sprites.sprite1, &shooter->typeSprites->sprite1, cur, shooter->color);
        cur->state = 2;
        cur->pos -= cur->velocity * 4.0f;
    }
    else if (shooter->flags & 4)
    {
        CopyBulletSprite(&cur->sprites.sprite2, &shooter->typeSprites->sprite2);
        ShowSpawnSprite(&cur->sprites.sprite2, &shooter->typeSprites->sprite2, cur, shooter->color);
        cur->state = 3;
        cur->pos -= cur->velocity * 4.0f;
    }
    else if (shooter->flags & 8)
    {
        CopyBulletSprite(&cur->sprites.sprite3, &shooter->typeSprites->sprite3);
        ShowSpawnSprite(&cur->sprites.sprite3, &shooter->typeSprites->sprite3, cur, shooter->color);
        cur->state = 4;
        cur->pos -= cur->velocity * 4.0f;
    }
    memcpy(cur->commands, shooter->commands, sizeof(cur->commands));
    cur->flags = shooter->flags;
    cur->activeFlags = 0;
    cur->commandIndex = 0;
    RunBulletCommands(cur);
    if (this->unk_37a12c != 0 && !(cur->flags & 0x1000))
    {
        cur->state = 5;
    }
    cur++;
    if (cur->state == 6)
    {
        this->nextBullet = this->bullets;
    }
    else
    {
        this->nextBullet = cur;
    }
    return 0;
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
        ((Vector2 *)&bullet->velocity)->SetPolar(bullet->angle, (speed + bullet->speed) * g_App.gameSpeed);
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
        bullet->velocity += bullet->ex[1].accel * g_App.gameSpeed;
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
        bullet->angle = AddNormalizeAngle(bullet->angle, g_App.gameSpeed * bullet->ex[2].unk_10);
        bullet->speed = g_App.gameSpeed * bullet->ex[2].unk_c + bullet->speed;
        ((Vector2 *)&bullet->velocity)->SetPolar(bullet->angle, g_App.gameSpeed * bullet->speed);
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
    ((Vector2 *)&bullet->velocity)->SetPolar(bullet->angle, speed * g_App.gameSpeed);
    bullet->ex[3].timer.Tick();
}

void StopTurnToAngle(Bullet *bullet)
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
            bullet->activeFlags &= ~0x100;
        }
        bullet->angle = bullet->ex[3].unk_10;
        bullet->speed = bullet->ex[3].unk_c;
        speed = bullet->speed;
        bullet->ex[3].timer.Set(0);
    }
    else
    {
        speed = bullet->speed - bullet->ex[3].timer.AsFloat() * bullet->speed / bullet->ex[3].duration;
    }
    ((Vector2 *)&bullet->velocity)->SetPolar(bullet->angle, speed * g_App.gameSpeed);
    bullet->ex[3].timer.Tick();
}

void StopTurnToPlayer(Bullet *bullet)
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
            bullet->activeFlags &= ~0x80;
        }
        bullet->angle = AddNormalizeAngle(g_Player.AngleFrom(&bullet->pos), bullet->ex[3].unk_10);
        bullet->speed = bullet->ex[3].unk_c;
        speed = bullet->speed;
        bullet->ex[3].timer.Set(0);
    }
    else
    {
        speed = bullet->speed - bullet->ex[3].timer.AsFloat() * bullet->speed / bullet->ex[3].duration;
    }
    ((Vector2 *)&bullet->velocity)->SetPolar(bullet->angle, speed * g_App.gameSpeed);
    bullet->ex[3].timer.Tick();
}

void BounceOffWalls(Bullet *bullet)
{
    float speed;

    if (!g_Unknown626270.IsInPlayArea(bullet->pos.x, bullet->pos.y, bullet->sprites.sprite0.spriteData->width, bullet->sprites.sprite0.spriteData->height))
    {
        if (bullet->sound >= 0)
        {
            g_SoundPlayer.QueueSound(bullet->sound, 0);
        }
        if (bullet->pos.x < 0.0f || bullet->pos.x >= 384.0f)
        {
            bullet->angle = -bullet->angle - 3.1415927f;
            bullet->angle = AddNormalizeAngle(bullet->angle, 0.0f);
        }
        if (bullet->pos.y < 0.0f || (bullet->pos.y >= 448.0f && (bullet->activeFlags & 0x400)))
        {
            bullet->angle = -bullet->angle;
        }
        bullet->speed = bullet->ex[4].unk_c;
        speed = bullet->speed;
        ((Vector2 *)&bullet->velocity)->SetPolar(bullet->angle, speed * g_App.gameSpeed);
        bullet->ex[4].duration++;
        if (bullet->ex[4].duration >= bullet->ex[4].unk_24)
        {
            bullet->activeFlags &= ~0xc00;
        }
    }
}

void RunBulletCommands(Bullet *bullet)
{
    BulletCommand *cmd;

    for (;;)
    {
        if (bullet->commandIndex >= 5)
        {
            return;
        }
        cmd = &bullet->commands[bullet->commandIndex];
        if (cmd->flag == 0)
        {
            return;
        }
        if (cmd->unk_14 == 0 && bullet->activeFlags != 0)
        {
            return;
        }
        if ((bullet->flags & cmd->flag) == 0)
        {
            bullet->commandIndex++;
            continue;
        }
        switch (cmd->flag)
        {
        case 1:
            bullet->activeFlags |= 1;
            bullet->ex[0].timer.Set(0);
            bullet->ex[0].accel.z = 0.0f;
            break;
        case 0x10:
            bullet->activeFlags |= 0x10;
            bullet->ex[1].unk_c = cmd->unk_0;
            bullet->ex[1].unk_10 = cmd->unk_4 > -990.0f ? cmd->unk_4 : bullet->angle;
            bullet->ex[1].timer.Set(0);
            bullet->ex[1].duration = cmd->unk_8;
            ((Vector2 *)&bullet->ex[1].accel)->SetPolar(bullet->ex[1].unk_10, g_App.gameSpeed * bullet->ex[1].unk_c);
            if (bullet->commandIndex != 0 && bullet->sound >= 0)
            {
                g_SoundPlayer.QueueSound(bullet->sound, 0);
            }
            break;
        case 0x20:
            bullet->activeFlags |= 0x20;
            bullet->ex[2].unk_c = cmd->unk_0;
            bullet->ex[2].unk_10 = cmd->unk_4;
            bullet->ex[2].timer.Set(0);
            bullet->ex[2].duration = cmd->unk_8;
            if (bullet->commandIndex != 0 && bullet->sound >= 0)
            {
                g_SoundPlayer.QueueSound(bullet->sound, 0);
            }
            break;
        case 0x40:
        case 0x80:
        case 0x100:
            bullet->activeFlags |= cmd->flag;
            bullet->ex[3].unk_10 = cmd->unk_0;
            bullet->ex[3].unk_c = cmd->unk_4 > -999.0f ? cmd->unk_4 : bullet->speed;
            bullet->ex[3].timer.Set(0);
            bullet->ex[3].duration = cmd->unk_8;
            bullet->ex[3].unk_24 = cmd->unk_c;
            bullet->ex[3].unk_28 = 0;
            break;
        case 0x400:
        case 0x800:
            bullet->activeFlags |= cmd->flag;
            if (cmd->unk_0 >= 0.0f)
            {
                bullet->ex[4].unk_c = cmd->unk_0;
            }
            else
            {
                bullet->ex[4].unk_c = bullet->speed;
            }
            bullet->ex[4].unk_24 = cmd->unk_8;
            bullet->ex[4].duration = 0;
            break;
        case 0x2000:
            bullet->unk_bf0 = cmd->unk_8;
            bullet->commandIndex++;
            continue;
        }
        bullet->commandIndex++;
        return;
    }
}

int BulletManager::OnUpdate(BulletManager *mgr)
{
    int fadeTime6;
    D3DXVECTOR3 center2;
    Laser *las;
    int alpha4;
    Bullet *bullet;
    D3DXVECTOR3 extent5;
    // Which bullet the walk is at: 0, then from the last one down.
    int order;
    float scale3;
    int k7;
    int result;

    order = 0;
    bullet = mgr->bullets;
    if (g_Unknown626270.isFrozen != 0)
    {
        return 1;
    }
    g_ItemManager.UpdateItems();
    mgr->bulletCount = 0;
    mgr->drawLists[0] = mgr->drawLists[1] = mgr->drawLists[2] = mgr->drawLists[3] = mgr->drawLists[4] =
        mgr->drawLists[5] = NULL;
    for (k7 = 0; k7 < 0x400; k7++)
    {
        if (bullet->state == 0)
        {
            goto next;
        }
        mgr->bulletCount++;
        switch (bullet->state)
        {
        spawned:
            bullet->state = 1;
            bullet->timer.Set(0);
        case 1:
            RunBulletCommands(bullet);
            if (bullet->activeFlags != 0)
            {
                if (bullet->activeFlags & 1)
                {
                    SlowBulletStart(bullet);
                }
                if (bullet->activeFlags & 0x10)
                {
                    AccelerateAlongVelocity(bullet);
                }
                if (bullet->activeFlags & 0x20)
                {
                    TurnAndAccelerate(bullet);
                }
                if (bullet->activeFlags & 0x40)
                {
                    StopTurnAndRestart(bullet);
                }
                if (bullet->activeFlags & 0x100)
                {
                    StopTurnToAngle(bullet);
                }
                if (bullet->activeFlags & 0x80)
                {
                    StopTurnToPlayer(bullet);
                }
                if (bullet->activeFlags & 0xc00)
                {
                    BounceOffWalls(bullet);
                }
            }
            if (bullet->unk_bf0 != 0)
            {
                bullet->unk_bf0--;
            }
            bullet->pos += bullet->velocity;
            if (bullet->unk_bf0 == 0)
            {
                if (!g_Unknown626270.IsInPlayArea(bullet->pos.x, bullet->pos.y, bullet->sprites.sprite0.spriteData->width,
                                                  bullet->sprites.sprite0.spriteData->height))
                {
                    if (bullet->activeFlags & 0xdc0)
                    {
                        bullet->unk_bfe++;
                        if (bullet->unk_bfe >= 0x80)
                        {
                            bullet->FreeBullet();
                            goto next;
                        }
                    }
                    else
                    {
                        if (bullet->unk_bfe == 0)
                        {
                            bullet->FreeBullet();
                            goto next;
                        }
                        bullet->unk_bfe--;
                    }
                }
                else
                {
                    bullet->unk_bfe = 0;
                }
            }
            if (bullet->grazed == 0 && bullet->unk_bd4.AsFrames() >= 0x10)
            {
                result = g_Player.CheckGraze(&bullet->pos, &bullet->sprites.hitboxSize);
                if (result == 1)
                {
                    bullet->grazed = 1;
                    goto checkHit;
                }
                if (result == 2 && !(bullet->flags & 0x1000))
                {
                    bullet->state = 5;
                    g_ItemManager.SpawnItem(&bullet->pos, g_Player.unk_2404, 1);
                }
            }
            else
            {
            checkHit:
                result = g_Player.CheckHit(&bullet->pos, &bullet->sprites.hitboxSize);
                if (result != 0 && (result != 2 || !(bullet->flags & 0x1000)))
                {
                    bullet->state = 5;
                    if (result == 2)
                    {
                        g_ItemManager.SpawnItem(&bullet->pos, g_Player.unk_2404, 1);
                    }
                }
            }
            if (bullet->sprites.sprite0.scriptPos != NULL)
            {
                g_SpriteEngine->Step(&bullet->sprites.sprite0);
            }
            break;
        case 2:
            bullet->unk_bd4.Sub(1);
            bullet->pos += bullet->velocity / 2.0f;
            if (g_SpriteEngine->Step(&bullet->sprites.sprite1) == 0)
            {
                break;
            }
            goto spawned;
        case 3:
            bullet->unk_bd4.Sub(1);
            bullet->pos += bullet->velocity / 2.5f;
            if (g_SpriteEngine->Step(&bullet->sprites.sprite2) == 0)
            {
                break;
            }
            goto spawned;
        case 4:
            bullet->unk_bd4.Sub(1);
            bullet->pos += bullet->velocity / 3.0f;
            if (g_SpriteEngine->Step(&bullet->sprites.sprite3) == 0)
            {
                break;
            }
            goto spawned;
        case 5:
            bullet->pos += bullet->velocity / 2.0f;
            if (g_SpriteEngine->Step(&bullet->sprites.sprite4) != 0)
            {
                bullet->FreeBullet();
                goto next;
            }
        }
        bullet->timer.Tick();
        bullet->unk_bd4.Tick();
        bullet->next = mgr->drawLists[bullet->sprites.unk_b8a];
        mgr->drawLists[bullet->sprites.unk_b8a] = bullet;
    next:
        order--;
        if (order < 0)
        {
            order = 0x3ff;
            bullet += 0x400;
        }
        bullet--;
    }
    las = mgr->lasers;
    for (k7 = 0; k7 < 0x40; k7++, las++)
    {
        if (!las->active)
        {
            continue;
        }
        las->endOffset = g_App.gameSpeed * las->speed + las->endOffset;
        if (las->endOffset - las->startOffset > las->maxLength)
        {
            las->startOffset = las->endOffset - las->maxLength;
        }
        if (las->startOffset < 0.0f)
        {
            las->startOffset = 0.0f;
        }
        extent5.y = las->width / 2.0f;
        extent5.x = las->endOffset - las->startOffset;
        center2.x = (las->endOffset - las->startOffset) / 2.0f + las->startOffset + las->pos.x;
        center2.y = las->pos.y;
        las->sprite0.scaleY = las->width / las->sprite0.spriteData->width;
        scale3 = las->endOffset - las->startOffset;
        las->sprite0.scaleX = scale3 / las->sprite0.spriteData->height;
        las->sprite0.SetRotationZ(AddNormalizeAngle(1.5707964f + las->angle, 0.0f));
        switch (las->state)
        {
        case 0:
            if (las->flags & 1)
            {
                alpha4 = las->timer.AsFloat() * 255.0f / las->startFrames;
                if (alpha4 > 0xff)
                {
                    alpha4 = 0xff;
                }
                las->sprite0.color = alpha4 << 24;
            }
            else
            {
                fadeTime6 = las->startFrames > 30 ? 30 : las->startFrames;
                if (las->startFrames - fadeTime6 < las->timer.AsFrames())
                {
                    scale3 = las->timer.AsFloat() * las->width / las->startFrames;
                }
                else
                {
                    scale3 = 1.2f;
                }
                las->unk_4b8 = scale3;
                las->sprite0.scaleY = scale3 / 16.0f;
                extent5.x = scale3 / 2.0f;
            }
            if (las->timer >= las->hitStartFrame)
            {
                g_Player.CheckLaserHit(&center2, &extent5, &las->pos, las->angle, las->timer.AsFrames() % 12 == 0);
            }
            if (las->timer < las->startFrames)
            {
                break;
            }
            las->timer.Set(0);
            las->state++;
            las->unk_4b8 = las->width;
        case 1:
            g_Player.CheckLaserHit(&center2, &extent5, &las->pos, las->angle, las->timer.AsFrames() % 12 == 0);
            if (las->timer < las->durationFrames)
            {
                break;
            }
            las->timer.Set(0);
            las->state++;
            if (las->endFrames == 0)
            {
                las->active = 0;
                continue;
            }
        case 2:
            if (las->flags & 1)
            {
                alpha4 = las->timer.AsFloat() * 255.0f / las->startFrames;
                if (alpha4 > 0xff)
                {
                    alpha4 = 0xff;
                }
                las->sprite0.color = alpha4 << 24;
            }
            else if (las->endFrames > 0)
            {
                las->sprite0.scaleY = (scale3 = las->width - las->timer.AsFloat() * las->width / las->endFrames) / 16.0f;
                extent5.x = scale3 / 2.0f;
            }
            if (las->timer < las->endHitFrames)
            {
                g_Player.CheckLaserHit(&center2, &extent5, &las->pos, las->angle, las->timer.AsFrames() % 12 == 0);
            }
            if (las->timer < las->endFrames)
            {
                break;
            }
            las->active = 0;
            continue;
        }
        if (las->startOffset >= 640.0f)
        {
            las->active = 0;
        }
        las->timer.Tick();
        g_SpriteEngine->Step(&las->sprite0);
    }
    if (mgr->unk_37a12c != 0)
    {
        mgr->unk_37a12c--;
    }
    mgr->unk_37a130.Tick();
    mgr->unk_37a13c++;
    return 1;
}

inline void Bullet::FreeBullet()
{
    this->state = 0;
    this->timer.Set(0);
    this->unk_bd4.Set(0);
}

void DrawBullet(Bullet *bullet)
{
    Sprite *sprite;

    switch (bullet->state)
    {
    case 2:
        sprite = &bullet->sprites.sprite1;
        break;
    case 3:
        sprite = &bullet->sprites.sprite2;
        break;
    case 4:
        sprite = &bullet->sprites.sprite3;
        break;
    case 5:
        sprite = &bullet->sprites.sprite4;
        break;
    default:
        sprite = &bullet->sprites.sprite0;
    }
    sprite->pos.x = g_Unknown626270.arena[0] + bullet->pos.x;
    sprite->pos.y = g_Unknown626270.arena[1] + bullet->pos.y;
    sprite->pos.z = 0.05f;
    sprite->color = sprite->color & 0xff000000 | 0xffffff;
    if (sprite->autoRotate != 0)
    {
        sprite->SetRotationZ(AddNormalizeAngle(1.5707964f + bullet->angle, 0.0f));
    }
    g_SpriteEngine->Draw(sprite);
}

int BulletManager::OnDraw(BulletManager *mgr)
{
    int i;
    float s;
    Laser *laser;
    float len;
    float ca;
    Bullet *blt;

    laser = mgr->lasers;
    for (i = 0; i < 0x40; i++, laser++)
    {
        if (!laser->active)
        {
            continue;
        }
        SinCos(&s, &ca, laser->angle);
        len = (laser->endOffset - laser->startOffset) / 2.0f + laser->startOffset;
        laser->sprite0.pos.x = ca * len + laser->pos.x;
        laser->sprite0.pos.y = s * len + laser->pos.y;
        laser->sprite0.pos.z = 0.05f;
        laser->color = laser->color & 0xff000000 | 0xffffff;
        laser->sprite0.pos.x = g_Unknown626270.arena[0] + laser->sprite0.pos.x;
        laser->sprite0.pos.y = g_Unknown626270.arena[1] + laser->sprite0.pos.y;
        g_SpriteEngine->Draw(&laser->sprite0);
        if ((laser->startOffset < 16.0f || laser->speed == 0.0f) && (laser->unk_4e9 == 0 || laser->state != 0))
        {
            laser->sprite1.pos.x = ca * laser->startOffset + laser->pos.x;
            laser->sprite1.pos.y = s * laser->startOffset + laser->pos.y;
            laser->sprite1.pos.z = 0.05f;
            laser->sprite1.color = laser->sprite0.color;
            laser->sprite1.flags |= 0x20;
            laser->sprite1.color = laser->sprite1.color & 0xffffff | 0xff000000;
            laser->sprite1.scaleY = laser->width / 10.0f * ((16.0f - laser->startOffset) / 16.0f);
            laser->sprite1.scaleX = laser->sprite1.scaleY;
            if (laser->sprite1.scaleX <= 0.0f)
            {
                laser->sprite1.scaleY = laser->width / 10.0f;
                laser->sprite1.scaleX = laser->sprite1.scaleY;
            }
            laser->sprite1.pos.x = g_Unknown626270.arena[0] + laser->sprite1.pos.x;
            laser->sprite1.pos.y = g_Unknown626270.arena[1] + laser->sprite1.pos.y;
            g_SpriteEngine->Draw(&laser->sprite1);
        }
    }
    g_ItemManager.DrawItems();
    for (i = 0; i < 6; i++)
    {
        blt = mgr->drawLists[i];
        while (blt != NULL)
        {
            DrawBullet(blt);
            blt = blt->next;
        }
    }
    return 1;
}

int BulletManager::AddedCallback(BulletManager *mgr)
{
    DWORD i;

    if ((g_App.scene != 3 && g_App.scene != 0xb && g_App.scene != 0xc) != 0)
    {
        if (g_SpriteEngine->LoadAnm(0xb, "data/etama.anm", 0x200) != 0)
        {
            return -1;
        }
    }
    for (i = 0; i < ARRAY_SIZE(g_BulletTypeScripts); i++)
    {
        g_SpriteEngine->StartScript(&mgr->typeSprites[i].sprite0, g_BulletTypeScripts[i][0]);
        g_SpriteEngine->StartScript(&mgr->typeSprites[i].sprite1, g_BulletTypeScripts[i][1]);
        g_SpriteEngine->StartScript(&mgr->typeSprites[i].sprite2, g_BulletTypeScripts[i][2]);
        g_SpriteEngine->StartScript(&mgr->typeSprites[i].sprite3, g_BulletTypeScripts[i][3]);
        g_SpriteEngine->StartScript(&mgr->typeSprites[i].sprite4, g_BulletTypeScripts[i][4]);
        mgr->typeSprites[i].sprite0.flags |= 0x1000;
        mgr->typeSprites[i].sprite1.flags |= 0x1000;
        mgr->typeSprites[i].sprite2.flags |= 0x1000;
        mgr->typeSprites[i].sprite3.flags |= 0x1000;
        mgr->typeSprites[i].sprite4.flags |= 0x1000;
        mgr->typeSprites[i].sprite0.baseSpriteIndex = mgr->typeSprites[i].sprite0.spriteIndex;
        mgr->typeSprites[i].height = mgr->typeSprites[i].sprite0.spriteData->height;
        if (mgr->typeSprites[i].sprite0.spriteData->height <= 8.0f)
        {
            mgr->typeSprites[i].hitboxSize.x = 4.0f;
            mgr->typeSprites[i].hitboxSize.y = 4.0f;
            mgr->typeSprites[i].unk_b8a = 5;
        }
        else if (mgr->typeSprites[i].sprite0.spriteData->height <= 16.0f)
        {
            switch (g_BulletTypeScripts[i][0])
            {
            case 0x202:
                mgr->typeSprites[i].hitboxSize.x = 4.0f;
                mgr->typeSprites[i].hitboxSize.y = 4.0f;
                mgr->typeSprites[i].unk_b8a = 4;
                break;
            case 0x204:
            case 0x206:
                mgr->typeSprites[i].hitboxSize.x = 4.0f;
                mgr->typeSprites[i].hitboxSize.y = 4.0f;
                mgr->typeSprites[i].unk_b8a = 4;
                break;
            case 0x205:
                mgr->typeSprites[i].hitboxSize.x = 4.0f;
                mgr->typeSprites[i].hitboxSize.y = 4.0f;
                mgr->typeSprites[i].unk_b8a = 4;
                break;
            default:
                mgr->typeSprites[i].hitboxSize.x = 6.0f;
                mgr->typeSprites[i].hitboxSize.y = 6.0f;
                mgr->typeSprites[i].unk_b8a = 3;
            }
        }
        else if (mgr->typeSprites[i].sprite0.spriteData->height <= 32.0f)
        {
            switch (g_BulletTypeScripts[i][0])
            {
            case 0x208:
                mgr->typeSprites[i].hitboxSize.x = 5.0f;
                mgr->typeSprites[i].hitboxSize.y = 5.0f;
                mgr->typeSprites[i].unk_b8a = 1;
                break;
            case 0x209:
                mgr->typeSprites[i].hitboxSize.x = 8.0f;
                mgr->typeSprites[i].hitboxSize.y = 8.0f;
                mgr->typeSprites[i].unk_b8a = 2;
                break;
            default:
                mgr->typeSprites[i].hitboxSize.x = 10.0f;
                mgr->typeSprites[i].hitboxSize.y = 10.0f;
                mgr->typeSprites[i].unk_b8a = 2;
            }
        }
        else
        {
            mgr->typeSprites[i].unk_b8a = 0;
            mgr->typeSprites[i].hitboxSize.x = 24.0f;
            mgr->typeSprites[i].hitboxSize.y = 24.0f;
        }
    }
    memset(&g_ItemManager, 0, sizeof(ItemManager));
    return 0;
}

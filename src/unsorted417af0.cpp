// The enemy script instructions' helpers, from 0x417af0: an object of
// its own in the original, which starts again with header inlines
// (SinCos) after EnemyScripts::RunEnemy. Split from enemyscript.cpp.

#include <math.h>
#include <string.h>

#include "ascii.h"
#include "bullet.h"
#include "effect.h"
#include "enemy.h"
#include "item.h"
#include "player.h"
#include "screeneffect.h"
#include "unsorted.h"

// As in bullet.cpp, though defined ahead of its callers here: it is
// still too large to expand (its Timer::Set calls are inline too), so
// they call the copy and keep a slot for it. The original keeps this
// file's copy, at 0x417b20.
inline void Bullet::FreeBullet()
{
    this->state = 0;
    this->timer.Set(0);
    this->grazeDelayTimer.Set(0);
}

void WarpToBoss(Enemy *enemy, EnemyScriptInstr *instr)
{
    int boss;

    boss = instr->args[1].i;
    enemy->pos = g_EnemyManager.bosses[boss]->pos;
    enemy->velocity = g_EnemyManager.bosses[boss]->velocity;
    enemy->angle = g_EnemyManager.bosses[boss]->angle;
    enemy->flags.isWarpingToBoss = 1;
}

void ShakeAndSpinBullets(Enemy *enemy, EnemyScriptInstr *instr)
{
    int i;
    Bullet *curBullet;
    float angle;

    curBullet = g_BulletManager.bullets;
    ScreenEffect::StartEffect(1, 0x1e, 0xc, 0, 0);
    ScreenEffect::StartEffect(3, 4, 3, 0x80ffcfcf, 0);
    for (i = 0; i < 0x400; i++, curBullet++)
    {
        if (curBullet->state == 0 || curBullet->state == 5)
        {
            continue;
        }
        if (curBullet->sprites.sprite0.spriteData != NULL && curBullet->scriptFuncState == 0)
        {
            if (instr->args[1].i == 1 && curBullet->color != 8)
            {
                continue;
            }
            if (instr->args[1].i == 2 && curBullet->color != 4)
            {
                continue;
            }
            if (curBullet->color == 2)
            {
                angle = -3.1415927f / (g_Rng.GetRandomF32ZeroToOne() * 60.0f + 180.0f);
            }
            else if (curBullet->color == 6)
            {
                angle = 3.1415927f / (g_Rng.GetRandomF32ZeroToOne() * 60.0f + 180.0f);
            }
            else if (curBullet->color == 8)
            {
                angle = 3.1415927f / (g_Rng.GetRandomF32ZeroToOne() * 60.0f + 180.0f);
            }
            else if (curBullet->color == 4)
            {
                angle = -3.1415927f / (g_Rng.GetRandomF32ZeroToOne() * 60.0f + 180.0f);
            }
            curBullet->speed = 0.3f;
            memset(curBullet->commands, 0, sizeof(curBullet->commands));
            if (g_GameManager.difficulty < 3)
            {
                curBullet->SetTurnCommand(0, 0, 60, angle, 1.0f / 60.0f);
            }
            else
            {
                curBullet->SetTurnCommand(0, 0, 240, angle, 1.0f / 190.0f);
            }
            curBullet->scriptFuncState = 1;
        }
    }
}

void SpawnFansFromNearbyBullets(Enemy *enemy, EnemyScriptInstr *instr)
{
    int variant;
    Bullet *bullet = g_BulletManager.bullets;
    float dist;
    float range;
    int i;
    BulletShooter shooter;

    variant = instr->args[1].i;
    switch (variant)
    {
    case 0:
        ScreenEffect::StartEffect(1, 0x20, 0xc, 0, 0);
        ScreenEffect::StartEffect(3, 4, 1, 0x80cfcfff, 0);
        range = 128.0f;
        break;
    case 1:
        range = 192.0f;
        break;
    case 2:
        range = 256.0f;
        break;
    case 3:
        range = 999.0f;
        break;
    }
    for (i = 0; i < 0x400; i++, bullet++)
    {
        if (bullet->state == 0 || bullet->state == 5)
        {
            continue;
        }
        if (bullet->sprites.sprite0.spriteData != NULL && bullet->color == 2)
        {
            dist = sqrtf((enemy->pos.x - bullet->pos.x) * (enemy->pos.x - bullet->pos.x) +
                         (enemy->pos.y - bullet->pos.y) * (enemy->pos.y - bullet->pos.y));
            if (dist < range)
            {
                shooter.pos = bullet->pos;
                shooter.sprite = 0;
                shooter.color = 6;
                shooter.angle1 = 0.0f;
                shooter.angle2 = -3.1415927f;
                shooter.speed1 = 0.7f;
                shooter.count1 = 2;
                shooter.count2 = 1;
                shooter.flags = 2;
                shooter.aimMode = 6;
                shooter.SetShooterAccelerateCommand(0, 0, 180, g_Rng.GetRandomF32InRange(0.005f) + 0.013f, 1.5707964f);
                g_BulletManager.FirePattern(&shooter);
                bullet->FreeBullet();
            }
        }
    }
}

void NoOp(Enemy *enemy, EnemyScriptInstr *instr)
{
}

void ReplaceTallBulletWithEffect(Enemy *enemy, EnemyScriptInstr *instr)
{
    int variant;
    Bullet *bullet = g_BulletManager.bullets;
    int i;
    BulletShooter shooter;

    variant = instr->args[1].i;
    enemy->ctx.vars.floats[0] = -999.0f;
    for (i = 0; i < 0x400; i++, bullet++)
    {
        if (bullet->state == 0 || bullet->state == 5)
        {
            continue;
        }
        if (bullet->sprites.sprite0.spriteData->height >= 60.0f)
        {
            enemy->ctx.vars.floats[0] = bullet->pos.x;
            enemy->ctx.vars.floats[1] = bullet->pos.y;
            g_EffectManager.SpawnEffect(2, &bullet->pos, 1, 0xffffffff);
            bullet->FreeBullet();
            break;
        }
    }
}

void CopyBossMoveStart(Enemy *enemy, EnemyScriptInstr *instr)
{
    Enemy *boss;

    boss = g_EnemyManager.bosses[0];
    enemy->moveStart = boss->pos;
    enemy->orbitRadius = boss->orbitRadius;
    enemy->orbitAngularVelocity = boss->orbitAngularVelocity;
    switch (boss->ctx.vars.ints2[0])
    {
    }
}

void FireBackwardFans(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    int i;
    BulletShooter shooter;

    for (i = 0; i < 0x400; i++, bullet++)
    {
        if (bullet->state == 0 || bullet->state == 5)
        {
            continue;
        }
        if (bullet->sprites.sprite0.spriteData != NULL &&
            ((instr->args[1].i == 0 && bullet->color == 6) || (instr->args[1].i == 1 && bullet->color == 15) ||
             (instr->args[1].i == 2 && bullet->color == 2)))
        {
            shooter.pos = bullet->pos;
            shooter.sprite = 6;
            shooter.color = 15;
            shooter.angle1 = AddNormalizeAngle(bullet->angle, 3.1415927f);
            shooter.angle2 = 0.5235988f;
            shooter.speed1 = bullet->speed * 1.1f;
            if (g_GameManager.difficulty < 3)
            {
                shooter.count1 = 4;
            }
            else
            {
                shooter.count1 = 2;
                shooter.angle2 = 1.5707964f;
            }
            shooter.count2 = 1;
            shooter.flags = 2;
            shooter.SetHiddenCommand(0, 0, 0x82);
            if (instr->args[1].i == 0)
            {
                shooter.flags = 0x2002;
            }
            else if (instr->args[1].i == 1)
            {
                if (g_GameManager.difficulty != 3)
                {
                    shooter.flags = 2;
                }
                else
                {
                    shooter.flags = 0x2002;
                }
                shooter.color = 2;
            }
            else if (instr->args[1].i == 2)
            {
                shooter.flags = 2;
                shooter.color = 10;
            }
            shooter.aimMode = 1;
            g_BulletManager.FirePattern(&shooter);
            shooter.angle2 = 1.0471976f;
            if (instr->args[1].i == 0)
            {
                shooter.flags = 0x2000;
            }
            else if (instr->args[1].i == 1)
            {
                if (g_GameManager.difficulty != 3)
                {
                    shooter.flags = 0;
                }
                else
                {
                    shooter.flags = 0x2000;
                }
            }
            else
            {
                shooter.flags = 0;
            }
            shooter.speed1 = bullet->speed * 0.7f;
            shooter.count1 = 2;
            g_BulletManager.FirePattern(&shooter);
            shooter.speed1 = bullet->speed * 0.85f;
            shooter.count1 = 1;
            g_BulletManager.FirePattern(&shooter);
            bullet->FreeBullet();
        }
    }
}

int PointInRotatedBox(D3DXVECTOR3 *point, D3DXVECTOR3 *center, D3DXVECTOR3 *size, D3DXVECTOR3 *origin,
               float sinAngle, float cosAngle)
{
    D3DXVECTOR3 low;
    D3DXVECTOR3 hi;
    D3DXVECTOR3 rotated;

    low = *point - *origin;
    hi.x = low.x * cosAngle + low.y * sinAngle;
    hi.y = low.y * cosAngle - low.x * sinAngle;
    rotated = hi + *origin;
    low = *center - *size / 2.0f;
    hi = *size / 2.0f + *center;
    if (rotated.x > hi.x || rotated.x < low.x || rotated.y > hi.y || rotated.y < low.y)
    {
        return 0;
    }
    return 1;
}

void UpdateLaserEdgeBullets(Enemy *enemy, EnemyScriptInstr *instr)
{
    D3DXVECTOR3 middle;
    float c;
    int li;
    Laser *l;
    Bullet *blt;
    D3DXVECTOR3 size;
    float proj;
    float s;
    int n;

    l = g_BulletManager.lasers;
    for (li = 0; li < 0x40; li++, l++)
    {
        if (l->active == 0)
        {
            continue;
        }
        if (enemy->callbackTimer.current % 2 != li)
        {
            continue;
        }
        if (l->state < 2)
        {
            size.y = l->width;
            size.x = l->endOffset - l->startOffset;
            middle.x = (l->endOffset - l->startOffset) / 2.0f + l->startOffset + l->pos.x;
            middle.y = l->pos.y;
            SinCos(&s, &c, l->angle);
            blt = g_BulletManager.bullets;
            for (n = 0; n < 0x400; n++, blt++)
            {
                if (blt->state == 0 || blt->state == 5)
                {
                    continue;
                }
                if (blt->sprites.sprite0.spriteData != NULL && PointInRotatedBox(&blt->pos, &middle, &size, &l->pos, s, c))
                {
                    if (blt->scriptFuncState > 0)
                    {
                        blt->scriptFuncState--;
                    }
                    if (blt->scriptFuncState == 0)
                    {
                        if (blt->speed > 0.5f)
                        {
                            blt->speed -= 0.1f;
                        }
                        proj = c * blt->velocity.y + s * blt->velocity.x;
                        if (proj >= 0.0f)
                        {
                            blt->angle = AddNormalizeAngle(l->angle, 1.5707964f);
                        }
                        else
                        {
                            blt->angle = AddNormalizeAngle(l->angle, -1.5707964f);
                        }
                        ((Vector2 *)&blt->velocity)->SetPolar(blt->angle, g_App.gameSpeed * blt->speed);
                        blt->scriptFuncState = 10;
                        blt->sprites = g_BulletManager.typeSprites[5];
                        g_SpriteRenderer->SetSprite(&blt->sprites.sprite0, blt->sprites.sprite0.spriteIndex + blt->color);
                    }
                }
            }
        }
    }
}

void DeflectBulletsOffLasers(Enemy *enemy, EnemyScriptInstr *instr)
{
    D3DXVECTOR3 middle;
    float c;
    int li;
    Laser *l;
    float nx;
    Bullet *blt;
    D3DXVECTOR3 size;
    float proj;
    float s;
    int n;
    float ny;

    l = g_BulletManager.lasers;
    for (li = 0; li < 0x40; li++, l++)
    {
        if (l->active == 0)
        {
            continue;
        }
        if (enemy->callbackTimer % 3 != li % 3)
        {
            continue;
        }
        if (l->state < 2)
        {
            size.y = l->width * 1.5f;
            size.x = l->endOffset - l->startOffset;
            middle.x = (l->endOffset - l->startOffset) / 2.0f + l->startOffset + l->pos.x;
            middle.y = l->pos.y;
            SinCos(&s, &c, l->angle);
            nx = -s;
            ny = c;
            blt = g_BulletManager.bullets;
            for (n = 0; n < 0x400; n++, blt++)
            {
                if (blt->state == 0 || blt->state == 5)
                {
                    continue;
                }
                if (blt->sprites.sprite0.spriteData != NULL && blt->scriptFuncState != li + 1 && blt->scriptFuncState >= 0 &&
                    PointInRotatedBox(&blt->pos, &middle, &size, &l->pos, s, c))
                {
                    if (g_GameManager.difficulty < 2)
                    {
                        blt->speed *= g_Rng.GetRandomF32ZeroToOne() * 0.3f + 0.7f;
                    }
                    else
                    {
                        blt->speed *= g_Rng.GetRandomF32ZeroToOne() * 0.4f + 0.8f;
                    }
                    proj = nx * blt->velocity.x + ny * blt->velocity.y;
                    if (proj >= 0.0f)
                    {
                        blt->velocity.x = nx;
                        blt->velocity.y = ny;
                    }
                    else
                    {
                        blt->velocity.x = -nx;
                        blt->velocity.y = -ny;
                    }
                    blt->sprites = g_BulletManager.typeSprites[5];
                    g_SpriteRenderer->SetSprite(&blt->sprites.sprite0, blt->sprites.sprite0.spriteIndex + blt->color);
                    blt->angle = atan2f(blt->velocity.y, blt->velocity.x);
                    ((Vector2 *)&blt->velocity)->SetPolar(blt->angle, blt->speed);
                    if (g_GameManager.difficulty < 2)
                    {
                        blt->scriptFuncState = -1;
                    }
                    else
                    {
                        blt->scriptFuncState = li + 1;
                    }
                }
            }
        }
    }
}

void ShakeAndStartFallingPetals(Enemy *enemy, EnemyScriptInstr *instr)
{
    ScreenEffect::StartEffect(1, 0x50, 8, 0, 0);
    StartPetalsFalling();
}

void StartSlowdown(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet;
    int i;

    g_App.gameSpeed = 1.0f / instr->args[1].i;
    g_Stage.dialogueSprites[0].pendingInterrupt = 2;
    g_Stage.dialogueSprites[1].pendingInterrupt = 2;
    bullet = g_BulletManager.bullets;
    for (i = 0; i < 0x400; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        bullet->velocity *= g_App.gameSpeed;
        bullet->sprites.sprite0.baseSpriteIndex = bullet->sprites.sprite0.spriteIndex;
        if (bullet->sprites.sprite0.spriteIndex >= 0x260 && bullet->sprites.sprite0.spriteIndex <= 0x26f)
        {
            g_SpriteRenderer->SetSprite(&bullet->sprites.sprite0, 0x26f);
        }
    }
}

void EndSlowdown(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    float scale = 1.0f / g_App.gameSpeed;
    int i;

    for (i = 0; i < 0x400; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        bullet->velocity *= scale;
        if (bullet->sprites.sprite0.spriteIndex >= 0x260 && bullet->sprites.sprite0.spriteIndex <= 0x26f)
        {
            g_SpriteRenderer->SetSprite(&bullet->sprites.sprite0, bullet->sprites.sprite0.baseSpriteIndex);
        }
    }
    g_App.gameSpeed = 1.0f / instr->args[1].i;
    if (g_App.gameSpeed < 1.0f)
    {
        g_App.stateFlags |= 0x20;
    }
    g_App.gameSpeed = 1.0f;
    g_Stage.dialogueSprites[0].pendingInterrupt = 1;
    g_Stage.dialogueSprites[1].pendingInterrupt = 1;
}

void BurstBulletsByDifficulty(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    int i;
    int count;
    BulletShooter shooter;
    int j;

    ScreenEffect::StartEffect(3, 8, 1, 0x50cfcfff, 0);
    count = g_GameManager.difficulty == 0   ? 10
            : g_GameManager.difficulty == 1 ? 18
            : g_GameManager.difficulty == 2 ? 22
                                              : 25;
    for (i = 0; i < 0x400; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if ((g_GameManager.difficulty < 2 && bullet->sprites.sprite0.spriteData->height > 48.0f &&
             enemy->pos.y - 64.0f < bullet->pos.y && enemy->pos.y + 64.0f > bullet->pos.y) ||
            (g_GameManager.difficulty >= 2 && bullet->sprites.sprite0.spriteData->height > 48.0f &&
             enemy->pos.y - 48.0f < bullet->pos.y && enemy->pos.y + 48.0f > bullet->pos.y))
        {
            for (j = 0; j < count; j++)
            {
                shooter.pos = bullet->pos;
                shooter.pos.x += g_Rng.GetRandomF32ZeroToOne() * 32.0f - 16.0f;
                shooter.pos.y += g_Rng.GetRandomF32ZeroToOne() * 32.0f - 16.0f;
                switch (g_Rng.GetRandomU16InRange(3))
                {
                case 0:
                    shooter.sprite = 0;
                    shooter.color = 2;
                    break;
                case 1:
                    shooter.sprite = 3;
                    shooter.color = 2;
                    break;
                case 2:
                    shooter.sprite = 7;
                    shooter.color = 1;
                    break;
                }
                if (instr->args[1].i == 0)
                {
                    shooter.angle1 = g_Rng.GetRandomF32ZeroToOne() * 4.712389f - 1.5707964f;
                }
                else
                {
                    shooter.angle1 = AddNormalizeAngle(g_Rng.GetRandomF32ZeroToOne() * 4.712389f, 0.7853982f);
                }
                shooter.speed1 = 0.1f;
                shooter.count1 = 1;
                shooter.count2 = 1;
                shooter.flags = (j & 1) ? 2 : 0;
                shooter.aimMode = 1;
                shooter.SetShooterTurnCommand(0, 0, 100, 0.0f, g_Rng.GetRandomF32ZeroToOne() * 0.008f + 0.01f);
                g_BulletManager.FirePattern(&shooter);
            }
            bullet->FreeBullet();
        }
    }
}

void TurnNearbyBullets(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    int i;

    for (i = 0; i < 0x400; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if (bullet->scriptFuncState == 0 && bullet->pos.y > enemy->pos.y && bullet->pos.y < 352.0f &&
            enemy->pos.x - 16.0f < bullet->pos.x && enemy->pos.x + 16.0f > bullet->pos.x)
        {
            bullet->SetTurnCommand(0, 0, 0xa0, (i & 1) ? 0.05235988f : -0.05235988f, -bullet->speed / 180.0f);
            bullet->scriptFuncState = 1;
        }
    }
}

void AimTurnedBulletsAtPlayer(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    int i;

    ScreenEffect::StartEffect(3, 0x10, 1, 0x50cfcfff, 0);
    for (i = 0; i < 0x400; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if (bullet->scriptFuncState == 1)
        {
            bullet->SetAccelerateCommand(0, 0, 0x5a, 0.026666667f, g_Player.AngleFrom(&bullet->pos));
            bullet->ClearCommandFlag(1);
            bullet->scriptFuncState = 2;
        }
    }
}

void FlashScreen(Enemy *enemy, EnemyScriptInstr *instr)
{
    ScreenEffect::StartEffect(3, instr->args[1].i, 1, 0xd0cfcfff, 0);
}

void FireBackFromUnturned(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    BulletShooter pattern;
    int i;

    for (i = 0; i < 0x400; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if (bullet->scriptFuncState == 0 && bullet->sprites.sprite0.spriteIndex >= 0x278 &&
            bullet->sprites.sprite0.spriteIndex <= 0x27f)
        {
            pattern.pos = bullet->pos;
            pattern.sprite = 0;
            pattern.color = 6;
            pattern.angle1 = AddNormalizeAngle(bullet->angle, 3.1415927f);
            pattern.angle2 = 0.3926991f;
            pattern.speed1 = enemy->ctx.vars.floats[1];
            pattern.count1 = 5;
            pattern.count2 = 1;
            pattern.flags = 2;
            pattern.aimMode = 1;
            g_BulletManager.FirePattern(&pattern);
        }
    }
}

void ReplaceBulletsWithEnemies(Enemy *enemy, EnemyScriptInstr *instr)
{
    EnemyScriptVars childVars;
    Bullet *blt = g_BulletManager.bullets;
    int i;
    float theta;

    childVars = enemy->ctx.vars;
    theta = -3.1415927f;

    ScreenEffect::StartEffect(3, 0xc, 1, 0x80cfcfff, 0);
    for (i = 0; i < 0x400; i++, blt++)
    {
        if (blt->state == 0)
        {
            continue;
        }
        if (blt->scriptFuncState == 0 && blt->sprites.sprite0.spriteIndex == 0x27c)
        {
            childVars.floats[0] = blt->angle;
            childVars.floats[7] = theta;
            theta += 0.7853982f;
            g_EnemyManager.SpawnWithVars(enemy->ctx.sub + 1, &blt->pos, 1, -2, 10, &childVars);
            blt->FreeBullet();
        }
        else if (blt->sprites.sprite0.spriteIndex >= 0x278 && blt->sprites.sprite0.spriteIndex <= 0x27f)
        {
            blt->FreeBullet();
        }
    }
}

void CountUnturnedBullets(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    int i;

    enemy->ctx.vars.ints[0] = 0;
    for (i = 0; i < 0x400; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if (bullet->scriptFuncState == 0 && bullet->sprites.sprite0.spriteIndex == 0x27c)
        {
            enemy->ctx.vars.ints[0]++;
        }
    }
}

void BurstBulletsFixedCount(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    int i;
    BulletShooter shooter;
    float w;
    int j;

    w = g_GameManager.difficulty == 2 ? 128.0f : 180.0f;
    ScreenEffect::StartEffect(3, 8, 1, 0x50cfcfff, 0);
    for (i = 0; i < 0x400; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if (bullet->sprites.sprite0.spriteData->height > 48.0f && enemy->pos.y - w < bullet->pos.y &&
            w + enemy->pos.y > bullet->pos.y)
        {
            for (j = 0; j < 15; j++)
            {
                shooter.pos = bullet->pos;
                shooter.pos.x += g_Rng.GetRandomF32ZeroToOne() * 32.0f - 16.0f;
                shooter.pos.y += g_Rng.GetRandomF32ZeroToOne() * 32.0f - 16.0f;
                switch (g_Rng.GetRandomU16InRange(3))
                {
                case 0:
                    shooter.sprite = 0;
                    shooter.color = 4;
                    break;
                case 1:
                    shooter.sprite = 3;
                    shooter.color = 4;
                    break;
                case 2:
                    shooter.sprite = 7;
                    shooter.color = 2;
                    break;
                }
                if (instr->args[1].i == 0)
                {
                    shooter.angle1 = g_Rng.GetRandomF32ZeroToOne() * 4.712389f - 1.5707964f;
                }
                else
                {
                    shooter.angle1 = AddNormalizeAngle(g_Rng.GetRandomF32ZeroToOne() * 4.712389f, 0.7853982f);
                }
                shooter.speed1 = 0.1f;
                shooter.count1 = 1;
                shooter.count2 = 1;
                shooter.flags = 0;
                shooter.aimMode = 1;
                shooter.SetShooterTurnCommand(0, 0, 100, 0.0f, g_Rng.GetRandomF32ZeroToOne() * 0.008f + 0.01f);
                g_BulletManager.FirePattern(&shooter);
            }
            bullet->FreeBullet();
        }
    }
}

void ScriptFadeOutMusic(Enemy *enemy, EnemyScriptInstr *instr)
{
    g_App.FadeOutMusic(3.0f);
}

void ScriptPlayMusic(Enemy *enemy, EnemyScriptInstr *instr)
{
    if (g_App.PlayMidi(2))
    {
        g_App.PlayMusic("bgm/th07_13b.mid");
    }
}

void FireAimedBulletPattern1(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    BulletShooter pattern;
    int variant;
    int i;

    if (enemy->callbackTimer.current % 3 == 0)
    {
        return;
    }
    variant = enemy->callbackTimer.current % 2;
    for (i = 0; i < 0x400; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if ((bullet->activeFlags & 0x40) == 0 && bullet->pos.y < 320.0f &&
            bullet->sprites.sprite0.spriteData->height > 60.0f)
        {
            pattern.pos = bullet->pos;
            if (variant != 0)
            {
                pattern.sprite = 1;
            }
            else
            {
                pattern.sprite = 3;
            }
            pattern.color = bullet->color == 1 ? 6 : 2;
            pattern.angle1 = g_Rng.GetRandomF32ZeroToOne() * 6.2831855f - 3.1415927f;
            pattern.angle2 = -3.1415927f;
            if (variant != 0)
            {
                pattern.speed1 = 1.2f;
            }
            else
            {
                pattern.speed1 = 0.8f;
            }
            if (variant != 0)
            {
                pattern.count1 = 1;
            }
            else
            {
                pattern.count1 = 2;
            }
            pattern.count2 = 1;
            pattern.flags = 0x208;
            pattern.aimMode = 3;
            pattern.bulletSound = 0x19;
            if (variant != 0)
            {
                pattern.SetStopTurnCommand(0, 0, 0x3c, 1, 0.0f, 3.1f);
            }
            g_BulletManager.FirePattern(&pattern);
        }
    }
}

void FireAimedBulletPattern2(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    BulletShooter pattern;
    int variant;
    int i;

    if (enemy->callbackTimer.current % 3 == 2)
    {
        return;
    }
    variant = enemy->callbackTimer.current % 3;
    for (i = 0; i < 0x400; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if ((bullet->activeFlags & 0x40) == 0 && bullet->pos.y < 320.0f &&
            bullet->sprites.sprite0.spriteData->height > 60.0f)
        {
            pattern.pos = bullet->pos;
            if (variant != 0)
            {
                pattern.sprite = 1;
            }
            else
            {
                pattern.sprite = 3;
            }
            pattern.color = bullet->color == 2 ? 10 : 13;
            pattern.angle1 = g_Rng.GetRandomF32ZeroToOne() * 6.2831855f - 3.1415927f;
            pattern.angle2 = -3.1415927f;
            if (variant != 0)
            {
                pattern.speed1 = 1.2f;
            }
            else
            {
                pattern.speed1 = 0.8f;
            }
            if (variant != 0)
            {
                pattern.count1 = 1;
            }
            else
            {
                pattern.count1 = 1;
            }
            pattern.count2 = 1;
            pattern.flags = 0x208;
            pattern.aimMode = 3;
            pattern.bulletSound = 0x19;
            if (variant != 0)
            {
                pattern.SetStopTurnCommand(0, 0, 0x28, 1, 0.0f, 2.9f);
            }
            g_BulletManager.FirePattern(&pattern);
        }
    }
}

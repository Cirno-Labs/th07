// The file with EnemyManager, from 0x41e920, as th06's enemy.cpp.

#include <string.h>

#include "ascii.h"
#include "bullet.h"
#include "enemy.h"
#include "item.h"
#include "player.h"
#include "replay.h"
#include "unsorted.h"

void MoveEnemyEffects(Enemy *enemy)
{
    Effect *eff;
    int i;

    for (i = 0; i < enemy->effectCount; i++)
    {
        eff = enemy->effects[i];
        if (eff == NULL)
        {
            continue;
        }
        eff->sprite.visible = !enemy->flags.hideFollowEffects;
        eff->origin = enemy->pos;
        if (eff->radius < enemy->effectRadius)
        {
            eff->radius += 0.3f;
        }
        eff->angle = AddNormalizeAngle(eff->angle, 0.031415928f);
    }
}

void ReleaseEnemyEffects(Enemy *enemy)
{
    int i;

    for (i = 0; i < enemy->effectCount; i++)
    {
        if (enemy->effects[i] == NULL)
        {
            continue;
        }
        enemy->effects[i]->fadeOut = 1;
        enemy->effects[i] = NULL;
    }
    enemy->effectCount = 0;
}

inline void Enemy::Move()
{
    this->posDelta = this->pos - this->prevPos;
    this->prevPos = this->pos;
    if (!this->flags.mirrored)
    {
        this->pos.x += g_App.gameSpeed * this->velocity.x;
    }
    else
    {
        this->pos.x -= g_App.gameSpeed * this->velocity.x;
    }
    this->pos.y += g_App.gameSpeed * this->velocity.y;
    this->pos.z += g_App.gameSpeed * this->velocity.z;
}

void EnemyManager::Initialize()
{
    int i;
    Enemy *enemy = this->enemies;

    memset(this, 0, sizeof(EnemyManager));
    enemy = &this->enemyTemplate;
    memset(enemy, 0, sizeof(Enemy));
    for (i = 0; i < 2; i++)
    {
        enemy->sprites[i].scriptIndex = -1;
    }
    for (i = 0; i < 0x60; i++)
    {
        enemy->trailBoxes[i].pos.x = -999.0f;
    }
    enemy->flags.active = 1;
    enemy->callbackTimer.Set(0);
    enemy->flags.onScreen = 0;
    enemy->hitboxSize = D3DXVECTOR3(12.0f, 12.0f, 12.0f);
    enemy->velocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    enemy->angularVelocity = 0.0f;
    enemy->angle = 0.0f;
    enemy->acceleration = 0.0f;
    enemy->speed = 0.0f;
    enemy->flags.moveMode = 0;
    enemy->flags.holdFire = 0;
    enemy->flags.mirrored = 0;
    enemy->flags.isBoss = 0;
    enemy->stackDepth = 0;
    enemy->life = 1;
    enemy->score = 100;
    enemy->deathEffect = 0;
    enemy->deathEffectIndex = 0;
    enemy->unk_2e16 = 0;
    enemy->shootInterval = 0;
    enemy->shootTimer.Set(0);
    enemy->shootOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    enemy->turnLeft = -1;
    enemy->turnRight = -1;
    enemy->turnStill = -1;
    enemy->flags.shootable = 1;
    enemy->flags.contactDamage = 1;
    enemy->flags.unk_a = 1;
    enemy->flags.hideFollowEffects = 0;
    enemy->flags.takesDamage = 1;
    enemy->flags.grazeable = 0;
    enemy->flags.deathMode = 0;
    enemy->deathSub = -1;
    enemy->flags.clampPos = 0;
    enemy->effectCount = 0;
    enemy->pendingInterrupt = -1;
    for (i = 0; i < 4; i++)
    {
        enemy->lifeCallbackLife[i] = -1;
    }
    enemy->timerCallbackTime = -1;
    enemy->periodicSubIndex = -1;
    enemy->laserIndex = 0;
    enemy->hitFlashTimer = 0;
    enemy->flags.rotateSprite = 0;
    enemy->rankSpeedLow = -0.15f;
    enemy->rankSpeedHigh = 0.15f;
    enemy->shooter.sound = 7;
    enemy->shooter.unk_cc = 0x19;
}

EnemyManager::EnemyManager()
{
    this->Initialize();
}

Enemy *EnemyManager::Spawn(int sub, D3DXVECTOR3 *pos, int life, int item, int score, BYTE mirrored)
{
    Enemy *enemy;
    int i;

    enemy = this->enemies;
    for (i = 0; i < 0x1e0; i++, enemy++)
    {
        if (enemy->flags.active)
        {
            continue;
        }
        *enemy = this->enemyTemplate;
        enemy->flags.mirrored = mirrored;
        if (life >= 0)
        {
            enemy->life = life;
        }
        enemy->pos = *pos;
        g_Unknown1347938.StartSub(&enemy->ctx, sub);
        if (g_Unknown1347938.RunEnemy(enemy) == -1)
        {
            enemy->flags.active = 0;
            break;
        }
        enemy->color = enemy->sprite.color;
        enemy->item = (char)item;
        if (score >= 0)
        {
            enemy->score = score;
        }
        enemy->maxLife = enemy->life;
        break;
    }
    return enemy;
}

Enemy *EnemyManager::SpawnWithVars(int sub, D3DXVECTOR3 *pos, int life, int item, int score,
                                 EnemyScriptVars *vars)
{
    Enemy *enemy;
    int i;

    enemy = this->enemies;
    for (i = 0; i < 0x1e0; i++, enemy++)
    {
        if (enemy->flags.active)
        {
            continue;
        }
        *enemy = this->enemyTemplate;
        if (life >= 0)
        {
            enemy->life = life;
        }
        enemy->pos = *pos;
        g_Unknown1347938.StartSub(&enemy->ctx, sub);
        enemy->ctx.vars = *vars;
        if (g_Unknown1347938.RunEnemy(enemy) == -1)
        {
            enemy->flags.active = 0;
            break;
        }
        enemy->color = enemy->sprite.color;
        enemy->item = (char)item;
        if (life >= 0)
        {
            enemy->life = life;
        }
        if (score >= 0)
        {
            enemy->score = score;
        }
        enemy->maxLife = enemy->life;
        break;
    }
    return enemy;
}

void EnemyTimeline::RunTimeline()
{
    Enemy *enemy;

    while (this->pos->time >= 0)
    {
        if (this->timer == this->pos->time)
        {
            switch (this->pos->opcode)
            {
            case 0:
                if (!g_Unknown49fbf0.IsBossPresent())
                {
                    EnemyTimelineArgs *args = &this->pos->args;
                    g_Unknown9a9b00.Spawn(this->pos->arg0, &args->pos, args->life, args->item, args->score, 0);
                }
                break;
            case 1:
                if (!g_Unknown49fbf0.IsBossPresent())
                {
                    g_Unknown9a9b00.Spawn(this->pos->arg0, &this->pos->args.pos, -1, -1, -1, 0);
                }
                break;
            case 2:
                if (!g_Unknown49fbf0.IsBossPresent())
                {
                    EnemyTimelineArgs *args = &this->pos->args;
                    enemy = g_Unknown9a9b00.Spawn(this->pos->arg0, &args->pos, args->life, args->item, args->score, 1);
                }
                break;
            case 3:
                if (!g_Unknown49fbf0.IsBossPresent())
                {
                    enemy = g_Unknown9a9b00.Spawn(this->pos->arg0, &this->pos->args.pos, -1, -1, -1, 1);
                }
                break;
            case 4:
                if (!g_Unknown49fbf0.IsBossPresent())
                {
                    EnemyTimelineArgs *args = &this->pos->args;
                    D3DXVECTOR3 pos;

                    pos = args->pos;
                    if (args->pos.x <= -990.0f)
                    {
                        pos.x = g_Rng.GetRandomF32InRange(g_Unknown626270.unk_9604[2]);
                    }
                    if (args->pos.y <= -990.0f)
                    {
                        pos.y = g_Rng.GetRandomF32InRange(g_Unknown626270.unk_9604[3]);
                    }
                    if (args->pos.z <= -990.0f)
                    {
                        pos.z = g_Rng.GetRandomF32InRange(800.0f);
                    }
                    g_Unknown9a9b00.Spawn(this->pos->arg0, &pos, args->life, args->item, args->score, 0);
                }
                break;
            case 5:
                if (!g_Unknown49fbf0.IsBossPresent())
                {
                    D3DXVECTOR3 pos;

                    pos = this->pos->args.pos;
                    if (pos.x <= -990.0f)
                    {
                        pos.x = g_Rng.GetRandomF32InRange(g_Unknown626270.unk_9604[2]);
                    }
                    if (pos.y <= -990.0f)
                    {
                        pos.y = g_Rng.GetRandomF32InRange(g_Unknown626270.unk_9604[3]);
                    }
                    if (pos.z <= -990.0f)
                    {
                        pos.z = g_Rng.GetRandomF32InRange(800.0f);
                    }
                    g_Unknown9a9b00.Spawn(this->pos->arg0, &pos, -1, -1, -1, 0);
                }
                break;
            case 6:
                if (!g_Unknown49fbf0.IsBossPresent())
                {
                    EnemyTimelineArgs *args = &this->pos->args;
                    D3DXVECTOR3 pos;

                    pos = args->pos;
                    if (args->pos.x <= -990.0f)
                    {
                        pos.x = g_Rng.GetRandomF32InRange(g_Unknown626270.unk_9604[2]);
                    }
                    if (args->pos.y <= -990.0f)
                    {
                        pos.y = g_Rng.GetRandomF32InRange(g_Unknown626270.unk_9604[3]);
                    }
                    if (args->pos.z <= -990.0f)
                    {
                        pos.z = g_Rng.GetRandomF32InRange(800.0f);
                    }
                    enemy = g_Unknown9a9b00.Spawn(this->pos->arg0, &pos, args->life, args->item, args->score, 0);
                    enemy->flags.mirrored = 1;
                }
                break;
            case 7:
                if (!g_Unknown49fbf0.IsBossPresent())
                {
                    D3DXVECTOR3 pos;

                    pos = this->pos->args.pos;
                    if (pos.x <= -990.0f)
                    {
                        pos.x = g_Rng.GetRandomF32InRange(g_Unknown626270.unk_9604[2]);
                    }
                    if (pos.y <= -990.0f)
                    {
                        pos.y = g_Rng.GetRandomF32InRange(g_Unknown626270.unk_9604[3]);
                    }
                    if (pos.z <= -990.0f)
                    {
                        pos.z = g_Rng.GetRandomF32InRange(800.0f);
                    }
                    enemy = g_Unknown9a9b00.Spawn(this->pos->arg0, &pos, -1, -1, -1, 0);
                    enemy->flags.mirrored = 1;
                }
                break;
            case 8:
                g_Unknown49fbf0.StartMsg(this->pos->arg0 + g_Unknown626270.character * 10);
                break;
            case 9:
                if (g_Unknown49fbf0.MsgHoldsTimeline())
                {
                    this->timer.Sub(1);
                    goto tick;
                }
                break;
            case 10:
                g_Unknown9a9b00.bosses[((int *)&this->pos->args)[0]]->pendingInterrupt = ((int *)&this->pos->args)[1];
                break;
            case 11:
                g_Unknown626270.SetPower(this->pos->arg0);
                g_Unknown626270.RerollChecksumKeys();
                break;
            case 12:
                if (g_Unknown9a9b00.bosses[this->pos->arg0] != NULL &&
                    g_Unknown9a9b00.bosses[this->pos->arg0]->flags.active)
                {
                    this->timer.Sub(1);
                    goto tick;
                }
                break;
            }
        }
        else if (this->timer < this->pos->time)
        {
            break;
        }
        this->pos = (EnemyTimelineEntry *)((char *)this->pos + this->pos->size);
    }
tick:
    this->timer.Tick();
}

int Enemy::CheckLifeCallback()
{
    Enemy *it = g_Unknown9a9b00.enemies;
    int i;
    int j;

    for (i = 0; i < 4; i++)
    {
        if (this->lifeCallbackLife[i] < 0)
        {
            continue;
        }
        if (this->life < this->lifeCallbackLife[i])
        {
            this->life = this->lifeCallbackLife[i];
            g_Unknown1347938.StartSub(&this->ctx, this->lifeCallbackSub[i]);
            this->lifeCallbackLife[i] = -1;
            this->timerCallbackTime = -1;
            this->periodicSubIndex = -1;
            this->rankSpeedLow = -0.5f;
            this->rankSpeedHigh = 0.5f;
            this->rankCount1Low = 0;
            this->rankCount1High = 0;
            this->rankCount2Low = 0;
            this->rankCount2High = 0;
            this->stackDepth = 0;
            this->shooter = g_Unknown9a9b00.enemyTemplate.shooter;
            this->shootInterval = 0;
            for (j = 0; j < 0x1e0; j++, it++)
            {
                if (!it->flags.active)
                {
                    continue;
                }
                if (it->flags.isBoss)
                {
                    continue;
                }
                it->life = 0;
                if (!it->flags.shootable && it->deathSub >= 0)
                {
                    g_Unknown1347938.StartSub(&it->ctx, it->deathSub);
                    it->deathSub = -1;
                }
            }
            return 1;
        }
    }
    return 0;
}

int Enemy::CheckTimerCallback()
{
    Enemy *enemy;
    int i;
    int j;
    int idx;
    int best;
    int lost;

    if (this->flags.isBoss && this->bossIndex == 0)
    {
        g_Unknown49fbf0.SetBossTimer((this->timerCallbackTime - this->callbackTimer.AsFrames()) / 60);
    }
    if (this->callbackTimer >= this->timerCallbackTime)
    {
        best = 0;
        for (i = 0; i < 4; i++)
        {
            if (this->lifeCallbackLife[i] < 0)
            {
                continue;
            }
            if (best < this->lifeCallbackLife[i])
            {
                best = this->lifeCallbackLife[i];
                idx = i;
            }
        }
        if (best > 0)
        {
            this->life = this->lifeCallbackLife[idx];
            this->lifeCallbackLife[idx] = -1;
        }
        g_Unknown1347938.StartSub(&this->ctx, this->timerCallbackSub);
        this->timerCallbackTime = -1;
        this->timerCallbackSub = this->deathSub;
        this->callbackTimer.Set(0);
        if (!this->flags.keepSpellCard)
        {
            g_Unknown12fe0cc = 0;
            g_Unknown12fe0c4 = 0;
            if (g_Unknown12fe0c8 != 0)
            {
                g_Unknown12fe0c8++;
            }
            g_BulletManager.ClearAll(10);
            lost = (g_Unknown626270.unk_961c - g_Unknown626270.stats->unk_88) * 0.25f;
            lost -= lost % 10;
            g_Unknown626270.unk_961c -= lost;
        }
        for (enemy = g_Unknown9a9b00.enemies, j = 0; j < 0x1e0; j++, enemy++)
        {
            if (!enemy->flags.active)
            {
                continue;
            }
            if (enemy->flags.isBoss)
            {
                continue;
            }
            enemy->life = 0;
            if (!enemy->flags.shootable && enemy->deathSub >= 0)
            {
                g_Unknown1347938.StartSub(&enemy->ctx, enemy->deathSub);
                enemy->deathSub = -1;
            }
        }
        this->periodicSubIndex = -1;
        this->shooter = g_Unknown9a9b00.enemyTemplate.shooter;
        this->shootInterval = 0;
        this->rankSpeedLow = -0.5f;
        this->rankSpeedHigh = 0.5f;
        this->rankCount1Low = 0;
        this->rankCount1High = 0;
        this->rankCount2Low = 0;
        this->rankCount2High = 0;
        this->stackDepth = 0;
        return 1;
    }
    else
    {
        return 0;
    }
}

void Enemy::Despawn()
{
    if (this->flags.deathMode == 0)
    {
        this->flags.active = 0;
    }
    else
    {
        this->flags.shootable = 0;
    }
    if (this->flags.isBoss && this->bossIndex < 4)
    {
        g_Unknown49fbf0.bossPresent = 0;
    }
    if (this->effectCount != 0)
    {
        ReleaseEnemyEffects(this);
    }
    if (this->flags.isBoss)
    {
        g_Unknown9a9b00.bosses[this->bossIndex] = NULL;
    }
    g_ReplayManager->unk_d6 |= 0x20;
}

void Enemy::ClampPos()
{
    if (this->flags.clampPos)
    {
        if (this->pos.x < this->lowerBound[0])
        {
            this->pos.x = this->lowerBound[0];
        }
        else if (this->pos.x > this->upperBound[0])
        {
            this->pos.x = this->upperBound[0];
        }
        if (this->pos.y < this->lowerBound[1])
        {
            this->pos.y = this->lowerBound[1];
        }
        else if (this->pos.y > this->upperBound[1])
        {
            this->pos.y = this->upperBound[1];
        }
    }
}

void Enemy::CheckPlayerContact(D3DXVECTOR3 *pos, D3DXVECTOR3 *size)
{
    D3DXVECTOR3 box;

    box = *size / 0.7f;
    if (this->flags.grazeable && this->callbackTimer.HasTicked() && this->callbackTimer.current % 6 == 0)
    {
        g_Player.CheckGraze(pos, &box);
    }
    box = *size / 1.5f;
    if (g_Player.CheckHit(pos, &box) == 1 && this->flags.shootable && !this->flags.isBoss &&
        !this->flags.grazeable)
    {
        this->life -= 10;
    }
}

float LerpAngle(float a, float b, float t)
{
    float diff;
    float wrap;

    if (a < b)
    {
        diff = b - a;
        wrap = a + 6.2831855f - b;
    }
    else
    {
        diff = a - b;
        wrap = b + 6.2831855f - a;
        a = b;
    }
    if (diff < wrap)
    {
        return diff * t + a;
    }
    else
    {
        return wrap * t + a;
    }
}

int EnemyManager::OnUpdate(EnemyManager *mgr)
{
    int bomb;
    int level;
    D3DXVECTOR3 toward;
    int hurt;
    int i;
    D3DXVECTOR3 boxSize;
    float angle2;
    int extraHurt;
    int hit;
    int seg;
    Enemy *enemy;
    D3DXVECTOR3 targetDiff;
    int points;

    bomb = 0;
    level = g_Unknown626270.stage >= 5 ? 10 : g_Unknown626270.stage * 2;
    if (!g_Unknown49fbf0.Func42ad66())
    {
        int period;

        period = 2400;
        period -= (int)g_Unknown626270.stats->lives * 4 * 60;
        if (mgr->unk_9546f4.HasTicked() && mgr->unk_9546f4.AsFrames() % period == 0)
        {
            g_Unknown626270.AddSubRank(100);
        }
        g_Unknown626270.progressFrames++;
    }
    for (i = 0; i < 4; i++)
    {
        mgr->unk_954700[i] = NULL;
    }
    for (i = 0; i < g_Unknown1347938.file->unk_2; i++)
    {
        if (mgr->unk_9545f4[i].pos == NULL)
        {
            mgr->unk_9545f4[i].pos = g_Unknown1347938.GetTimeline(i);
        }
        mgr->unk_9545f4[i].RunTimeline();
    }
    enemy = mgr->enemies;
    mgr->enemyCount = 0;
    for (i = 0; i < 0x1e0; i++, enemy++)
    {
        if (!enemy->flags.active)
        {
            continue;
        }
        mgr->enemyCount++;
        if (enemy->flags.haltedByBomb && (g_Player.bomb.running || g_Player.playerState))
        {
            enemy->callbackTimer.Sub(1);
            goto flash;
        }
    again:
        if (g_Unknown1347938.RunEnemy(enemy) == -1)
        {
            enemy->flags.active = 0;
            enemy->Despawn();
            continue;
        }
        if (!enemy->flags.isWarpingToBoss)
        {
            enemy->ClampPos();
            enemy->Move();
            enemy->ClampPos();
            if (enemy->effects[24] != NULL && !enemy->flags.unk_19)
            {
                enemy->effects[24]->pos = (enemy->pos - enemy->effects[24]->pos) / 16.0f + enemy->effects[24]->pos;
            }
        }
        if (enemy->trailFlags)
        {
            for (seg = enemy->trailSegmentCount - 1; seg > 0; seg--)
            {
                enemy->trailBoxes[seg].pos = enemy->trailBoxes[seg - 1].pos;
                enemy->trailBoxes[seg].velocity = enemy->trailBoxes[seg - 1].velocity;
                enemy->trailBoxes[seg].angle = enemy->trailBoxes[seg - 1].angle;
            }
            enemy->trailBoxes[0].pos = enemy->pos;
            enemy->trailBoxes[0].velocity = enemy->velocity;
            enemy->trailBoxes[0].angle = enemy->angle;
        }
        if (enemy->sprite.spriteData == NULL)
        {
            enemy->flags.hideFollowEffects = 1;
        }
        if (!enemy->flags.hideFollowEffects && !enemy->flags.onScreen &&
            g_Unknown626270.IsInPlayArea(enemy->pos.x, enemy->pos.y, enemy->sprite.spriteData->width,
                                         enemy->sprite.spriteData->height))
        {
            enemy->flags.onScreen = 1;
        }
        if (enemy->flags.onScreen == 1 &&
            ((!enemy->trailFlags &&
              !g_Unknown626270.IsInPlayArea(enemy->pos.x, enemy->pos.y, enemy->sprite.spriteData->width,
                                            enemy->sprite.spriteData->height)) ||
             (enemy->trailFlags &&
              !g_Unknown626270.IsInPlayArea(enemy->pos.x, enemy->pos.y, enemy->sprite.spriteData->width,
                                            enemy->sprite.spriteData->height) &&
              !g_Unknown626270.IsInPlayArea(enemy->trailBoxes[enemy->trailSegmentCount - 1].pos.x,
                                            enemy->trailBoxes[enemy->trailSegmentCount - 1].pos.y,
                                            enemy->sprite.spriteData->width,
                                            enemy->sprite.spriteData->height))) &&
            !enemy->flags.staysOffscreen)
        {
            enemy->flags.active = 0;
            enemy->Despawn();
            continue;
        }
        if (enemy->CheckLifeCallback())
        {
            goto again;
        }
        if (enemy->timerCallbackTime >= 0 && enemy->CheckTimerCallback())
        {
            goto again;
        }
        enemy->sprite.color = enemy->color;
        g_SpriteEngine->Step(&enemy->sprite);
        enemy->color = enemy->sprite.color;
        for (seg = 0; seg < 2; seg++)
        {
            if (enemy->sprites[seg].scriptIndex >= 0 && g_SpriteEngine->Step(&enemy->sprites[seg]))
            {
                enemy->sprites[seg].scriptIndex = -1;
            }
        }
        bomb = 0;
        hit = 0;
        if (!enemy->flags.hideFollowEffects && !enemy->flags.unk_1a)
        {
            if (enemy->flags.shootable && enemy->flags.contactDamage)
            {
                enemy->CheckPlayerContact(&enemy->pos, &enemy->hitboxSize);
                if (enemy->trailFlags)
                {
                    boxSize = enemy->hitboxSize;
                    for (seg = 1; seg < enemy->trailContactSegments; seg += 6)
                    {
                        if (enemy->trailFlags & 2)
                        {
                            boxSize = enemy->hitboxSize - enemy->hitboxSize * (float)seg / (float)enemy->trailContactSegments;
                        }
                        enemy->CheckPlayerContact(&enemy->trailBoxes[seg].pos, &boxSize);
                    }
                }
            }
            enemy->lastDamage = 0;
            if (enemy->flags.shootable && enemy->flags.takesDamage)
            {
                hurt = g_Player.CalcDamage(&enemy->pos, &enemy->hitboxSize, &bomb);
                if (enemy->extraHitboxSize.x > 0.0f)
                {
                    extraHurt = g_Player.CalcDamage(&enemy->pos, &enemy->extraHitboxSize, &bomb);
                    if (bomb == 0)
                    {
                        hurt = hurt + extraHurt / 2.5f;
                    }
                }
                if (hurt > 0)
                {
                    if ((enemy->flags.isBoss || !g_Player.focused) && !g_Player.bomb.running)
                    {
                        if (enemy->flags.isBoss && !g_Player.focused)
                        {
                            points = hurt / (10 - level / 3) * 10;
                        }
                        else
                        {
                            points = hurt / (30 - level) * 10;
                        }
                        if (points > 70)
                        {
                            points = 70;
                        }
                        if (points == 0 && (!g_Player.focused || enemy->callbackTimer.AsFrames() & 1))
                        {
                            points = 10;
                        }
                        switch (g_Unknown626270.unk_93d7)
                        {
                        case 0:
                            if ((points == 20 || points == 30) && enemy->callbackTimer.AsFrames() & 1)
                            {
                                points -= 10;
                            }
                            if (g_Unknown626270.stage >= 5 && g_Unknown626270.stage <= 6 &&
                                !enemy->flags.isBoss)
                            {
                                hurt /= 2;
                            }
                            if (g_Unknown626270.stage == 4 && !enemy->flags.isBoss)
                            {
                                hurt -= hurt / 4 + hurt / 16;
                            }
                        }
                        if (points != 0)
                        {
                            g_Unknown626270.AddBonusPoints(points);
                        }
                    }
                    if (hurt >= 70)
                    {
                        hurt = 70;
                    }
                    g_Unknown626270.AddScore(hurt / 5 * 10);
                    if (enemy->flags.unk_a)
                    {
                        if (mgr->spellCardActive)
                        {
                            if (bomb == 0)
                            {
                                if (hurt > 7)
                                {
                                    hurt /= 7;
                                }
                                else if (hurt != 0)
                                {
                                    hurt = 1;
                                }
                            }
                            else if (mgr->unk_9545dc)
                            {
                                if (hurt > 2)
                                {
                                    hurt = hurt / 2.5f;
                                }
                                else if (hurt != 0)
                                {
                                    hurt = 1;
                                }
                            }
                            else
                            {
                                hurt = 0;
                            }
                        }
                        if (enemy->invulnTimer > 0)
                        {
                            if (enemy->flags.isBoss)
                            {
                                hurt /= 9;
                            }
                            else
                            {
                                hurt = 0;
                            }
                        }
                        enemy->life -= hurt;
                        enemy->lastDamage = hurt;
                    }
                    hit = 1;
                }
                if (enemy->flags.isBoss)
                {
                    targetDiff = g_Player.unk_2428 - g_Player.position;
                    toward = enemy->pos - g_Player.position;
                    if (!g_Player.unk_2440 || fabsf(targetDiff.x) > fabsf(toward.x))
                    {
                        g_Player.unk_2428 = enemy->pos;
                    }
                    if (g_Unknown626270.character == 2)
                    {
                        targetDiff = g_Player.homingTarget - g_Player.position;
                        angle2 = atan2f(enemy->pos.y - g_Player.position.y, enemy->pos.x - g_Player.position.x);
                        if (angle2 >= -2.0943952f && angle2 <= -1.0471976f &&
                            (!g_Player.unk_2440 || fabsf(targetDiff.x) > fabsf(toward.x)))
                        {
                            g_Player.homingTarget = enemy->pos;
                            g_Player.unk_2440 = 1;
                        }
                    }
                    else
                    {
                        g_Player.unk_2440 = 1;
                    }
                }
                if (!g_Player.unk_2440)
                {
                    if (g_Player.unk_2428.y < enemy->pos.y)
                    {
                        g_Player.unk_2428 = enemy->pos;
                    }
                    if (g_Unknown626270.character == 2 && g_Player.homingTarget.y < -900.0f)
                    {
                        angle2 = atan2f(enemy->pos.y - g_Player.position.y, enemy->pos.x - g_Player.position.x);
                        if (angle2 >= -2.0943952f && angle2 <= -1.0471976f)
                        {
                            g_Player.homingTarget = enemy->pos;
                        }
                    }
                }
            }
        }
        if (enemy->life <= 0 && enemy->flags.shootable)
        {
            int k = 0;

            for (k = 0; k < 4; k++)
            {
                enemy->lifeCallbackLife[k] = -1;
            }
            enemy->timerCallbackTime = -1;
            enemy->periodicSubIndex = -1;
            switch (enemy->flags.deathMode)
            {
            case 3:
                enemy->life = 1;
                enemy->flags.unk_a = 0;
                enemy->flags.deathMode = 0;
                g_Unknown49fbf0.bossPresent = 0;
                g_ReplayManager->unk_d6 |= 0x20;
                if (enemy->deathEffect >= 0)
                {
                    g_EffectManager.SpawnEffect(enemy->deathEffect, &enemy->pos, 1, 0xffffffff);
                    g_EffectManager.SpawnEffect(enemy->deathEffect, &enemy->pos, 1, 0xffffffff);
                    g_EffectManager.SpawnEffect(enemy->deathEffect, &enemy->pos, 1, 0xffffffff);
                }
                break;
            case 1:
                g_Unknown626270.AddScore(enemy->score);
                enemy->flags.shootable = 0;
                goto dead;
            case 0:
                g_Unknown626270.AddScore(enemy->score);
                enemy->flags.active = 0;
            dead:
                if (enemy->flags.isBoss)
                {
                    g_Unknown49fbf0.bossPresent = 0;
                    ReleaseEnemyEffects(enemy);
                }
            case 2:
                if (enemy->item >= 0)
                {
                    g_EffectManager.SpawnEffect(enemy->deathEffectIndex + 4, &enemy->pos, 3, 0xffffffff);
                    g_ItemManager.SpawnItem(&enemy->pos, enemy->item, bomb);
                }
                else if (enemy->item == -1)
                {
                    if (mgr->dropCounter % 3 == 0)
                    {
                        g_EffectManager.SpawnEffect(enemy->deathEffectIndex + 4, &enemy->pos, 6, 0xffffffff);
                        g_ItemManager.SpawnItem(&enemy->pos, g_EnemyItemDrops[mgr->dropIndex], bomb);
                        mgr->dropIndex++;
                        if (mgr->dropIndex >= 0x20)
                        {
                            mgr->dropIndex = 0;
                        }
                    }
                    mgr->dropCounter++;
                }
                if (enemy->flags.isBoss && !g_Unknown12fe0c8)
                {
                    int bonus;

                    bonus = g_BulletManager.ClearAllWithBonus(8000, 1);
                    bonus = g_Unknown9a9b00.ClearEnemiesForBonus(8000, bonus);
                    if (bonus != 0)
                    {
                        g_Unknown626270.AddScore(bonus);
                        g_Unknown49fbf0.ShowBonusPopup(bonus);
                    }
                }
                enemy->life = 0;
                g_ReplayManager->unk_d6 |= 0x20;
                break;
            }
            g_SoundPlayer.QueueSound(i % 2 + 2, 0);
            if (enemy->deathEffect >= 0)
            {
                g_EffectManager.SpawnEffect(enemy->deathEffect, &enemy->pos, 1, 0xffffffff);
                g_EffectManager.SpawnEffect(enemy->deathEffectIndex + 4, &enemy->pos, 4, 0xffffffff);
            }
            if (enemy->deathSub >= 0)
            {
                int l;

                enemy->rankSpeedLow = -0.5f;
                enemy->rankSpeedHigh = 0.5f;
                enemy->rankCount1Low = 0;
                enemy->rankCount1High = 0;
                enemy->rankCount2Low = 0;
                enemy->rankCount2High = 0;
                enemy->stackDepth = 0;
                for (l = 0; l < 4; l++)
                {
                    enemy->lifeCallbackLife[l] = -1;
                }
                enemy->timerCallbackTime = -1;
                enemy->periodicSubIndex = -1;
                enemy->shooter = g_Unknown9a9b00.enemyTemplate.shooter;
                enemy->shootInterval = 0;
                g_Unknown1347938.StartSub(&enemy->ctx, enemy->deathSub);
                enemy->deathSub = -1;
            }
        }
    flash:
        if (enemy->hitFlashTimer)
        {
            enemy->hitFlashTimer--;
            enemy->sprite.flags &= ~0x10000;
        }
        else if (hit)
        {
            g_SoundPlayer.QueueSound(0x14, 0);
            ((BYTE *)&enemy->sprite.color2)[2] = 0xff;
            ((BYTE *)&enemy->sprite.color2)[1] = 0x80;
            ((BYTE *)&enemy->sprite.color2)[0] = 0xc0;
            ((BYTE *)&enemy->sprite.color2)[3] = enemy->sprite.alpha;
            enemy->sprite.flags |= 0x10000;
            enemy->hitFlashTimer = 1;
        }
        else
        {
            enemy->sprite.flags &= ~0x10000;
        }
        if (enemy->flags.isBoss)
        {
            if (!g_Unknown49fbf0.Func42ad66() && enemy->bossIndex == 0)
            {
                g_Unknown49fbf0.SetBossHealth(enemy->life / (float)enemy->maxLife);
            }
            if (enemy->flags.isBoss < 4)
            {
                D3DXVECTOR3 markerPos;

                if (!enemy->flags.hideFollowEffects)
                {
                    markerPos.x = enemy->pos.x + 32.0f;
                }
                else
                {
                    markerPos.x = -999.0f;
                }
                markerPos.y = 472.0f;
                markerPos.z = 0.0f;
                g_AsciiText.SetUnkB7cPos(enemy->bossIndex, &markerPos);
                g_AsciiText.SetUnk14ac(enemy->bossIndex, (enemy->sprite.flags >> 16) & 1);
            }
        }
        MoveEnemyEffects(enemy);
        if (!g_Unknown626270.isFrozen)
        {
            enemy->callbackTimer.Tick();
        }
        if (enemy->invulnTimer > 0)
        {
            enemy->invulnTimer.Sub(1);
        }
        if (!enemy->flags.hideFollowEffects && enemy->flags.active)
        {
            enemy->nextInDrawList = mgr->unk_954700[enemy->drawListIndex];
            mgr->unk_954700[enemy->drawListIndex] = enemy;
        }
    }
    if (mgr->unk_9546f4.current % 200 == 0 && g_Unknown626270.ChecksumMismatch())
    {
        return 4;
    }
    mgr->unk_9546f4.Tick();
    return 1;
}

int EnemyManager::DrawEnemies(EnemyManager *mgr, int from, int to)
{
    float uSpan2;
    float cs2;
    float theta;
    float uDecr;
    VertexTex1DiffuseXyzrwh *out;
    float lastTheta;
    float tu;
    float sinA;
    int vertices;
    float width;
    float height;
    Enemy *enemy;
    int b;
    Sprite *sprite;
    D3DCOLOR savedColor;
    int i;
    D3DXVECTOR2 savedScale;

    for (i = from; i < to; i++)
    {
        enemy = mgr->unk_954700[i];
        while (enemy != NULL)
        {
            for (sprite = enemy->sprites, b = 0; b < 1; b++, sprite++)
            {
                if (sprite->scriptIndex >= 0)
                {
                    if (sprite->autoRotate != 0)
                    {
                        sprite->SetRotationZ(enemy->angle);
                    }
                    sprite->pos = enemy->pos + sprite->posOffset;
                    sprite->pos.z = 0.3f;
                    sprite->pos.x = g_Unknown626270.arena[0] + sprite->pos.x;
                    sprite->pos.y = g_Unknown626270.arena[1] + sprite->pos.y;
                    g_SpriteEngine->Draw(sprite);
                }
            }
            if (enemy->flags.rotateSprite)
            {
                enemy->sprite.SetRotationZ(enemy->angle);
            }
            enemy->sprite.pos = enemy->pos + enemy->sprite.posOffset;
            enemy->sprite.pos.z = 0.29f;
            if (!(enemy->trailFlags & 0x10) && !enemy->flags.unk_1a)
            {
                enemy->sprite.pos.x = g_Unknown626270.arena[0] + enemy->sprite.pos.x;
                enemy->sprite.pos.y = g_Unknown626270.arena[1] + enemy->sprite.pos.y;
                g_SpriteEngine->Draw(&enemy->sprite);
            }
            for (b = 1; b < 2; b++, sprite++)
            {
                if (sprite->scriptIndex >= 0)
                {
                    if (sprite->autoRotate != 0)
                    {
                        sprite->SetRotationZ(-enemy->angle);
                    }
                    sprite->pos = enemy->pos + sprite->posOffset;
                    sprite->pos.z = 0.3f;
                    sprite->pos.x = g_Unknown626270.arena[0] + sprite->pos.x;
                    sprite->pos.y = g_Unknown626270.arena[1] + sprite->pos.y;
                    g_SpriteEngine->Draw(sprite);
                }
            }
            if (enemy->trailFlags)
            {
                savedScale = *(D3DXVECTOR2 *)&enemy->sprite.scaleY;
                savedColor = enemy->sprite.color;
                if (!(enemy->trailFlags & 8))
                {
                    for (b = enemy->trailSegmentStep; b < enemy->trailSegmentCount; b += enemy->trailSegmentStep)
                    {
                        if (enemy->trailBoxes[b].pos.x < -990.0f)
                        {
                            continue;
                        }
                        if (enemy->flags.rotateSprite)
                        {
                            enemy->sprite.SetRotationZ(enemy->trailBoxes[b].angle);
                        }
                        if (enemy->trailFlags & 2)
                        {
                            enemy->sprite.scaleY = savedScale.x - b * savedScale.x / enemy->trailSegmentCount;
                        }
                        if (enemy->trailFlags & 4)
                        {
                            enemy->sprite.alpha =
                                ((BYTE *)&savedColor)[3] - ((BYTE *)&savedColor)[3] * b / enemy->trailSegmentCount;
                        }
                        enemy->sprite.pos = enemy->trailBoxes[b].pos + enemy->sprite.posOffset;
                        enemy->sprite.pos.z = 0.3f;
                        enemy->sprite.pos.x = g_Unknown626270.arena[0] + enemy->sprite.pos.x;
                        enemy->sprite.pos.y = g_Unknown626270.arena[1] + enemy->sprite.pos.y;
                        g_SpriteEngine->Draw(&enemy->sprite);
                    }
                }
                else
                {
                    vertices = 0;
                    for (b = 0; b < enemy->trailSegmentCount; b += enemy->trailSegmentStep)
                    {
                        if (enemy->trailBoxes[b].pos.x < -990.0f)
                        {
                            break;
                        }
                        vertices += 2;
                    }
                    if (vertices > 2)
                    {
                        uSpan2 = enemy->sprite.spriteData->uvEndX - enemy->sprite.spriteData->uvStartX;
                        uDecr = uSpan2 / ((vertices + 1) / 2 - 1);
                        tu = enemy->sprite.spriteData->uvEndX + enemy->sprite.uvScrollX;
                        out = (VertexTex1DiffuseXyzrwh *)enemy->spriteStripVertices;
                        for (b = 0; b < enemy->trailSegmentCount; b += enemy->trailSegmentStep, tu -= uDecr)
                        {
                            if (enemy->trailBoxes[b].pos.x < -990.0f)
                            {
                                break;
                            }
                            if (b == 0)
                            {
                                theta = enemy->trailBoxes[0].angle;
                            }
                            else
                            {
                                theta = LerpAngle(enemy->trailBoxes[b - 1].angle, enemy->trailBoxes[b].angle, 0.5f);
                            }
                            if ((enemy->trailFlags & 2) && b > 0 && b + enemy->trailSegmentStep < enemy->trailSegmentCount)
                            {
                                sinA = LerpAngle(enemy->trailBoxes[b + enemy->trailSegmentStep - 1].angle,
                                                 enemy->trailBoxes[enemy->trailSegmentStep].angle, 0.5f);
                                if (fabsf(lastTheta - theta) < 0.00001f && fabsf(theta - sinA) < 0.00001f)
                                {
                                    vertices -= 2;
                                    continue;
                                }
                            }
                            lastTheta = theta;
                            sinA = sinf(theta);
                            cs2 = cosf(theta);
                            width = 0.0f;
                            height = savedScale.y * enemy->sprite.spriteData->height / 2.0f;
                            if (enemy->trailFlags & 2)
                            {
                                theta = 1.0f - (float)b / enemy->trailSegmentCount;
                                width *= theta;
                                height *= theta;
                            }
                            out[1].diffuse = enemy->sprite.color;
                            out[0].diffuse = out[1].diffuse;
                            if (enemy->trailFlags & 4)
                            {
                                ((BYTE *)&out[1].diffuse)[3] =
                                    ((BYTE *)&savedColor)[3] - ((BYTE *)&savedColor)[3] * b / enemy->trailSegmentCount;
                                ((BYTE *)&out[0].diffuse)[3] = ((BYTE *)&out[1].diffuse)[3];
                            }
                            out->pos = enemy->trailBoxes[b].pos;
                            out->pos.x = cs2 * width - sinA * height + 32.0f + out->pos.x;
                            out->pos.y = sinA * width + cs2 * height + 16.0f + out->pos.y;
                            out->uv.x = tu;
                            out->uv.y = enemy->sprite.spriteData->uvStartY + enemy->sprite.uvScrollY;
                            out++;
                            out->pos = enemy->trailBoxes[b].pos;
                            out->pos.x = cs2 * width + sinA * height + 32.0f + out->pos.x;
                            out->pos.y = sinA * width - cs2 * height + 16.0f + out->pos.y;
                            out->uv.x = tu;
                            out->uv.y = enemy->sprite.spriteData->uvEndY + enemy->sprite.uvScrollY;
                            out++;
                        }
                        if (vertices > 2)
                        {
                            g_SpriteEngine->DrawVertexStrip(&enemy->sprite,
                                                            (VertexTex1DiffuseXyzrwh *)enemy->spriteStripVertices, vertices);
                        }
                    }
                }
                *(D3DXVECTOR2 *)&enemy->sprite.scaleY = savedScale;
                enemy->sprite.color = savedColor;
            }
            enemy = enemy->nextInDrawList;
        }
    }
    return 1;
}

int EnemyManager::OnDraw(EnemyManager *mgr)
{
    return EnemyManager::DrawEnemies(mgr, 0, 2);
}

int EnemyManager::OnDraw2(EnemyManager *mgr)
{
    return EnemyManager::DrawEnemies(mgr, 2, 4);
}

int EnemyManager::AddedCallback(EnemyManager *mgr)
{
    Enemy *unused = mgr->enemies;

    if (mgr->anmPath != NULL && g_SpriteEngine->LoadAnm(0xf, mgr->anmPath, 0x900) != 0)
    {
        return -1;
    }
    if (mgr->anmPath2 != NULL && g_SpriteEngine->LoadAnm(0x10, mgr->anmPath2, 0x900) != 0)
    {
        return -1;
    }
    mgr->dropCounter = g_Rng.GetRandomU16InRange(3);
    mgr->dropIndex = g_Rng.GetRandomU16InRange(8);
    mgr->spellCardActive = 0;
    D3DXVECTOR3 nowhere(-999.0f, -999.0f, -999.0f);
    g_AsciiText.SetUnkB7cPos(0, &nowhere);
    g_AsciiText.SetUnkB7cPos(1, &nowhere);
    g_AsciiText.SetUnkB7cPos(2, &nowhere);
    g_AsciiText.SetUnkB7cPos(3, &nowhere);
    return 0;
}

int EnemyManager::DeletedCallback(EnemyManager *mgr)
{
    g_SpriteEngine->ReleaseAnm(0x10);
    g_SpriteEngine->ReleaseAnm(0xf);
    D3DXVECTOR3 nowhere(-999.0f, -999.0f, -999.0f);
    g_AsciiText.SetUnkB7cPos(0, &nowhere);
    g_AsciiText.SetUnkB7cPos(1, &nowhere);
    g_AsciiText.SetUnkB7cPos(2, &nowhere);
    g_AsciiText.SetUnkB7cPos(3, &nowhere);
    return 0;
}

int EnemyManager::RegisterChain(char *anmPath, char *anmPath2)
{
    EnemyManager *mgr = &g_Unknown9a9b00;

    mgr->Initialize();
    mgr->anmPath = anmPath;
    mgr->anmPath2 = anmPath2;
    g_Unknown9a9b00CalcChain.SetCallback((ChainCallback)EnemyManager::OnUpdate);
    g_Unknown9a9b00CalcChain.addedCallback = (ChainCallback)EnemyManager::AddedCallback;
    g_Unknown9a9b00CalcChain.deletedCallback = (ChainCallback)EnemyManager::DeletedCallback;
    g_Unknown9a9b00CalcChain.arg = mgr;
    if (g_Chain.AddToCalcChain(&g_Unknown9a9b00CalcChain, 10) != 0)
    {
        return -1;
    }
    g_Unknown9a9b00DrawChain.SetCallback((ChainCallback)EnemyManager::OnDraw);
    g_Unknown9a9b00DrawChain.arg = mgr;
    if (g_Chain.AddToDrawChain(&g_Unknown9a9b00DrawChain, 5) != 0)
    {
        return -1;
    }
    g_Unknown9a9b00DrawChain2.SetCallback((ChainCallback)EnemyManager::OnDraw2);
    g_Unknown9a9b00DrawChain2.arg = mgr;
    if (g_Chain.AddToDrawChain(&g_Unknown9a9b00DrawChain2, 7) != 0)
    {
        return -1;
    }
    return 0;
}

void EnemyManager::RemoveChain()
{
    g_Chain.Remove(&g_Unknown9a9b00CalcChain);
    g_Chain.Remove(&g_Unknown9a9b00DrawChain);
    g_Chain.Remove(&g_Unknown9a9b00DrawChain2);
}

int EnemyManager::AnyBossActive()
{
    int i;

    for (i = 0; i < 8; i++)
    {
        if (this->bosses[i] != NULL)
        {
            return 1;
        }
    }
    return 0;
}

int EnemyManager::ClearEnemiesForBonus(int maxBonus, int start)
{
    Enemy *enemy = this->enemies;
    int total = start;
    int bonus = 2000;
    int k;
    int j;

    for (k = 0; k < 0x1e0; k++, enemy++)
    {
        if (!enemy->flags.active)
        {
            continue;
        }
        if (enemy->flags.isBoss)
        {
            continue;
        }
        enemy->life = 0;
        if (enemy->flags.grazeable)
        {
            g_ItemManager.SpawnItem(&enemy->pos, 6, 1);
            g_AsciiText.AddPopup(&enemy->pos, bonus, bonus >= maxBonus ? 0xffffff00 : 0xffffffff);
            total += bonus;
            bonus += 30;
            if (bonus > maxBonus)
            {
                bonus = maxBonus;
            }
            if (enemy->trailFlags)
            {
                for (j = 0; j < enemy->trailSegmentCount; j += 6)
                {
                    g_ItemManager.SpawnItem(&enemy->trailBoxes[j].pos, 6, 1);
                    g_AsciiText.AddPopup(&enemy->trailBoxes[j].pos, bonus,
                                         bonus >= maxBonus ? 0xffffff00 : 0xffffffff);
                    total += bonus;
                    bonus += 30;
                    if (bonus > maxBonus)
                    {
                        bonus = maxBonus;
                    }
                }
            }
        }
        if (!enemy->flags.shootable && enemy->deathSub >= 0)
        {
            g_Unknown1347938.StartSub(&enemy->ctx, enemy->deathSub);
            enemy->deathSub = -1;
        }
    }
    return total;
}

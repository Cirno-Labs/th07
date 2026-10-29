// The file with EnemyManager, from 0x41e920, as th06's enemy.cpp.

#include <string.h>

#include "ascii.h"
#include "bullet.h"
#include "enemy.h"
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
        eff->sprite.visible = !enemy->flags.unk_b;
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

void Enemy::Move()
{
    this->posDelta = this->pos - this->prevPos;
    this->prevPos = this->pos;
    if (!this->flags.mirrored)
    {
        this->pos.x += g_App.unk_178 * this->velocity.x;
    }
    else
    {
        this->pos.x -= g_App.unk_178 * this->velocity.x;
    }
    this->pos.y += g_App.unk_178 * this->velocity.y;
    this->pos.z += g_App.unk_178 * this->velocity.z;
}

void Unknown9a9b00::Initialize()
{
    int i;
    Enemy *enemy = this->enemies;

    memset(this, 0, sizeof(Unknown9a9b00));
    enemy = &this->enemyTemplate;
    memset(enemy, 0, sizeof(Enemy));
    for (i = 0; i < 2; i++)
    {
        enemy->sprites[i].scriptIndex = -1;
    }
    for (i = 0; i < 0x60; i++)
    {
        enemy->unk_2f78[i].pos.x = -999.0f;
    }
    enemy->flags.active = 1;
    enemy->callbackTimer.Set(0);
    enemy->flags.unk_13 = 0;
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
    enemy->unk_2e15 = 0;
    enemy->unk_2e16 = 0;
    enemy->shootInterval = 0;
    enemy->shootTimer.Set(0);
    enemy->shootOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    enemy->turnLeft = -1;
    enemy->turnRight = -1;
    enemy->turnStill = -1;
    enemy->flags.shootable = 1;
    enemy->flags.unk_9 = 1;
    enemy->flags.unk_a = 1;
    enemy->flags.unk_b = 0;
    enemy->flags.unk_c = 1;
    enemy->flags.unk_d = 0;
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
    enemy->unk_2ee4 = -1;
    enemy->laserIndex = 0;
    enemy->hitFlashTimer = 0;
    enemy->flags.rotateSprite = 0;
    enemy->rankSpeedLow = -0.15f;
    enemy->rankSpeedHigh = 0.15f;
    enemy->shooter.sound = 7;
    enemy->shooter.unk_cc = 0x19;
}

Enemy *Unknown9a9b00::Spawn(short sub, D3DXVECTOR3 *pos, int life, char item, int score, BYTE mirrored)
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
        enemy->item = item;
        if (score >= 0)
        {
            enemy->score = score;
        }
        enemy->maxLife = enemy->life;
        break;
    }
    return enemy;
}

Enemy *Unknown9a9b00::SpawnWithVars(short sub, D3DXVECTOR3 *pos, int life, char item, int score,
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
        enemy->item = item;
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
            this->unk_2ee4 = -1;
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
        this->unk_2ee4 = -1;
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
    if (this->flags.unk_d && this->callbackTimer.HasTicked() && this->callbackTimer.current % 6 == 0)
    {
        g_Player.CheckGraze(pos, &box);
    }
    box = *size / 1.5f;
    if (g_Player.CheckHit(pos, &box) == 1 && this->flags.shootable && !this->flags.isBoss &&
        !this->flags.unk_d)
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

int Unknown9a9b00::OnDraw(Unknown9a9b00 *mgr)
{
    return Unknown9a9b00::Func422170(mgr, 0, 2);
}

int Unknown9a9b00::OnDraw2(Unknown9a9b00 *mgr)
{
    return Unknown9a9b00::Func422170(mgr, 2, 4);
}

int Unknown9a9b00::AddedCallback(Unknown9a9b00 *mgr)
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

int Unknown9a9b00::DeletedCallback(Unknown9a9b00 *mgr)
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

int Unknown9a9b00::RegisterChain(char *anmPath, char *anmPath2)
{
    Unknown9a9b00 *mgr = &g_Unknown9a9b00;

    mgr->Initialize();
    mgr->anmPath = anmPath;
    mgr->anmPath2 = anmPath2;
    g_Unknown9a9b00CalcChain.SetCallback((ChainCallback)Unknown9a9b00::OnUpdate);
    g_Unknown9a9b00CalcChain.addedCallback = (ChainCallback)Unknown9a9b00::AddedCallback;
    g_Unknown9a9b00CalcChain.deletedCallback = (ChainCallback)Unknown9a9b00::DeletedCallback;
    g_Unknown9a9b00CalcChain.arg = mgr;
    if (g_Chain.AddToCalcChain(&g_Unknown9a9b00CalcChain, 10) != 0)
    {
        return -1;
    }
    g_Unknown9a9b00DrawChain.SetCallback((ChainCallback)Unknown9a9b00::OnDraw);
    g_Unknown9a9b00DrawChain.arg = mgr;
    if (g_Chain.AddToDrawChain(&g_Unknown9a9b00DrawChain, 5) != 0)
    {
        return -1;
    }
    g_Unknown9a9b00DrawChain2.SetCallback((ChainCallback)Unknown9a9b00::OnDraw2);
    g_Unknown9a9b00DrawChain2.arg = mgr;
    if (g_Chain.AddToDrawChain(&g_Unknown9a9b00DrawChain2, 7) != 0)
    {
        return -1;
    }
    return 0;
}

void Unknown9a9b00::RemoveChain()
{
    g_Chain.Remove(&g_Unknown9a9b00CalcChain);
    g_Chain.Remove(&g_Unknown9a9b00DrawChain);
    g_Chain.Remove(&g_Unknown9a9b00DrawChain2);
}

int Unknown9a9b00::AnyBossActive()
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

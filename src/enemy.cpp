// The file with EnemyManager, from 0x41e920, as th06's enemy.cpp.

#include <string.h>

#include "enemy.h"
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

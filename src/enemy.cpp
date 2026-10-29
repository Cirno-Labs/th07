// The file with EnemyManager, from 0x41e920, as th06's enemy.cpp.

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

// The file with EnemyManager, from 0x41e920, as th06's enemy.cpp.

#include "enemy.h"
#include "unsorted.h"

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

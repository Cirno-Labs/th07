// The file with the player's bombs, from 0x4083f0, as th06's bomb.cpp.

#include "effect.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"

void DimPlayAreaForBomb(Player *player)
{
    ColorBytes color;

    color.a = 0x80;
    if (player->bomb.timer < 60)
    {
        color.b = 0x80 - player->bomb.timer.AsFrames() * 80 / 60;
        color.g = color.b;
        color.r = color.g;
    }
    else if (player->bomb.timer >= player->bomb.duration - 60)
    {
        color.b = 0x80 - (player->bomb.duration - player->bomb.timer.AsFrames()) * 80 / 60;
        color.g = color.b;
        color.r = color.g;
    }
    else
    {
        color.b = 0x30;
        color.g = color.b;
        color.r = color.g;
    }
    g_SpriteEngine->ResetUnk0();
    g_Unknown1347b00.SetTintColor(color.color);
    g_Unknown1347b00.unk_52b0 = 1;
}

void SpawnBombAura(Player *player)
{
    Effect *effect;

    if (player->unk_b7e68 != NULL)
    {
        player->unk_b7e68->active = 0;
    }
    effect = g_EffectManager.SpawnReservedEffect(0x19, &player->position, 0, 1, 0xffffffff);
    effect->sprite.unk_48[4].Set(0);
    effect->sprite.unk_84[4] = player->stateTimer;
    effect->sprite.unk_c4 = 0;
    *(D3DXVECTOR2 *)&effect->sprite.unk_218 = *(D3DXVECTOR2 *)&effect->sprite.scaleY;
    effect->sprite.unk_218.z = 0.0625f;
    effect->sprite.unk_224.x = 0.0625f;
    effect->sprite.unk_c8 = player->stateTimer.AsFrames();
    effect->sprite.unk_14 *= -1.0f;
    effect->sprite.unk_rgb[2] = 0xff;
    effect->sprite.unk_rgb[1] = 0x40;
    effect->sprite.unk_rgb[0] = 0x40;
    player->unk_b7e68 = effect;
}

void SetBombDrain(Player *player, int minimum, float share)
{
    int drain;

    drain = (g_Unknown626270.unk_961c - g_Unknown626270.stats->unk_88) * share;
    switch (g_Unknown626270.difficulty)
    {
    case 2:
        drain /= 2;
        break;
    case 3:
        drain /= 4;
        break;
    case 4:
    case 5:
        drain /= 3;
        break;
    }
    drain /= player->bomb.duration;
    drain -= drain % 10;
    minimum /= player->bomb.duration;
    minimum -= minimum % 10;
    player->bomb.unk_c = drain < minimum ? minimum : drain;
}

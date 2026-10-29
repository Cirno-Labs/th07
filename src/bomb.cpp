// The file with the player's bombs, from 0x4083f0, as th06's bomb.cpp.

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

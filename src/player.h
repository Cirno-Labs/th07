// The player and its shots. Only the fields known so far.

#pragma once

#include <d3dx8math.h>

#include "chain.h"
#include "common.h"
#include "sprite.h"
#include "timer.h"

// One of the player's bullets. Its implicit constructor is 0x43ba10.
struct PlayerBullet
{
    Sprite sprite;
    D3DXVECTOR3 pos;
    D3DXVECTOR3 unk_258[0x10];
    unsigned char unk_318[0x24];
    Timer timer;
    unsigned char unk_348[0x1c];
};
ASSERT_SIZE(PlayerBullet, 0x364);

// One of the 0x70 entries at Player+0x9dc, probably th06's bomb
// hitboxes (bombHitPos, bombHitSize, ...) as one struct.
struct PlayerBombHit
{
    D3DXVECTOR3 pos;
    D3DXVECTOR3 size;
    int unk_18;
    int unk_1c;
};
ASSERT_SIZE(PlayerBombHit, 0x20);

// One of PlayerBomb's 0x80 parts. Its implicit constructor is 0x43bb10.
struct PlayerBombPart
{
    unsigned char unk_0[0x20];
    D3DXVECTOR3 unk_20[0x20];
    unsigned char unk_1a0[0x18];
    Sprite sprites[8];
    unsigned char unk_1418[0x4];
    Timer timer;
};
ASSERT_SIZE(PlayerBombPart, 0x1428);

// Presumably th06's PlayerBomb. Its implicit constructor is 0x43baa0.
struct PlayerBomb
{
    unsigned char unk_0[0x10];
    Timer timer;
    unsigned char unk_1c[0x10];
    PlayerBombPart parts[0x80];
};
ASSERT_SIZE(PlayerBomb, 0xa142c);

// One of the three entries at Player+0x169c4: a Timer and a dword.
// Maybe th06's laserTimers, grown.
struct PlayerUnk169c4
{
    Timer timer;
    int unk_c;
};
ASSERT_SIZE(PlayerUnk169c4, 0x10);

// The player, the global at 0x4bdad8.
class Player
{
public:
    // Empty, and defined out of line: constructs the members (0x43b7f0).
    Player();

    Sprite sprite;
    Sprite optionSprites[3];
    D3DXVECTOR3 position;
    unsigned char unk_93c[0x78];
    D3DXVECTOR3 unk_9b4[2];
    unsigned char unk_9cc[0x10];
    PlayerBombHit unk_9dc[0x70];
    unsigned char unk_17dc[0xc34];
    Timer unk_2410;
    unsigned char unk_241c[0x28];
    PlayerBullet bullets[0x60];
    PlayerUnk169c4 unk_169c4[3];
    Timer unk_169f4;
    Timer unk_16a00;
    Timer unk_16a0c;
    unsigned char unk_16a18[0x8];
    PlayerBomb bomb;
    unsigned char unk_b7e4c[0x10];
    // The update element and the two draw elements, as in th06
    // (removed together by 0x442b13's function).
    ChainElem *calcChain;
    ChainElem *drawChain;
    ChainElem *drawChain2;
};
ASSERT_SIZE(Player, 0xb7e68);
extern Player g_Player;

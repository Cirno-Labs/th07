// The items the enemies drop and the player collects.

#pragma once

#include <d3dx8math.h>

#include "sprite.h"
#include "timer.h"

struct Item
{
    Sprite sprite;
    D3DXVECTOR3 pos;
    D3DXVECTOR3 velocity;
    D3DXVECTOR3 target;
    Timer timer;
    // Its kind: picks its script, 0x2c4 + type.
    char type;
    char active;
    char onScreen;
    char state;
    char unk_280;
    char unk_281[7];
};
ASSERT_SIZE(Item, 0x288);

class ItemManager
{
public:
    // Drops item `type` at `pos` in `state` into the next free slot and
    // returns it, or the spare last item when all are taken. At full
    // power (128), power items (0 and 2) turn into type 7. States 3 and 4
    // spawn in state 1 and 0.
    Item *SpawnItem(D3DXVECTOR3 *pos, int type, int state);
    // Draws every active item to the player (state 1), starting it off
    // at a slow upward drift.
    void CollectAll();
    // Turns every active power item (types 0 and 2) except item
    // `except` into type 7, with effect 0 on it. Those not rising
    // faster are set to drift up at 0.5.
    void ConvertPowerItems(int except);
    // Lets every item drawn to the player (state 1) fall again, starting
    // at a slow upward drift.
    void RecycleCollectedItems();

    // The last one is what SpawnItem returns when all others are taken.
    Item items[0x44d];
    int nextIndex;
};

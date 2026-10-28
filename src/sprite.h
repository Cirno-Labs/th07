// Sprites run by ANM scripts. Only the fields known so far.

#pragma once

#include <d3d8.h>

#include "common.h"

class Sprite
{
public:
    unsigned char unk_0[0x1b8];
    D3DCOLOR color;
    unsigned char unk_1bc[0x4];
    // Bit 0x1000 is set on items as they spawn.
    DWORD flags;
    unsigned char unk_1c4[0x10];
    short spriteIndex;
    unsigned char unk_1d6[0x2];
    // The script SpriteEngine::StartScript started.
    short scriptIndex;
    unsigned char unk_1da[0x72];
};
ASSERT_SIZE(Sprite, 0x24c);

// Runs the sprites' ANM scripts. Only the fields known so far.
class SpriteEngine
{
public:
    // Runs `script` on `vm` from its start.
    void SetScript(Sprite *vm, void *script);

    // Runs script `script` of the loaded ANM files on `vm`.
    void StartScript(Sprite *vm, int script)
    {
        vm->scriptIndex = script;
        this->SetScript(vm, this->scripts[script]);
    }

    unsigned char unk_0[0x28ef0];
    void *scripts[1];
};
extern SpriteEngine *g_SpriteEngine;

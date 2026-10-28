// Sprites run by ANM scripts. Only the fields known so far.

#pragma once

#include <d3d8.h>
#include <d3dx8math.h>

#include "common.h"

class Sprite
{
public:
    unsigned char unk_0[0x1b8];
    union
    {
        D3DCOLOR color;
        struct
        {
            BYTE unk_rgb[3];
            // The alpha byte of color.
            BYTE alpha;
        };
    };
    unsigned char unk_1bc[0x4];
    // Bit 0x1000 is set on items as they spawn.
    DWORD flags;
    unsigned char unk_1c4[0x2];
    // The interrupt label to jump to on the next Step; 0 for none.
    short pendingInterrupt;
    // Where it is drawn.
    D3DXVECTOR3 pos;
    short spriteIndex;
    unsigned char unk_1d6[0x2];
    // The script SpriteEngine::StartScript started.
    short scriptIndex;
    unsigned char unk_1da[0x56];
    // Added to pos when it is drawn.
    D3DXVECTOR3 posOffset;
    unsigned char unk_23c[0x10];
};
ASSERT_SIZE(Sprite, 0x24c);

// Runs the sprites' ANM scripts. Only the fields known so far.
class SpriteEngine
{
public:
    SpriteEngine();
    ~SpriteEngine();

    // Runs `script` on `vm` from its start.
    void SetScript(Sprite *vm, void *script);
    // Shows sprite `sprite` of the loaded ANM files on `vm`.
    int SetSprite(Sprite *vm, int sprite);
    // Draws `vm` at its position.
    int Draw(Sprite *vm);
    // Runs `vm`'s script for one frame.
    int Step(Sprite *vm);
    // Renders text from `fmt` into `sprite`'s texture.
    void RenderText(Sprite *sprite, D3DCOLOR textColor, D3DCOLOR shadowColor, char *fmt, ...);
    // RenderText, centered on the sprite.
    void RenderTextCentered(Sprite *vm, D3DCOLOR color, D3DCOLOR shadowColor, char *fmt, ...);
    // Releases the loaded surfaces, before the device is reset.
    void ReleaseSurfaces();
    // Starts a new batch of quads. Not in th06.
    void Func44f580();
    // Draws the batched quads, if any. Not in th06.
    void Func44f5c0();
    // Copies the `srcWidth` by `srcHeight` rectangle at (`srcLeft`,
    // `srcTop`) of the back buffer into the `dstWidth` by `dstHeight`
    // rectangle at (`dstLeft`, `dstTop`) of texture `texture`.
    void CopyBackBufferToTexture(int texture, int srcLeft, int srcTop, int srcWidth, int srcHeight, int dstLeft,
                                 int dstTop, int dstWidth, int dstHeight);
    // Does the copy CopyBackBufferToTexture that `pendingCopy` asks for,
    // if any, and clears it.
    void DoPendingCopy();

    void ClearBlendMode()
    {
        this->blendMode = 0xff;
    }
    void ClearColorOp()
    {
        this->colorOp = 0xff;
    }
    void ClearVertexFormat()
    {
        this->vertexFormat = 0xff;
    }
    void ClearTexture()
    {
        this->currentTexture = NULL;
    }
    void ClearUnk2e4d4()
    {
        this->unk_2e4d4 = 0xff;
    }

    // Runs script `script` of the loaded ANM files on `vm`.
    void StartScript(Sprite *vm, int script)
    {
        vm->scriptIndex = script;
        this->SetScript(vm, this->scripts[script]);
    }

    unsigned char unk_0[0x28ef0];
    void *scripts[1];
    unsigned char unk_28ef4[0x5254];
    // Images loaded as surfaces to copy to the back buffer from.
    IDirect3DSurface8 *surfaces[0x20];
    unsigned char unk_2e1c8[0x304];
    // The texture last set on the device.
    IDirect3DTexture8 *currentTexture;
    // The states Draw last set, 0xff when unknown.
    BYTE blendMode;
    BYTE colorOp;
    BYTE vertexFormat;
    unsigned char unk_2e4d3;
    BYTE unk_2e4d4;
    unsigned char unk_2e4d5[0x150067];
    // A copy of the back buffer into a texture to do after the frame is
    // drawn: CopyBackBufferToTexture's arguments, the texture -1 when
    // there is none.
    int pendingCopyTexture;
    int pendingCopySrcLeft;
    int pendingCopySrcTop;
    int pendingCopySrcWidth;
    int pendingCopySrcHeight;
    int pendingCopyDstLeft;
    int pendingCopyDstTop;
    int pendingCopyDstWidth;
    int pendingCopyDstHeight;
};
ASSERT_SIZE(SpriteEngine, 0x17e560);
extern SpriteEngine *g_SpriteEngine;

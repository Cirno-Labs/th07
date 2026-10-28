// Sprites run by ANM scripts. Only the fields known so far.

#pragma once

#include <d3d8.h>
#include <d3dx8math.h>

#include "common.h"
#include "timer.h"

// An entry of a loaded ANM file's sprite list. Only the fields known so
// far.
struct SpriteData
{
    // The ANM slot the sprite comes from; negative when unused.
    int sourceFileIndex;
};

// The part of Sprite up to pos. Its implicit constructor (0x4011b0)
// constructs the Timers; Sprite's calls it rather than expanding it.
struct SpriteBase
{
    unsigned char unk_0[0x8];
    // The sprite's rotation about z.
    float rotationZ;
    unsigned char unk_c[0xc];
    // SpriteEngine::Draw scales the sprite's height and width by these.
    float scaleY;
    float scaleX;
    unsigned char unk_20[0x10];
    Timer unk_30;
    Timer unk_3c;
    Timer unk_48[5];
    Timer unk_84[5];
    unsigned char unk_c0[0x38];
    D3DXMATRIX matrix;
    D3DXVECTOR3 unk_138;
    D3DXVECTOR3 unk_144;
    D3DXVECTOR3 unk_150;
    unsigned char unk_15c[0x5c];
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
};

class Sprite : public SpriteBase
{
public:
    // Inline, expanded in /Od files; /Os files call the copy at
    // 0x401170.
    Sprite()
    {
        memset(this, 0, sizeof(Sprite));
        this->spriteIndex = -1;
    }
    // Resets the sprite's state up to pos: a scale of 1, white, the
    // identity matrix, flags 7. Inline, but too large to expand: callers
    // call the copy at 0x4010f0 and keep a slot for it.
    void Initialize()
    {
        memset(this, 0, sizeof(SpriteBase));
        this->scaleY = 1.0f;
        this->scaleX = 1.0f;
        this->color = 0xffffffff;
        D3DXMatrixIdentity(&this->matrix);
        *(WORD *)&this->flags = 7;
        this->unk_30.Initialize();
    }
    // Whether the sprite shows a sprite whose texture is loaded. Defined
    // after SpriteEngine.
    __forceinline BOOL HasTexture();
    void SetPendingInterrupt(short interrupt)
    {
        this->pendingInterrupt = interrupt;
    }

    // Where it is drawn.
    D3DXVECTOR3 pos;
    short spriteIndex;
    // The first sprite of the script: games show variants of it as
    // baseSpriteIndex + an offset, such as digits.
    short baseSpriteIndex;
    // The script SpriteEngine::StartScript started.
    short scriptIndex;
    unsigned char unk_1da[0xa];
    // The entry SetSprite chose.
    SpriteData *spriteData;
    D3DXVECTOR3 unk_1e8;
    D3DXVECTOR3 unk_1f4;
    D3DXVECTOR3 unk_200;
    D3DXVECTOR3 unk_20c;
    D3DXVECTOR3 unk_218;
    D3DXVECTOR3 unk_224;
    // Added to pos when it is drawn.
    D3DXVECTOR3 posOffset;
    unsigned char unk_23c[0x4];
    // The size of the characters RenderText draws.
    BYTE fontWidth;
    BYTE fontHeight;
    unsigned char unk_242[0xa];
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
    // Draws `vm` when it is not rotated. Not in th06 as such.
    int Func44f770(Sprite *vm);
    // Copies surface `surface` to the back buffer at (`left`, `top`),
    // as in th06.
    void CopySurfaceToBackBuffer(int surface, int left, int top, int x, int y);
    // Runs `vm`'s script for one frame.
    int Step(Sprite *vm);
    // Steps the `count` sprites from `vms` that run a script. Not in
    // th06.
    void Func4550c0(Sprite *vms, int count);
    // Sends interrupt `interrupt` to the `count` sprites from `vms` that
    // show a loaded sprite. Not in th06.
    void Func455030(Sprite *vms, int count, short interrupt);
    // Loads the image at `path` as surface `surface`. Returns 0, or -1 on
    // failure.
    int LoadSurface(int surface, char *path);
    // Loads the ANM file at `path` into slot `slot`, its sprites from
    // `firstSprite` on. Returns 0, or -1 on failure.
    int LoadAnm(int slot, char *path, int firstSprite);
    // Renders text from `fmt` into `sprite`'s texture.
    void RenderText(Sprite *sprite, D3DCOLOR textColor, D3DCOLOR shadowColor, char *fmt, ...);
    // RenderText, centered on the sprite.
    void RenderTextCentered(Sprite *vm, D3DCOLOR color, D3DCOLOR shadowColor, char *fmt, ...);
    // Releases ANM slot `slot`.
    void ReleaseAnm(int slot);
    // Releases surface `surface`.
    void ReleaseSurface(int surface);
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
    void ClearZWrite()
    {
        this->zWriteDisabled = 0xff;
    }
    void ClearSpriteData()
    {
        this->currentSpriteData = NULL;
    }
    void ClearUnk2e4d4()
    {
        this->unk_2e4d4 = 0xff;
    }
    void ClearUnk8()
    {
        this->unk_c = 0;
        this->unk_10 = 0;
        this->unk_8 = 0;
        this->unk_14 = 0;
    }
    void ResetUnk0()
    {
        this->unk_4 = 0;
        this->unk_0 = 0x80808080;
    }

    // Resets `vm` and shows sprite `sprite` on it.
    void InitSprite(Sprite *vm, int sprite)
    {
        vm->Initialize();
        this->SetSprite(vm, sprite);
    }

    // Runs script `script` of the loaded ANM files on `vm`.
    void StartScript(Sprite *vm, int script)
    {
        vm->scriptIndex = script;
        this->SetScript(vm, this->scripts[script]);
    }

    D3DCOLOR unk_0;
    DWORD unk_4;
    DWORD unk_8;
    DWORD unk_c;
    DWORD unk_10;
    DWORD unk_14;
    // How far the screen is shaken, set by ScreenEffect's shake.
    float shakeX;
    float shakeY;
    unsigned char unk_20[0x2828c];
    // The textures of the loaded ANM files, by slot.
    IDirect3DTexture8 *textures[1];
    unsigned char unk_282b0[0xc40];
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
    BYTE zWriteDisabled;
    BYTE unk_2e4d4;
    unsigned char unk_2e4d5[0x3];
    // The sprite data Draw last used, NULL when unknown.
    void *currentSpriteData;
    unsigned char unk_2e4dc[0x150060];
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

inline BOOL Sprite::HasTexture()
{
    if (this->spriteData == NULL)
    {
        return FALSE;
    }
    if (this->spriteData->sourceFileIndex < 0)
    {
        return FALSE;
    }
    return g_SpriteEngine->textures[this->spriteData->sourceFileIndex] != NULL;
}

// A vertex with a transformed position and a color, as
// D3DFVF_XYZRHW | D3DFVF_DIFFUSE.
struct VertexDiffuseXyzrwh
{
    D3DXVECTOR3 pos;
    float w;
    D3DCOLOR diffuse;
};

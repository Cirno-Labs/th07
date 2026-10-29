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
    // The sprite's rectangle in the texture, in pixels.
    float startX;
    float startY;
    unsigned char unk_c[0x8];
    // The texture's size.
    float textureHeight;
    float textureWidth;
    // The rectangle in texture coordinates.
    float uvStartX;
    float uvStartY;
    float uvEndX;
    float uvEndY;
    // Its size in pixels, as in th06.
    float height;
    float width;
    // How much larger the texture is than the sprite's size: LoadAnm
    // divides the rectangle's size by these. Not in th06.
    float scaleX;
    float scaleY;
};

// The part of Sprite up to pos. Its implicit constructor (0x4011b0)
// constructs the Timers; Sprite's calls it rather than expanding it.
struct SpriteBase
{
    unsigned char unk_0[0x8];
    // The sprite's rotation about z.
    float rotationZ;
    unsigned char unk_c[0x8];
    // Player's StartBombState negates it on its effect.
    float unk_14;
    // SpriteEngine::Draw scales the sprite's height and width by these.
    float scaleY;
    float scaleX;
    unsigned char unk_20[0x8];
    // Added to the sprite's texture coordinates, as in th06.
    float uvScrollX;
    float uvScrollY;
    Timer unk_30;
    Timer unk_3c;
    Timer unk_48[5];
    Timer unk_84[5];
    unsigned char unk_c0[0x2];
    // Player's StopBombState sets it on its effect.
    BYTE unk_c2;
    unsigned char unk_c3[0x1];
    // Player's StartBombState sets these on its effect.
    BYTE unk_c4;
    unsigned char unk_c5[0x3];
    int unk_c8;
    unsigned char unk_cc[0x2c];
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
    // Whether the bullets' draw functions turn it along their angle, as
    // th06's autoRotate.
    short autoRotate;
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
    // Turns the sprite to `angle` about z and sets flag 0x4. Not in th06.
    void SetRotationZ(float angle)
    {
        this->rotationZ = angle;
        this->flags |= 4;
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

// A vertex with a transformed position, a color and texture
// coordinates, as in th06.
struct VertexTex1DiffuseXyzrwh
{
    D3DXVECTOR3 pos;
    float w;
    D3DCOLOR diffuse;
    D3DXVECTOR2 uv;
};

// A vertex with a position and texture coordinates, as in th06.
struct VertexTex1Xyz
{
    D3DXVECTOR3 pos;
    D3DXVECTOR2 uv;
};
// A vertex with a position, a color and texture coordinates, as in
// th06.
struct VertexTex1DiffuseXyz
{
    D3DXVECTOR3 pos;
    D3DCOLOR diffuse;
    D3DXVECTOR2 uv;
};

// An image stored in an ANM file, its pixels following the header.
// Not in th06.
struct AnmImage
{
    unsigned char unk_0[0x6];
    // The pixels' format, an index into g_TextureFormats.
    short format;
    short width;
    short height;
    unsigned char unk_c[0x4];
    BYTE data[1];
};

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
    // Copies the `width` by `height` rectangle at (`left`, `top`) of
    // surface `surface` to the back buffer at (`x`, `y`), as in th06.
    void CopySurfaceRectToBackBuffer(int surface, int x, int y, int left, int top, int width, int height);
    // Runs `vm`'s script for one frame.
    int Step(Sprite *vm);
    // Steps the `count` sprites from `vms` that run a script. Not in
    // th06.
    void StepSprites(Sprite *vms, int count);
    // Starts script `script` + n on sprite n of the `count` sprites from
    // `vms`, and makes their sprites their base sprites. Not in th06.
    void StartSpriteSeries(Sprite *vms, int script, int count);
    // Sends interrupt `interrupt` to the `count` sprites from `vms` that
    // show a loaded sprite. Not in th06.
    void SendInterruptToLoadedSprites(Sprite *vms, int count, short interrupt);
    // Loads the image at `path` as surface `surface`. Returns 0, or -1 on
    // failure.
    int LoadSurface(int surface, char *path);
    // Loads the ANM file at `path` into slot `slot`, its sprites from
    // `firstSprite` on. Returns 0, or -1 on failure.
    int LoadAnm(int slot, char *path, int firstSprite);
    // Renders `text` into texture `textureIndex` with
    // RenderTextToTexture, its font sizes 15 when not positive and
    // scaled by `scaleX` and `scaleY`.
    void RenderTextToTextureIndex(int textureIndex, int x, int y, int width, int unk4, int fontHeight, int unk6,
                                  D3DCOLOR textColor, D3DCOLOR shadowColor, char *text, float scaleX, float scaleY);
    // Renders text from `fmt` into `sprite`'s texture.
    void RenderText(Sprite *sprite, D3DCOLOR textColor, D3DCOLOR shadowColor, char *fmt, ...);
    // RenderText, aligned to the right of the sprite.
    void RenderTextRightAligned(Sprite *vm, D3DCOLOR color, D3DCOLOR shadowColor, char *fmt, ...);
    // RenderText, centered on the sprite.
    void RenderTextCentered(Sprite *vm, D3DCOLOR color, D3DCOLOR shadowColor, char *fmt, ...);
    // Sets the texture coordinates, color and w of the `count` vertices
    // from `vertices`, a strip across `vm`'s sprite from right to left:
    // even ones along its top, odd ones along its bottom. Returns 0, or
    // -1 for fewer than 3. Not in th06.
    int FillSpriteStripVertices(Sprite *vm, VertexTex1DiffuseXyzrwh *vertices, int count);
    // Draws the `count` vertices from `vertices` as a triangle strip with
    // `vm`'s texture and render states, after the pending batch. Returns
    // 0, or -1 when `vm` is hidden or transparent. Not in th06.
    int DrawVertexStrip(Sprite *vm, VertexTex1DiffuseXyzrwh *vertices, int count);
    // Sets the render states `vm` wants on the device, as in th06.
    void SetRenderStateForSprite(Sprite *vm);
    // Creates texture `index` from the image file at `path`, in the format
    // g_TextureFormats[`format`] (16 bits when forced), with `colorKey`
    // transparent, and keeps the file. Returns 0, or -1 on failure.
    int LoadTexture(int index, char *path, int format, D3DCOLOR colorKey);
    // Creates texture `index` from the pixels of `image`, in the format
    // g_TextureFormats[`format`] (16 bits when forced). Returns 0, or -1
    // on failure. Not in th06.
    int CreateTextureFromImage(int index, AnmImage *image, int format);
    // Releases texture `index` and its file.
    void ReleaseTexture(int index);
    // Releases ANM slot `slot`.
    void ReleaseAnm(int slot);
    // Releases surface `surface`.
    void ReleaseSurface(int surface);
    // Releases the loaded surfaces, before the device is reset.
    void ReleaseSurfaces();
    // Creates the vertex buffer and the quads sprites are drawn with.
    void CreateQuad();
    // Releases the vertex buffer, if there is one.
    void ReleaseVertexBuffer();
    // Starts a new batch of quads. Not in th06.
    void Func44f580();
    // Draws the batched quads, if any. Not in th06.
    void Func44f5c0();
    // Copies the `srcWidth` by `srcHeight` rectangle at (`srcLeft`,
    // `srcTop`) of the back buffer into the `dstWidth` by `dstHeight`
    // rectangle at (`dstLeft`, `dstTop`) of texture `texture`.
    void CopyBackBufferToTexture(int texture, int srcLeft, int srcTop, int srcWidth, int srcHeight, int dstLeft,
                                 int dstTop, int dstWidth, int dstHeight);
    // Copies rectangle `srcRect` of texture `srcTexture` into rectangle
    // `dstRect` of texture `dstTexture`. Not in th06.
    void CopyTextureRect(int dstTexture, int srcTexture, RECT *dstRect, RECT *srcRect);
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

    // Like StartScript, after zeroing the position and posOffset and
    // setting fontWidth and fontHeight to 15, as in th06. Inline, but
    // too large to expand: callers call the copy at 0x404f30.
    void StartScriptAtOrigin(Sprite *vm, int script)
    {
        vm->scriptIndex = script;
        vm->pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        vm->posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        vm->fontHeight = 15;
        vm->fontWidth = 15;
        this->SetScript(vm, this->scripts[script]);
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
    IDirect3DTexture8 *textures[0x108];
    // The image files the textures were created from, kept loaded.
    void *textureFiles[0x100];
    unsigned char unk_28acc[0x424];
    void *scripts[1];
    unsigned char unk_28ef4[0x5254];
    // Images loaded as surfaces to copy to the back buffer from.
    IDirect3DSurface8 *surfaces[0x20];
    // System memory copies of them, to recreate them from.
    IDirect3DSurface8 *surfacesBackup[0x20];
    // The images' sizes and formats.
    D3DXIMAGE_INFO surfaceInfos[0x20];
    unsigned char unk_2e4c8[0x4];
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
    IDirect3DVertexBuffer8 *vertexBuffer;
    // The quad the vertex buffer holds, 256 pixels across.
    VertexTex1Xyz quad[4];
    // The number of quads batched to draw with Func44f5c0.
    int batchQuadCount;
    unsigned char unk_2e534[0x150008];
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
// CreateQuad's quad with a color, for drawing without the vertex
// buffer.
extern VertexTex1DiffuseXyz g_DiffuseQuadXyz[4];
// The texture formats LoadTexture takes by number.
extern D3DFORMAT g_TextureFormats[];
// The bytes per pixel of each of g_TextureFormats.
extern int g_TextureFormatSizes[];

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

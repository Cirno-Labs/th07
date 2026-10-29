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
    float endX;
    float endY;
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
    // Numbers the sprites in the order they were loaded, as in th06.
    int spriteId;
};
ASSERT_SIZE(SpriteData, 0x40);

// An argument of an ANM instruction.
union AnmArg
{
    int i;
    float f;
    BYTE b[4];
};

// An instruction of an ANM script.
struct AnmInstr
{
    short opcode;
    // The size of the instruction, arguments included.
    WORD size;
    // The frame of the script it runs on.
    short time;
    // Bit n: argument n names a script variable (10000 to 10009).
    WORD paramMask;
    AnmArg args[1];
};

// The part of Sprite up to pos. Its implicit constructor (0x4011b0)
// constructs the Timers; Sprite's calls it rather than expanding it.
struct SpriteBase
{
    // The sprite's rotations about x, y and z.
    float rotationX;
    float rotationY;
    float rotationZ;
    // The rotation speeds about x, y and z, which Step adds each frame.
    float rotationSpeedX;
    float rotationSpeedY;
    // Player's StartBombState negates it on its effect.
    float rotationSpeedZ;
    // SpriteEngine::Draw scales the sprite's height and width by these.
    float scaleY;
    float scaleX;
    // The scale speeds, which Step adds to scaleY and scaleX each frame.
    float scaleSpeedY;
    float scaleSpeedX;
    // Added to the sprite's texture coordinates, as in th06.
    float uvScrollX;
    float uvScrollY;
    Timer unk_30;
    Timer unk_3c;
    Timer interpDuration[5];
    Timer interpFrame[5];
    // How Step eases each of the five interpolations (position, color,
    // alpha, rotation, scale) that unk_48 and unk_84 time.
    BYTE interpEasing;
    BYTE unk_c1;
    // Player's StopBombState sets it on its effect.
    BYTE unk_c2;
    BYTE unk_c3;
    // Player's StartBombState sets these on its effect.
    BYTE unk_c4;
    unsigned char unk_c5[0x3];
    // The script's variables: an argument of 10000 to 10009 stands for
    // one of them. 10000-10003 and 10008-10009 are ints, 10004-10007
    // floats.
    int scriptVar0;
    int scriptVar1;
    int scriptVar2;
    int scriptVar3;
    float scriptVarF0;
    float scriptVarF1;
    float scriptVarF2;
    float scriptVarF3;
    int scriptVar8;
    int scriptVar9;
    // The texture scroll speeds, added to uvScrollX and uvScrollY.
    float uvScrollSpeedX;
    float uvScrollSpeedY;
    D3DXMATRIX matrix;
    // A copy of matrix, made by SetSprite.
    D3DXMATRIX drawMatrix;
    // The sprite's size relative to its texture, scaled by the sprite
    // data's scale, set by SetSprite.
    D3DXMATRIX sizeMatrix;
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
    // The color drawn instead of color while flag bit 16 is set.
    D3DCOLOR color2;
    // Bit 0x1000 is set on items as they spawn.
    union
    {
        DWORD flags;
        struct
        {
            // Bits 0 and 1, for bitfield stores.
            DWORD shown : 1;
            DWORD visible : 1;
            DWORD unk_2 : 1;
            DWORD unk_3 : 1;
            DWORD blendMode : 1;
            DWORD unk_5 : 1;
            DWORD unk_6 : 1;
            // Step's position instructions move posOffset instead of
            // pos.
            DWORD usePosOffset : 1;
            DWORD flip : 2;
            DWORD anchor : 2;
            DWORD zWriteDisabled : 1;
            // Set while the script waits for an interrupt.
            DWORD stopped : 1;
            // Bit 14, which Step's instruction 31 sets.
            DWORD unk_14b : 1;
        };
    };
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
    // The value of `value` as a float argument: the variable it stands
    // for, if any, or itself.
    float ResolveFloatArgument(float value);
    // The same for an int argument; float variables are truncated.
    int ResolveIntArgument(int value);
    // Where a float script argument is read from or written to: when bit
    // `bit` of `mask` is set and `*value` is 10004 to 10007, the float
    // variable it stands for, else `value`.
    float *ResolveFloatArgumentPtr(float *value, WORD mask, int bit);
    // The same for an int argument and the int variables.
    int *ResolveIntArgumentPtr(int *value, WORD mask, int bit);
    void SetPendingInterrupt(short interrupt)
    {
        this->pendingInterrupt = interrupt;
    }
    // Clears flag bit 0, as th06's.
    void ClearShown()
    {
        this->shown = 0;
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
    unsigned char unk_1da[0x2];
    // The script SetScript started, and the instruction Step runs next.
    AnmInstr *scriptStart;
    AnmInstr *scriptPos;
    // The entry SetSprite chose.
    SpriteData *spriteData;
    D3DXVECTOR3 moveStart;
    D3DXVECTOR3 moveEnd;
    D3DXVECTOR3 rotateStart;
    D3DXVECTOR3 rotateEnd;
    D3DXVECTOR3 scaleTween;
    D3DXVECTOR3 colorTween;
    // Added to pos when it is drawn.
    D3DXVECTOR3 posOffset;
    // The script frame of the last sprite change, as th06's spriteTime.
    int spriteTime;
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

// A vertex with a transformed position and texture coordinates, as in
// th06.
struct VertexTex1Xyzrwh
{
    D3DXVECTOR3 pos;
    float w;
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

// A sprite in an ANM file: its number and rectangle, in pixels, as in
// th06.
struct AnmSprite
{
    int id;
    float x;
    float y;
    float width;
    float height;
};

// An ANM file's header. The sprites' and then the scripts' offsets
// follow it.
struct AnmFile
{
    int numSprites;
    int numScripts;
    // Set to the slot it is loaded into.
    int textureIndex;
    int width;
    int height;
    int format;
    D3DCOLOR colorKey;
    // The image's file name, or "@" for a texture created blank.
    int nameOffset;
    // Set to the first sprite number.
    int spriteOffset;
    // The file name of an image to take the alpha from, or 0.
    int alphaNameOffset;
    // 2 in th07's files.
    int version;
    // The texture's resource priority.
    DWORD priority;
    // The image stored in the file, when hasImage is set. Not in th06.
    int imageOffset;
    BYTE hasImage;
    // Set on the first ANM of a file: its memory is the file's, which
    // ReleaseAnm frees.
    BYTE ownsData;
    unsigned char unk_36[0x2];
    // The offset of the next ANM in the same file, 0 for the last.
    int nextOffset;
    unsigned char unk_3c[0x4];
    int spriteOffsets[1];
};

// The ANMs loaded into one slot. Not in th06.
struct AnmSlot
{
    AnmFile *file;
    int spriteOffset;
    // How many slots, from this one, the file's ANMs take.
    int count;
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
    // Rotates (`x`, `y`) by the angle with `sine` and `cosine`, the other
    // way from th06, and moves it to (`cx`, `cy`), into `out`.
    void RotatePoint(float *out, float x, float y, float sine, float cosine, float cx, float cy);
    // Draws `vm` when it is not rotated. Not in th06 as such.
    int DrawUnrotated(Sprite *vm);
    // DrawUnrotated without rounding to pixels, as th06's DrawNoRound.
    int DrawNoRound(Sprite *vm);
    // Sets g_DiffuseQuad to `vm` projected from its 3D position, rotated,
    // its size scaled by how far g_Unknown134cd28 from it reaches on the
    // screen. Returns 0, or -1 when it is outside the depth range. Not in
    // th06.
    int ProjectSprite(Sprite *vm);
    // Draws `vm` at its 3D position projected to the screen with
    // ProjectSprite. Not in th06.
    int DrawProjected(Sprite *vm);
    // Sets g_DiffuseQuad to the vertex buffer's quad transformed by
    // `vm`'s matrix at +0x138 and its position, projected to the screen.
    // Remakes that matrix first from matrix, the scale and the rotations
    // while flag 3 or 2 is set and flag 15 is not. Not in th06.
    void ProjectSprite3D(Sprite *vm);
    // Draws `vm` in 3D, projected by ProjectSprite3D. Not in th06.
    int DrawProjected3D(Sprite *vm);
    // Draws `vm` in 3D with the vertex buffer's quad, as th06's Draw3D,
    // after the pending batch. The world matrix is the one ProjectSprite3D
    // makes, shaken.
    int Draw3D(Sprite *vm);
    // Copies surface `surface` to the back buffer at (`left`, `top`),
    // as in th06.
    void CopySurfaceToBackBuffer(int surface, int left, int top, int x, int y);
    // Copies the `width` by `height` rectangle at (`left`, `top`) of
    // surface `surface` to the back buffer at (`x`, `y`), as in th06.
    void CopySurfaceRectToBackBuffer(int surface, int x, int y, int left, int top, int width, int height);
    // Runs `vm`'s script for one frame, as th06's: the instructions due
    // (th07's set, with script variables, arithmetic, conditional jumps
    // and five eased interpolations timed by unk_48 and unk_84), then the
    // rotation, scale and texture scroll speeds, and counts the call in
    // unk_c. Returns 1 once the script has ended, else 0.
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
    // Adds g_DiffuseQuad to the batch with `vm`'s texture coordinates,
    // shaken, rounded to pixels with flag 1, with `vm`'s color unless
    // flag 2 is set, unless it is outside the viewport. Returns 0.
    int DrawQuad(Sprite *vm, int flags);
    // Sets the render states `vm` wants on the device, as in th06.
    void SetRenderStateForSprite(Sprite *vm);
    // Sets the blend mode and z writes `vm` wants through
    // App::SetRenderState, which flushes the batch first. Not in th06.
    void SetBlendModeForSprite(Sprite *vm);
    // Creates texture `index` from the image file at `path`, in the format
    // g_TextureFormats[`format`] (16 bits when forced), with `colorKey`
    // transparent, and keeps the file. Returns 0, or -1 on failure.
    int LoadTexture(int index, char *path, int format, D3DCOLOR colorKey);
    // Creates texture `index` from the pixels of `image`, in the format
    // g_TextureFormats[`format`] (16 bits when forced). Returns 0, or -1
    // on failure. Not in th06.
    int CreateTextureFromImage(int index, AnmImage *image, int format);
    // Replaces the alpha of texture `index` with the blue of the image
    // file at `path`. Returns 0, or -1 on failure.
    int LoadTextureAlpha(int index, char *path, int format, D3DCOLOR colorKey);
    // Creates texture `index` at `width` by `height` in the format
    // g_TextureFormats[`format`]. Returns 0.
    int CreateTexture(int index, int width, int height, int format);
    // Releases texture `index` and its file.
    void ReleaseTexture(int index);
    // Loads `anm` into slot `slot`, its sprites from `firstSprite` on.
    // Returns the number of sprites it takes, or -1 on failure. Not in
    // th06 as such.
    int LoadAnmSprites(int slot, AnmFile *anm, int firstSprite, int ownsData);
    // Copies `sprite` into sprite `index` and works out its texture
    // coordinates and size.
    void LoadSprite(int index, SpriteData *sprite);
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
    void BeginQuadBatch();
    // Draws the batched quads, if any. Not in th06.
    void FlushQuadBatch();
    // Adds `quad`, a triangle strip, to the batch as two triangles.
    // Returns 0. Not in th06.
    int AddQuadToBatch(VertexTex1DiffuseXyzrwh *quad);
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
        this->lastCameraSetupFlag = 0xff;
    }
    void ClearUnk8()
    {
        this->stepCount = 0;
        this->renderStateSetCount = 0;
        this->scriptStartCount = 0;
        this->batchDrawCount = 0;
    }
    void ResetUnk0()
    {
        this->modulateColorEnabled = 0;
        this->modulateColor = 0x80808080;
    }
    // Modulates sprites by `color` (sets unk_4).
    void SetUnk0(D3DCOLOR color)
    {
        this->modulateColorEnabled = 1;
        this->modulateColor = color;
    }
    void SetUnk2e4d4(int value)
    {
        this->lastCameraSetupFlag = value;
    }
    // Draws `vm`, then the batched quads.
    void DrawAndFlush(Sprite *vm)
    {
        this->Draw(vm);
        this->FlushQuadBatch();
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

    // Has CopyBackBufferToTexture's copy done after the frame is drawn,
    // as th06's, unless one is already pending: then returns -1, else 0.
    int SetPendingCopy(int texture, int srcLeft, int srcTop, int srcWidth, int srcHeight, int dstLeft, int dstTop,
                       int dstWidth, int dstHeight)
    {
        if (this->pendingCopyTexture >= 0)
        {
            return -1;
        }
        this->pendingCopyTexture = texture;
        this->pendingCopySrcLeft = srcLeft;
        this->pendingCopySrcTop = srcTop;
        this->pendingCopySrcWidth = srcWidth;
        this->pendingCopySrcHeight = srcHeight;
        this->pendingCopyDstLeft = dstLeft;
        this->pendingCopyDstTop = dstTop;
        this->pendingCopyDstWidth = dstWidth;
        this->pendingCopyDstHeight = dstHeight;
        return 0;
    }

    // The color sprites are modulated by while unk_4 is set, 0x80 per
    // channel for 1.
    D3DCOLOR modulateColor;
    DWORD modulateColorEnabled;
    // Counts the scripts SetScript started.
    DWORD scriptStartCount;
    DWORD stepCount;
    // Counts SetRenderStateForSprite's calls.
    DWORD renderStateSetCount;
    // Counts the batches FlushQuadBatch drew.
    DWORD batchDrawCount;
    // How far the screen is shaken, set by ScreenEffect's shake.
    float shakeX;
    float shakeY;
    // The world matrix ProjectSprite3D last projected with.
    D3DXMATRIX lastWorldMatrix;
    SpriteData sprites[0xa00];
    Sprite unk_28060;
    // The textures of the loaded ANM files, by slot.
    IDirect3DTexture8 *textures[0x108];
    // The image files the textures were created from, kept loaded.
    void *textureFiles[0x100];
    // The image file names of the ANMs loaded into each slot.
    char *anmImageNames[0x32];
    unsigned char unk_28b94[0x358];
    // Counts the sprites loaded, for SpriteData::spriteId.
    int spriteCount;
    // The scripts of the loaded ANM files.
    void *scripts[0xa00];
    // The first sprite number of each script's ANM file.
    int scriptSpriteOffsets[0xa00];
    AnmSlot anmSlots[0x32];
    // Images loaded as surfaces to copy to the back buffer from.
    IDirect3DSurface8 *surfaces[0x20];
    // System memory copies of them, to recreate them from.
    IDirect3DSurface8 *surfacesBackup[0x20];
    // The images' sizes and formats.
    D3DXIMAGE_INFO surfaceInfos[0x20];
    // The texture factor last set on the device.
    D3DCOLOR currentColor;
    // The texture last set on the device.
    IDirect3DTexture8 *currentTexture;
    // The states Draw last set, 0xff when unknown.
    BYTE blendMode;
    BYTE colorOp;
    BYTE vertexFormat;
    BYTE zWriteDisabled;
    // Flag bit 14 of the sprite last drawn: which of
    // g_Stage's two set-ups the device has.
    BYTE lastCameraSetupFlag;
    unsigned char unk_2e4d5[0x3];
    // The sprite data Draw last used, NULL when unknown.
    void *currentSpriteData;
    IDirect3DVertexBuffer8 *vertexBuffer;
    // The quad the vertex buffer holds, 256 pixels across.
    VertexTex1Xyz quad[4];
    // The number of quads batched to draw with FlushQuadBatch.
    int batchQuadCount;
    // The batched quads, two triangles each.
    VertexTex1DiffuseXyzrwh batchVertices[0xc000];
    // Where the next quad goes, and the first one not drawn yet.
    VertexTex1DiffuseXyzrwh *batchCursor;
    VertexTex1DiffuseXyzrwh *batchStart;
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
// The quad Draw draws with a color, without the vertex buffer.
extern VertexTex1DiffuseXyzrwh g_DiffuseQuad[4];
// A transformed quad without a color, as in th06.
extern VertexTex1Xyzrwh g_Quad[4];
// 0.5, for DrawQuad's rounding.
extern float g_Half;
// The offset from a sprite's position whose projection sets the size
// ProjectSprite draws it at.
extern D3DXVECTOR3 g_Unknown134cd28;
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

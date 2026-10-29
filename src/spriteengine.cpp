// The file with SpriteEngine, from 0x44d3e0.

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "sprite.h"
#include "unsorted.h"

SpriteEngine::SpriteEngine()
{
    int i;

    memset(this, 0, sizeof(*this));
    for (i = 0; i < 0xa00; i++)
    {
        this->sprites[i].sourceFileIndex = -1;
    }
    g_Quad[0].w = g_Quad[1].w = g_Quad[2].w = g_Quad[3].w = 1.0f;
    g_Quad[0].uv.x = 0.0f;
    g_Quad[0].uv.y = 0.0f;
    g_Quad[1].uv.x = 1.0f;
    g_Quad[1].uv.y = 0.0f;
    g_Quad[2].uv.x = 0.0f;
    g_Quad[2].uv.y = 1.0f;
    g_Quad[3].uv.x = 1.0f;
    g_Quad[3].uv.y = 1.0f;
    g_DiffuseQuad[0].w = g_DiffuseQuad[1].w = g_DiffuseQuad[2].w = g_DiffuseQuad[3].w = 1.0f;
    g_DiffuseQuad[0].uv.x = 0.0f;
    g_DiffuseQuad[0].uv.y = 0.0f;
    g_DiffuseQuad[1].uv.x = 1.0f;
    g_DiffuseQuad[1].uv.y = 0.0f;
    g_DiffuseQuad[2].uv.x = 0.0f;
    g_DiffuseQuad[2].uv.y = 1.0f;
    g_DiffuseQuad[3].uv.x = 1.0f;
    g_DiffuseQuad[3].uv.y = 1.0f;
    this->vertexBuffer = NULL;
    this->currentTexture = NULL;
    this->blendMode = 0;
    this->colorOp = 0;
    this->currentColor = 1;
    this->vertexFormat = 0;
    this->unk_2e4d4 = 0xff;
    this->zWriteDisabled = 0;
    this->pendingCopyTexture = -1;
}

void SpriteEngine::CreateQuad()
{
    VertexTex1Xyz *data;

    this->quad[0].pos.x = this->quad[2].pos.x = -128.0f;
    this->quad[1].pos.x = this->quad[3].pos.x = 128.0f;
    this->quad[0].pos.y = this->quad[1].pos.y = -128.0f;
    this->quad[2].pos.y = this->quad[3].pos.y = 128.0f;
    this->quad[0].pos.z = this->quad[1].pos.z = this->quad[2].pos.z = this->quad[3].pos.z = 0.0f;
    this->quad[0].uv.x = this->quad[2].uv.x = 0.0f;
    this->quad[1].uv.x = this->quad[3].uv.x = 1.0f;
    this->quad[0].uv.y = this->quad[1].uv.y = 0.0f;
    this->quad[2].uv.y = this->quad[3].uv.y = 1.0f;
    g_DiffuseQuadXyz[0].pos = this->quad[0].pos;
    g_DiffuseQuadXyz[1].pos = this->quad[1].pos;
    g_DiffuseQuadXyz[2].pos = this->quad[2].pos;
    g_DiffuseQuadXyz[3].pos = this->quad[3].pos;
    g_DiffuseQuadXyz[0].uv.x = this->quad[0].uv.x;
    g_DiffuseQuadXyz[0].uv.y = this->quad[0].uv.y;
    g_DiffuseQuadXyz[1].uv.x = this->quad[1].uv.x;
    g_DiffuseQuadXyz[1].uv.y = this->quad[1].uv.y;
    g_DiffuseQuadXyz[2].uv.x = this->quad[2].uv.x;
    g_DiffuseQuadXyz[2].uv.y = this->quad[2].uv.y;
    g_DiffuseQuadXyz[3].uv.x = this->quad[3].uv.x;
    g_DiffuseQuadXyz[3].uv.y = this->quad[3].uv.y;
    if (!g_App.flags.noVertexBuffer)
    {
        g_App.d3dDevice->CreateVertexBuffer(sizeof(this->quad), 0, D3DFVF_XYZ | D3DFVF_TEX1, D3DPOOL_MANAGED,
                                            &this->vertexBuffer);
        this->vertexBuffer->Lock(0, 0, (BYTE **)&data, 0);
        memcpy(data, this->quad, sizeof(this->quad));
        this->vertexBuffer->Unlock();
        g_App.d3dDevice->SetStreamSource(0, g_SpriteEngine->vertexBuffer, sizeof(VertexTex1Xyz));
    }
}

int SpriteEngine::LoadTexture(int index, char *path, int format, D3DCOLOR colorKey)
{
    void *data;

    this->ReleaseTexture(index);
    if (g_App.flags.force16BitTextures)
    {
        if (g_TextureFormats[format] == D3DFMT_A8R8G8B8 || g_TextureFormats[format] == D3DFMT_UNKNOWN)
        {
            format = 5;
        }
        else if (g_TextureFormats[format] == D3DFMT_R8G8B8)
        {
            format = 3;
        }
    }
    data = LoadFileData(path, 1);
    if (data == NULL)
    {
        return -1;
    }
    if (D3DXCreateTextureFromFileInMemoryEx(g_App.d3dDevice, data, g_FileSize, 0, 0, 0, 0, g_TextureFormats[format],
                                            D3DPOOL_MANAGED, D3DX_FILTER_LINEAR, D3DX_DEFAULT, colorKey, NULL, NULL,
                                            &this->textures[index]) != D3D_OK)
    {
        free(data);
        return -1;
    }
    this->textureFiles[index] = data;
    return 0;
}

int SpriteEngine::CreateTextureFromImage(int index, AnmImage *image, int format)
{
    BYTE *dstRow;
    BYTE *srcRow;
    int y;
    D3DLOCKED_RECT rect;
    AnmImage *img;
    IDirect3DSurface8 *level;
    IDirect3DSurface8 *srcSurface;

    this->ReleaseTexture(index);
    if (g_App.flags.force16BitTextures)
    {
        if (g_TextureFormats[format] == D3DFMT_A8R8G8B8 || g_TextureFormats[format] == D3DFMT_UNKNOWN)
        {
            format = 5;
        }
        else if (g_TextureFormats[format] == D3DFMT_R8G8B8)
        {
            format = 3;
        }
    }
    img = image;
    g_App.d3dDevice->CreateImageSurface(img->width, img->height, g_TextureFormats[img->format], &srcSurface);
    srcSurface->LockRect(&rect, NULL, 0);
    for (y = 0; y < img->height; y++)
    {
        dstRow = (BYTE *)rect.pBits + y * rect.Pitch;
        srcRow = &image->data[y * img->width * g_TextureFormatSizes[img->format]];
        memcpy(dstRow, srcRow, img->width * g_TextureFormatSizes[img->format]);
    }
    srcSurface->UnlockRect();
    if (D3DXCreateTexture(g_App.d3dDevice, img->width, img->height, 1, 0, g_TextureFormats[format], D3DPOOL_MANAGED,
                          &this->textures[index]) != D3D_OK)
    {
        return -1;
    }
    this->textures[index]->GetSurfaceLevel(0, &level);
    if (D3DXLoadSurfaceFromSurface(level, NULL, NULL, srcSurface, NULL, NULL, D3DX_FILTER_LINEAR, 0) != D3D_OK)
    {
        return -1;
    }
    if (srcSurface != NULL)
    {
        srcSurface->Release();
        srcSurface = NULL;
    }
    if (level != NULL)
    {
        level->Release();
        level = NULL;
    }
    return 0;
}

// Pixels in the texture formats LoadTextureAlpha handles.
struct PixelA8R8G8B8
{
    BYTE b;
    BYTE g;
    BYTE r;
    BYTE a;
};
struct PixelA1R5G5B5
{
    WORD b : 5;
    WORD g : 5;
    WORD r : 5;
    WORD a : 1;
};
struct PixelA4R4G4B4
{
    WORD b : 4;
    WORD g : 4;
    WORD r : 4;
    WORD a : 4;
};

int SpriteEngine::LoadTextureAlpha(int index, char *path, int format, D3DCOLOR colorKey)
{
    D3DSURFACE_DESC desc;
    void *fileData;
    D3DLOCKED_RECT dstRect;
    D3DLOCKED_RECT srcRect;
    IDirect3DTexture8 *srcTexture = NULL;

    fileData = LoadFileData(path, 0);
    if (fileData == NULL)
    {
        return -1;
    }
    this->textures[index]->GetLevelDesc(0, &desc);
    if (desc.Format != D3DFMT_A8R8G8B8 && desc.Format != D3DFMT_A4R4G4B4 && desc.Format != D3DFMT_A1R5G5B5)
    {
        LogError(&g_Log, "error : イメージがαを持っていません\r\n");
        goto fail;
    }
    if (D3DXCreateTextureFromFileInMemoryEx(g_App.d3dDevice, fileData, g_FileSize, 0, 0, 0, 0, desc.Format,
                                            D3DPOOL_SYSTEMMEM, D3DX_FILTER_LINEAR, D3DX_DEFAULT, colorKey, NULL, NULL,
                                            &srcTexture) != D3D_OK)
    {
        goto fail;
    }
    if (this->textures[index]->LockRect(0, &dstRect, NULL, 0) != D3D_OK)
    {
        goto fail;
    }
    if (srcTexture->LockRect(0, &srcRect, NULL, D3DLOCK_NO_DIRTY_UPDATE) != D3D_OK)
    {
        goto fail;
    }
    switch (desc.Format)
    {
    case D3DFMT_A8R8G8B8:
    {
        PixelA8R8G8B8 *dstPx;
        PixelA8R8G8B8 *srcPx;
        UINT py;
        UINT tx;

        for (py = 0; py < desc.Height; py++)
        {
            dstPx = (PixelA8R8G8B8 *)((BYTE *)dstRect.pBits + py * dstRect.Pitch);
            srcPx = (PixelA8R8G8B8 *)((BYTE *)srcRect.pBits + py * srcRect.Pitch);
            for (tx = 0; tx < desc.Width; tx++, srcPx++, dstPx++)
            {
                dstPx->a = srcPx->b;
            }
        }
        break;
    }
    case D3DFMT_A1R5G5B5:
    {
        PixelA1R5G5B5 *dstPx;
        PixelA1R5G5B5 *srcPx;
        UINT py;
        UINT tx;

        for (py = 0; py < desc.Height; py++)
        {
            dstPx = (PixelA1R5G5B5 *)((BYTE *)dstRect.pBits + py * dstRect.Pitch);
            srcPx = (PixelA1R5G5B5 *)((BYTE *)srcRect.pBits + py * srcRect.Pitch);
            for (tx = 0; tx < desc.Width; tx++, srcPx++, dstPx++)
            {
                dstPx->a = srcPx->b >> 4;
            }
        }
        break;
    }
    case D3DFMT_A4R4G4B4:
    {
        PixelA4R4G4B4 *dstPx;
        PixelA4R4G4B4 *srcPx;
        UINT py;
        UINT tx;

        for (py = 0; py < desc.Height; py++)
        {
            dstPx = (PixelA4R4G4B4 *)((BYTE *)dstRect.pBits + py * dstRect.Pitch);
            srcPx = (PixelA4R4G4B4 *)((BYTE *)srcRect.pBits + py * srcRect.Pitch);
            for (tx = 0; tx < desc.Width; tx++, srcPx++, dstPx++)
            {
                dstPx->a = srcPx->b;
            }
        }
        break;
    }
    }
    srcTexture->UnlockRect(0);
    this->textures[index]->UnlockRect(0);
    if (srcTexture != NULL)
    {
        srcTexture->Release();
        srcTexture = NULL;
    }
    free(fileData);
    return 0;

fail:
    if (srcTexture != NULL)
    {
        srcTexture->Release();
        srcTexture = NULL;
    }
    free(fileData);
    return -1;
}

int SpriteEngine::CreateTexture(int index, int width, int height, int format)
{
    D3DXCreateTexture(g_App.d3dDevice, width, height, 1, 0, g_TextureFormats[format], D3DPOOL_MANAGED,
                      &this->textures[index]);
    return 0;
}

int SpriteEngine::LoadAnm(int slot, char *path, int firstSprite)
{
    AnmFile *anmFile;
    int ownsData;
    int firstSlot;
    int count;

    anmFile = (AnmFile *)LoadFileData(path, 0);
    ownsData = 1;
    firstSlot = slot;
    if (anmFile == NULL)
    {
        LogError(&g_Log, "アニメが読み込めません。データが失われてるか壊れています\r\n");
        return -1;
    }
    while (1)
    {
        count = this->LoadAnmSprites(slot, anmFile, firstSprite, ownsData);
        if (count < 0)
        {
            this->anmSlots[firstSlot].count = slot - firstSlot;
            return count;
        }
        slot++;
        if (anmFile->nextOffset == 0)
        {
            this->anmSlots[firstSlot].count = slot - firstSlot;
            return 0;
        }
        anmFile = (AnmFile *)((BYTE *)anmFile + anmFile->nextOffset);
        ownsData = 0;
        firstSprite += count;
    }
}

int SpriteEngine::LoadAnmSprites(int slot, AnmFile *anm, int firstSprite, int ownsData)
{
    SpriteData data;
    int *entries;
    int k;
    AnmSprite *rawSprite;
    char *name;
    D3DSURFACE_DESC desc;
    AnmFile *anmHeader;
    int highest;

    highest = 0;
    if (anm == NULL)
    {
        LogError(&g_Log, "アニメが読み込めません。データが失われてるか壊れています\r\n");
        return -1;
    }
    if (slot >= 0x32)
    {
        LogError(&g_Log, "テクスチャ格納先が足りません\r\n");
        return -1;
    }
    this->ReleaseAnm(slot);
    anmHeader = anm;
    if (anmHeader->version != 2)
    {
        LogError(&g_Log, "アニメのバージョンが違います\r\n");
        return -1;
    }
    anmHeader->textureIndex = slot;
    anmHeader->ownsData = ownsData;
    if (!anmHeader->hasImage)
    {
        name = (char *)anmHeader + anmHeader->nameOffset;
        if (*name == '@')
        {
            this->CreateTexture(anmHeader->textureIndex, anmHeader->width, anmHeader->height, anmHeader->format);
        }
        else if (this->LoadTexture(anmHeader->textureIndex, name, anmHeader->format, anmHeader->colorKey) != 0)
        {
            LogError(&g_Log, "テクスチャ %s が読み込めません。データが失われてるか壊れています\r\n", name);
            return -1;
        }
        if (anmHeader->alphaNameOffset != 0)
        {
            name = (char *)anmHeader + anmHeader->alphaNameOffset;
            if (this->LoadTextureAlpha(anmHeader->textureIndex, name, anmHeader->format, anmHeader->colorKey) != 0)
            {
                LogError(&g_Log, "テクスチャ %s が読み込めません。データが失われてるか壊れています\r\n", name);
                return -1;
            }
        }
    }
    else if (this->CreateTextureFromImage(anmHeader->textureIndex, (AnmImage *)((BYTE *)anmHeader + anmHeader->imageOffset),
                              anmHeader->format) != 0)
    {
        LogError(&g_Log, "テクスチャが読み込めません。データが失われてるか壊れています\r\n");
        return -1;
    }
    this->anmImageNames[slot] = (char *)anmHeader + anmHeader->nameOffset;
    this->textures[slot]->SetPriority(anmHeader->priority);
    this->textures[slot]->PreLoad();
    this->textures[slot]->GetLevelDesc(0, &desc);
    anmHeader->spriteOffset = firstSprite;
    entries = anmHeader->spriteOffsets;
    for (k = 0; k < anmHeader->numSprites; k++, entries++)
    {
        rawSprite = (AnmSprite *)((char *)anmHeader + *entries);
        data.sourceFileIndex = anmHeader->textureIndex;
        data.scaleX = (float)desc.Width / anmHeader->width;
        data.scaleY = (float)desc.Height / anmHeader->height;
        data.startX = data.scaleX * rawSprite->x;
        data.startY = data.scaleY * rawSprite->y;
        data.endX = (rawSprite->x + rawSprite->width) * data.scaleX;
        data.endY = (rawSprite->y + rawSprite->height) * data.scaleY;
        data.textureWidth = desc.Width;
        data.textureHeight = desc.Height;
        if (highest < rawSprite->id)
        {
            highest = rawSprite->id;
        }
        if (rawSprite->id + firstSprite >= 0xa00)
        {
            LogError(&g_Log, "スプライトが格納できません。テーブルが不足しています\r\n");
            return -1;
        }
        this->LoadSprite(rawSprite->id + firstSprite, &data);
    }
    for (k = 0; k < anmHeader->numScripts; k++, entries += 2)
    {
        if (entries[0] + firstSprite >= 0xa00)
        {
            LogError(&g_Log, "アニメが格納できません。テーブルが不足しています\r\n");
            return -1;
        }
        if (highest < entries[0])
        {
            highest = entries[0];
        }
        this->scripts[entries[0] + firstSprite] = (char *)anmHeader + entries[1];
        this->scriptSpriteOffsets[entries[0] + firstSprite] = firstSprite;
    }
    this->anmSlots[slot].file = anmHeader;
    this->anmSlots[slot].spriteOffset = firstSprite;
    return highest + 1;
}

void SpriteEngine::ReleaseAnm(int slot)
{
    AnmFile *header;
    int *entries;
    int sub;
    int k;
    int startSprite;
    AnmSprite *rawSprite;

    if (slot < 0 || slot >= ARRAY_SIZE(this->anmSlots))
    {
        return;
    }
    if (this->anmSlots[slot].file != NULL)
    {
        entries = this->anmSlots[slot].file->spriteOffsets;
        startSprite = this->anmSlots[slot].spriteOffset;
        header = this->anmSlots[slot].file;
        for (sub = slot + 1, k = 1; k < this->anmSlots[slot].count; k++, sub++)
        {
            this->ReleaseAnm(sub);
        }
        for (k = 0; k < header->numSprites; k++, entries++)
        {
            rawSprite = (AnmSprite *)((char *)header + *entries);
            memset(&this->sprites[rawSprite->id + startSprite], 0, sizeof(SpriteData));
            this->sprites[rawSprite->id + startSprite].sourceFileIndex = -1;
        }
        for (k = 0; k < header->numScripts; k++, entries += 2)
        {
            this->scripts[entries[0] + startSprite] = NULL;
            this->scriptSpriteOffsets[entries[0] + startSprite] = 0;
        }
        this->anmSlots[slot].spriteOffset = 0;
        this->ReleaseTexture(header->textureIndex);
        if (header->ownsData)
        {
            free(header);
        }
        this->anmSlots[slot].file = NULL;
        this->ClearBlendMode();
        this->ClearColorOp();
        this->vertexFormat = 0;
        this->ClearTexture();
        this->anmSlots[slot].count = 0;
    }
}

void SpriteEngine::ReleaseTexture(int index)
{
    if (index < 0 || index >= ARRAY_SIZE(this->textures))
    {
        return;
    }
    if (this->textures[index] != NULL)
    {
        this->textures[index]->Release();
        this->textures[index] = NULL;
    }
    MemFree(this->textureFiles[index]);
    this->textureFiles[index] = NULL;
}

void SpriteEngine::LoadSprite(int index, SpriteData *sprite)
{
    this->sprites[index] = *sprite;
    this->sprites[index].spriteId = this->spriteCount++;
    // The comma operators reproduce the original's operand order: it
    // loads the texture size first (fdivr).
    this->sprites[index].uvStartX = this->sprites[index].startX / (0, this->sprites[index].textureWidth);
    this->sprites[index].uvEndX = this->sprites[index].endX / (0, this->sprites[index].textureWidth);
    this->sprites[index].uvStartY = this->sprites[index].startY / (0, this->sprites[index].textureHeight);
    this->sprites[index].uvEndY = this->sprites[index].endY / (0, this->sprites[index].textureHeight);
    this->sprites[index].width = (this->sprites[index].endX - this->sprites[index].startX) / sprite->scaleX;
    this->sprites[index].height = (this->sprites[index].endY - this->sprites[index].startY) / sprite->scaleY;
}

int SpriteEngine::SetSprite(Sprite *vm, int sprite)
{
    if (this->sprites[sprite].sourceFileIndex < 0)
    {
        return -1;
    }
    vm->spriteIndex = sprite;
    vm->spriteData = &this->sprites[sprite];
    D3DXMatrixIdentity(&vm->matrix);
    D3DXMatrixIdentity(&vm->unk_178);
    if (vm->spriteData->scaleX < 1.0f)
    {
        sprite = 0;
    }
    vm->matrix.m[0][0] = vm->spriteData->width / 256.0f;
    vm->matrix.m[1][1] = vm->spriteData->height / 256.0f;
    vm->unk_178.m[0][0] = vm->spriteData->width / vm->spriteData->textureWidth * vm->spriteData->scaleX;
    vm->unk_178.m[1][1] = vm->spriteData->height / vm->spriteData->textureHeight * vm->spriteData->scaleY;
    vm->unk_138 = vm->matrix;
    return 0;
}

void SpriteEngine::SetScript(Sprite *vm, void *script)
{
    if (script == NULL)
    {
        memset(vm, 0, sizeof(Sprite));
    }
    else
    {
        vm->flags &= ~0x300;
        vm->Initialize();
        vm->scriptStart = (AnmInstr *)script;
        vm->scriptPos = vm->scriptStart;
        vm->unk_30.Set(0);
        vm->flags &= ~1;
        this->Step(vm);
        this->unk_8++;
    }
}

// Scales byte `i` of `color` by byte `i` of `factor`, 0x80 for 1, up to
// 0xff.
#define MODULATE_CHANNEL(color, factor, i)                                          \
    {                                                                               \
        DWORD channel = (DWORD)((BYTE *)&(color))[i] * ((BYTE *)&(factor))[i] >> 7; \
        if (channel >= 0x100)                                                       \
        {                                                                           \
            channel = 0xff;                                                         \
        }                                                                           \
        ((BYTE *)&(color))[i] = channel;                                            \
    }

void SpriteEngine::SetRenderStateForSprite(Sprite *vm)
{
    D3DCOLOR color;

    if (this->blendMode != (vm->flags >> 4 & 1))
    {
        this->blendMode = vm->flags >> 4 & 1;
        if (this->blendMode == 0)
        {
            g_App.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        }
        else
        {
            g_App.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        }
    }
    color = (vm->flags >> 16 & 1) ? vm->color2 : vm->color;
    if (!g_App.flags.noVertexBuffer)
    {
        if (this->unk_4 != 0)
        {
            MODULATE_CHANNEL(color, this->unk_0, 2);
            MODULATE_CHANNEL(color, this->unk_0, 1);
            MODULATE_CHANNEL(color, this->unk_0, 0);
            MODULATE_CHANNEL(color, this->unk_0, 3);
        }
        if (this->currentColor != color)
        {
            this->currentColor = color;
            g_App.d3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, this->currentColor);
        }
    }
    else
    {
        if (this->unk_4 != 0)
        {
            MODULATE_CHANNEL(color, this->unk_0, 2);
            MODULATE_CHANNEL(color, this->unk_0, 1);
            MODULATE_CHANNEL(color, this->unk_0, 0);
            MODULATE_CHANNEL(color, this->unk_0, 3);
        }
        g_DiffuseQuad[0].diffuse = color;
        g_DiffuseQuad[1].diffuse = color;
        g_DiffuseQuad[2].diffuse = color;
        g_DiffuseQuad[3].diffuse = color;
        g_DiffuseQuadXyz[0].diffuse = color;
        g_DiffuseQuadXyz[1].diffuse = color;
        g_DiffuseQuadXyz[2].diffuse = color;
        g_DiffuseQuadXyz[3].diffuse = color;
    }
    if (!g_App.flags.noDepthTest && this->zWriteDisabled != (vm->flags >> 12 & 1))
    {
        this->zWriteDisabled = vm->flags >> 12 & 1;
        if (this->zWriteDisabled == 0)
        {
            g_App.d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
        }
        else
        {
            g_App.d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        }
    }
    if (this->unk_2e4d4 != (vm->flags >> 14 & 1))
    {
        g_SpriteEngine->FlushQuadBatch();
        this->unk_2e4d4 = vm->flags >> 14 & 1;
        if (this->unk_2e4d4 == 0)
        {
            g_Unknown1347b00.SetupFixedCamera();
            g_App.d3dDevice->SetViewport(&g_App.viewport);
        }
        else
        {
            g_Unknown1347b00.SetupCamera();
            g_App.d3dDevice->SetViewport(&g_App.viewport);
        }
    }
    this->unk_10++;
}

void SpriteEngine::SetBlendModeForSprite(Sprite *vm)
{
    if (this->blendMode != (vm->flags >> 4 & 1))
    {
        this->blendMode = vm->flags >> 4 & 1;
        if (this->blendMode == 0)
        {
            g_App.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        }
        else
        {
            g_App.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        }
    }
    if (!g_App.flags.noDepthTest && this->zWriteDisabled != (vm->flags >> 12 & 1))
    {
        this->zWriteDisabled = vm->flags >> 12 & 1;
        if (this->zWriteDisabled == 0)
        {
            g_App.SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
        }
        else
        {
            g_App.SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        }
    }
    this->unk_10++;
}

int SpriteEngine::DrawQuad(Sprite *vm, int flags)
{
    float bottom;
    float minY;
    float minX;
    float maxX;
    D3DCOLOR diffuse;

    g_DiffuseQuad[0].pos.x += this->shakeX;
    g_DiffuseQuad[0].pos.y += this->shakeY;
    g_DiffuseQuad[1].pos.x += this->shakeX;
    g_DiffuseQuad[1].pos.y += this->shakeY;
    g_DiffuseQuad[2].pos.x += this->shakeX;
    g_DiffuseQuad[2].pos.y += this->shakeY;
    g_DiffuseQuad[3].pos.x += this->shakeX;
    g_DiffuseQuad[3].pos.y += this->shakeY;
    if (flags & 1)
    {
        __asm
        {
            fld dword ptr [g_DiffuseQuad + 0x00]
            frndint
            fsub g_Half
            fld dword ptr [g_DiffuseQuad + 0x1c]
            frndint
            fsub g_Half
            fld dword ptr [g_DiffuseQuad + 0x04]
            frndint
            fsub g_Half
            fld dword ptr [g_DiffuseQuad + 0x3c]
            frndint
            fsub g_Half
            fst dword ptr [g_DiffuseQuad + 0x3c]
            fstp dword ptr [g_DiffuseQuad + 0x58]
            fst dword ptr [g_DiffuseQuad + 0x04]
            fstp dword ptr [g_DiffuseQuad + 0x20]
            fst dword ptr [g_DiffuseQuad + 0x1c]
            fstp dword ptr [g_DiffuseQuad + 0x54]
            fst dword ptr [g_DiffuseQuad + 0x00]
            fstp dword ptr [g_DiffuseQuad + 0x38]
        }
    }
    g_DiffuseQuad[0].uv.x = g_DiffuseQuad[2].uv.x = vm->spriteData->uvStartX + vm->uvScrollX;
    g_DiffuseQuad[1].uv.x = g_DiffuseQuad[3].uv.x = vm->spriteData->uvEndX + vm->uvScrollX;
    g_DiffuseQuad[0].uv.y = g_DiffuseQuad[1].uv.y = vm->spriteData->uvStartY + vm->uvScrollY;
    g_DiffuseQuad[2].uv.y = g_DiffuseQuad[3].uv.y = vm->spriteData->uvEndY + vm->uvScrollY;
    maxX = g_DiffuseQuad[0].pos.x > g_DiffuseQuad[1].pos.x ? g_DiffuseQuad[0].pos.x : g_DiffuseQuad[1].pos.x;
    maxX = g_DiffuseQuad[2].pos.x > maxX ? g_DiffuseQuad[2].pos.x : maxX;
    maxX = g_DiffuseQuad[3].pos.x > maxX ? g_DiffuseQuad[3].pos.x : maxX;
    bottom = g_DiffuseQuad[0].pos.y > g_DiffuseQuad[1].pos.y ? g_DiffuseQuad[0].pos.y : g_DiffuseQuad[1].pos.y;
    bottom = g_DiffuseQuad[2].pos.y > bottom ? g_DiffuseQuad[2].pos.y : bottom;
    bottom = g_DiffuseQuad[3].pos.y > bottom ? g_DiffuseQuad[3].pos.y : bottom;
    minX = g_DiffuseQuad[0].pos.x < g_DiffuseQuad[1].pos.x ? g_DiffuseQuad[0].pos.x : g_DiffuseQuad[1].pos.x;
    minX = g_DiffuseQuad[2].pos.x < minX ? g_DiffuseQuad[2].pos.x : minX;
    minX = g_DiffuseQuad[3].pos.x < minX ? g_DiffuseQuad[3].pos.x : minX;
    minY = g_DiffuseQuad[0].pos.y < g_DiffuseQuad[1].pos.y ? g_DiffuseQuad[0].pos.y : g_DiffuseQuad[1].pos.y;
    minY = g_DiffuseQuad[2].pos.y < minY ? g_DiffuseQuad[2].pos.y : minY;
    minY = g_DiffuseQuad[3].pos.y < minY ? g_DiffuseQuad[3].pos.y : minY;
    if (g_App.viewport.X > maxX || g_App.viewport.Y > bottom || g_App.viewport.X + g_App.viewport.Width < minX ||
        g_App.viewport.Y + g_App.viewport.Height < minY)
    {
        return 0;
    }
    if (this->currentTexture != this->textures[vm->spriteData->sourceFileIndex])
    {
        this->currentTexture = this->textures[vm->spriteData->sourceFileIndex];
        this->FlushQuadBatch();
        g_App.d3dDevice->SetTexture(0, this->currentTexture);
    }
    if (this->vertexFormat != 1)
    {
        this->FlushQuadBatch();
        this->vertexFormat = 1;
    }
    if ((flags & 2) == 0)
    {
        diffuse = (vm->flags >> 16 & 1) ? vm->color2 : vm->color;
        if (this->unk_4 != 0)
        {
            MODULATE_CHANNEL(diffuse, this->unk_0, 2);
            MODULATE_CHANNEL(diffuse, this->unk_0, 1);
            MODULATE_CHANNEL(diffuse, this->unk_0, 0);
            MODULATE_CHANNEL(diffuse, this->unk_0, 3);
        }
        g_DiffuseQuad[0].diffuse = diffuse;
        g_DiffuseQuad[1].diffuse = diffuse;
        g_DiffuseQuad[2].diffuse = diffuse;
        g_DiffuseQuad[3].diffuse = diffuse;
    }
    this->SetBlendModeForSprite(vm);
    this->AddQuadToBatch(g_DiffuseQuad);
    return 0;
}

void SpriteEngine::BeginQuadBatch()
{
    this->batchQuadCount = 0;
    this->batchCursor = this->batchVertices;
    this->batchStart = this->batchCursor;
}

void SpriteEngine::FlushQuadBatch()
{
    if (this->batchQuadCount == 0)
    {
        return;
    }
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    g_App.d3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
    g_App.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, this->batchQuadCount * 2, this->batchStart,
                                     sizeof(VertexTex1DiffuseXyzrwh));
    this->batchStart = this->batchCursor;
    this->batchQuadCount = 0;
    this->unk_14++;
}

int SpriteEngine::AddQuadToBatch(VertexTex1DiffuseXyzrwh *quad)
{
    this->batchCursor[0] = quad[0];
    this->batchCursor[1] = quad[1];
    this->batchCursor[2] = quad[2];
    this->batchCursor[3] = quad[1];
    this->batchCursor[4] = quad[2];
    this->batchCursor[5] = quad[3];
    this->batchCursor += 6;
    this->batchQuadCount++;
    return 0;
}

int SpriteEngine::DrawUnrotated(Sprite *vm)
{
    float w;
    float h;

    if ((vm->flags & 1) == 0)
    {
        return -1;
    }
    if ((vm->flags >> 1 & 1) == 0)
    {
        return -1;
    }
    if (vm->alpha == 0)
    {
        return -1;
    }
    w = vm->spriteData->width * vm->scaleY / 2.0f;
    h = vm->spriteData->height * vm->scaleX / 2.0f;
    if (((vm->flags >> 10 & 3) & 1) == 0)
    {
        g_DiffuseQuad[0].pos.x = g_DiffuseQuad[2].pos.x = vm->pos.x - w;
        g_DiffuseQuad[1].pos.x = g_DiffuseQuad[3].pos.x = w + vm->pos.x;
    }
    else
    {
        g_DiffuseQuad[0].pos.x = g_DiffuseQuad[2].pos.x = vm->pos.x;
        g_DiffuseQuad[1].pos.x = g_DiffuseQuad[3].pos.x = w + vm->pos.x + w;
    }
    if (((vm->flags >> 10 & 3) & 2) == 0)
    {
        g_DiffuseQuad[0].pos.y = g_DiffuseQuad[1].pos.y = vm->pos.y - h;
        g_DiffuseQuad[2].pos.y = g_DiffuseQuad[3].pos.y = h + vm->pos.y;
    }
    else
    {
        g_DiffuseQuad[0].pos.y = g_DiffuseQuad[1].pos.y = vm->pos.y;
        g_DiffuseQuad[2].pos.y = g_DiffuseQuad[3].pos.y = h + vm->pos.y + h;
    }
    g_DiffuseQuad[0].pos.z = g_DiffuseQuad[1].pos.z = g_DiffuseQuad[2].pos.z = g_DiffuseQuad[3].pos.z = vm->pos.z;
    return this->DrawQuad(vm, 1);
}

void SpriteEngine::RotatePoint(float *out, float x, float y, float sine, float cosine, float cx, float cy)
{
    out[0] = x * cosine - y * sine + cx;
    out[1] = x * sine + y * cosine + cy;
}

int SpriteEngine::Draw(Sprite *vm)
{
    float x0;
    float cy;
    float halfH;
    float halfWidth;
    float cosine;
    float angle;
    float s;

    if (vm->rotationZ == 0.0f)
    {
        return this->DrawUnrotated(vm);
    }
    if ((vm->flags & 1) == 0)
    {
        return -1;
    }
    if ((vm->flags >> 1 & 1) == 0)
    {
        return -1;
    }
    if (vm->alpha == 0)
    {
        return -1;
    }
    angle = vm->rotationZ;
    __asm
    {
        fld angle
        fsincos
        fstp cosine
        fstp s
    }
    x0 = vm->pos.x;
    cy = vm->pos.y;
    halfWidth = vm->spriteData->width * vm->scaleY / 2.0f;
    halfH = vm->spriteData->height * vm->scaleX / 2.0f;
    this->RotatePoint((float *)&g_DiffuseQuad[0].pos, -halfWidth, -halfH, s, cosine, x0, cy);
    this->RotatePoint((float *)&g_DiffuseQuad[1].pos, halfWidth, -halfH, s, cosine, x0, cy);
    this->RotatePoint((float *)&g_DiffuseQuad[2].pos, -halfWidth, halfH, s, cosine, x0, cy);
    this->RotatePoint((float *)&g_DiffuseQuad[3].pos, halfWidth, halfH, s, cosine, x0, cy);
    g_DiffuseQuad[0].pos.z = g_DiffuseQuad[1].pos.z = g_DiffuseQuad[2].pos.z = g_DiffuseQuad[3].pos.z = vm->pos.z;
    if ((vm->flags >> 10 & 3) & 1)
    {
        g_DiffuseQuad[0].pos.x += halfWidth;
        g_DiffuseQuad[1].pos.x += halfWidth;
        g_DiffuseQuad[2].pos.x += halfWidth;
        g_DiffuseQuad[3].pos.x += halfWidth;
    }
    if ((vm->flags >> 10 & 3) & 2)
    {
        g_DiffuseQuad[0].pos.y += halfH;
        g_DiffuseQuad[1].pos.y += halfH;
        g_DiffuseQuad[2].pos.y += halfH;
        g_DiffuseQuad[3].pos.y += halfH;
    }
    return this->DrawQuad(vm, 0);
}

int SpriteEngine::DrawNoRound(Sprite *vm)
{
    float w;
    float h;

    if ((vm->flags & 1) == 0)
    {
        return -1;
    }
    if ((vm->flags >> 1 & 1) == 0)
    {
        return -1;
    }
    if (vm->alpha == 0)
    {
        return -1;
    }
    w = vm->spriteData->width * vm->scaleY / 2.0f;
    h = vm->spriteData->height * vm->scaleX / 2.0f;
    if (((vm->flags >> 10 & 3) & 1) == 0)
    {
        g_DiffuseQuad[0].pos.x = g_DiffuseQuad[2].pos.x = vm->pos.x - w;
        g_DiffuseQuad[1].pos.x = g_DiffuseQuad[3].pos.x = w + vm->pos.x;
    }
    else
    {
        g_DiffuseQuad[0].pos.x = g_DiffuseQuad[2].pos.x = vm->pos.x;
        g_DiffuseQuad[1].pos.x = g_DiffuseQuad[3].pos.x = w + vm->pos.x + w;
    }
    if (((vm->flags >> 10 & 3) & 2) == 0)
    {
        g_DiffuseQuad[0].pos.y = g_DiffuseQuad[1].pos.y = vm->pos.y - h;
        g_DiffuseQuad[2].pos.y = g_DiffuseQuad[3].pos.y = h + vm->pos.y;
    }
    else
    {
        g_DiffuseQuad[0].pos.y = g_DiffuseQuad[1].pos.y = vm->pos.y;
        g_DiffuseQuad[2].pos.y = g_DiffuseQuad[3].pos.y = h + vm->pos.y + h;
    }
    g_DiffuseQuad[0].pos.z = g_DiffuseQuad[1].pos.z = g_DiffuseQuad[2].pos.z = g_DiffuseQuad[3].pos.z = vm->pos.z;
    return this->DrawQuad(vm, 0);
}

int SpriteEngine::ProjectSprite(Sprite *vm)
{
    D3DXVECTOR3 o;
    float c;
    D3DXVECTOR3 d;
    D3DXVECTOR3 projPos;
    D3DXVECTOR3 edge;
    float angle;
    D3DXMATRIX world;
    float s;
    float x;
    float y;
    float h;
    float w;

    angle = vm->rotationZ;
    __asm
    {
        fld angle
        fsincos
        fstp c
        fstp s
    }
    o.x = 0.0f;
    o.y = 0.0f;
    o.z = 0.0f;
    D3DXMatrixIdentity(&world);
    world._41 = vm->pos.x;
    world._42 = vm->pos.y;
    world._43 = vm->pos.z;
    D3DXVec3Project(&projPos, &o, &g_App.viewport, &g_App.projectionMatrix, &g_App.viewMatrix, &world);
    if (projPos.z < 0.0f || projPos.z > 1.0f)
    {
        return -1;
    }
    D3DXVec3Project(&edge, &g_Unknown134cd28, &g_App.viewport, &g_App.projectionMatrix, &g_App.viewMatrix, &world);
    d = edge - projPos;
    x = D3DXVec3Length(&d) * 0.5f;
    w = x * vm->spriteData->width * vm->scaleY;
    h = x * vm->spriteData->height * vm->scaleX;
    x = projPos.x;
    y = projPos.y;
    this->RotatePoint((float *)&g_DiffuseQuad[0].pos, -w, -h, s, c, x, y);
    this->RotatePoint((float *)&g_DiffuseQuad[1].pos, w, -h, s, c, x, y);
    this->RotatePoint((float *)&g_DiffuseQuad[2].pos, -w, h, s, c, x, y);
    this->RotatePoint((float *)&g_DiffuseQuad[3].pos, w, h, s, c, x, y);
    g_DiffuseQuad[0].pos.z = g_DiffuseQuad[1].pos.z = g_DiffuseQuad[2].pos.z = g_DiffuseQuad[3].pos.z = projPos.z;
    if ((vm->flags >> 10 & 3) & 1)
    {
        g_DiffuseQuad[0].pos.x += w;
        g_DiffuseQuad[1].pos.x += w;
        g_DiffuseQuad[2].pos.x += w;
        g_DiffuseQuad[3].pos.x += w;
    }
    if ((vm->flags >> 10 & 3) & 2)
    {
        g_DiffuseQuad[0].pos.y += h;
        g_DiffuseQuad[1].pos.y += h;
        g_DiffuseQuad[2].pos.y += h;
        g_DiffuseQuad[3].pos.y += h;
    }
    return 0;
}

int SpriteEngine::DrawProjected(Sprite *vm)
{
    if ((vm->flags & 1) == 0)
    {
        return -1;
    }
    if ((vm->flags >> 1 & 1) == 0)
    {
        return -1;
    }
    if (vm->alpha == 0)
    {
        return -1;
    }
    if (this->ProjectSprite(vm) != 0)
    {
        return -1;
    }
    return this->DrawQuad(vm, 0);
}

void SpriteEngine::ProjectSprite3D(Sprite *vm)
{
    D3DXMATRIX worldMatrix;
    D3DXMATRIX rotationMatrix;

    if ((vm->flags >> 15 & 1) == 0 && ((vm->flags >> 3 & 1) != 0 || (vm->flags >> 2 & 1) != 0))
    {
        vm->unk_138 = vm->matrix;
        vm->unk_138._11 *= vm->scaleY;
        vm->unk_138._22 *= vm->scaleX;
        vm->flags &= ~8;
        if (vm->rotationX != 0.0)
        {
            D3DXMatrixRotationX(&rotationMatrix, vm->rotationX);
            D3DXMatrixMultiply(&vm->unk_138, &vm->unk_138, &rotationMatrix);
        }
        if (vm->rotationY != 0.0)
        {
            D3DXMatrixRotationY(&rotationMatrix, vm->rotationY);
            D3DXMatrixMultiply(&vm->unk_138, &vm->unk_138, &rotationMatrix);
        }
        if (vm->rotationZ != 0.0)
        {
            D3DXMatrixRotationZ(&rotationMatrix, vm->rotationZ);
            D3DXMatrixMultiply(&vm->unk_138, &vm->unk_138, &rotationMatrix);
        }
        vm->flags &= ~4;
    }
    worldMatrix = vm->unk_138;
    if (((vm->flags >> 10 & 3) & 1) == 0)
    {
        worldMatrix._41 = vm->pos.x;
    }
    else
    {
        worldMatrix._41 = fabsf(vm->spriteData->width * vm->scaleY / 2.0f) + vm->pos.x;
    }
    if (((vm->flags >> 10 & 3) & 2) == 0)
    {
        worldMatrix._42 = vm->pos.y;
    }
    else
    {
        worldMatrix._42 = fabsf(vm->spriteData->height * vm->scaleX / 2.0f) + vm->pos.y;
    }
    worldMatrix._43 = vm->pos.z;
    D3DXVec3Project(&g_DiffuseQuad[0].pos, &this->quad[0].pos, &g_App.viewport, &g_App.projectionMatrix,
                    &g_App.viewMatrix, &worldMatrix);
    D3DXVec3Project(&g_DiffuseQuad[1].pos, &this->quad[1].pos, &g_App.viewport, &g_App.projectionMatrix,
                    &g_App.viewMatrix, &worldMatrix);
    D3DXVec3Project(&g_DiffuseQuad[2].pos, &this->quad[2].pos, &g_App.viewport, &g_App.projectionMatrix,
                    &g_App.viewMatrix, &worldMatrix);
    D3DXVec3Project(&g_DiffuseQuad[3].pos, &this->quad[3].pos, &g_App.viewport, &g_App.projectionMatrix,
                    &g_App.viewMatrix, &worldMatrix);
    this->unk_20 = worldMatrix;
}

int SpriteEngine::DrawProjected3D(Sprite *vm)
{
    if ((vm->flags & 1) == 0)
    {
        return -1;
    }
    if ((vm->flags >> 1 & 1) == 0)
    {
        return -1;
    }
    if (vm->alpha == 0)
    {
        return -1;
    }
    this->ProjectSprite3D(vm);
    return this->DrawQuad(vm, 0);
}

int SpriteEngine::Draw3D(Sprite *vm)
{
    D3DXMATRIX worldMatrix;
    D3DXMATRIX rotationMatrix;
    D3DXMATRIX textureMatrix;

    if ((vm->flags & 1) == 0)
    {
        return -1;
    }
    if ((vm->flags >> 1 & 1) == 0)
    {
        return -1;
    }
    if (vm->alpha == 0)
    {
        return -1;
    }
    if (this->batchQuadCount != 0)
    {
        this->FlushQuadBatch();
    }
    if ((vm->flags >> 15 & 1) == 0 && ((vm->flags >> 3 & 1) != 0 || (vm->flags >> 2 & 1) != 0))
    {
        vm->unk_138 = vm->matrix;
        vm->unk_138._11 *= vm->scaleY;
        vm->unk_138._22 *= vm->scaleX;
        vm->flags &= ~8;
        if (vm->rotationX != 0.0)
        {
            D3DXMatrixRotationX(&rotationMatrix, vm->rotationX);
            D3DXMatrixMultiply(&vm->unk_138, &vm->unk_138, &rotationMatrix);
        }
        if (vm->rotationY != 0.0)
        {
            D3DXMatrixRotationY(&rotationMatrix, vm->rotationY);
            D3DXMatrixMultiply(&vm->unk_138, &vm->unk_138, &rotationMatrix);
        }
        if (vm->rotationZ != 0.0)
        {
            D3DXMatrixRotationZ(&rotationMatrix, vm->rotationZ);
            D3DXMatrixMultiply(&vm->unk_138, &vm->unk_138, &rotationMatrix);
        }
        vm->flags &= ~4;
    }
    worldMatrix = vm->unk_138;
    if (((vm->flags >> 10 & 3) & 1) == 0)
    {
        worldMatrix._41 = vm->pos.x;
    }
    else
    {
        worldMatrix._41 = fabsf(vm->spriteData->width * vm->scaleY / 2.0f) + vm->pos.x;
    }
    if (((vm->flags >> 10 & 3) & 2) == 0)
    {
        worldMatrix._42 = vm->pos.y;
    }
    else
    {
        worldMatrix._42 = fabsf(vm->spriteData->height * vm->scaleX / 2.0f) + vm->pos.y;
    }
    worldMatrix._41 += this->shakeX;
    worldMatrix._42 += this->shakeY;
    this->SetRenderStateForSprite(vm);
    worldMatrix._43 = vm->pos.z;
    g_App.d3dDevice->SetTransform(D3DTS_WORLD, &worldMatrix);
    if (this->currentSpriteData != vm->spriteData)
    {
        this->currentSpriteData = vm->spriteData;
        textureMatrix = vm->unk_178;
        textureMatrix._31 = vm->spriteData->uvStartX + vm->uvScrollX;
        textureMatrix._32 = vm->spriteData->uvStartY + vm->uvScrollY;
        g_App.d3dDevice->SetTransform(D3DTS_TEXTURE0, &textureMatrix);
        if (this->currentTexture != this->textures[vm->spriteData->sourceFileIndex])
        {
            this->currentTexture = this->textures[vm->spriteData->sourceFileIndex];
            g_App.d3dDevice->SetTexture(0, this->currentTexture);
        }
    }
    if (this->vertexFormat != 2)
    {
        if (!g_App.flags.noVertexBuffer)
        {
            g_App.d3dDevice->SetVertexShader(D3DFVF_XYZ | D3DFVF_TEX1);
            g_App.d3dDevice->SetStreamSource(0, this->vertexBuffer, sizeof(VertexTex1Xyz));
        }
        else
        {
            g_App.d3dDevice->SetVertexShader(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
        }
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
        this->vertexFormat = 2;
    }
    if (!g_App.flags.noVertexBuffer)
    {
        g_App.d3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
    }
    else
    {
        g_App.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, g_DiffuseQuadXyz, sizeof(g_DiffuseQuadXyz[0]));
    }
    return 0;
}

float Sprite::ResolveFloatArgument(float value)
{
    switch ((int)value)
    {
    case 10000:
        return this->unk_c8;
    case 10001:
        return this->unk_cc;
    case 10002:
        return this->unk_d0;
    case 10003:
        return this->unk_d4;
    case 10004:
        return this->unk_d8;
    case 10005:
        return this->unk_dc;
    case 10006:
        return this->unk_e0;
    case 10007:
        return this->unk_e4;
    case 10008:
        return this->unk_e8;
    case 10009:
        return this->unk_ec;
    }
    return value;
}

int Sprite::ResolveIntArgument(int value)
{
    switch (value)
    {
    case 10000:
        return this->unk_c8;
    case 10001:
        return this->unk_cc;
    case 10002:
        return this->unk_d0;
    case 10003:
        return this->unk_d4;
    case 10004:
        return this->unk_d8;
    case 10005:
        return this->unk_dc;
    case 10006:
        return this->unk_e0;
    case 10007:
        return this->unk_e4;
    case 10008:
        return this->unk_e8;
    case 10009:
        return this->unk_ec;
    }
    return value;
}

float *Sprite::ResolveFloatArgumentPtr(float *value, WORD mask, int bit)
{
    if ((mask & 1 << bit) == 0)
    {
        return value;
    }
    switch ((int)*value)
    {
    case 10004:
        return &this->unk_d8;
    case 10005:
        return &this->unk_dc;
    case 10006:
        return &this->unk_e0;
    case 10007:
        return &this->unk_e4;
    }
    return value;
}

int *Sprite::ResolveIntArgumentPtr(int *value, WORD mask, int bit)
{
    if ((mask & 1 << bit) == 0)
    {
        return value;
    }
    switch (*value)
    {
    case 10000:
        return &this->unk_c8;
    case 10001:
        return &this->unk_cc;
    case 10002:
        return &this->unk_d0;
    case 10003:
        return &this->unk_d4;
    case 10008:
        return &this->unk_e8;
    case 10009:
        return &this->unk_ec;
    }
    return value;
}

#define ANM_INT_ARG(n) \
    ((curInstr->paramMask & (1 << (n))) ? vm->ResolveIntArgument(curInstr->args[n].i) : curInstr->args[n].i)
#define ANM_FLOAT_ARG(n) \
    ((curInstr->paramMask & (1 << (n))) ? vm->ResolveFloatArgument(curInstr->args[n].f) : curInstr->args[n].f)
#define ANM_INT_VAR(n) (*vm->ResolveIntArgumentPtr(&curInstr->args[n].i, curInstr->paramMask, (n)))
#define ANM_FLOAT_VAR(n) (*vm->ResolveFloatArgumentPtr(&curInstr->args[n].f, curInstr->paramMask, (n)))

int SpriteEngine::Step(Sprite *vm)
{
    AnmInstr *curInstr;
    AnmInstr *fallback;
    int k;
    float t;

    if (vm->scriptPos == NULL)
    {
        return 1;
    }
    if (vm->pendingInterrupt != 0)
    {
        goto interrupt;
    }
    while (curInstr = vm->scriptPos, curInstr->time <= vm->unk_30.AsFrames())
    {
        switch (curInstr->opcode)
        {
        case -1:
        case 1:
            vm->shown = 0;
        case 2:
            vm->scriptPos = NULL;
            return 1;
        case 3:
            vm->shown = 1;
            this->SetSprite(vm, ANM_INT_ARG(0) + this->scriptSpriteOffsets[vm->scriptIndex]);
            vm->unk_23c = vm->unk_30.AsFrames();
            break;
        case 7:
            vm->scaleY = ANM_FLOAT_ARG(0);
            vm->scaleX = ANM_FLOAT_ARG(1);
            vm->flags |= 8;
            break;
        case 8:
            vm->alpha = curInstr->args[0].i & 0xff;
            break;
        case 9:
            vm->color = vm->color & 0xff000000 | curInstr->args[0].i & 0xffffff;
            break;
        case 4:
            vm->unk_30.Set(curInstr->args[1].i);
            vm->scriptPos = (AnmInstr *)((BYTE *)vm->scriptStart + curInstr->args[0].i);
            continue;
        case 5:
            ANM_INT_VAR(0)--;
            if (ANM_INT_ARG(0) > 0)
            {
                vm->unk_30.Set(curInstr->args[2].i);
                vm->scriptPos = (AnmInstr *)((BYTE *)vm->scriptStart + curInstr->args[1].i);
                continue;
            }
            break;
        case 10:
            vm->flip ^= 1;
            vm->scaleY *= -1.0f;
            vm->flags |= 8;
            break;
        case 24:
            vm->usePosOffset = curInstr->args[0].i;
            break;
        case 11:
            vm->flip ^= 2;
            vm->scaleX *= -1.0f;
            vm->flags |= 8;
            break;
        case 12:
            vm->rotationX = ANM_FLOAT_ARG(0);
            vm->rotationY = ANM_FLOAT_ARG(1);
            vm->rotationZ = ANM_FLOAT_ARG(2);
            vm->flags |= 4;
            break;
        case 13:
            vm->unk_c = ANM_FLOAT_ARG(0);
            vm->unk_10 = ANM_FLOAT_ARG(1);
            vm->unk_14 = ANM_FLOAT_ARG(2);
            vm->flags |= 4;
            break;
        case 14:
            vm->unk_20 = ANM_FLOAT_ARG(0);
            vm->unk_24 = ANM_FLOAT_ARG(1);
            break;
        case 29:
            vm->unk_48[4].Set(0);
            vm->unk_84[4].Set(ANM_INT_ARG(2));
            vm->unk_c4 = 0;
            *(D3DXVECTOR2 *)&vm->unk_218 = *(D3DXVECTOR2 *)&vm->scaleY;
            vm->unk_218.z = ANM_FLOAT_ARG(0);
            vm->unk_224.x = ANM_FLOAT_ARG(1);
            break;
        case 15:
            ((BYTE *)&vm->unk_224.y)[3] = vm->alpha;
            ((BYTE *)&vm->unk_224.z)[3] = curInstr->args[0].i;
            vm->unk_48[2].Set(0);
            vm->unk_84[2].Set(ANM_INT_ARG(1));
            vm->unk_c2 = 0;
            break;
        case 16:
            vm->blendMode = curInstr->args[0].i;
            break;
        case 6:
            if (!vm->usePosOffset)
            {
                vm->pos = D3DXVECTOR3(ANM_FLOAT_ARG(0), ANM_FLOAT_ARG(1), ANM_FLOAT_ARG(2));
            }
            else
            {
                vm->posOffset = D3DXVECTOR3(ANM_FLOAT_ARG(0), ANM_FLOAT_ARG(1), ANM_FLOAT_ARG(2));
            }
            break;
        case 19:
            vm->unk_c0 = 6;
            goto posInterp;
        case 18:
            vm->unk_c0 = 4;
            goto posInterp;
        case 17:
            vm->unk_c0 = 0;
        posInterp:
            if (!vm->usePosOffset)
            {
                vm->unk_1e8 = vm->pos;
            }
            else
            {
                vm->unk_1e8 = vm->posOffset;
            }
            vm->unk_1f4 = D3DXVECTOR3(ANM_FLOAT_ARG(0), ANM_FLOAT_ARG(1), ANM_FLOAT_ARG(2));
            vm->unk_84[0].Set(ANM_INT_ARG(3));
            vm->unk_48[0].Set(0);
            break;
        case 79:
            if (vm->unk_3c == 0)
            {
                vm->unk_3c.Set(ANM_INT_ARG(0));
            }
            else
            {
                vm->unk_3c.Sub(1);
            }
            if (vm->unk_3c <= 0)
            {
                vm->unk_3c.Set(0);
                break;
            }
            vm->unk_30.Sub(1);
            goto done;
        case 23:
            vm->shown = 0;
        case 20:
            if (vm->pendingInterrupt == 0)
            {
                vm->stopped = 1;
                vm->unk_30.Sub(1);
                goto done;
            }
        interrupt:
            fallback = NULL;
            curInstr = vm->scriptStart;
            while ((curInstr->opcode != 21 || vm->pendingInterrupt != curInstr->args[0].i) &&
                   curInstr->opcode != -1)
            {
                if (curInstr->opcode == 21 && curInstr->args[0].i == -1)
                {
                    fallback = curInstr;
                }
                curInstr = (AnmInstr *)((BYTE *)curInstr + curInstr->size);
            }
            vm->pendingInterrupt = 0;
            vm->stopped = 0;
            if (curInstr->opcode != 21)
            {
                if (fallback == NULL)
                {
                    vm->unk_30.Sub(1);
                    goto done;
                }
                curInstr = fallback;
            }
            curInstr = (AnmInstr *)((BYTE *)curInstr + curInstr->size);
            vm->scriptPos = curInstr;
            vm->unk_30.Set(vm->scriptPos->time);
            vm->shown = 1;
            continue;
        case 28:
            vm->shown = curInstr->args[0].i;
            break;
        case 22:
            vm->anchor = 3;
            break;
        case 25:
            vm->autoRotate = curInstr->args[0].i;
            break;
        case 26:
            vm->uvScrollX += ANM_FLOAT_ARG(0);
            if (vm->uvScrollX >= 1.0f)
            {
                vm->uvScrollX -= 1.0f;
            }
            else if (vm->uvScrollX < 0.0f)
            {
                vm->uvScrollX += 1.0f;
            }
            break;
        case 27:
            vm->uvScrollY += ANM_FLOAT_ARG(0);
            if (vm->uvScrollY >= 1.0f)
            {
                vm->uvScrollY -= 1.0f;
            }
            else if (vm->uvScrollY < 0.0f)
            {
                vm->uvScrollY += 1.0f;
            }
            break;
        case 80:
            vm->unk_f0 = ANM_FLOAT_ARG(0);
            break;
        case 81:
            vm->unk_f4 = ANM_FLOAT_ARG(0);
            break;
        case 30:
            vm->zWriteDisabled = curInstr->args[0].i;
            break;
        case 31:
            vm->unk_14b = curInstr->args[0].i;
            break;
        case 32:
            vm->unk_48[0].Set(0);
            vm->unk_84[0].Set(ANM_INT_ARG(0));
            vm->unk_c0 = curInstr->args[1].i;
            if (!vm->usePosOffset)
            {
                vm->unk_1e8 = vm->pos;
            }
            else
            {
                vm->unk_1e8 = vm->posOffset;
            }
            vm->unk_1f4.x = ANM_FLOAT_ARG(2);
            vm->unk_1f4.y = ANM_FLOAT_ARG(3);
            vm->unk_1f4.z = ANM_FLOAT_ARG(4);
            break;
        case 33:
            vm->unk_48[1].Set(0);
            vm->unk_84[1].Set(ANM_INT_ARG(0));
            vm->unk_c1 = curInstr->args[1].i;
            ((BYTE *)&vm->unk_224.y)[2] = vm->unk_rgb[2];
            ((BYTE *)&vm->unk_224.y)[1] = vm->unk_rgb[1];
            ((BYTE *)&vm->unk_224.y)[0] = vm->unk_rgb[0];
            ((BYTE *)&vm->unk_224.z)[2] = curInstr->args[2].b[0];
            ((BYTE *)&vm->unk_224.z)[1] = curInstr->args[2].b[1];
            ((BYTE *)&vm->unk_224.z)[0] = curInstr->args[2].b[2];
            break;
        case 34:
            vm->unk_48[2].Set(0);
            vm->unk_84[2].Set(ANM_INT_ARG(0));
            vm->unk_c2 = curInstr->args[1].i;
            ((BYTE *)&vm->unk_224.y)[3] = vm->alpha;
            ((BYTE *)&vm->unk_224.z)[3] = curInstr->args[2].i;
            break;
        case 35:
            vm->unk_48[3].Set(0);
            vm->unk_84[3].Set(ANM_INT_ARG(0));
            vm->unk_c3 = curInstr->args[1].i;
            vm->unk_200 = *(D3DXVECTOR3 *)&vm->rotationX;
            vm->unk_20c.x = ANM_FLOAT_ARG(2);
            vm->unk_20c.y = ANM_FLOAT_ARG(3);
            vm->unk_20c.z = ANM_FLOAT_ARG(4);
            vm->flags |= 4;
            break;
        case 36:
            vm->unk_48[4].Set(0);
            vm->unk_84[4].Set(ANM_INT_ARG(0));
            vm->unk_c4 = curInstr->args[1].i;
            *(D3DXVECTOR2 *)&vm->unk_218 = *(D3DXVECTOR2 *)&vm->scaleY;
            vm->unk_218.z = ANM_FLOAT_ARG(2);
            vm->unk_224.x = ANM_FLOAT_ARG(3);
            vm->flags |= 8;
            break;
        case 37:
            ANM_INT_VAR(0) = ANM_INT_ARG(1);
            break;
        case 38:
            ANM_FLOAT_VAR(0) = ANM_FLOAT_ARG(1);
            break;
        case 49:
            ANM_INT_VAR(0) = ANM_INT_ARG(1) + ANM_INT_ARG(2);
            break;
        case 50:
            ANM_FLOAT_VAR(0) = ANM_FLOAT_ARG(1) + ANM_FLOAT_ARG(2);
            break;
        case 51:
            ANM_INT_VAR(0) = ANM_INT_ARG(1) - ANM_INT_ARG(2);
            break;
        case 52:
            ANM_FLOAT_VAR(0) = ANM_FLOAT_ARG(1) - ANM_FLOAT_ARG(2);
            break;
        case 53:
            ANM_INT_VAR(0) = ANM_INT_ARG(1) * ANM_INT_ARG(2);
            break;
        case 54:
            ANM_FLOAT_VAR(0) = ANM_FLOAT_ARG(1) * ANM_FLOAT_ARG(2);
            break;
        case 55:
            ANM_INT_VAR(0) = ANM_INT_ARG(1) / ANM_INT_ARG(2);
            break;
        case 56:
            ANM_FLOAT_VAR(0) = ANM_FLOAT_ARG(1) / ANM_FLOAT_ARG(2);
            break;
        case 57:
            ANM_INT_VAR(0) = ANM_INT_ARG(1) % ANM_INT_ARG(2);
            break;
        case 58:
            ANM_FLOAT_VAR(0) = fmodf(ANM_FLOAT_ARG(1), ANM_FLOAT_ARG(2));
            break;
        case 39:
            ANM_INT_VAR(0) += ANM_INT_ARG(1);
            break;
        case 40:
            ANM_FLOAT_VAR(0) += ANM_FLOAT_ARG(1);
            break;
        case 41:
            ANM_INT_VAR(0) -= ANM_INT_ARG(1);
            break;
        case 42:
            ANM_FLOAT_VAR(0) -= ANM_FLOAT_ARG(1);
            break;
        case 43:
            ANM_INT_VAR(0) *= ANM_INT_ARG(1);
            break;
        case 44:
            ANM_FLOAT_VAR(0) *= ANM_FLOAT_ARG(1);
            break;
        case 45:
            ANM_INT_VAR(0) /= ANM_INT_ARG(1);
            break;
        case 46:
            ANM_FLOAT_VAR(0) /= ANM_FLOAT_ARG(1);
            break;
        case 47:
            ANM_INT_VAR(0) %= ANM_INT_ARG(1);
            break;
        case 48:
            ANM_FLOAT_VAR(0) = fmodf(ANM_FLOAT_ARG(0), ANM_FLOAT_ARG(1));
            break;
        case 59:
            ANM_INT_VAR(0) = g_Rng.GetRandomU32InRange(ANM_INT_ARG(1));
            break;
        case 60:
            ANM_FLOAT_VAR(0) = g_Rng.GetRandomF32InRange(ANM_FLOAT_ARG(1));
            break;
        case 61:
            ANM_FLOAT_VAR(0) = sinf(ANM_FLOAT_ARG(1));
            break;
        case 62:
            ANM_FLOAT_VAR(0) = cosf(ANM_FLOAT_ARG(1));
            break;
        case 63:
            ANM_FLOAT_VAR(0) = tanf(ANM_FLOAT_ARG(1));
            break;
        case 64:
            ANM_FLOAT_VAR(0) = acosf(ANM_FLOAT_ARG(1));
            break;
        case 65:
            ANM_FLOAT_VAR(0) = atanf(ANM_FLOAT_ARG(1));
            break;
        case 66:
            ANM_FLOAT_VAR(0) = AddNormalizeAngle(ANM_FLOAT_ARG(0), 0.0f);
            break;
        case 67:
            if (ANM_INT_ARG(0) == ANM_INT_ARG(1))
            {
                goto jump;
            }
            break;
        case 68:
            if (ANM_FLOAT_ARG(0) == ANM_FLOAT_ARG(1))
            {
                goto jump;
            }
            break;
        case 69:
            if (ANM_INT_ARG(0) != ANM_INT_ARG(1))
            {
                goto jump;
            }
            break;
        case 70:
            if (ANM_FLOAT_ARG(0) != ANM_FLOAT_ARG(1))
            {
                goto jump;
            }
            break;
        case 71:
            if (ANM_INT_ARG(0) < ANM_INT_ARG(1))
            {
                goto jump;
            }
            break;
        case 72:
            if (ANM_FLOAT_ARG(0) < ANM_FLOAT_ARG(1))
            {
                goto jump;
            }
            break;
        case 73:
            if (ANM_INT_ARG(0) <= ANM_INT_ARG(1))
            {
                goto jump;
            }
            break;
        case 74:
            if (ANM_FLOAT_ARG(0) <= ANM_FLOAT_ARG(1))
            {
                goto jump;
            }
            break;
        case 75:
            if (ANM_INT_ARG(0) > ANM_INT_ARG(1))
            {
                goto jump;
            }
            break;
        case 76:
            if (ANM_FLOAT_ARG(0) > ANM_FLOAT_ARG(1))
            {
                goto jump;
            }
            break;
        case 77:
            if (ANM_INT_ARG(0) >= ANM_INT_ARG(1))
            {
                goto jump;
            }
            break;
        case 78:
            if (ANM_FLOAT_ARG(0) >= ANM_FLOAT_ARG(1))
            {
                goto jump;
            }
            break;
        jump:
            vm->unk_30.Set(curInstr->args[3].i);
            vm->scriptPos = (AnmInstr *)((BYTE *)vm->scriptStart + curInstr->args[2].i);
            continue;
        }
        vm->scriptPos = (AnmInstr *)((BYTE *)curInstr + curInstr->size);
    }

done:
    if (vm->unk_c != 0.0f)
    {
        vm->rotationX = AddNormalizeAngle(vm->rotationX, g_App.gameSpeed * vm->unk_c);
        vm->flags |= 4;
    }
    if (vm->unk_10 != 0.0f)
    {
        vm->rotationY = AddNormalizeAngle(vm->rotationY, g_App.gameSpeed * vm->unk_10);
        vm->flags |= 4;
    }
    if (vm->unk_14 != 0.0f)
    {
        vm->rotationZ = AddNormalizeAngle(vm->rotationZ, g_App.gameSpeed * vm->unk_14);
        vm->flags |= 4;
    }
    for (k = 0; k < 5; k++)
    {
        if (vm->unk_84[k] > 0)
        {
            vm->unk_48[k].Tick();
            if (vm->unk_48[k] >= vm->unk_84[k].AsFrames())
            {
                t = 1.0f;
                vm->unk_84[k].Set(0);
            }
            else
            {
                t = vm->unk_48[k].AsFloat() / vm->unk_84[k].AsFloat();
            }
            switch ((&vm->unk_c0)[k])
            {
            case 1:
                t = t * t;
                break;
            case 2:
                t = t * t * t;
                break;
            case 3:
                t = (t = t * t) * t;
                break;
            case 4:
                t = (t = 1.0f - t) * t;
                t = 1.0f - t;
                break;
            case 5:
                t = (t = 1.0f - t) * t * t;
                t = 1.0f - t;
                break;
            case 6:
                t = (t = (t = 1.0f - t) * t) * t;
                t = 1.0f - t;
                break;
            }
            switch (k)
            {
            case 0:
                if (!vm->usePosOffset)
                {
                    vm->pos.x = (vm->unk_1f4.x - vm->unk_1e8.x) * t + vm->unk_1e8.x;
                    vm->pos.y = (vm->unk_1f4.y - vm->unk_1e8.y) * t + vm->unk_1e8.y;
                    vm->pos.z = (vm->unk_1f4.z - vm->unk_1e8.z) * t + vm->unk_1e8.z;
                }
                else
                {
                    vm->posOffset.x = (vm->unk_1f4.x - vm->unk_1e8.x) * t + vm->unk_1e8.x;
                    vm->posOffset.y = (vm->unk_1f4.y - vm->unk_1e8.y) * t + vm->unk_1e8.y;
                    vm->posOffset.z = (vm->unk_1f4.z - vm->unk_1e8.z) * t + vm->unk_1e8.z;
                }
                break;
            case 1:
                vm->unk_rgb[2] = (((BYTE *)&vm->unk_224.z)[2] - ((BYTE *)&vm->unk_224.y)[2]) * t +
                                 ((BYTE *)&vm->unk_224.y)[2];
                vm->unk_rgb[1] = (((BYTE *)&vm->unk_224.z)[1] - ((BYTE *)&vm->unk_224.y)[1]) * t +
                                 ((BYTE *)&vm->unk_224.y)[1];
                vm->unk_rgb[0] = (((BYTE *)&vm->unk_224.z)[0] - ((BYTE *)&vm->unk_224.y)[0]) * t +
                                 ((BYTE *)&vm->unk_224.y)[0];
                break;
            case 2:
                vm->alpha = (((BYTE *)&vm->unk_224.z)[3] - ((BYTE *)&vm->unk_224.y)[3]) * t +
                            ((BYTE *)&vm->unk_224.y)[3];
                break;
            case 3:
                vm->rotationX = AddNormalizeAngle((vm->unk_20c.x - vm->unk_200.x) * t, vm->unk_200.x);
                vm->rotationY = AddNormalizeAngle((vm->unk_20c.y - vm->unk_200.y) * t, vm->unk_200.y);
                vm->rotationZ = AddNormalizeAngle((vm->unk_20c.z - vm->unk_200.z) * t, vm->unk_200.z);
                vm->flags |= 4;
                break;
            case 4:
                vm->scaleY = (vm->unk_218.z - vm->unk_218.x) * t + vm->unk_218.x;
                vm->scaleX = (vm->unk_224.x - vm->unk_218.y) * t + vm->unk_218.y;
                vm->flags |= 8;
                break;
            }
        }
    }
    if (vm->unk_24 != 0.0f)
    {
        vm->scaleX += g_App.gameSpeed * vm->unk_24;
        vm->flags |= 8;
    }
    if (vm->unk_20 != 0.0f)
    {
        vm->scaleY += g_App.gameSpeed * vm->unk_20;
        vm->flags |= 8;
        vm->flags |= 4;
    }
    vm->uvScrollX = vm->uvScrollX + vm->unk_f0;
    if (vm->uvScrollX >= 1.0f)
    {
        vm->uvScrollX -= 1.0f;
    }
    else if (vm->uvScrollX < 0.0f)
    {
        vm->uvScrollX += 1.0f;
    }
    vm->uvScrollY = vm->uvScrollY + vm->unk_f4;
    if (vm->uvScrollY >= 1.0f)
    {
        vm->uvScrollY -= 1.0f;
    }
    else if (vm->uvScrollY < 0.0f)
    {
        vm->uvScrollY += 1.0f;
    }
    vm->unk_30.Tick();
    this->unk_c++;
    return 0;
}

#undef ANM_INT_ARG
#undef ANM_FLOAT_ARG
#undef ANM_INT_VAR
#undef ANM_FLOAT_VAR

void SpriteEngine::RenderTextToTextureIndex(int textureIndex, int x, int y, int width, int unk4, int fontHeight,
                                            int unk6, D3DCOLOR textColor, D3DCOLOR shadowColor, char *text,
                                            float scaleX, float scaleY)
{
    if (fontHeight <= 0)
    {
        fontHeight = 15;
    }
    if (unk6 <= 0)
    {
        unk6 = 15;
    }
    RenderTextToTexture(x, y, width, unk4, fontHeight * scaleX, unk6 * scaleY, textColor, shadowColor, text,
                        this->textures[textureIndex]);
}

void SpriteEngine::RenderText(Sprite *sprite, D3DCOLOR textColor, D3DCOLOR shadowColor, char *fmt, ...)
{
    va_list args;
    char buf[0x48];
    int fontW;

    fontW = sprite->fontWidth;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    this->RenderTextToTextureIndex(sprite->spriteData->sourceFileIndex, sprite->spriteData->startX,
                                   sprite->spriteData->startY, sprite->spriteData->textureWidth,
                                   sprite->spriteData->textureHeight, fontW, sprite->fontHeight, textColor, shadowColor,
                                   buf, sprite->spriteData->scaleX, sprite->spriteData->scaleY);
    sprite->flags |= 1;
}

void SpriteEngine::RenderTextRightAligned(Sprite *vm, D3DCOLOR color, D3DCOLOR shadowColor, char *fmt, ...)
{
    va_list args;
    int xPos;
    char buf[0x48];
    int fontW;

    fontW = vm->fontWidth <= 0 ? 15 : vm->fontWidth;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    this->RenderTextToTextureIndex(vm->spriteData->sourceFileIndex, vm->spriteData->startX, vm->spriteData->startY,
                                   vm->spriteData->textureWidth, vm->spriteData->textureHeight, fontW, vm->fontHeight,
                                   color, shadowColor, " ", vm->spriteData->scaleX, vm->spriteData->scaleY);
    xPos = vm->spriteData->startX + vm->spriteData->width * vm->spriteData->scaleX -
           (float)strlen(buf) * fontW * vm->spriteData->scaleX / 2.0f;
    this->RenderTextToTextureIndex(vm->spriteData->sourceFileIndex, xPos, vm->spriteData->startY,
                                   vm->spriteData->textureWidth, vm->spriteData->textureHeight, fontW, vm->fontHeight,
                                   color, shadowColor, buf, vm->spriteData->scaleX, vm->spriteData->scaleY);
    vm->flags |= 1;
}

void SpriteEngine::RenderTextCentered(Sprite *vm, D3DCOLOR color, D3DCOLOR shadowColor, char *fmt, ...)
{
    va_list args;
    int xPos;
    char buf[0x48];
    int fontW;

    fontW = vm->fontWidth <= 0 ? 15 : vm->fontWidth;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    this->RenderTextToTextureIndex(vm->spriteData->sourceFileIndex, vm->spriteData->startX, vm->spriteData->startY,
                                   vm->spriteData->textureWidth, vm->spriteData->textureHeight, fontW, vm->fontHeight,
                                   color, shadowColor, " ", vm->spriteData->scaleX, vm->spriteData->scaleY);
    xPos = vm->spriteData->startX + vm->spriteData->width * vm->spriteData->scaleX / 2.0f -
           (float)strlen(buf) * fontW * vm->spriteData->scaleX / 4.0f;
    this->RenderTextToTextureIndex(vm->spriteData->sourceFileIndex, xPos, vm->spriteData->startY,
                                   vm->spriteData->textureWidth, vm->spriteData->textureHeight, fontW, vm->fontHeight,
                                   color, shadowColor, buf, vm->spriteData->scaleX, vm->spriteData->scaleY);
    vm->flags |= 1;
}

void SpriteEngine::ReleaseSurface(int surface)
{
    if (this->surfaces[surface] != NULL)
    {
        this->surfaces[surface]->Release();
        this->surfaces[surface] = NULL;
    }
    if (this->surfacesBackup[surface] != NULL)
    {
        this->surfacesBackup[surface]->Release();
        this->surfacesBackup[surface] = NULL;
    }
}

int SpriteEngine::LoadSurface(int surface, char *path)
{
    IDirect3DSurface8 *srcSurface;
    void *fileData;

    if (this->surfaces[surface] != NULL)
    {
        this->ReleaseSurface(surface);
    }
    fileData = LoadFileData(path, 0);
    if (fileData == NULL)
    {
        LogError(&g_Log, "%sが読み込めないです。\r\n", path);
        return -1;
    }
    if (g_App.d3dDevice->CreateImageSurface(640, 1024, g_App.presentParams.BackBufferFormat, &srcSurface) != D3D_OK)
    {
        return -1;
    }
    if (D3DXLoadSurfaceFromFileInMemory(srcSurface, NULL, NULL, fileData, g_FileSize, NULL, D3DX_FILTER_NONE, 0,
                                        &this->surfaceInfos[surface]) != D3D_OK)
    {
        goto fail;
    }
    if (g_App.d3dDevice->CreateRenderTarget(this->surfaceInfos[surface].Width, this->surfaceInfos[surface].Height,
                                            g_App.presentParams.BackBufferFormat, D3DMULTISAMPLE_NONE, TRUE,
                                            &this->surfaces[surface]) != D3D_OK &&
        g_App.d3dDevice->CreateImageSurface(this->surfaceInfos[surface].Width, this->surfaceInfos[surface].Height,
                                            g_App.presentParams.BackBufferFormat,
                                            &this->surfaces[surface]) != D3D_OK)
    {
        goto fail;
    }
    if (g_App.d3dDevice->CreateImageSurface(this->surfaceInfos[surface].Width, this->surfaceInfos[surface].Height,
                                            g_App.presentParams.BackBufferFormat,
                                            &this->surfacesBackup[surface]) != D3D_OK)
    {
        goto fail;
    }
    if (D3DXLoadSurfaceFromSurface(this->surfaces[surface], NULL, NULL, srcSurface, NULL, NULL, D3DX_FILTER_NONE, 0) !=
        D3D_OK)
    {
        goto fail;
    }
    if (D3DXLoadSurfaceFromSurface(this->surfacesBackup[surface], NULL, NULL, srcSurface, NULL, NULL, D3DX_FILTER_NONE,
                                   0) != D3D_OK)
    {
        goto fail;
    }
    if (srcSurface != NULL)
    {
        srcSurface->Release();
        srcSurface = NULL;
    }
    free(fileData);
    return 0;

fail:
    if (srcSurface != NULL)
    {
        srcSurface->Release();
        srcSurface = NULL;
    }
    free(fileData);
    return -1;
}

void SpriteEngine::CopySurfaceToBackBuffer(int surface, int left, int top, int x, int y)
{
    IDirect3DSurface8 *backBuffer;
    RECT rect;
    POINT point;

    if (this->surfacesBackup[surface] == NULL)
    {
        return;
    }
    if (g_App.d3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backBuffer) != D3D_OK)
    {
        return;
    }
    if (this->surfaces[surface] == NULL)
    {
        if (g_App.d3dDevice->CreateRenderTarget(
                this->surfaceInfos[surface].Width, this->surfaceInfos[surface].Height,
                g_App.presentParams.BackBufferFormat, D3DMULTISAMPLE_NONE, TRUE,
                &this->surfaces[surface]) != D3D_OK &&
            g_App.d3dDevice->CreateImageSurface(this->surfaceInfos[surface].Width,
                                                         this->surfaceInfos[surface].Height,
                                                         g_App.presentParams.BackBufferFormat,
                                                         &this->surfaces[surface]) != D3D_OK)
        {
            backBuffer->Release();
            return;
        }
        if (D3DXLoadSurfaceFromSurface(this->surfaces[surface], NULL, NULL, this->surfacesBackup[surface], NULL, NULL,
                                       D3DX_FILTER_NONE, 0) != D3D_OK)
        {
            backBuffer->Release();
            return;
        }
    }
    rect.left = left;
    rect.top = top;
    rect.right = this->surfaceInfos[surface].Width;
    rect.bottom = this->surfaceInfos[surface].Height;
    point.x = x;
    point.y = y;
    g_App.d3dDevice->CopyRects(this->surfaces[surface], &rect, 1, backBuffer, &point);
    backBuffer->Release();
}


void SpriteEngine::CopySurfaceRectToBackBuffer(int surface, int x, int y, int left, int top, int width, int height)
{
    IDirect3DSurface8 *backBuffer;
    RECT rect;
    POINT point;

    if (this->surfacesBackup[surface] == NULL)
    {
        return;
    }
    if (g_App.d3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backBuffer) != D3D_OK)
    {
        return;
    }
    if (this->surfaces[surface] == NULL)
    {
        if (g_App.d3dDevice->CreateRenderTarget(
                this->surfaceInfos[surface].Width, this->surfaceInfos[surface].Height,
                g_App.presentParams.BackBufferFormat, D3DMULTISAMPLE_NONE, TRUE,
                &this->surfaces[surface]) != D3D_OK &&
            g_App.d3dDevice->CreateImageSurface(this->surfaceInfos[surface].Width,
                                                         this->surfaceInfos[surface].Height,
                                                         g_App.presentParams.BackBufferFormat,
                                                         &this->surfaces[surface]) != D3D_OK)
        {
            backBuffer->Release();
            return;
        }
        if (D3DXLoadSurfaceFromSurface(this->surfaces[surface], NULL, NULL, this->surfacesBackup[surface], NULL, NULL,
                                       D3DX_FILTER_NONE, 0) != D3D_OK)
        {
            backBuffer->Release();
            return;
        }
    }
    rect.left = left;
    rect.top = top;
    rect.right = left + width;
    rect.bottom = top + height;
    point.x = x;
    point.y = y;
    g_App.d3dDevice->CopyRects(this->surfaces[surface], &rect, 1, backBuffer, &point);
    backBuffer->Release();
}

void SpriteEngine::CopyBackBufferToTexture(int texture, int srcLeft, int srcTop, int srcWidth, int srcHeight,
                                           int dstLeft, int dstTop, int dstWidth, int dstHeight)
{
    IDirect3DSurface8 *backBuffer;
    IDirect3DSurface8 *dest;
    RECT rect;
    RECT destinationRect;

    if (this->textures[texture] == NULL)
    {
        return;
    }
    this->FlushQuadBatch();
    if (g_App.d3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backBuffer) != D3D_OK)
    {
        return;
    }
    if (this->textures[texture]->GetSurfaceLevel(0, &dest) != D3D_OK)
    {
        backBuffer->Release();
        return;
    }
    rect.left = srcLeft;
    rect.top = srcTop;
    rect.right = srcLeft + srcWidth;
    rect.bottom = srcTop + srcHeight;
    destinationRect.left = dstLeft;
    destinationRect.top = dstTop;
    destinationRect.right = dstLeft + dstWidth;
    destinationRect.bottom = dstTop + dstHeight;
    if (D3DXLoadSurfaceFromSurface(dest, NULL, &destinationRect, backBuffer, NULL, &rect, D3DX_DEFAULT, 0) != D3D_OK)
    {
        dest->Release();
        backBuffer->Release();
        return;
    }
    dest->Release();
    backBuffer->Release();
}

void SpriteEngine::CopyTextureRect(int dstTexture, int srcTexture, RECT *dstRect, RECT *srcRect)
{
    IDirect3DSurface8 *dest;
    IDirect3DSurface8 *src;

    if (this->textures[dstTexture] == NULL)
    {
        return;
    }
    if (this->textures[srcTexture] == NULL)
    {
        return;
    }
    this->FlushQuadBatch();
    if (this->textures[dstTexture]->GetSurfaceLevel(0, &dest) != D3D_OK)
    {
        return;
    }
    if (this->textures[srcTexture]->GetSurfaceLevel(0, &src) != D3D_OK)
    {
        dest->Release();
        return;
    }
    if (D3DXLoadSurfaceFromSurface(dest, NULL, dstRect, src, NULL, srcRect, D3DX_DEFAULT, 0) != D3D_OK)
    {
        dest->Release();
        src->Release();
        return;
    }
    dest->Release();
    src->Release();
}

void SpriteEngine::SendInterruptToLoadedSprites(Sprite *vms, int count, short interrupt)
{
    while (count != 0)
    {
        if (vms->HasTexture())
        {
            vms->pendingInterrupt = interrupt;
        }
        vms++;
        count--;
    }
}

void SpriteEngine::StepSprites(Sprite *vms, int count)
{
    while (count != 0)
    {
        if (vms->scriptIndex >= 0)
        {
            g_SpriteEngine->Step(vms);
        }
        vms++;
        count--;
    }
}

void SpriteEngine::StartSpriteSeries(Sprite *vms, int script, int count)
{
    while (count != 0)
    {
        g_SpriteEngine->StartScriptAtOrigin(vms, script);
        vms->baseSpriteIndex = vms->spriteIndex;
        script++;
        vms++;
        count--;
    }
}

int SpriteEngine::FillSpriteStripVertices(Sprite *vm, VertexTex1DiffuseXyzrwh *vertices, int count)
{
    float width;
    float step;
    float u;
    float u0;
    VertexTex1DiffuseXyzrwh *cur;
    int i;
    float v;

    if (count < 3)
    {
        return -1;
    }
    u0 = vm->spriteData->uvEndX + vm->uvScrollX;
    width = vm->spriteData->uvEndX - vm->spriteData->uvStartX;
    v = vm->spriteData->uvStartY + vm->uvScrollY;
    cur = vertices;
    step = width / ((count + 1) / 2 - 1);
    for (i = 0, u = u0; i < count; i += 2, cur += 2, u -= step)
    {
        cur->uv.x = u;
        cur->uv.y = v;
        cur->diffuse = vm->color;
        cur->w = 1.0f;
    }
    v = vm->spriteData->uvEndY + vm->uvScrollY;
    cur = vertices + 1;
    for (i = 1, u = u0; i < count; i += 2, cur += 2, u -= step)
    {
        cur->uv.x = u;
        cur->uv.y = v;
        cur->diffuse = vm->color;
        cur->w = 1.0f;
    }
    return 0;
}

int SpriteEngine::DrawVertexStrip(Sprite *vm, VertexTex1DiffuseXyzrwh *vertices, int count)
{
    if ((vm->flags & 1) == 0)
    {
        return -1;
    }
    if ((vm->flags >> 1 & 1) == 0)
    {
        return -1;
    }
    if (vm->alpha == 0)
    {
        return -1;
    }
    if (this->batchQuadCount != 0)
    {
        this->FlushQuadBatch();
    }
    if (this->currentTexture != this->textures[vm->spriteData->sourceFileIndex])
    {
        this->currentTexture = this->textures[vm->spriteData->sourceFileIndex];
        g_App.d3dDevice->SetTexture(0, this->currentTexture);
    }
    if (this->vertexFormat != 3)
    {
        g_App.d3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
        this->vertexFormat = 3;
    }
    this->SetRenderStateForSprite(vm);
    g_App.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, count - 2, vertices, sizeof(vertices[0]));
    return 0;
}

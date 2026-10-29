// The file with SpriteEngine, from 0x44d3e0.

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "sprite.h"
#include "unsorted.h"

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
    this->Func44f5c0();
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
    this->Func44f5c0();
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

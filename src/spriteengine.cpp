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

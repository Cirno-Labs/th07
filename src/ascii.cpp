// The file starting at 0x401000: text in the ASCII font, as th06's
// ascii.cpp.

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "ascii.h"
#include "player.h"
#include "unsorted.h"

// Probably an inline of unsorted.h whose out-of-line copy the original
// keeps with this file's (0x401000-0x4013f0); defined here, not inline,
// so that the other files' calls stay calls.
void GameManager::RerollChecksumKeys()
{
    this->stats->lifeChecksumKeys[2] = g_Rng.GetRandomU32InRange(100000) + 6543;
    this->stats->checksumKeysD[3] = g_Rng.GetRandomU32InRange(100000) + 6543;
    this->stats->checksum = this->stats->lifeChecksumKeys[2];
    this->stats->computedChecksum = this->ComputeChecksum();
    this->checksumValue = this->stats->computedChecksum + this->stats->checksumKeysD[3];
}

// The same for Timer::NextTick, which only the Gui file calls.
int Timer::NextTick()
{
    this->previous = this->current;
    g_App.TickTimer(&this->current, &this->subFrame);
    return this->current;
}

// Like RerollChecksumKeys, without rerolling the keys.
void GameManager::SetBombs(int bombs)
{
    this->stats->bombs = bombs;
    this->stats->checksum = this->stats->lifeChecksumKeys[2];
    this->stats->computedChecksum = this->ComputeChecksum();
    this->checksumValue = this->stats->computedChecksum + this->stats->checksumKeysD[3];
}

AsciiText::AsciiText()
{
}

void IncrementCounterCapped(DWORD *count, DWORD max)
{
    if (*count < 999999)
    {
        *count = *count + 1;
    }
}

void AsciiText::RunScripts()
{
    g_SpriteEngine->Step(&this->summaryPopup);
    g_SpriteEngine->Step(&this->summaryDigit);
    g_SpriteEngine->Step(&this->nearbyLabels[0]);
    g_SpriteEngine->Step(&this->nearbyLabels[1]);
    g_SpriteEngine->Step(&this->nearbyLabels[2]);
    g_SpriteEngine->Step(&this->nearbyLabels[3]);
    g_SpriteEngine->Step(&this->summaryBombIcon);
}

int AsciiText::OnUpdate(AsciiText *text)
{
    int i;
    AsciiPopup *entry;

    if (!g_GameManager.isPaused && !g_GameManager.isInContinueMenu)
    {
        for (entry = text->popups, i = 0; i < 0x2d3; i++, entry++)
        {
            if (!entry->active)
            {
                continue;
            }
            entry->pos.y -= 0.5f * g_App.gameSpeed;
            entry->timer.Tick();
            if ((entry->timer.current > 60) != 0)
            {
                entry->active = 0;
            }
        }
    }
    else if (g_GameManager.isPaused)
    {
        text->pauseMenu.UpdatePauseMenu();
    }
    if (g_GameManager.isInContinueMenu)
    {
        text->continueMenu.UpdateContinueMenu();
    }
    text->RunScripts();
    if ((g_GameManager.unk_93d8 >> 1) & 1)
    {
        if (text->demoIcon.scriptIndex == 0)
        {
            g_SpriteEngine->StartScript(&text->demoIcon, 7);
        }
        g_SpriteEngine->Step(&text->demoIcon);
    }
    else
    {
        text->demoIcon.scriptIndex = 0;
    }
    return 1;
}

int AsciiText::OnDraw(AsciiText *text)
{
    text->DrawStrings();
    text->stringCount = 0;
    text->pauseMenu.DrawPauseMenu();
    text->continueMenu.DrawContinueMenu();
    if (text->demoIcon.scriptIndex)
    {
        g_SpriteEngine->DrawUnrotated(&text->demoIcon);
    }
    return 1;
}

int AsciiText::OnDraw2(AsciiText *text)
{
    text->DrawPopups();
    return 1;
}

void AsciiText::Initialize()
{
    memset(&this->digit, 0, sizeof(this->digit));
    memset(&this->glyph, 0, sizeof(this->glyph));
    memset(this->strings, 0, sizeof(this->strings));
    memset(&this->pauseMenu, 0, sizeof(this->pauseMenu));
    memset(&this->continueMenu, 0, sizeof(this->continueMenu));
    memset(this->popups, 0, sizeof(this->popups));
    this->stringCount = 0;
    this->inArena = 0;
    this->highlight = 0;
    this->nextPopup = 0;
    this->nextPopup2 = 0;
    this->unusedAfterInit74e4 = 0;
    this->color = 0xffffffff;
    this->scaleX = 1.0f;
    this->scaleY = 1.0f;
    this->digit.flags |= 0xc00;
    g_SpriteEngine->InitSprite(&this->digit, 0);
    g_SpriteEngine->InitSprite(&this->glyph, 0x20);
    this->digit.pos.z = 0.1f;
    this->highlight = 0;
    this->glyphWidth = 14;
    this->SetUnk74d4(this->summaryPendingInterrupt);
}

void AsciiText::StartScripts()
{
    g_SpriteEngine->StartScript(&this->summaryPopup, 4);
    g_SpriteEngine->StartScript(&this->summaryDigit, 3);
    g_SpriteEngine->StartScript(&this->summaryBombIcon, 5);
    g_SpriteEngine->StartScript(&this->nearbyLabels[0], 6);
    g_SpriteEngine->StartScript(&this->nearbyLabels[1], 6);
    g_SpriteEngine->StartScript(&this->nearbyLabels[2], 6);
    g_SpriteEngine->StartScript(&this->nearbyLabels[3], 6);
}

int AsciiText::AddedCallback(AsciiText *text)
{
    memset(text, 0, sizeof(AsciiText));
    if (g_SpriteEngine->LoadAnm(1, "data/ascii.anm", 0) != 0)
    {
        return -1;
    }
    if (g_SpriteEngine->LoadAnm(4, "data/capture.anm", 0x724) != 0)
    {
        return -1;
    }
    text->Initialize();
    text->StartScripts();
    return 0;
}

int AsciiText::DeletedCallback(AsciiText *text)
{
    g_SpriteEngine->ReleaseAnm(1);
    g_SpriteEngine->ReleaseAnm(2);
    g_SpriteEngine->ReleaseAnm(4);
    g_SpriteEngine->ReleaseAnm(3);
    return 0;
}

int AsciiText::RegisterChain()
{
    AsciiText *text = &g_AsciiText;

    g_AsciiCalcChain.SetCallback((ChainCallback)AsciiText::OnUpdate);
    g_AsciiCalcChain.addedCallback = (ChainCallback)AsciiText::AddedCallback;
    g_AsciiCalcChain.deletedCallback = (ChainCallback)AsciiText::DeletedCallback;
    g_AsciiCalcChain.arg = text;
    if (g_Chain.AddToCalcChain(&g_AsciiCalcChain, 1) != 0)
    {
        return -1;
    }
    g_AsciiDrawChain.SetCallback((ChainCallback)AsciiText::OnDraw);
    g_AsciiDrawChain.arg = text;
    g_Chain.AddToDrawChain(&g_AsciiDrawChain, 0x10);
    g_AsciiDrawChain2.SetCallback((ChainCallback)AsciiText::OnDraw2);
    g_AsciiDrawChain2.arg = text;
    g_Chain.AddToDrawChain(&g_AsciiDrawChain2, 0xb);
    return 0;
}

void AsciiText::RemoveChain()
{
    g_Chain.Remove(&g_AsciiCalcChain);
    g_Chain.Remove(&g_AsciiDrawChain);
}

void AsciiText::AddString(D3DXVECTOR3 *pos, char *text)
{
    AsciiString *str;

    if (this->stringCount >= 0x100)
    {
        return;
    }
    str = &this->strings[this->stringCount];
    this->stringCount++;
    strcpy(str->text, text);
    str->pos = *pos;
    str->color = this->color;
    str->scaleX = this->scaleX;
    str->scaleY = this->scaleY;
    str->inArena = this->inArena;
    if (g_App.HasFlag0Or8())
    {
        str->highlight = this->highlight;
    }
    else
    {
        str->highlight = 0;
    }
}

void AsciiText::AddFormat(D3DXVECTOR3 *pos, char *fmt, ...)
{
    char buf[0x200];
    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    this->AddString(pos, buf);
    va_end(args);
}

void AsciiText::DrawStrings()
{
    float advance;
    int idx;
    AsciiString *string;
    BYTE *c;
    int clipped = 1;
    D3DXVECTOR3 pos;

    string = this->strings;
    this->glyph.flags |= 1;
    this->glyph.flags |= 0xc00;
    for (idx = 0; idx < this->stringCount; idx++, string++)
    {
        this->glyph.pos = string->pos;
        c = (BYTE *)string->text;
        this->glyph.scaleY = string->scaleX;
        this->glyph.scaleX = string->scaleY;
        advance = this->glyphWidth * string->scaleX;
        if (clipped != string->inArena)
        {
            clipped = string->inArena;
            g_SpriteEngine->FlushQuadBatch();
            if (clipped != 0)
            {
                g_App.viewport.X = g_GameManager.arena[0];
                g_App.viewport.Y = g_GameManager.arena[1];
                g_App.viewport.Width = g_GameManager.arena[2];
                g_App.viewport.Height = g_GameManager.arena[3];
                g_App.d3dDevice->SetViewport(&g_App.viewport);
            }
            else
            {
                g_App.viewport.X = 0;
                g_App.viewport.Y = 0;
                g_App.viewport.Width = 640;
                g_App.viewport.Height = 480;
                g_App.d3dDevice->SetViewport(&g_App.viewport);
            }
        }
        while (*c != 0)
        {
            if (*c == '\n')
            {
                this->glyph.pos.y += 16.0f * string->scaleY;
                this->glyph.pos.x = string->pos.x;
            }
            else if (*c == ' ')
            {
                this->glyph.pos.x += advance;
            }
            else
            {
                if (string->highlight == 0)
                {
                    this->glyph.spriteData = &g_SpriteEngine->sprites[*c - 1];
                    this->glyph.color = string->color;
                }
                else
                {
                    this->glyph.spriteData = &g_SpriteEngine->sprites[*c + 0x7c];
                    this->glyph.color = 0xffffffff;
                }
                g_SpriteEngine->DrawUnrotated(&this->glyph);
                this->glyph.pos.x += advance;
            }
            c++;
        }
    }
    for (idx = 0; idx < 4; idx++)
    {
        if (this->nearbyLabels[idx].pos.x >= 56.0f && this->nearbyLabels[idx].pos.x <= 392.0f)
        {
            advance = fabsf(this->nearbyLabels[idx].pos.x - 32.0f - g_Player.position.x);
            if (advance < 64.0f)
            {
                this->nearbyLabels[idx].alpha = advance * 128.0f / 64.0f + 48.0f;
            }
            else
            {
                this->nearbyLabels[idx].alpha = 0xb0;
            }
            if (this->nearbyLabelDimmed[idx])
            {
                this->nearbyLabels[idx].alpha = 0x80;
                this->nearbyLabels[idx].unk_rgb[2] = 0x40;
                this->nearbyLabels[idx].unk_rgb[1] = 0x40;
                this->nearbyLabels[idx].unk_rgb[0] = 0xff;
            }
            else
            {
                this->nearbyLabels[idx].unk_rgb[2] = 0xff;
                this->nearbyLabels[idx].unk_rgb[1] = 0xff;
                this->nearbyLabels[idx].unk_rgb[0] = 0xff;
            }
            g_SpriteEngine->DrawUnrotated(&this->nearbyLabels[idx]);
        }
    }
}

void AsciiText::AddPopup(D3DXVECTOR3 *pos, int value, D3DCOLOR color)
{
    AsciiPopup *popup;
    int n;

    if (this->nextPopup >= 0x2d0)
    {
        this->nextPopup = 0;
    }
    popup = &this->popups[this->nextPopup];
    popup->active = 1;
    n = 0;
    if (value >= 0)
    {
        while (value != 0)
        {
            popup->digits[n] = value % 10;
            n++;
            value /= 10;
        }
    }
    else
    {
        popup->digits[n] = 10;
        n++;
    }
    if (n == 0)
    {
        popup->digits[n] = 0;
        n++;
    }
    popup->digitCount = n;
    popup->color = color;
    popup->timer.Set(0);
    popup->pos = *pos;
    popup->pos.x += g_GameManager.arena[0];
    popup->pos.y += g_GameManager.arena[1];
    this->nextPopup++;
}

void AsciiText::AddPopup2(D3DXVECTOR3 *pos, int value, D3DCOLOR color)
{
    AsciiPopup *popup;
    int n;

    if (this->nextPopup2 >= 3)
    {
        this->nextPopup2 = 0;
    }
    popup = &this->popups[this->nextPopup2 + 0x2d0];
    popup->active = 1;
    n = 0;
    if (value >= 0)
    {
        while (value != 0)
        {
            popup->digits[n] = value % 10;
            n++;
            value /= 10;
        }
    }
    else
    {
        popup->digits[n] = 10;
        n++;
    }
    if (n == 0)
    {
        popup->digits[n] = 0;
        n++;
    }
    popup->digitCount = n;
    popup->color = color;
    popup->timer.Set(0);
    popup->pos = *pos;
    popup->pos.x += g_GameManager.arena[0];
    popup->pos.y += g_GameManager.arena[1];
    this->nextPopup2++;
}

int PauseMenu::UpdatePauseMenu()
{
    int i;

    if (WAS_PRESSED(8) && this->state != 4)
    {
        g_SoundPlayer.QueueSound(10, 0);
        this->state = 4;
        for (i = 0; i < ARRAY_SIZE(this->sprites); i++)
        {
            if (this->sprites[i].flags & 1)
            {
                this->sprites[i].pendingInterrupt = 2;
            }
        }
        this->stateFrames = 0;
        this->background.pendingInterrupt = 1;
    }
    if (WAS_PRESSED(0x200) && this->state != 9)
    {
        g_SoundPlayer.QueueSound(10, 0);
        this->state = 9;
        for (i = 0; i < ARRAY_SIZE(this->sprites); i++)
        {
            if (this->sprites[i].flags & 1)
            {
                this->sprites[i].pendingInterrupt = 2;
            }
        }
        this->stateFrames = 0;
    }
    if (!((g_GameManager.unk_93d8 >> 3) & 1) && WAS_PRESSED(0x4000) && this->state != 9)
    {
        g_SoundPlayer.QueueSound(10, 0);
        this->state = 10;
        for (i = 0; i < ARRAY_SIZE(this->sprites); i++)
        {
            if (this->sprites[i].flags & 1)
            {
                this->sprites[i].pendingInterrupt = 2;
            }
        }
        this->stateFrames = 0;
    }
    switch (this->state)
    {
    case 0:
        for (i = 0; i < ARRAY_SIZE(this->sprites); i++)
        {
            g_SpriteEngine->StartScript(&this->sprites[i], i + 0xfe);
        }
        for (i = 0; i < 4; i++)
        {
            this->sprites[i].pendingInterrupt = 1;
        }
        g_SpriteEngine->SetSprite(&this->sprites[7], g_GameManager.difficulty + 0x10d);
        if (!(g_GameManager.unk_93d8 & 1))
        {
            this->sprites[8].ClearShown();
        }
        if (!g_GameManager.unk_4->scoreDisabled)
        {
            this->sprites[9].ClearShown();
        }
        if ((g_GameManager.unk_93d8 >> 3) & 1)
        {
            this->sprites[3].scriptPos = NULL;
        }
        this->state++;
        this->stateFrames = 0;
        if ((g_App.stateFlags >> 1) & 1)
        {
            g_SpriteEngine->StartScript(&this->background, 0x724);
            if (g_SpriteEngine->SetPendingCopy(4, 0x20, 0x10, 0x180, 0x1c0, this->background.spriteData->startX,
                                               this->background.spriteData->startY,
                                               this->background.spriteData->height,
                                               this->background.spriteData->width) != 0)
            {
                this->state = 0;
                return 0;
            }
            this->background.pos.x = 32.0f;
            this->background.pos.y = 16.0f;
            this->background.pos.z = 0.0f;
        }
    case 1:
        this->sprites[1].color = 0xffffffff;
        this->sprites[3].color = 0x80303030;
        this->sprites[2].color = 0x80303030;
        this->sprites[1].posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
        this->sprites[2].posOffset = this->sprites[3].posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        if (this->stateFrames >= 4)
        {
            if (!((g_GameManager.unk_93d8 >> 3) & 1))
            {
                if (WAS_PRESSED(0x10))
                {
                    this->state = 3;
                    g_SoundPlayer.QueueSound(0, 0);
                }
            }
            else if (WAS_PRESSED(0x10))
            {
                this->state = 2;
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (WAS_PRESSED(0x20))
            {
                this->state = 2;
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (WAS_PRESSED(0x1001))
            {
                g_SoundPlayer.QueueSound(10, 0);
                for (i = 0; i < 4; i++)
                {
                    this->sprites[i].pendingInterrupt = 2;
                }
                this->state = 4;
                this->stateFrames = 0;
                this->background.pendingInterrupt = 1;
            }
        }
        break;
    case 2:
        this->sprites[3].color = 0x80303030;
        this->sprites[1].color = 0x80303030;
        this->sprites[2].color = 0xffffffff;
        this->sprites[1].posOffset = this->sprites[3].posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        this->sprites[2].posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
        if (this->stateFrames >= 4)
        {
            if (WAS_PRESSED(0x10))
            {
                this->state = 1;
                g_SoundPlayer.QueueSound(0, 0);
            }
            if ((g_GameManager.unk_93d8 >> 3) & 1)
            {
                if (WAS_PRESSED(0x20))
                {
                    this->state = 1;
                    g_SoundPlayer.QueueSound(0, 0);
                }
            }
            else if (WAS_PRESSED(0x20))
            {
                this->state = 3;
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (WAS_PRESSED(0x1001))
            {
                g_SoundPlayer.QueueSound(10, 0);
                for (i = 0; i < 4; i++)
                {
                    this->sprites[i].pendingInterrupt = 2;
                }
                for (; i < 7; i++)
                {
                    this->sprites[i].pendingInterrupt = 1;
                }
                this->state = 6;
                this->stateFrames = 0;
            }
        }
        break;
    case 3:
        this->sprites[2].color = 0x80303030;
        this->sprites[1].color = 0x80303030;
        this->sprites[3].color = 0xffffffff;
        this->sprites[1].posOffset = this->sprites[2].posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        this->sprites[3].posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
        if (this->stateFrames >= 4)
        {
            if (WAS_PRESSED(0x10))
            {
                this->state = 2;
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (WAS_PRESSED(0x20))
            {
                this->state = 1;
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (WAS_PRESSED(0x1001))
            {
                g_SoundPlayer.QueueSound(10, 0);
                for (i = 0; i < 4; i++)
                {
                    this->sprites[i].pendingInterrupt = 2;
                }
                for (; i < 7; i++)
                {
                    this->sprites[i].pendingInterrupt = 1;
                }
                this->state = 8;
                this->stateFrames = 0;
            }
        }
        break;
    case 4:
        if (this->stateFrames >= 20)
        {
            this->state = 0;
            g_GameManager.isPaused = 0;
            for (i = 0; i < ARRAY_SIZE(this->sprites); i++)
            {
                this->sprites[i].ClearShown();
            }
            if (g_GameManager.stage != 6 || g_Gui.frameCounter >= 300)
            {
                g_SoundPlayer.QueueSoundCommand(7, 0, "UnPause");
            }
            g_App.lastTotalPlayTimeTick = timeGetTime();
        }
        break;
    case 5:
    case 7:
        this->sprites[5].color = 0xffff8080;
        this->sprites[6].color = 0x80808080;
        this->sprites[5].posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
        this->sprites[6].posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        if (this->stateFrames >= 4)
        {
            if (WAS_PRESSED(0x10) || WAS_PRESSED(0x20))
            {
                if (this->state == 5)
                {
                    this->state = 6;
                }
                else
                {
                    this->state = 8;
                }
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (WAS_PRESSED(0x1001))
            {
                g_SoundPlayer.QueueSound(10, 0);
                for (i = 4; i < 7; i++)
                {
                    this->sprites[i].pendingInterrupt = 2;
                }
                if (this->state == 5)
                {
                    this->state = 9;
                }
                else
                {
                    this->state = 10;
                }
                this->stateFrames = 0;
            }
        }
        break;
    case 6:
    case 8:
        this->sprites[5].color = 0x80808080;
        this->sprites[6].color = 0xffff8080;
        this->sprites[5].posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        this->sprites[6].posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
        if (this->stateFrames >= 4)
        {
            if (WAS_PRESSED(0x10) || WAS_PRESSED(0x20))
            {
                if (this->state == 6)
                {
                    this->state = 5;
                }
                else
                {
                    this->state = 7;
                }
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (WAS_PRESSED(0x1001))
            {
                g_SoundPlayer.QueueSound(10, 0);
                for (i = 0; i < 4; i++)
                {
                    this->sprites[i].pendingInterrupt = 1;
                }
                for (; i < 7; i++)
                {
                    this->sprites[i].pendingInterrupt = 2;
                }
                if (this->state == 6)
                {
                    this->state = 2;
                }
                else
                {
                    this->state = 3;
                }
                this->stateFrames = 0;
            }
        }
        break;
    case 9:
        if (this->stateFrames >= 20)
        {
            this->state = 0;
            g_GameManager.isPaused = 0;
            g_App.scene = 1;
            for (i = 0; i < ARRAY_SIZE(this->sprites); i++)
            {
                this->sprites[i].ClearShown();
            }
            g_App.lastTotalPlayTimeTick = timeGetTime();
        }
        break;
    case 10:
        if (this->stateFrames >= 20)
        {
            this->state = 0;
            g_GameManager.isPaused = 0;
            g_App.scene = 10;
            for (i = 0; i < ARRAY_SIZE(this->sprites); i++)
            {
                this->sprites[i].ClearShown();
            }
            g_App.lastTotalPlayTimeTick = timeGetTime();
        }
    }
    for (i = 0; i < ARRAY_SIZE(this->sprites); i++)
    {
        g_SpriteEngine->Step(&this->sprites[i]);
    }
    if ((g_App.stateFlags >> 1) & 1)
    {
        g_SpriteEngine->Step(&this->background);
    }
    this->stateFrames++;
    return 0;
}

void PauseMenu::DrawPauseMenu()
{
    DWORD i;

    if (g_GameManager.isPaused)
    {
        g_SpriteEngine->FlushQuadBatch();
        g_App.viewport.X = g_GameManager.arena[0];
        g_App.viewport.Y = g_GameManager.arena[1];
        g_App.viewport.Width = g_GameManager.arena[2];
        g_App.viewport.Height = g_GameManager.arena[3];
        g_App.d3dDevice->SetViewport(&g_App.viewport);
        if ((g_App.stateFlags >> 1) & 1 && this->state != 0)
        {
            Sprite sprite = this->background;

            sprite.flags |= 0x1000;
            g_SpriteEngine->DrawUnrotated(&sprite);
        }
        for (i = 0; i < ARRAY_SIZE(this->sprites); i++)
        {
            if (this->sprites[i].flags & 1)
            {
                g_SpriteEngine->DrawUnrotated(&this->sprites[i]);
            }
        }
    }
}

int ContinueMenu::UpdateContinueMenu()
{
    int i;

    if (g_GameManager.unk_93d8 & 1)
    {
        g_GameManager.isInContinueMenu = 0;
        g_GameManager.stats->guiScore = g_GameManager.stats->score;
        g_App.scene = 6;
        return 1;
    }
    if ((g_GameManager.unk_93d8 >> 3) & 1)
    {
        g_GameManager.isInContinueMenu = 0;
        g_App.scene = 7;
        g_GameManager.stats->guiScore = g_GameManager.stats->score;
        return 1;
    }
    if (g_GameManager.stats->continuesUsed >= g_GameManager.maxContinues || g_GameManager.difficulty >= 4)
    {
        g_GameManager.isInContinueMenu = 0;
        g_App.scene = 6;
        g_GameManager.stats->guiScore = g_GameManager.stats->score;
        return 1;
    }
    switch (this->state)
    {
    case 0:
        if (this->stateFrames == 0)
        {
            g_SoundPlayer.QueueSoundCommand(6, 0, "Pause");
            for (i = 0; i < 4; i++)
            {
                g_SpriteEngine->StartScript(&this->sprites[i], i + 0x108);
                this->sprites[i].pendingInterrupt = 1;
            }
            g_SpriteEngine->StartScript(&this->sprites[4], 0x10c);
            g_SpriteEngine->SetSprite(&this->sprites[4], g_GameManager.maxContinues + 0x106 -
                                                             g_GameManager.stats->continuesUsed);
            this->sprites[4].pendingInterrupt = 1;
            if ((g_App.stateFlags >> 1) & 1)
            {
                g_SpriteEngine->StartScript(&this->background, 0x724);
                if (g_SpriteEngine->SetPendingCopy(4, 0x20, 0x10, 0x180, 0x1c0, this->background.spriteData->startX,
                                                   this->background.spriteData->startY,
                                                   this->background.spriteData->height,
                                                   this->background.spriteData->width) != 0)
                {
                    this->state = 0;
                    return 0;
                }
                this->background.pos.x = 32.0f;
                this->background.pos.y = 16.0f;
                this->background.pos.z = 0.0f;
            }
            g_App.AddTotalPlayTime();
        }
        if (this->stateFrames > 8)
        {
            break;
        }
        this->state += 2;
        this->stateFrames = 0;
    case 1:
        this->sprites[2].color = 0xffff8080;
        this->sprites[3].color = 0x80808080;
        this->sprites[2].posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
        this->sprites[3].posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        if (this->stateFrames >= 4)
        {
            if (WAS_PRESSED(0x10) || WAS_PRESSED(0x20))
            {
                this->state = 2;
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (WAS_PRESSED(0x1001))
            {
                g_SoundPlayer.QueueSound(10, 0);
                for (i = 0; i < 5; i++)
                {
                    this->sprites[i].pendingInterrupt = 2;
                }
                this->state = 3;
                this->background.pendingInterrupt = 1;
                this->stateFrames = 0;
            }
        }
        break;
    case 2:
        this->sprites[3].color = 0xffff8080;
        this->sprites[2].color = 0x80808080;
        this->sprites[3].posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
        this->sprites[2].posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        if (this->stateFrames >= 30)
        {
            if (WAS_PRESSED(0x10) || WAS_PRESSED(0x20))
            {
                this->state = 1;
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (WAS_PRESSED(0x1001))
            {
                g_SoundPlayer.QueueSound(10, 0);
                for (i = 0; i < 5; i++)
                {
                    this->sprites[i].pendingInterrupt = 2;
                }
                this->state = 4;
                this->stateFrames = 0;
            }
        }
        break;
    case 4:
        if (this->stateFrames >= 20)
        {
            this->state = 0;
            this->stateFrames = 0;
            g_GameManager.isInContinueMenu = 0;
            g_App.scene = 6;
            for (i = 0; i < 5; i++)
            {
                this->sprites[i].ClearShown();
            }
            g_GameManager.stats->guiScore = g_GameManager.stats->score;
            g_App.lastTotalPlayTimeTick = timeGetTime();
            return 0;
        }
        break;
    case 3:
        if (this->stateFrames >= 30)
        {
            this->state = 0;
            this->stateFrames = 0;
            g_GameManager.isInContinueMenu = 0;
            for (i = 0; i < 5; i++)
            {
                this->sprites[i].ClearShown();
            }
            g_GameManager.stats->continuesUsed++;
            g_GameManager.stats->guiScore = g_GameManager.stats->continuesUsed;
            g_GameManager.stats->unk_8 = 0;
            g_GameManager.stats->score = g_GameManager.stats->guiScore;
            g_GameManager.SetLives(g_GameManager.unk_4->lifeCount);
            g_GameManager.RerollChecksumKeys();
            g_GameManager.SetBombs(g_Player.shotType->bombs);
            g_GameManager.stats->graze = 0;
            g_GameManager.stats->stageClearPoints = 0;
            g_GameManager.stats->pointItemsTotal = 0;
            g_GameManager.stats->power = 0.0f;
            g_GameManager.RerollChecksumKeys();
            g_GameManager.stats->checksumKeyStage = 0;
            g_GameManager.stats->checksumKeyPoints = 50;
            g_GameManager.bonusPoints = g_GameManager.stats->grazeCount;
            g_Gui.livesChanged = 2;
            g_Gui.bombsChanged = 2;
            g_Gui.grazeChanged = 2;
            g_Gui.unk_4_8 = 2;
            g_Gui.powerChanged = 2;
            IncrementCounterCapped((DWORD *)&g_GameManager.plst.counts[g_GameManager.difficulty].plays[0], 999999);
            IncrementCounterCapped((DWORD *)&g_GameManager.plst.counts[6].plays[0], 999999);
            IncrementCounterCapped(
                (DWORD *)&g_GameManager.plst.counts[g_GameManager.difficulty].plays[g_GameManager.shotIndex + 1],
                999999);
            IncrementCounterCapped((DWORD *)&g_GameManager.plst.counts[6].plays[g_GameManager.shotIndex + 1],
                                   999999);
            IncrementCounterCapped((DWORD *)&g_GameManager.plst.counts[g_GameManager.difficulty].continues,
                                   999999);
            IncrementCounterCapped((DWORD *)&g_GameManager.plst.counts[6].continues, 999999);
            g_SoundPlayer.QueueSoundCommand(7, 0, "UnPause");
            g_App.lastTotalPlayTimeTick = timeGetTime();
            return 0;
        }
    }
    for (i = 0; i < 5; i++)
    {
        g_SpriteEngine->Step(&this->sprites[i]);
    }
    if ((g_App.stateFlags >> 1) & 1)
    {
        g_SpriteEngine->Step(&this->background);
    }
    this->stateFrames++;
    return 0;
}

void ContinueMenu::DrawContinueMenu()
{
    int i;

    if (g_GameManager.isInContinueMenu)
    {
        g_SpriteEngine->FlushQuadBatch();
        g_App.viewport.X = g_GameManager.arena[0];
        g_App.viewport.Y = g_GameManager.arena[1];
        g_App.viewport.Width = g_GameManager.arena[2];
        g_App.viewport.Height = g_GameManager.arena[3];
        g_App.d3dDevice->SetViewport(&g_App.viewport);
        if ((g_App.stateFlags >> 1) & 1 && (this->state != 0 || this->stateFrames > 2))
        {
            g_SpriteEngine->DrawUnrotated(&this->background);
        }
        if (this->state == 1 || this->state == 2)
        {
            g_SpriteEngine->DrawUnrotated(&this->sprites[4]);
        }
        for (i = 0; i < 5; i++)
        {
            if (this->sprites[i].flags & 1)
            {
                g_SpriteEngine->DrawUnrotated(&this->sprites[i]);
            }
        }
    }
}

void AsciiText::DrawPopups()
{
    AsciiPopup *p = this->popups;
    int count;
    float dx2;
    float dy;
    int alpha;
    int i;
    BYTE *digitPtr;
    int amount;
    D3DXVECTOR3 unusedPos;
    int step;
    int div;
    int nonzero;

    if (!g_App.flags.noFog)
    {
        g_App.DisableFog();
    }
    g_App.SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
    for (i = 0; i < 0x2d3; i++, p++)
    {
        if (!p->active)
        {
            continue;
        }
        this->digit.pos.x = p->pos.x - (p->digitCount << 2);
        this->digit.pos.y = p->pos.y;
        this->digit.color = p->color;
        dx2 = g_Player.position.x - p->pos.x;
        dy = g_Player.position.y - p->pos.y;
        alpha = dx2 * dx2 + dy * dy;
        if (alpha > 0x1000)
        {
            alpha = 0xd0;
        }
        else if (alpha > 0x400)
        {
            alpha = ((alpha - 0x400) * 0x80) / 0xc00 + 0x50;
        }
        else
        {
            alpha = 0x50;
        }
        digitPtr = p->digits + p->digitCount - 1;
        for (count = p->digitCount; count > 0; count--)
        {
            if (p->timer < 0x34 || *digitPtr == 10)
            {
                this->digit.spriteData = &g_SpriteEngine->sprites[*digitPtr];
                this->digit.alpha = alpha;
            }
            else if (p->timer < 0x38)
            {
                this->digit.spriteData = &g_SpriteEngine->sprites[*digitPtr + 0xb];
                this->digit.alpha = alpha;
            }
            else
            {
                this->digit.spriteData = &g_SpriteEngine->sprites[*digitPtr + 0x15];
                this->digit.alpha = alpha;
            }
            g_SpriteEngine->DrawUnrotated(&this->digit);
            this->digit.pos.x += 8.0f;
            digitPtr--;
        }
    }
    if (this->summaryPopup.flags & 1)
    {
        div = 100000;
        nonzero = 0;
        amount = g_GameManager.bonusPoints - g_GameManager.stats->grazeCount;
        g_SpriteEngine->DrawUnrotated(&this->summaryPopup);
        this->summaryDigit.pos.x = this->summaryPopup.pos.x + 40.0f + 6.0f;
        this->summaryDigit.pos.y = this->summaryPopup.pos.y + 11.0f;
        this->summaryDigit.pos.z = this->summaryPopup.pos.z;
        this->summaryDigit.alpha = this->summaryPopup.alpha;
        if ((g_GameManager.bonusPoints >= g_GameManager.bonusPointsCap) != 0)
        {
            this->summaryDigit.unk_rgb[2] = 0xff;
            this->summaryDigit.unk_rgb[1] = 0xd0;
            this->summaryDigit.unk_rgb[0] = 0x80;
        }
        else if (amount >= 50000)
        {
            this->summaryDigit.unk_rgb[2] = 0xff;
            this->summaryDigit.unk_rgb[1] = 0xff;
            this->summaryDigit.unk_rgb[0] = 0x80;
        }
        else
        {
            this->summaryDigit.unk_rgb[2] = 0xff;
            this->summaryDigit.unk_rgb[1] = 0xff;
            this->summaryDigit.unk_rgb[0] = 0xff;
        }
        for (i = 0; i < 6; i++, div /= 10)
        {
            count = amount / div;
            amount = amount % div;
            if (count != 0)
            {
                nonzero = 1;
            }
            if (nonzero != 0 || div == 1)
            {
                g_SpriteEngine->SetSprite(&this->summaryDigit, count + 0x84);
                g_SpriteEngine->DrawUnrotated(&this->summaryDigit);
            }
            this->summaryDigit.pos.x += 7.0f;
        }
        nonzero = 0;
        amount = g_GameManager.bonusPointsCap - g_GameManager.stats->grazeCount;
        this->summaryDigit.unk_rgb[2] = 0xf0;
        this->summaryDigit.unk_rgb[1] = 0xd0;
        this->summaryDigit.unk_rgb[0] = 0xe0;
        this->summaryDigit.pos.x += 9.0f;
        if (amount < 1000000)
        {
            div = 100000;
            i = 6;
        }
        else
        {
            div = 1000000;
            i = 7;
        }
        for (; i > 0; i--, div /= 10)
        {
            count = amount / div;
            amount = amount % div;
            if (count != 0)
            {
                nonzero = 1;
            }
            if (nonzero != 0 || div == 1)
            {
                g_SpriteEngine->SetSprite(&this->summaryDigit, count + 0x84);
                g_SpriteEngine->DrawUnrotated(&this->summaryDigit);
            }
            this->summaryDigit.pos.x += 7.0f;
        }
        this->summaryDigit.scaleY = 1.0f;
        this->summaryDigit.scaleX = 1.0f;
        nonzero = 0;
        this->summaryDigit.pos.x = this->summaryPopup.pos.x + 40.0f + 6.0f + 7.0f;
        this->summaryDigit.pos.y = this->summaryPopup.pos.y + 2.0f;
        amount = g_GameManager.invulnerabilityBonus - g_GameManager.stats->grazeCount;
        if (g_Player.bombState)
        {
            this->summaryDigit.unk_rgb[2] = 0xff;
            div = amount % 4000;
            if (div >= 2000)
            {
                div = 4000 - div;
            }
            this->summaryDigit.unk_rgb[1] = amount * 0xc0 / 50000 + div * 0x40 / 2000;
            this->summaryDigit.unk_rgb[0] = amount * 0xc0 / 50000 + div * 0x40 / 2000;
            this->summaryDigit.scaleY = 1.41f;
            this->summaryDigit.scaleX = 1.41f;
            step = 10;
            this->summaryDigit.pos.x += 2.0f;
            this->summaryDigit.pos.y -= 2.0f;
        }
        else
        {
            this->summaryDigit.unk_rgb[2] = 0xc0;
            this->summaryDigit.unk_rgb[1] = 0x80;
            this->summaryDigit.unk_rgb[0] = 0xb0;
            step = 7;
        }
        div = 10000;
        for (i = 0; i < 5; i++, div /= 10)
        {
            count = amount / div;
            amount = amount % div;
            if (count != 0)
            {
                nonzero = 1;
            }
            if (nonzero != 0 || div == 1)
            {
                g_SpriteEngine->SetSprite(&this->summaryDigit, count + 0x84);
                g_SpriteEngine->DrawUnrotated(&this->summaryDigit);
            }
            this->summaryDigit.pos.x += step;
        }
        this->summaryDigit.scaleY = 1.0f;
        this->summaryDigit.scaleX = 1.0f;
        if (g_Player.bombState == 1)
        {
            this->summaryBombIcon.pos = this->summaryPopup.pos;
            this->summaryBombIcon.pos.x += 24.0f;
            this->summaryBombIcon.pos.y += 8.0f;
            this->summaryBombIcon.color = this->summaryPopup.color;
            g_SpriteEngine->DrawUnrotated(&this->summaryBombIcon);
        }
    }
}

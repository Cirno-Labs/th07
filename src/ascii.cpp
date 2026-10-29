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
    this->stats->unk_34[2] = g_Rng.GetRandomU32InRange(100000) + 6543;
    this->stats->unk_8c[3] = g_Rng.GetRandomU32InRange(100000) + 6543;
    this->stats->unk_ac = this->stats->unk_34[2];
    this->stats->unk_b0 = this->ComputeChecksum();
    this->unk_9614 = this->stats->unk_b0 + this->stats->unk_8c[3];
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
    this->stats->unk_ac = this->stats->unk_34[2];
    this->stats->unk_b0 = this->ComputeChecksum();
    this->unk_9614 = this->stats->unk_b0 + this->stats->unk_8c[3];
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
    g_SpriteEngine->Step(&this->unk_498);
    g_SpriteEngine->Step(&this->unk_6e4);
    g_SpriteEngine->Step(&this->unk_b7c[0]);
    g_SpriteEngine->Step(&this->unk_b7c[1]);
    g_SpriteEngine->Step(&this->unk_b7c[2]);
    g_SpriteEngine->Step(&this->unk_b7c[3]);
    g_SpriteEngine->Step(&this->unk_930);
}

int AsciiText::OnUpdate(AsciiText *text)
{
    int i;
    AsciiPopup *entry;

    if (!g_Unknown626270.unk_93dc && !g_Unknown626270.isInContinueMenu)
    {
        for (entry = text->popups, i = 0; i < 0x2d3; i++, entry++)
        {
            if (!entry->active)
            {
                continue;
            }
            entry->pos.y -= 0.5f * g_App.unk_178;
            entry->timer.Tick();
            if ((entry->timer.current > 60) != 0)
            {
                entry->active = 0;
            }
        }
    }
    else if (g_Unknown626270.unk_93dc)
    {
        text->pauseMenu.UpdatePauseMenu();
    }
    if (g_Unknown626270.isInContinueMenu)
    {
        text->continueMenu.UpdateContinueMenu();
    }
    text->RunScripts();
    if ((g_Unknown626270.unk_93d8 >> 1) & 1)
    {
        if (text->unk_9e50.scriptIndex == 0)
        {
            g_SpriteEngine->StartScript(&text->unk_9e50, 7);
        }
        g_SpriteEngine->Step(&text->unk_9e50);
    }
    else
    {
        text->unk_9e50.scriptIndex = 0;
    }
    return 1;
}

int AsciiText::OnDraw(AsciiText *text)
{
    text->DrawStrings();
    text->stringCount = 0;
    text->pauseMenu.DrawPauseMenu();
    text->continueMenu.DrawContinueMenu();
    if (text->unk_9e50.scriptIndex)
    {
        g_SpriteEngine->DrawUnrotated(&text->unk_9e50);
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
    this->unk_74dc = 0;
    this->unk_74e0 = 0;
    this->unk_74e4 = 0;
    this->color = 0xffffffff;
    this->scaleX = 1.0f;
    this->scaleY = 1.0f;
    this->digit.flags |= 0xc00;
    g_SpriteEngine->InitSprite(&this->digit, 0);
    g_SpriteEngine->InitSprite(&this->glyph, 0x20);
    this->digit.pos.z = 0.1f;
    this->highlight = 0;
    this->unk_74d8 = 14;
    this->SetUnk74d4(this->unk_74d4);
}

void AsciiText::StartScripts()
{
    g_SpriteEngine->StartScript(&this->unk_498, 4);
    g_SpriteEngine->StartScript(&this->unk_6e4, 3);
    g_SpriteEngine->StartScript(&this->unk_930, 5);
    g_SpriteEngine->StartScript(&this->unk_b7c[0], 6);
    g_SpriteEngine->StartScript(&this->unk_b7c[1], 6);
    g_SpriteEngine->StartScript(&this->unk_b7c[2], 6);
    g_SpriteEngine->StartScript(&this->unk_b7c[3], 6);
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
        advance = this->unk_74d8 * string->scaleX;
        if (clipped != string->inArena)
        {
            clipped = string->inArena;
            g_SpriteEngine->FlushQuadBatch();
            if (clipped != 0)
            {
                g_App.viewport.X = g_Unknown626270.arena[0];
                g_App.viewport.Y = g_Unknown626270.arena[1];
                g_App.viewport.Width = g_Unknown626270.arena[2];
                g_App.viewport.Height = g_Unknown626270.arena[3];
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
        if (this->unk_b7c[idx].pos.x >= 56.0f && this->unk_b7c[idx].pos.x <= 392.0f)
        {
            advance = fabsf(this->unk_b7c[idx].pos.x - 32.0f - g_Player.position.x);
            if (advance < 64.0f)
            {
                this->unk_b7c[idx].alpha = advance * 128.0f / 64.0f + 48.0f;
            }
            else
            {
                this->unk_b7c[idx].alpha = 0xb0;
            }
            if (this->unk_14ac[idx])
            {
                this->unk_b7c[idx].alpha = 0x80;
                this->unk_b7c[idx].unk_rgb[2] = 0x40;
                this->unk_b7c[idx].unk_rgb[1] = 0x40;
                this->unk_b7c[idx].unk_rgb[0] = 0xff;
            }
            else
            {
                this->unk_b7c[idx].unk_rgb[2] = 0xff;
                this->unk_b7c[idx].unk_rgb[1] = 0xff;
                this->unk_b7c[idx].unk_rgb[0] = 0xff;
            }
            g_SpriteEngine->DrawUnrotated(&this->unk_b7c[idx]);
        }
    }
}

void AsciiText::AddPopup(D3DXVECTOR3 *pos, int value, D3DCOLOR color)
{
    AsciiPopup *popup;
    int n;

    if (this->unk_74dc >= 0x2d0)
    {
        this->unk_74dc = 0;
    }
    popup = &this->popups[this->unk_74dc];
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
    popup->pos.x += g_Unknown626270.arena[0];
    popup->pos.y += g_Unknown626270.arena[1];
    this->unk_74dc++;
}

void AsciiText::AddPopup2(D3DXVECTOR3 *pos, int value, D3DCOLOR color)
{
    AsciiPopup *popup;
    int n;

    if (this->unk_74e0 >= 3)
    {
        this->unk_74e0 = 0;
    }
    popup = &this->popups[this->unk_74e0 + 0x2d0];
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
    popup->pos.x += g_Unknown626270.arena[0];
    popup->pos.y += g_Unknown626270.arena[1];
    this->unk_74e0++;
}

void PauseMenu::DrawPauseMenu()
{
    DWORD i;

    if (g_Unknown626270.unk_93dc)
    {
        g_SpriteEngine->FlushQuadBatch();
        g_App.viewport.X = g_Unknown626270.arena[0];
        g_App.viewport.Y = g_Unknown626270.arena[1];
        g_App.viewport.Width = g_Unknown626270.arena[2];
        g_App.viewport.Height = g_Unknown626270.arena[3];
        g_App.d3dDevice->SetViewport(&g_App.viewport);
        if ((g_App.unk_18c >> 1) & 1 && this->state != 0)
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

    if (g_Unknown626270.unk_93d8 & 1)
    {
        g_Unknown626270.isInContinueMenu = 0;
        g_Unknown626270.stats->guiScore = g_Unknown626270.stats->score;
        g_App.scene = 6;
        return 1;
    }
    if ((g_Unknown626270.unk_93d8 >> 3) & 1)
    {
        g_Unknown626270.isInContinueMenu = 0;
        g_App.scene = 7;
        g_Unknown626270.stats->guiScore = g_Unknown626270.stats->score;
        return 1;
    }
    if (g_Unknown626270.stats->continuesUsed >= g_Unknown626270.unk_9630 || g_Unknown626270.difficulty >= 4)
    {
        g_Unknown626270.isInContinueMenu = 0;
        g_App.scene = 6;
        g_Unknown626270.stats->guiScore = g_Unknown626270.stats->score;
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
            g_SpriteEngine->SetSprite(&this->sprites[4], g_Unknown626270.unk_9630 + 0x106 -
                                                             g_Unknown626270.stats->continuesUsed);
            this->sprites[4].pendingInterrupt = 1;
            if ((g_App.unk_18c >> 1) & 1)
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
            g_Unknown626270.isInContinueMenu = 0;
            g_App.scene = 6;
            for (i = 0; i < 5; i++)
            {
                this->sprites[i].ClearShown();
            }
            g_Unknown626270.stats->guiScore = g_Unknown626270.stats->score;
            g_App.unk_194 = timeGetTime();
            return 0;
        }
        break;
    case 3:
        if (this->stateFrames >= 30)
        {
            this->state = 0;
            this->stateFrames = 0;
            g_Unknown626270.isInContinueMenu = 0;
            for (i = 0; i < 5; i++)
            {
                this->sprites[i].ClearShown();
            }
            g_Unknown626270.stats->continuesUsed++;
            g_Unknown626270.stats->guiScore = g_Unknown626270.stats->continuesUsed;
            g_Unknown626270.stats->unk_8 = 0;
            g_Unknown626270.stats->score = g_Unknown626270.stats->guiScore;
            g_Unknown626270.SetLives(g_Unknown626270.unk_4->lifeCount);
            g_Unknown626270.RerollChecksumKeys();
            g_Unknown626270.SetBombs(g_Player.shotType->bombs);
            g_Unknown626270.stats->graze = 0;
            g_Unknown626270.stats->unk_24 = 0;
            g_Unknown626270.stats->pointItemsTotal = 0;
            g_Unknown626270.stats->power = 0.0f;
            g_Unknown626270.RerollChecksumKeys();
            g_Unknown626270.stats->unk_2c = 0;
            g_Unknown626270.stats->unk_30 = 50;
            g_Unknown626270.unk_961c = g_Unknown626270.stats->unk_88;
            g_Unknown49fbf0.livesChanged = 2;
            g_Unknown49fbf0.bombsChanged = 2;
            g_Unknown49fbf0.grazeChanged = 2;
            g_Unknown49fbf0.unk_4_8 = 2;
            g_Unknown49fbf0.powerChanged = 2;
            IncrementCounterCapped((DWORD *)&g_Unknown626270.plst.counts[g_Unknown626270.difficulty].plays[0], 999999);
            IncrementCounterCapped((DWORD *)&g_Unknown626270.plst.counts[6].plays[0], 999999);
            IncrementCounterCapped(
                (DWORD *)&g_Unknown626270.plst.counts[g_Unknown626270.difficulty].plays[g_Unknown626270.unk_93d7 + 1],
                999999);
            IncrementCounterCapped((DWORD *)&g_Unknown626270.plst.counts[6].plays[g_Unknown626270.unk_93d7 + 1],
                                   999999);
            IncrementCounterCapped((DWORD *)&g_Unknown626270.plst.counts[g_Unknown626270.difficulty].continues,
                                   999999);
            IncrementCounterCapped((DWORD *)&g_Unknown626270.plst.counts[6].continues, 999999);
            g_SoundPlayer.QueueSoundCommand(7, 0, "UnPause");
            g_App.unk_194 = timeGetTime();
            return 0;
        }
    }
    for (i = 0; i < 5; i++)
    {
        g_SpriteEngine->Step(&this->sprites[i]);
    }
    if ((g_App.unk_18c >> 1) & 1)
    {
        g_SpriteEngine->Step(&this->background);
    }
    this->stateFrames++;
    return 0;
}

void ContinueMenu::DrawContinueMenu()
{
    int i;

    if (g_Unknown626270.isInContinueMenu)
    {
        g_SpriteEngine->FlushQuadBatch();
        g_App.viewport.X = g_Unknown626270.arena[0];
        g_App.viewport.Y = g_Unknown626270.arena[1];
        g_App.viewport.Width = g_Unknown626270.arena[2];
        g_App.viewport.Height = g_Unknown626270.arena[3];
        g_App.d3dDevice->SetViewport(&g_App.viewport);
        if ((g_App.unk_18c >> 1) & 1 && (this->state != 0 || this->stateFrames > 2))
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
    if (this->unk_498.flags & 1)
    {
        div = 100000;
        nonzero = 0;
        amount = g_Unknown626270.unk_961c - g_Unknown626270.stats->unk_88;
        g_SpriteEngine->DrawUnrotated(&this->unk_498);
        this->unk_6e4.pos.x = this->unk_498.pos.x + 40.0f + 6.0f;
        this->unk_6e4.pos.y = this->unk_498.pos.y + 11.0f;
        this->unk_6e4.pos.z = this->unk_498.pos.z;
        this->unk_6e4.alpha = this->unk_498.alpha;
        if ((g_Unknown626270.unk_961c >= g_Unknown626270.unk_9618) != 0)
        {
            this->unk_6e4.unk_rgb[2] = 0xff;
            this->unk_6e4.unk_rgb[1] = 0xd0;
            this->unk_6e4.unk_rgb[0] = 0x80;
        }
        else if (amount >= 50000)
        {
            this->unk_6e4.unk_rgb[2] = 0xff;
            this->unk_6e4.unk_rgb[1] = 0xff;
            this->unk_6e4.unk_rgb[0] = 0x80;
        }
        else
        {
            this->unk_6e4.unk_rgb[2] = 0xff;
            this->unk_6e4.unk_rgb[1] = 0xff;
            this->unk_6e4.unk_rgb[0] = 0xff;
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
                g_SpriteEngine->SetSprite(&this->unk_6e4, count + 0x84);
                g_SpriteEngine->DrawUnrotated(&this->unk_6e4);
            }
            this->unk_6e4.pos.x += 7.0f;
        }
        nonzero = 0;
        amount = g_Unknown626270.unk_9618 - g_Unknown626270.stats->unk_88;
        this->unk_6e4.unk_rgb[2] = 0xf0;
        this->unk_6e4.unk_rgb[1] = 0xd0;
        this->unk_6e4.unk_rgb[0] = 0xe0;
        this->unk_6e4.pos.x += 9.0f;
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
                g_SpriteEngine->SetSprite(&this->unk_6e4, count + 0x84);
                g_SpriteEngine->DrawUnrotated(&this->unk_6e4);
            }
            this->unk_6e4.pos.x += 7.0f;
        }
        this->unk_6e4.scaleY = 1.0f;
        this->unk_6e4.scaleX = 1.0f;
        nonzero = 0;
        this->unk_6e4.pos.x = this->unk_498.pos.x + 40.0f + 6.0f + 7.0f;
        this->unk_6e4.pos.y = this->unk_498.pos.y + 2.0f;
        amount = g_Unknown626270.unk_9620 - g_Unknown626270.stats->unk_88;
        if (g_Player.unk_240d)
        {
            this->unk_6e4.unk_rgb[2] = 0xff;
            div = amount % 4000;
            if (div >= 2000)
            {
                div = 4000 - div;
            }
            this->unk_6e4.unk_rgb[1] = amount * 0xc0 / 50000 + div * 0x40 / 2000;
            this->unk_6e4.unk_rgb[0] = amount * 0xc0 / 50000 + div * 0x40 / 2000;
            this->unk_6e4.scaleY = 1.41f;
            this->unk_6e4.scaleX = 1.41f;
            step = 10;
            this->unk_6e4.pos.x += 2.0f;
            this->unk_6e4.pos.y -= 2.0f;
        }
        else
        {
            this->unk_6e4.unk_rgb[2] = 0xc0;
            this->unk_6e4.unk_rgb[1] = 0x80;
            this->unk_6e4.unk_rgb[0] = 0xb0;
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
                g_SpriteEngine->SetSprite(&this->unk_6e4, count + 0x84);
                g_SpriteEngine->DrawUnrotated(&this->unk_6e4);
            }
            this->unk_6e4.pos.x += step;
        }
        this->unk_6e4.scaleY = 1.0f;
        this->unk_6e4.scaleX = 1.0f;
        if (g_Player.unk_240d == 1)
        {
            this->unk_930.pos = this->unk_498.pos;
            this->unk_930.pos.x += 24.0f;
            this->unk_930.pos.y += 8.0f;
            this->unk_930.color = this->unk_498.color;
            g_SpriteEngine->DrawUnrotated(&this->unk_930);
        }
    }
}

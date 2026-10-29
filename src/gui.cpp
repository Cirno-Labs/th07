// cflags: /Os
// The /Os file with Unknown49fbf0, th06's Gui, from 0x427b21 to
// Unknown626270 at 0x42d560.

#include <stdio.h>
#include <string.h>

#include "ascii.h"
#include "replay.h"
#include "unsorted.h"

void Unknown49fbf0::EndBombName()
{
    this->impl->bombName.pendingInterrupt = 1;
    this->impl->bombNameBar.SetPendingInterrupt(2);
}

void Unknown49fbf0::EndSpellCardName()
{
    this->impl->spellName.pendingInterrupt = 1;
    this->impl->spellNameBar.SetPendingInterrupt(2);
    this->impl->unk_7774.SetPendingInterrupt(2);
}

void Unknown49fbf0::HideInterface()
{
    this->impl->unk_6e44.spriteIndex = -1;
    this->impl->unk_7090.spriteIndex = -1;
    this->impl->unk_72dc.spriteIndex = -1;
    this->impl->unk_1fba0 = 0;
}

int Unknown49fbf0::Func427be2()
{
    return this->impl->msg.skippable;
}

void Unknown49fbf0::ShowBonusPopup(int bonus)
{
    this->impl->bonusPopup.pos = D3DXVECTOR3(416.0f, 48.0f, 0.0f);
    this->impl->bonusPopup.shown = 1;
    this->impl->bonusPopup.timer.Set(0);
    this->impl->bonusPopup.value = bonus;
    g_App.redrawFrames = 2;
}

void Unknown49fbf0::Func427c81(int points, int unk)
{
    this->impl->unk_209e0.pos = D3DXVECTOR3(416.0f, 168.0f, 0.0f);
    this->impl->unk_209e0.shown = unk;
    this->impl->unk_209e0.timer.Set(0);
    this->impl->unk_209e0.value = points;
    g_App.redrawFrames = 2;
}

void Unknown49fbf0::ShowSpellCardBonus(int value)
{
    this->impl->spellBonusPopup.pos = D3DXVECTOR3(224.0f, 16.0f, 0.0f);
    this->impl->spellBonusPopup.shown = 1;
    this->impl->spellBonusPopup.timer.Set(0);
    this->impl->spellBonusPopup.value = value;
    g_App.redrawFrames = 2;
}

// The rectangle of sprite `sprite` in its texture, in pixels.
__forceinline void GetSpriteRect(RECT *rect, int sprite)
{
    rect->left = g_SpriteEngine->sprites[sprite].startX;
    rect->top = g_SpriteEngine->sprites[sprite].startY;
    rect->right = g_SpriteEngine->sprites[sprite].endX;
    rect->bottom = g_SpriteEngine->sprites[sprite].endY;
}

void CopySpriteToTexture(int sprite)
{
    RECT dstRect;
    RECT spriteRect;

    GetSpriteRect(&dstRect, 0x609);
    GetSpriteRect(&spriteRect, sprite);
    g_SpriteEngine->CopyTextureRect(0x15, 0x16, &dstRect, &spriteRect);
}

int Unknown49fbf0::OnUpdate(Unknown49fbf0 *gui)
{
    if (g_Unknown626270.isFrozen)
    {
        return 1;
    }
    if (gui->impl->unk_209bc != 0)
    {
        g_App.scene = 3;
        gui->impl->unk_209bc = 0;
    }
    gui->UpdateInterface();
    gui->impl->RunMsg();
    gui->unk_0++;
    if (g_Unknown626270.stage == 6 && gui->unk_0 == 300)
    {
        g_App.PlayMidi(0);
    }
    if ((g_Buttons & 0x100) && g_App.redrawFrames < 8)
    {
        g_App.redrawFrames = 8;
    }
    return 1;
}

int Unknown49fbf0::OnDraw(Unknown49fbf0 *gui)
{
    D3DXVECTOR3 cursor;
    char buf[32];

    g_SpriteEngine->shakeY = 0.0f;
    g_SpriteEngine->shakeX = 0.0f;
    if (gui->impl->stageClearShown != 0)
    {
        cursor.x = 144.0f;
        cursor.y = 128.0f;
        cursor.z = 0.0f;
        g_AsciiText.color = 0xffffff40;
        if (g_Unknown626270.stage < 6)
        {
            g_AsciiText.AddFormat(&cursor, "Stage Clear");
        }
        else
        {
            g_AsciiText.AddFormat(&cursor, "All Clear!");
        }
        cursor.y += 32.0f;
        g_AsciiText.color = 0xffffffff;
        g_AsciiText.AddFormat(&cursor, "Clear  = %8d", g_Unknown626270.stage * 1000000);
        cursor.y += 16.0f;
        g_AsciiText.color = 0xffe0e0ff;
        g_AsciiText.AddFormat(&cursor, "Point  = %8d", gui->impl->stageClearPoints * 50000);
        cursor.y += 16.0f;
        g_AsciiText.color = 0xffd0d0ff;
        g_AsciiText.AddFormat(&cursor, "Graze  = %8d", gui->impl->stageClearGraze * 500);
        cursor.y += 16.0f;
        g_AsciiText.color = 0xffd0d0ff;
        g_AsciiText.AddFormat(&cursor, "Cherry = %8d", gui->impl->stageClearCherry * 10);
        if (g_Unknown626270.stage >= 7 ||
            (g_Unknown626270.stage == 6 && !(g_Unknown626270.unk_93d8 & 1) &&
             (!g_Unknown626270.unk_93d8Bit3 || g_ReplayManager->data->stages[4] != NULL)))
        {
            cursor.y += 16.0f;
            g_AsciiText.color = 0xffffff80;
            g_AsciiText.AddFormat(&cursor, "Player =%9d", (int)g_Unknown626270.stats->lives * 20000000);
            cursor.y += 16.0f;
            g_AsciiText.color = 0xffffff80;
            g_AsciiText.AddFormat(&cursor, "Bomb   = %8d", (int)g_Unknown626270.stats->bombs * 4000000);
        }
        cursor.y += 32.0f;
        switch (g_Unknown626270.difficulty)
        {
        case 0:
            g_AsciiText.color = 0xffff8080;
            g_AsciiText.AddFormat(&cursor, "Easy Rank    *0.5");
            break;
        case 1:
            g_AsciiText.color = 0xffff8080;
            g_AsciiText.AddFormat(&cursor, "Normal Rank  *1.0");
            break;
        case 2:
            g_AsciiText.color = 0xffff8080;
            g_AsciiText.AddFormat(&cursor, "Hard Rank    *1.2");
            break;
        case 3:
            g_AsciiText.color = 0xffff8080;
            g_AsciiText.AddFormat(&cursor, "Lunatic Rank *1.5");
            break;
        case 4:
            g_AsciiText.color = 0xffff8080;
            g_AsciiText.AddFormat(&cursor, "Extra Rank   *2.0");
            break;
        case 5:
            g_AsciiText.color = 0xffff8080;
            g_AsciiText.AddFormat(&cursor, "Phantasm Rank*2.0");
            break;
        }
        cursor.y += 16.0f;
        if (g_Unknown626270.difficulty < 4 && !(g_Unknown626270.unk_93d8 & 1))
        {
            switch (g_Unknown626270.unk_4->lifeCount)
            {
            case 3:
                g_AsciiText.color = 0xffff8080;
                g_AsciiText.AddFormat(&cursor, "Player Penalty*0.5");
                cursor.y += 16.0f;
                break;
            case 4:
                g_AsciiText.color = 0xffff8080;
                g_AsciiText.AddFormat(&cursor, "Player Penalty*0.2");
                cursor.y += 16.0f;
                break;
            }
        }
        g_AsciiText.color = 0xffffffff;
        g_AsciiText.AddFormat(&cursor, "Total = %8d0", gui->impl->stageClearTotal);
        g_AsciiText.color = 0xffffffff;
    }
    gui->impl->DrawMsg();
    gui->DrawBanners();
    gui->DrawHud();
    g_AsciiText.inArena = 1;
    if (gui->impl->bonusPopup.shown)
    {
        g_AsciiText.color = 0xffffff80;
        g_AsciiText.AddFormat(&gui->impl->bonusPopup.pos, "BONUS %8d", gui->impl->bonusPopup.value);
        g_AsciiText.color = 0xffffffff;
    }
    switch (gui->impl->unk_209e0.shown)
    {
    case 1:
        g_AsciiText.color = 0xffc0b0ff;
        g_AsciiText.AddFormat(&gui->impl->unk_209e0.pos, "Full Power Mode!");
        g_AsciiText.color = 0xffffffff;
        break;
    case 2:
        g_AsciiText.scaleX = 0.9f;
        g_AsciiText.scaleY = 1.0f;
        g_AsciiText.unk_74d8 = 11;
        g_AsciiText.color = 0xffe0b0ff;
        g_AsciiText.AddFormat(&gui->impl->unk_209e0.pos, "Supernatural Border!!");
        g_AsciiText.color = 0xffffffff;
        g_AsciiText.scaleX = 1.0f;
        g_AsciiText.scaleY = 1.0f;
        g_AsciiText.unk_74d8 = 14;
        break;
    case 3:
        g_AsciiText.color = 0xffc0b0ff;
        g_AsciiText.AddFormat(&gui->impl->unk_209e0.pos, "CherryPoint Max!");
        g_AsciiText.color = 0xffffffff;
        break;
    case 4:
        g_AsciiText.scaleX = 0.9f;
        g_AsciiText.scaleY = 1.0f;
        g_AsciiText.unk_74d8 = 11;
        g_AsciiText.color = 0xffe0b0ff;
        g_AsciiText.AddFormat(&gui->impl->unk_209e0.pos, "Border Bonus %7d", gui->impl->unk_209e0.value);
        g_AsciiText.color = 0xffffffff;
        g_AsciiText.scaleX = 1.0f;
        g_AsciiText.scaleY = 1.0f;
        g_AsciiText.unk_74d8 = 14;
        break;
    }
    if (gui->impl->spellBonusPopup.shown)
    {
        g_AsciiText.color = 0xffff0000;
        gui->impl->spellBonusPopup.pos.x = (384.0f - strlen("Spell Card Bonus!") * 16.0f) / 2.0f + 32.0f;
        gui->impl->spellBonusPopup.pos.y = 80.0f;
        g_AsciiText.AddFormat(&gui->impl->spellBonusPopup.pos, "Spell Card Bonus!");
        gui->impl->spellBonusPopup.pos.y += 16.0f;
        sprintf(buf, "+%d", gui->impl->spellBonusPopup.value);
        gui->impl->spellBonusPopup.pos.x = (384.0f - strlen(buf) * 32.0f) / 2.0f + 32.0f;
        g_AsciiText.scaleX = 2.0f;
        g_AsciiText.scaleY = 2.0f;
        g_AsciiText.color = 0xffff8080;
        g_AsciiText.AddString(&gui->impl->spellBonusPopup.pos, buf);
        g_AsciiText.scaleX = 1.0f;
        g_AsciiText.scaleY = 1.0f;
        g_AsciiText.color = 0xffffffff;
    }
    g_AsciiText.inArena = 0;
    return 1;
}

void Unknown49fbf0::ShowBombName(int sprite, char *name)
{
    g_SpriteEngine->StartScript(&this->impl->bombPortrait, 0x4a1);
    g_SpriteEngine->SetSprite(&this->impl->bombPortrait, sprite);
    g_SpriteEngine->StartScript(&this->impl->unk_5be4, 0x4a4);
    g_SpriteEngine->SetSprite(&this->impl->unk_5be4, 0x4ac);
    g_SpriteEngine->StartScript(&this->impl->unk_607c, 0x4a6);
    g_SpriteEngine->SetSprite(&this->impl->unk_607c, 0x4ac);
    g_SpriteEngine->StartScript(&this->impl->bombName, 0x704);
    g_SpriteEngine->RenderText(&this->impl->bombName, 0xf0f0ff, 0, name);
    this->bombNameWidth = strlen(name) * 15 / 2.0f + 16.0f;
    this->impl->bombNameBar.SetPendingInterrupt(1);
    g_SoundPlayer.QueueSound(14, 0);
    g_App.redrawFrames = 2;
}

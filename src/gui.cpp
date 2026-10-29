// cflags: /Os
// The /Os file with Gui, th06's Gui, from 0x427ae0 to
// GameManager at 0x42d560.

#include <stdio.h>
#include <string.h>

#include "ascii.h"
#include "bullet.h"
#include "enemy.h"
#include "item.h"
#include "player.h"
#include "replay.h"
#include "screeneffect.h"
#include "unsorted.h"

BOOL Gui::IsLoadingShown()
{
    return this->impl->loadingIcon.spriteIndex >= 0 && (this->impl->loadingIcon.flags >> 13 & 1);
}

void Gui::EndBombName()
{
    this->impl->bombName.pendingInterrupt = 1;
    this->impl->bombNameBar.SetPendingInterrupt(2);
}

void Gui::EndSpellCardName()
{
    this->impl->spellName.pendingInterrupt = 1;
    this->impl->spellNameBar.SetPendingInterrupt(2);
    this->impl->spellScoreBar.SetPendingInterrupt(2);
}

void Gui::HideInterface()
{
    this->impl->loadingIcon.spriteIndex = -1;
    this->impl->loadingCaption.spriteIndex = -1;
    this->impl->loadingBar.spriteIndex = -1;
    this->impl->backgroundTileCount = 0;
}

int Gui::IsMsgSkippable()
{
    return this->impl->msg.skippable;
}

void Gui::ShowBonusPopup(int bonus)
{
    this->impl->bonusPopup.pos = D3DXVECTOR3(416.0f, 48.0f, 0.0f);
    this->impl->bonusPopup.shown = 1;
    this->impl->bonusPopup.timer.Set(0);
    this->impl->bonusPopup.value = bonus;
    g_App.redrawFrames = 2;
}

void Gui::ShowFullPowerPopup(int points, int kind)
{
    this->impl->fullPowerPopup.pos = D3DXVECTOR3(416.0f, 168.0f, 0.0f);
    this->impl->fullPowerPopup.shown = kind;
    this->impl->fullPowerPopup.timer.Set(0);
    this->impl->fullPowerPopup.value = points;
    g_App.redrawFrames = 2;
}

void Gui::ShowSpellCardBonus(int value)
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

int Gui::OnUpdate(Gui *gui)
{
    if (g_GameManager.isFrozen)
    {
        return 1;
    }
    if (gui->impl->returnToTitleRequested != 0)
    {
        g_App.scene = 3;
        gui->impl->returnToTitleRequested = 0;
    }
    gui->UpdateInterface();
    gui->impl->RunMsg();
    gui->frameCounter++;
    if (g_GameManager.stage == 6 && gui->frameCounter == 300)
    {
        g_App.PlayMidi(0);
    }
    if ((g_Buttons & 0x100) && g_App.redrawFrames < 8)
    {
        g_App.redrawFrames = 8;
    }
    return 1;
}

int Gui::OnDraw(Gui *gui)
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
        if (g_GameManager.stage < 6)
        {
            g_AsciiText.AddFormat(&cursor, "Stage Clear");
        }
        else
        {
            g_AsciiText.AddFormat(&cursor, "All Clear!");
        }
        cursor.y += 32.0f;
        g_AsciiText.color = 0xffffffff;
        g_AsciiText.AddFormat(&cursor, "Clear  = %8d", g_GameManager.stage * 1000000);
        cursor.y += 16.0f;
        g_AsciiText.color = 0xffe0e0ff;
        g_AsciiText.AddFormat(&cursor, "Point  = %8d", gui->impl->stageClearPoints * 50000);
        cursor.y += 16.0f;
        g_AsciiText.color = 0xffd0d0ff;
        g_AsciiText.AddFormat(&cursor, "Graze  = %8d", gui->impl->stageClearGraze * 500);
        cursor.y += 16.0f;
        g_AsciiText.color = 0xffd0d0ff;
        g_AsciiText.AddFormat(&cursor, "Cherry = %8d", gui->impl->stageClearCherry * 10);
        if (g_GameManager.stage >= 7 ||
            (g_GameManager.stage == 6 && !(g_GameManager.stateFlags & 1) &&
             (!g_GameManager.isInReplay || g_ReplayManager->data->stages[4] != NULL)))
        {
            cursor.y += 16.0f;
            g_AsciiText.color = 0xffffff80;
            g_AsciiText.AddFormat(&cursor, "Player =%9d", (int)g_GameManager.stats->lives * 20000000);
            cursor.y += 16.0f;
            g_AsciiText.color = 0xffffff80;
            g_AsciiText.AddFormat(&cursor, "Bomb   = %8d", (int)g_GameManager.stats->bombs * 4000000);
        }
        cursor.y += 32.0f;
        switch (g_GameManager.difficulty)
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
        if (g_GameManager.difficulty < 4 && !(g_GameManager.stateFlags & 1))
        {
            switch (g_GameManager.savedConfig->lifeCount)
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
    switch (gui->impl->fullPowerPopup.shown)
    {
    case 1:
        g_AsciiText.color = 0xffc0b0ff;
        g_AsciiText.AddFormat(&gui->impl->fullPowerPopup.pos, "Full Power Mode!");
        g_AsciiText.color = 0xffffffff;
        break;
    case 2:
        g_AsciiText.scaleX = 0.9f;
        g_AsciiText.scaleY = 1.0f;
        g_AsciiText.glyphWidth = 11;
        g_AsciiText.color = 0xffe0b0ff;
        g_AsciiText.AddFormat(&gui->impl->fullPowerPopup.pos, "Supernatural Border!!");
        g_AsciiText.color = 0xffffffff;
        g_AsciiText.scaleX = 1.0f;
        g_AsciiText.scaleY = 1.0f;
        g_AsciiText.glyphWidth = 14;
        break;
    case 3:
        g_AsciiText.color = 0xffc0b0ff;
        g_AsciiText.AddFormat(&gui->impl->fullPowerPopup.pos, "CherryPoint Max!");
        g_AsciiText.color = 0xffffffff;
        break;
    case 4:
        g_AsciiText.scaleX = 0.9f;
        g_AsciiText.scaleY = 1.0f;
        g_AsciiText.glyphWidth = 11;
        g_AsciiText.color = 0xffe0b0ff;
        g_AsciiText.AddFormat(&gui->impl->fullPowerPopup.pos, "Border Bonus %7d", gui->impl->fullPowerPopup.value);
        g_AsciiText.color = 0xffffffff;
        g_AsciiText.scaleX = 1.0f;
        g_AsciiText.scaleY = 1.0f;
        g_AsciiText.glyphWidth = 14;
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

void Gui::ShowBombName(int sprite, char *name)
{
    g_SpriteEngine->StartScript(&this->impl->bombPortrait, 0x4a1);
    g_SpriteEngine->SetSprite(&this->impl->bombPortrait, sprite);
    g_SpriteEngine->StartScript(&this->impl->bombPortraitGlow, 0x4a4);
    g_SpriteEngine->SetSprite(&this->impl->bombPortraitGlow, 0x4ac);
    g_SpriteEngine->StartScript(&this->impl->bombPortraitFrame, 0x4a6);
    g_SpriteEngine->SetSprite(&this->impl->bombPortraitFrame, 0x4ac);
    g_SpriteEngine->StartScript(&this->impl->bombName, 0x704);
    g_SpriteEngine->RenderText(&this->impl->bombName, 0xf0f0ff, 0, name);
    this->bombNameWidth = strlen(name) * 15 / 2.0f + 16.0f;
    this->impl->bombNameBar.SetPendingInterrupt(1);
    g_SoundPlayer.QueueSound(14, 0);
    g_App.redrawFrames = 2;
}

void Gui::ShowSpellCardName(int sprite, char *text)
{
    if (sprite >= 0)
    {
        g_SpriteEngine->StartScript(&this->impl->spellPortrait, 0x4a3);
        g_SpriteEngine->SetSprite(&this->impl->spellPortrait, sprite + 0x4ad);
        if (this->impl->spellPortrait.spriteData->width > 256.0f)
        {
            this->impl->spellPortrait.posOffset.x = -288.0f;
        }
        else if (this->impl->spellPortrait.spriteData->width > 128.0f)
        {
            this->impl->spellPortrait.posOffset.x = -112.0f;
        }
        else
        {
            this->impl->spellPortrait.posOffset.x = 0.0f;
        }
    }
    g_SpriteEngine->StartScript(&this->impl->spellPortraitGlow, 0x4a5);
    g_SpriteEngine->SetSprite(&this->impl->spellPortraitGlow, 0x4ac);
    g_SpriteEngine->StartScript(&this->impl->spellPortraitFrame, 0x4a7);
    g_SpriteEngine->SetSprite(&this->impl->spellPortraitFrame, 0x4ac);
    g_SpriteEngine->StartScript(&this->impl->spellName, 0x705);
    g_SpriteEngine->RenderTextRightAligned(&this->impl->spellName, 0xfff0f0, 0, text);
    this->spellNameWidth = strlen(text) * 15 / 2.0f + 16.0f;
    this->impl->spellNameBar.SetPendingInterrupt(1);
    this->impl->spellScoreBar.SetPendingInterrupt(1);
    g_SoundPlayer.QueueSound(14, 0);
    g_App.redrawFrames = 2;
}

int Gui::LoadResources()
{
    int x;
    int y;
    int i;

    this->frameCounter = 0;
    if ((g_App.scene != 3 && g_App.scene != 0xb && g_App.scene != 0xc) != 0)
    {
        memset(this->impl, 0, sizeof(GuiImpl));
        if (g_SpriteEngine->LoadAnm(0x15, "data/front.anm", 0x600) != 0)
        {
            return -1;
        }
        this->HideInterface();
        switch (g_GameManager.character)
        {
        case 0:
            if (g_SpriteEngine->LoadAnm(0x19, "data/face_rm00.anm", 0x4a0) != 0)
            {
                return -1;
            }
            if (g_SpriteEngine->LoadAnm(0x17, "data/loading.anm", 0x61e) != 0)
            {
                return -1;
            }
            break;
        case 1:
            if (g_SpriteEngine->LoadAnm(0x19, "data/face_mr00.anm", 0x4a0) != 0)
            {
                return -1;
            }
            if (g_SpriteEngine->LoadAnm(0x17, "data/loading2.anm", 0x61e) != 0)
            {
                return -1;
            }
            break;
        case 2:
            if (g_SpriteEngine->LoadAnm(0x19, "data/face_sk00.anm", 0x4a0) != 0)
            {
                return -1;
            }
            if (g_SpriteEngine->LoadAnm(0x17, "data/loading3.anm", 0x61e) != 0)
            {
                return -1;
            }
            break;
        }
    }
    else
    {
        this->HideInterface();
        g_SpriteEngine->StartScript(&this->impl->loadingBar, 0x725);
        this->impl->loadingBar.pendingInterrupt = 1;
        g_SpriteEngine->SetPendingCopy(4, 0x20, 0x10, 0x180, 0x1c0, this->impl->loadingBar.spriteData->startX,
                                       this->impl->loadingBar.spriteData->startY,
                                       this->impl->loadingBar.spriteData->width,
                                       this->impl->loadingBar.spriteData->height);
        for (y = 0; y < 0xe; y++)
        {
            for (x = 0; x < 0xc; x++)
            {
                g_SpriteEngine->StartScript(&this->impl->backgroundTiles[y * 0xc + x], ((y + x) & 1) + 0x726);
                this->impl->backgroundTiles[y * 0xc + x].scriptVar8 = y + x * 2;
                this->impl->backgroundTiles[y * 0xc + x].pos.x = x * 32.0f - 0.5f + 16.0f;
                this->impl->backgroundTiles[y * 0xc + x].pos.y = y * 32.0f - 0.5f + 16.0f;
                this->impl->backgroundTiles[y * 0xc + x].pos.z = 0.0f;
                this->impl->backgroundTiles[y * 0xc + x].uvScrollX = x * 32.0f / 512.0f;
                this->impl->backgroundTiles[y * 0xc + x].uvScrollY = y * 32.0f / 512.0f;
            }
        }
        this->impl->backgroundTileCount = 0xa8;
    }
    switch (g_GameManager.stage)
    {
    case 1:
        CopySpriteToTexture(0x60e);
        if (g_SpriteEngine->LoadAnm(0x1c, "data/face_01_00.anm", 0x4ad) != 0)
        {
            return -1;
        }
        if (g_SpriteEngine->LoadAnm(0x18, "data/std1txt.anm", 0x800) != 0)
        {
            return -1;
        }
        if (this->LoadMsg("data/msg1.dat") != 0)
        {
            return -1;
        }
        break;
    case 2:
        CopySpriteToTexture(0x610);
        if (g_SpriteEngine->LoadAnm(0x1c, "data/face_02_00.anm", 0x4ad) != 0)
        {
            return -1;
        }
        if (g_SpriteEngine->LoadAnm(0x18, "data/std2txt.anm", 0x800) != 0)
        {
            return -1;
        }
        if (this->LoadMsg("data/msg2.dat") != 0)
        {
            return -1;
        }
        break;
    case 3:
        CopySpriteToTexture(0x612);
        if (g_SpriteEngine->LoadAnm(0x1c, "data/face_03_00.anm", 0x4ad) != 0)
        {
            return -1;
        }
        if (g_SpriteEngine->LoadAnm(0x18, "data/std3txt.anm", 0x800) != 0)
        {
            return -1;
        }
        if (this->LoadMsg("data/msg3.dat") != 0)
        {
            return -1;
        }
        break;
    case 4:
        CopySpriteToTexture(0x614);
        if (g_SpriteEngine->LoadAnm(0x1c, "data/face_04_00.anm", 0x4ad) != 0)
        {
            return -1;
        }
        if (g_SpriteEngine->LoadAnm(0x18, "data/std4txt.anm", 0x800) != 0)
        {
            return -1;
        }
        if (this->LoadMsg("data/msg4.dat") != 0)
        {
            return -1;
        }
        break;
    case 5:
        CopySpriteToTexture(0x616);
        if (g_SpriteEngine->LoadAnm(0x1c, "data/face_05_00.anm", 0x4ad) != 0)
        {
            return -1;
        }
        if (g_SpriteEngine->LoadAnm(0x18, "data/std5txt.anm", 0x800) != 0)
        {
            return -1;
        }
        if (this->LoadMsg("data/msg5.dat") != 0)
        {
            return -1;
        }
        break;
    case 6:
        CopySpriteToTexture(0x618);
        if (g_SpriteEngine->LoadAnm(0x1c, "data/face_06_00.anm", 0x4ad) != 0)
        {
            return -1;
        }
        if (g_SpriteEngine->LoadAnm(0x18, "data/std6txt.anm", 0x800) != 0)
        {
            return -1;
        }
        if (this->LoadMsg("data/msg6.dat") != 0)
        {
            return -1;
        }
        break;
    case 7:
        CopySpriteToTexture(0x61a);
        if (g_SpriteEngine->LoadAnm(0x1c, "data/face_07_00.anm", 0x4ad) != 0)
        {
            return -1;
        }
        if (g_SpriteEngine->LoadAnm(0x18, "data/std7txt.anm", 0x800) != 0)
        {
            return -1;
        }
        if (this->LoadMsg("data/msg7.dat") != 0)
        {
            return -1;
        }
        break;
    case 8:
        CopySpriteToTexture(0x61c);
        if (g_SpriteEngine->LoadAnm(0x1c, "data/face_08_00.anm", 0x4ad) != 0)
        {
            return -1;
        }
        if (g_SpriteEngine->LoadAnm(0x18, "data/std8txt.anm", 0x800) != 0)
        {
            return -1;
        }
        if (this->LoadMsg("data/msg8.dat") != 0)
        {
            return -1;
        }
        break;
    default:
        return -1;
    }
    if ((g_App.scene != 3 && g_App.scene != 0xb && g_App.scene != 0xc) != 0)
    {
        for (i = 0; i < 0x21; i++)
        {
            g_SpriteEngine->StartScript(&this->impl->sprites[i], i + 0x600);
        }
    }
    this->bossPresent = 0;
    this->impl->bossBarState = 0;
    this->bossHealth = 0.0f;
    this->bossHealthShown = 0.0f;
    g_SpriteEngine->StartScript(&this->impl->bombPortrait, 0x4a1);
    g_SpriteEngine->StartScript(&this->impl->spellPortrait, 0x4a3);
    g_SpriteEngine->StartScript(&this->impl->bombName, 0x704);
    g_SpriteEngine->StartScript(&this->impl->spellName, 0x705);
    g_SpriteEngine->StartSpriteSeries(this->impl->trackNumberSprites, 0x800, 5);
    g_SpriteEngine->StartScript(&this->impl->bombNameBar, 1);
    g_SpriteEngine->StartScript(&this->impl->spellNameBar, 0);
    g_SpriteEngine->StartScript(&this->impl->spellScoreBar, 2);
    g_SpriteEngine->StartScript(&this->impl->spellScoreDigit, 3);
    this->impl->bombPortrait.scriptPos = NULL;
    this->impl->bombPortraitGlow.scriptPos = NULL;
    this->impl->bombPortraitFrame.scriptPos = NULL;
    this->impl->bombName.scriptPos = NULL;
    this->impl->spellPortrait.scriptPos = NULL;
    this->impl->spellPortraitGlow.scriptPos = NULL;
    this->impl->spellPortraitFrame.scriptPos = NULL;
    this->impl->spellName.scriptPos = NULL;
    this->impl->bombPortrait.shown = 0;
    this->impl->bombPortraitGlow.shown = 0;
    this->impl->bombPortraitFrame.shown = 0;
    this->impl->bombName.shown = 0;
    this->impl->spellPortrait.shown = 0;
    this->impl->spellPortraitGlow.shown = 0;
    this->impl->spellPortraitFrame.shown = 0;
    this->impl->spellName.shown = 0;
    this->impl->bombName.fontWidth = 15;
    this->impl->bombName.fontHeight = 15;
    this->impl->spellName.fontWidth = 15;
    this->impl->spellName.fontHeight = 15;
    this->impl->msg.scriptIndex = -1;
    this->impl->stageClearShown = 0;
    this->impl->bonusPopup.shown = 0;
    this->impl->fullPowerPopup.shown = 0;
    this->impl->spellBonusPopup.shown = 0;
    this->livesChanged = 2;
    this->bombsChanged = 2;
    this->grazeChanged = 2;
    this->frameItemsHideTimer = 2;
    this->powerChanged = 2;
    g_App.redrawFrames = 0x10;
    return 0;
}


int Gui::LoadMsg(char *path)
{
    int i;

    this->FreeMsgFile();
    this->impl->msg.msgFile = (MsgFile *)LoadFileData(path, 0);
    if (this->impl->msg.msgFile == NULL)
    {
        LogPrintf(&g_Log, "error : メッセージファイル %s が読み込めませんでした\r\n", path);
        return -1;
    }
    this->impl->msg.scriptIndex = -1;
    this->impl->msg.instr = NULL;
    for (i = 0; i < this->impl->msg.msgFile->count; i++)
    {
        this->impl->msg.msgFile->scripts[i] = (char *)this->impl->msg.msgFile->scripts[i] + (int)this->impl->msg.msgFile;
    }
    return 0;
}

void Gui::FreeMsgFile()
{
    if (this->impl->msg.msgFile != NULL)
    {
        MemFree(this->impl->msg.msgFile);
        this->impl->msg.msgFile = NULL;
    }
}

void Gui::StartMsg(int index)
{
    this->impl->StartMsgScript(index);
}

void GuiImpl::StartMsgScript(int index)
{
    MsgFile *msgFile;

    if (this->msg.msgFile->count <= index)
    {
        return;
    }
    msgFile = this->msg.msgFile;
    memset(&this->msg, 0, sizeof(GuiMsg));
    this->msg.scriptIndex = index;
    this->msg.msgFile = msgFile;
    this->msg.instr = (MsgInstr *)this->msg.msgFile->scripts[index];
    this->msg.text[0].scriptIndex = -1;
    this->msg.text[1].scriptIndex = -1;
    this->msg.fontSize = 15;
    this->msg.textColors[0] = 0xe8f0ff;
    this->msg.textColors[1] = 0xffe8f0;
    this->msg.shadowColors[0] = 0;
    this->msg.shadowColors[1] = 0;
    this->msg.skippable = 1;
    g_BulletManager.ClearAll(1);
    g_EnemyManager.ClearEnemiesForBonus(0, 0);
    g_ItemManager.CollectAll();
    if (index % 10 == 0)
    {
        switch (g_GameManager.stage)
        {
        case 1:
            CopySpriteToTexture(0x60f);
            break;
        case 2:
            CopySpriteToTexture(0x611);
            break;
        case 3:
            CopySpriteToTexture(0x613);
            break;
        case 4:
            CopySpriteToTexture(0x615);
            break;
        case 5:
            CopySpriteToTexture(0x617);
            break;
        case 6:
            CopySpriteToTexture(0x619);
            g_Stage.dialogueScriptOffset = 2;
            g_ClearedBulletItem = 9;
            break;
        case 7:
            CopySpriteToTexture(0x61b);
            g_Stage.dialogueScriptOffset = 1;
            g_Stage.dialogueSpriteCount = 2;
            g_ClearedBulletItem = 9;
            break;
        case 8:
            CopySpriteToTexture(0x61d);
            g_Stage.dialogueScriptOffset = 2;
            g_ClearedBulletItem = 9;
            break;
        }
    }
}

int GuiImpl::RunMsg()
{
    MsgArgs *args;

    if (this->msg.scriptIndex < 0)
    {
        return -1;
    }
    if (this->msg.timelineRelease > 0)
    {
        this->msg.timelineRelease--;
    }
    if (this->msg.skippable && (g_GameButtons & 0x100))
    {
        this->msg.timer.Set(this->msg.instr->time);
    }
    if (g_Player.bombState)
    {
        g_Player.EndBombState();
    }
    if (g_Player.playerState != 2)
    {
        g_ItemManager.CollectAll();
    }
    while (this->msg.timer >= this->msg.instr->time)
    {
        switch (this->msg.instr->opcode)
        {
        case 0:
            this->msg.scriptIndex = -1;
            return -1;
        case 1:
            args = &this->msg.instr->args;
            g_SpriteEngine->StartScript(&this->msg.portraits[args->who],
                                        args->index + (args->who != 0 ? 0x4a2 : 0x4a0));
            if (this->msg.portraits[args->who].spriteData->width > 128.0f)
            {
                this->msg.portraits[args->who].posOffset.x = -112.0f;
            }
            else
            {
                this->msg.portraits[args->who].posOffset.x = 0.0f;
            }
            break;
        case 2:
            args = &this->msg.instr->args;
            g_SpriteEngine->SetSprite(&this->msg.portraits[args->who], args->index + (args->who != 0 ? 0x4ad : 0x4a0));
            if (this->msg.portraits[args->who].spriteData->width > 256.0f)
            {
                this->msg.portraits[args->who].posOffset.x = -208.0f;
                this->msg.portraits[args->who].posOffset.y = -50.0f;
            }
            else if (this->msg.portraits[args->who].spriteData->width > 128.0f)
            {
                this->msg.portraits[args->who].posOffset.x = -80.0f;
            }
            else
            {
                this->msg.portraits[args->who].posOffset.x = 0.0f;
            }
            break;
        case 3:
            args = &this->msg.instr->args;
            if (args->index == 0 && this->msg.text[1].scriptIndex >= 0)
            {
                g_SpriteEngine->RenderText(&this->msg.text[1], this->msg.textColors[args->who],
                                           this->msg.shadowColors[args->who], " ");
            }
            g_SpriteEngine->StartScript(&this->msg.text[args->index], args->index + 0x700);
            this->msg.text[args->index].fontHeight = this->msg.fontSize;
            this->msg.text[args->index].fontWidth = this->msg.text[args->index].fontHeight;
            g_SpriteEngine->RenderText(&this->msg.text[args->index], this->msg.textColors[args->who],
                                       this->msg.shadowColors[args->who], args->text);
            this->msg.waitFrames = 0;
            break;
        case 4:
            if ((!this->msg.skippable || !(g_GameButtons & 0x100)) &&
                (!(g_GameButtons & 1) || (g_GameButtons & 1) == (g_PrevGameButtons & 1) || this->msg.waitFrames < 12))
            {
                if (this->msg.waitFrames >= this->msg.instr->i)
                {
                    break;
                }
                this->msg.waitFrames++;
                goto stepSprites;
            }
            break;
        case 5:
            args = &this->msg.instr->args;
            if (args->who < 2)
            {
                this->msg.portraits[args->who].pendingInterrupt = args->value;
            }
            else
            {
                this->msg.text[args->who - 2].pendingInterrupt = args->value;
            }
            break;
        case 6:
            this->msg.timelineRelease++;
            break;
        case 7:
            if (g_GameManager.stage != 6)
            {
                g_SpriteEngine->StartScript(&this->trackNumberSprites[0], 0x804);
            }
            else
            {
                g_SpriteEngine->StartScript(&this->trackNumberSprites[0], 0x805);
            }
            g_SpriteEngine->SetSprite(&this->trackNumberSprites[0], this->msg.instr->i + 0x803);
            if (g_App.PlayMidi(this->msg.instr->i) != 0)
            {
                g_App.PlayMusic(g_Stage.data->songPaths[this->msg.instr->i]);
            }
            break;
        case 8:
            args = &this->msg.instr->args;
            g_SpriteEngine->StartScript(&this->msg.nameplates[args->index], args->index + 0x702);
            g_SpriteEngine->RenderTextRightAligned(&this->msg.nameplates[args->index], this->msg.textColors[args->who],
                                                   this->msg.shadowColors[args->who], args->text);
            this->msg.waitFrames = 0;
            break;
        case 9:
            this->stageClearPower = g_GameManager.stats->power;
            this->stageClearPoints = g_GameManager.stats->stageClearPoints;
            this->stageClearCherry = g_GameManager.bonusPointsCap - g_GameManager.stats->grazeCount;
            this->stageClearGraze = g_GameManager.stats->graze;
            this->stageClearShown = 1;
            if (g_GameManager.stage < 6)
            {
                g_SpriteEngine->StartScript(&this->loadingIcon, 0x61e);
                g_SpriteEngine->StartScript(&this->loadingBar, 0x725);
                g_SpriteEngine->SetPendingCopy(4, 0x20, 0x10, 0x180, 0x1c0, this->loadingBar.spriteData->startX,
                                               this->loadingBar.spriteData->startY,
                                               this->loadingBar.spriteData->width,
                                               this->loadingBar.spriteData->height);
            }
            else
            {
                g_GameManager.stats->checksumKeyStage = -1;
            }
            break;
        case 10:
            goto stepSprites;
        case 12:
            g_App.FadeOutMusic(4.0f);
            break;
        case 14:
            ScreenEffect::StartEffect(4, 0x192, 0xffffff, 0, 0);
            g_App.redrawFrames = 0x192;
            break;
        case 11:
            g_App.clockMeasurementPending = 0;
            g_GameManager.stats->guiScore = g_GameManager.stats->score;
            if (g_GameManager.stateFlags & 1)
            {
                g_GameManager.stats->guiScore = g_GameManager.stats->score;
                g_App.scene = 6;
                goto stepSprites;
            }
            if (g_GameManager.stage < 6)
            {
                if ((g_GameManager.stateFlags >> 3 & 1) && !g_ReplayManager->HasStage(g_GameManager.stage))
                {
                    g_App.scene = 7;
                    goto stepSprites;
                }
                g_SpriteEngine->InitSprite(&this->loadingCaption, 0x10c);
                this->returnToTitleRequested = 1;
                this->msg.scriptIndex = -2;
            }
            else if (!(g_GameManager.stateFlags >> 3 & 1))
            {
                if (g_GameManager.difficulty >= 4)
                {
                    if (g_GameManager.difficulty == 4)
                    {
                        g_GameManager.clrds[g_GameManager.shotIndex]
                            .stagesClearedNoContinue[g_GameManager.difficulty] = 99;
                    }
                    g_GameManager.plst.counts[g_GameManager.difficulty].clears++;
                    g_GameManager.isGameCleared = 1;
                    g_GameManager.stats->guiScore = g_GameManager.stats->score;
                    g_App.scene = 6;
                    goto stepSprites;
                }
                g_GameManager.isGameCleared = 1;
                g_GameManager.stats->guiScore = g_GameManager.stats->score;
                g_App.scene = 9;
                goto stepSprites;
            }
            else
            {
                if (g_GameManager.stage == 8 && g_GameManager.stats->score != g_ReplayManager->data->score)
                {
                    ResaveReplay(g_GameManager.replayFile);
                }
                g_App.scene = 7;
            }
            goto stepSprites;
        case 13:
            this->msg.skippable = this->msg.instr->b;
            break;
        }
        this->msg.instr = (MsgInstr *)((BYTE *)this->msg.instr + 4 + this->msg.instr->argsSize);
    }
    this->msg.timer.NextTick();
stepSprites:
    g_SpriteEngine->Step(&this->msg.portraits[0]);
    g_SpriteEngine->Step(&this->msg.portraits[1]);
    g_SpriteEngine->Step(&this->msg.text[0]);
    g_SpriteEngine->Step(&this->msg.text[1]);
    g_SpriteEngine->Step(&this->msg.nameplates[0]);
    g_SpriteEngine->Step(&this->msg.nameplates[1]);
    if (this->msg.timer < 60 && this->msg.skippable && (g_GameButtons & 0x100))
    {
        this->msg.timer.Set(60);
    }
    return 0;
}

BOOL Gui::MsgHoldsTimeline()
{
    if (this->impl == NULL)
    {
        return FALSE;
    }
    if (this->impl->msg.timelineRelease > 0)
    {
        return FALSE;
    }
    return this->impl->msg.scriptIndex >= 0;
}

int Gui::IsMsgRunning()
{
    if (this->impl == NULL)
    {
        return 0;
    }
    return this->impl->msg.scriptIndex >= 0 || this->impl->msg.scriptIndex == -2 ? 1 : 0;
}

int Gui::AddedCallback(Gui *gui)
{
    return gui->LoadResources();
}

int Gui::DeletedCallback(Gui *gui)
{
    g_SpriteEngine->ReleaseAnm(0x18);
    g_SpriteEngine->ReleaseAnm(0x1c);
    g_SpriteEngine->ReleaseAnm(0x1d);
    g_SpriteEngine->ReleaseAnm(0x1e);
    g_SpriteEngine->ReleaseAnm(0x1f);
    gui->FreeMsgFile();
    if ((g_App.scene != 3 && g_App.scene != 0xb && g_App.scene != 0xc) != 0)
    {
        g_SpriteEngine->ReleaseAnm(0x15);
        g_SpriteEngine->ReleaseAnm(0x17);
        g_SpriteEngine->ReleaseAnm(0x19);
        g_SpriteEngine->ReleaseAnm(0x1a);
        g_SpriteEngine->ReleaseAnm(0x1b);
        g_SpriteEngine->ReleaseAnm(0x16);
        delete gui->impl;
        gui->impl = NULL;
    }
    return 0;
}

int Gui::RegisterChain()
{
    Gui *gui = &g_Gui;

    if ((g_App.scene != 3 && g_App.scene != 0xb && g_App.scene != 0xc) != 0)
    {
        memset(gui, 0, sizeof(Gui));
        gui->impl = new GuiImpl;
    }
    g_GuiCalcChain.SetCallback((ChainCallback)Gui::OnUpdate);
    g_GuiCalcChain.addedCallback = (ChainCallback)Gui::AddedCallback;
    g_GuiCalcChain.deletedCallback = (ChainCallback)Gui::DeletedCallback;
    g_GuiCalcChain.arg = gui;
    if (g_Chain.AddToCalcChain(&g_GuiCalcChain, 13) != 0)
    {
        return -1;
    }
    g_GuiDrawChain.SetCallback((ChainCallback)Gui::OnDraw);
    g_GuiDrawChain.arg = gui;
    g_Chain.AddToDrawChain(&g_GuiDrawChain, 12);
    return 0;
}

void Gui::RemoveChain()
{
    g_Chain.Remove(&g_GuiCalcChain);
    g_Chain.Remove(&g_GuiDrawChain);
}

int GuiImpl::DrawMsg()
{
    float height;
    D3DXVECTOR3 pos;

    if (this->msg.scriptIndex < 0)
    {
        return -1;
    }
    if (g_GameManager.stage == 6 && (this->msg.scriptIndex == 1 || this->msg.scriptIndex == 11))
    {
        return 0;
    }
    if (this->msg.timer < 60)
    {
        height = this->msg.timer.AsFloat() * 48.0f / 60.0f;
    }
    else
    {
        height = 48.0f;
    }
    VertexDiffuseXyzrwh vertices[4];
    vertices[0].pos = D3DXVECTOR3(g_GameManager.arena[0] + 16.0f, 384.0f, 0.0f);
    vertices[1].pos = D3DXVECTOR3(g_GameManager.arena[0] + 384.0f - 16.0f, 384.0f, 0.0f);
    vertices[2].pos = D3DXVECTOR3(g_GameManager.arena[0] + 16.0f, 384.0f + height, 0.0f);
    vertices[3].pos = D3DXVECTOR3(g_GameManager.arena[0] + 384.0f - 16.0f, 384.0f + height, 0.0f);
    vertices[0].diffuse = vertices[1].diffuse = 0xd0000000;
    vertices[2].diffuse = vertices[3].diffuse = 0x90000000;
    vertices[0].w = vertices[1].w = vertices[2].w = vertices[3].w = 1.0f;
    g_SpriteEngine->DrawUnrotated(&this->msg.portraits[0]);
    pos = this->msg.portraits[1].pos;
    this->msg.portraits[1].pos += this->msg.portraits[1].posOffset;
    g_SpriteEngine->DrawUnrotated(&this->msg.portraits[1]);
    this->msg.portraits[1].pos = pos;
    g_SpriteEngine->FlushQuadBatch();
    if (!g_App.flags.noColorBlending)
    {
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    }
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    if (!g_App.flags.noDepthTest)
    {
        g_App.SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    }
    g_App.d3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
    g_App.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(VertexDiffuseXyzrwh));
    g_SpriteEngine->ClearVertexFormat();
    g_SpriteEngine->ClearColorOp();
    g_SpriteEngine->ClearBlendMode();
    g_SpriteEngine->ClearZWrite();
    if (!g_App.flags.noColorBlending)
    {
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    }
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    g_SpriteEngine->DrawUnrotated(&this->msg.text[0]);
    g_SpriteEngine->DrawUnrotated(&this->msg.text[1]);
    g_SpriteEngine->DrawUnrotated(&this->msg.nameplates[0]);
    g_SpriteEngine->DrawUnrotated(&this->msg.nameplates[1]);
    return 0;
}

void Gui::UpdateInterface()
{
    int i;
    int count;
    int bonus;

    if (this->impl->msg.scriptIndex < 0)
    {
        if (this->bossPresent)
        {
            if (this->impl->bossBarState == 0)
            {
                this->impl->sprites[11].SetPendingInterrupt(1);
                this->impl->bossBarState = 1;
                this->bossBarAlpha = 0;
            }
            else
            {
                if ((this->impl->sprites[11].flags >> 13) & 1)
                {
                    this->impl->bossBarState = 2;
                }
                if (this->bossBarAlpha < 0xfc)
                {
                    this->bossBarAlpha += 4;
                }
                else
                {
                    this->bossBarAlpha = 0xff;
                }
            }
        }
        else if (this->impl->bossBarState != 0)
        {
            if (this->impl->bossBarState <= 2)
            {
                this->impl->sprites[11].SetPendingInterrupt(2);
                this->impl->bossBarState = 3;
            }
            if (this->bossBarAlpha > 0)
            {
                this->bossBarAlpha -= 4;
            }
            else
            {
                this->bossBarAlpha = 0;
            }
            if ((this->impl->sprites[11].flags >> 13) & 1)
            {
                this->impl->bossBarState = 0;
                this->bossHealthShown = 0.0f;
                this->bossBarAlpha = 0;
            }
        }
        if (this->impl->bossBarState >= 2)
        {
            if (this->bossHealth > this->bossHealthShown)
            {
                this->bossHealthShown += 0.01f;
                if (this->bossHealth < this->bossHealthShown)
                {
                    this->bossHealthShown = this->bossHealth;
                }
            }
            else if (this->bossHealth < this->bossHealthShown)
            {
                this->bossHealthShown -= 0.02f;
                if (this->bossHealth > this->bossHealthShown)
                {
                    this->bossHealthShown = this->bossHealth;
                }
            }
        }
    }
    g_SpriteEngine->StepSprites(this->impl->sprites, 0x21);
    g_SpriteEngine->StepSprites(this->impl->trackNumberSprites, 5);
    g_SpriteEngine->Step(&this->impl->bombPortrait);
    g_SpriteEngine->Step(&this->impl->bombPortraitGlow);
    g_SpriteEngine->Step(&this->impl->bombPortraitFrame);
    g_SpriteEngine->Step(&this->impl->bombName);
    g_SpriteEngine->Step(&this->impl->spellPortrait);
    g_SpriteEngine->Step(&this->impl->spellPortraitGlow);
    g_SpriteEngine->Step(&this->impl->spellPortraitFrame);
    g_SpriteEngine->Step(&this->impl->spellName);
    g_SpriteEngine->Step(&this->impl->bombNameBar);
    g_SpriteEngine->Step(&this->impl->spellNameBar);
    g_SpriteEngine->Step(&this->impl->spellScoreBar);
    if (this->impl->loadingIcon.spriteIndex >= 0)
    {
        if (g_SpriteEngine->Step(&this->impl->loadingIcon) != 0)
        {
            this->impl->loadingIcon.spriteIndex = -1;
        }
        if (g_SpriteEngine->Step(&this->impl->loadingBar) != 0)
        {
            this->impl->loadingBar.spriteIndex = -1;
        }
    }
    if (this->impl->backgroundTileCount != 0)
    {
        count = 0xa8;
        for (i = 0; i < 0xa8; i++)
        {
            if (g_SpriteEngine->Step(&this->impl->backgroundTiles[i]) != 0)
            {
                count--;
            }
        }
        this->impl->backgroundTileCount = count;
    }
    if (this->impl->bonusPopup.shown)
    {
        if (this->impl->bonusPopup.timer < 30)
        {
            this->impl->bonusPopup.pos.x = this->impl->bonusPopup.timer.AsFloat() * -312.0f / 30.0f + 416.0f;
        }
        else
        {
            this->impl->bonusPopup.pos.x = 104.0f;
        }
        if (this->impl->bonusPopup.timer >= 250)
        {
            this->impl->bonusPopup.shown = 0;
        }
        ++this->impl->bonusPopup.timer;
    }
    if (this->impl->fullPowerPopup.shown)
    {
        if (this->impl->fullPowerPopup.timer < 30)
        {
            this->impl->fullPowerPopup.pos.x = this->impl->fullPowerPopup.timer.AsFloat() * -312.0f / 30.0f + 416.0f;
        }
        else
        {
            this->impl->fullPowerPopup.pos.x = 104.0f;
        }
        if (this->impl->fullPowerPopup.timer >= 180)
        {
            this->impl->fullPowerPopup.shown = 0;
        }
        ++this->impl->fullPowerPopup.timer;
    }
    if (this->impl->spellBonusPopup.shown)
    {
        if (this->impl->spellBonusPopup.timer >= 280)
        {
            this->impl->spellBonusPopup.shown = 0;
        }
        ++this->impl->spellBonusPopup.timer;
    }
    if (this->impl->stageClearShown == 1)
    {
        bonus = 0;
        bonus += g_GameManager.stage * 100000;
        bonus += this->impl->stageClearGraze * 50;
        bonus += this->impl->stageClearPoints * 5000;
        bonus += this->impl->stageClearCherry;
        if (g_GameManager.stage >= 7 ||
            (g_GameManager.stage == 6 && !(g_GameManager.stateFlags & 1) &&
             (!g_GameManager.isInReplay || g_ReplayManager->data->stages[4] != NULL)))
        {
            bonus += (int)g_GameManager.stats->lives * 2000000;
            bonus += (int)g_GameManager.stats->bombs * 400000;
        }
        switch (g_GameManager.difficulty)
        {
        case 0:
            bonus /= 2;
            break;
        case 2:
            bonus = bonus * 12 / 10;
            break;
        case 3:
            bonus = bonus * 15 / 10;
            break;
        case 4:
            bonus *= 2;
            break;
        case 5:
            bonus *= 2;
            break;
        }
        switch (g_GameManager.savedConfig->lifeCount)
        {
        case 3:
            bonus = bonus * 5 / 10;
            break;
        case 4:
            bonus = bonus * 2 / 10;
            break;
        }
        this->impl->stageClearTotal = bonus;
        g_GameManager.AddScore(bonus);
        g_GameManager.AddScore(bonus);
        g_GameManager.AddScore(bonus);
        g_GameManager.AddScore(bonus);
        g_GameManager.AddScore(bonus);
        g_GameManager.AddScore(bonus);
        g_GameManager.AddScore(bonus);
        g_GameManager.AddScore(bonus);
        g_GameManager.AddScore(bonus);
        g_GameManager.AddScore(bonus);
        this->impl->stageClearShown++;
    }
}

void Gui::DrawHud()
{
    float y;
    float xx;
    int idx;
    Sprite *vm;

    g_SpriteEngine->FlushQuadBatch();
    g_App.viewport.X = 0;
    g_App.viewport.Y = 0;
    g_App.viewport.Width = 640;
    g_App.viewport.Height = 480;
    g_App.d3dDevice->SetViewport(&g_App.viewport);
    vm = &this->impl->sprites[12];
    if (g_App.flags.redrawHudEveryFrame || vm->scriptPos != NULL || g_App.redrawFrames != 0)
    {
        for (y = 0.0f; y < 464.0f; y += 32.0f)
        {
            vm->pos = D3DXVECTOR3(0.0f, y, 0.49f);
            g_SpriteEngine->DrawUnrotated(vm);
        }
        for (xx = 416.0f; xx < 624.0f; xx += 32.0f)
        {
            for (y = 16.0f; y < 464.0f; y += 32.0f)
            {
                vm->pos = D3DXVECTOR3(xx, y, 0.49f);
                g_SpriteEngine->DrawUnrotated(vm);
            }
        }
        vm = &this->impl->sprites[13];
        for (xx = 0.0f; xx < 624.0f; xx += 128.0f)
        {
            vm->pos = D3DXVECTOR3(xx, 0.0f, 0.49f);
            g_SpriteEngine->DrawUnrotated(vm);
            vm->pos = D3DXVECTOR3(xx, 464.0f, 0.49f);
            g_SpriteEngine->DrawUnrotated(vm);
        }
        g_SpriteEngine->DrawUnrotated(&this->impl->sprites[0]);
        g_SpriteEngine->Draw(&this->impl->sprites[1]);
        g_SpriteEngine->DrawUnrotated(&this->impl->sprites[2]);
        g_SpriteEngine->DrawUnrotated(&this->impl->sprites[3]);
        g_SpriteEngine->DrawUnrotated(&this->impl->sprites[4]);
        g_SpriteEngine->DrawUnrotated(&this->impl->sprites[5]);
        g_SpriteEngine->DrawUnrotated(&this->impl->sprites[6]);
        g_SpriteEngine->DrawUnrotated(&this->impl->sprites[7]);
        g_SpriteEngine->DrawUnrotated(&this->impl->sprites[8]);
        this->livesChanged = 2;
        this->bombsChanged = 2;
        this->grazeChanged = 2;
        this->frameItemsHideTimer = 2;
        this->powerChanged = 2;
    }
    if (!g_App.flags.noFrameItems)
    {
        vm = &this->impl->sprites[13];
        xx = 496.0f;
        vm->pos = D3DXVECTOR3(xx, 48.0f, 0.49f);
        g_SpriteEngine->DrawUnrotated(vm);
        vm->pos = D3DXVECTOR3(xx, 64.0f, 0.49f);
        g_SpriteEngine->DrawUnrotated(vm);
        if (this->livesChanged)
        {
            vm->pos = D3DXVECTOR3(xx, 96.0f, 0.48f);
            g_SpriteEngine->DrawUnrotated(vm);
        }
        if (this->bombsChanged)
        {
            vm->pos = D3DXVECTOR3(xx, 112.0f, 0.48f);
            g_SpriteEngine->DrawUnrotated(vm);
        }
        if (this->powerChanged)
        {
            vm->pos = D3DXVECTOR3(xx, 144.0f, 0.48f);
            g_SpriteEngine->DrawUnrotated(vm);
        }
        if (this->grazeChanged)
        {
            vm->pos = D3DXVECTOR3(xx, 160.0f, 0.48f);
            g_SpriteEngine->DrawUnrotated(vm);
        }
        if (this->frameItemsHideTimer)
        {
            vm->pos = D3DXVECTOR3(xx, 176.0f, 0.48f);
            g_SpriteEngine->DrawUnrotated(vm);
        }
        vm->pos = D3DXVECTOR3(512.0f, 464.0f, 0.48f);
        g_SpriteEngine->DrawUnrotated(vm);
    }
    if (this->livesChanged)
    {
        vm = &this->impl->sprites[9];
        for (idx = 0, xx = 496.0f; idx < (int)g_GameManager.stats->lives; idx++, xx += 16.0f)
        {
            vm->pos = D3DXVECTOR3(xx, 96.0f, 0.46f);
            g_SpriteEngine->DrawUnrotated(vm);
        }
    }
    if (this->bombsChanged)
    {
        vm = &this->impl->sprites[10];
        for (idx = 0, xx = 496.0f; idx < (int)g_GameManager.stats->bombs; idx++, xx += 16.0f)
        {
            vm->pos = D3DXVECTOR3(xx, 112.0f, 0.46f);
            g_SpriteEngine->DrawUnrotated(vm);
        }
    }
    vm = &this->impl->sprites[13];
    for (xx = 32.0f; xx < 368.0f; xx += 128.0f)
    {
        vm->pos = D3DXVECTOR3(xx, 464.0f, 0.49f);
        g_SpriteEngine->DrawUnrotated(vm);
    }
    {
        D3DXVECTOR3 scorePos(496.0f, 64.0f, 0.0f);

        if (g_GameManager.stats->guiScore < 100000000)
        {
            g_AsciiText.AddFormat(&scorePos, "%.8d", g_GameManager.stats->guiScore);
            scorePos.x += 112.0f;
            g_AsciiText.AddFormat(&scorePos, "%1d", g_GameManager.stats->continuesUsed);
        }
        else
        {
            g_AsciiText.scaleX = 0.9f;
            g_AsciiText.scaleY = 1.0f;
            g_AsciiText.AddFormat(&scorePos, "%.9d", g_GameManager.stats->guiScore);
            // Nine digits 14 wide at the 0.9 scale; the folded product is
            // one ulp below 113.4f.
            scorePos.x += 126.0f * 0.9f;
            g_AsciiText.AddFormat(&scorePos, "%1d", g_GameManager.stats->continuesUsed);
            g_AsciiText.scaleX = 1.0f;
            g_AsciiText.scaleY = 1.0f;
        }
        scorePos = D3DXVECTOR3(496.0f, 48.0f, 0.0f);
        if (g_GameManager.stats->highScore < 100000000)
        {
            g_AsciiText.AddFormat(&scorePos, "%.8d", g_GameManager.stats->highScore);
            scorePos.x += 112.0f;
            g_AsciiText.AddFormat(&scorePos, "%1d", g_GameManager.stats->bestContinuesUsed);
        }
        else
        {
            g_AsciiText.scaleX = 0.9f;
            g_AsciiText.scaleY = 1.0f;
            g_AsciiText.AddFormat(&scorePos, "%.9d", g_GameManager.stats->highScore);
            // Nine digits 14 wide at the 0.9 scale; the folded product is
            // one ulp below 113.4f.
            scorePos.x += 126.0f * 0.9f;
            g_AsciiText.AddFormat(&scorePos, "%1d", g_GameManager.stats->bestContinuesUsed);
            g_AsciiText.scaleX = 1.0f;
            g_AsciiText.scaleY = 1.0f;
        }
        if (this->grazeChanged || g_App.flags.noFrameItems)
        {
            scorePos = D3DXVECTOR3(496.0f, 160.0f, 0.0f);
            g_AsciiText.AddFormat(&scorePos, "%d", g_GameManager.stats->grazeTotal);
        }
        if (this->frameItemsHideTimer || g_App.flags.noFrameItems)
        {
            scorePos = D3DXVECTOR3(496.0f, 176.0f, 0.0f);
            g_AsciiText.AddFormat(&scorePos, "%d/%d", g_GameManager.stats->pointItemsTotal,
                                  g_GameManager.stats->checksumKeyPoints);
        }
    }
    g_SpriteEngine->FlushQuadBatch();
    if (this->powerChanged || g_App.flags.noFrameItems)
    {
        VertexDiffuseXyzrwh vertices[4];

        if ((int)g_GameManager.stats->power > 0)
        {
            vertices[0].pos = D3DXVECTOR3(496.0f, 144.0f, 0.1f);
            vertices[1].pos = D3DXVECTOR3((int)g_GameManager.stats->power + 496 + 0.0f, 144.0f, 0.1f);
            vertices[2].pos = D3DXVECTOR3(496.0f, 160.0f, 0.1f);
            vertices[3].pos = D3DXVECTOR3((int)g_GameManager.stats->power + 496 + 0.0f, 160.0f, 0.1f);
            vertices[0].diffuse = vertices[2].diffuse = 0xe0e0e0ff;
            vertices[1].diffuse = vertices[3].diffuse = 0x80e0e0ff;
            vertices[0].w = vertices[1].w = vertices[2].w = vertices[3].w = 1.0f;
            if (!g_App.flags.noColorBlending)
            {
                g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
                g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
            }
            g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
            g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
            if (!g_App.flags.noDepthTest)
            {
                g_App.SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
            }
            g_App.d3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
            g_App.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(VertexDiffuseXyzrwh));
            g_SpriteEngine->ClearVertexFormat();
            g_SpriteEngine->ClearColorOp();
            g_SpriteEngine->ClearBlendMode();
            g_SpriteEngine->ClearZWrite();
            if (!g_App.flags.noColorBlending)
            {
                g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
                g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
            }
            g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        }
        if ((int)g_GameManager.stats->power < 128)
        {
            g_AsciiText.AddFormat(&D3DXVECTOR3(496.0f, 144.0f, 0.0f), "%d", (int)g_GameManager.stats->power);
        }
        else
        {
            g_AsciiText.AddFormat(&D3DXVECTOR3(496.0f, 144.0f, 0.0f), "MAX");
        }
    }
    if (this->livesChanged)
    {
        this->livesChanged--;
    }
    if (this->powerChanged)
    {
        this->powerChanged--;
    }
    if (this->bombsChanged)
    {
        this->bombsChanged--;
    }
    if (this->grazeChanged)
    {
        this->grazeChanged--;
    }
}

// Sprite `sprite`'s entry in the sprite engine.
inline SpriteData *GetSpriteData(int sprite)
{
    return &g_SpriteEngine->sprites[sprite];
}

void Gui::DrawBanners()
{
    Catk *catks;
    int score;
    D3DXVECTOR3 oldPos2;
    int iVal;
    int digit;
    int div2;
    D3DCOLOR colorA2;
    D3DCOLOR dark;
    int shown;
    float r2[4];
    D3DCOLOR timerColor;
    float end;
    int index2;
    int total;
    int shrink;
    D3DXVECTOR3 pos;

    for (iVal = 0; iVal < 5; iVal++)
    {
        g_SpriteEngine->Draw(&this->impl->trackNumberSprites[iVal]);
    }
    if (this->impl->bombPortrait.shown)
    {
        g_SpriteEngine->DrawUnrotated(&this->impl->bombPortrait);
        g_SpriteEngine->DrawUnrotated(&this->impl->bombPortraitGlow);
        g_SpriteEngine->Draw(&this->impl->bombPortraitFrame);
    }
    if (this->impl->spellPortrait.shown)
    {
        oldPos2 = this->impl->spellPortrait.pos;
        this->impl->spellPortrait.pos += this->impl->spellPortrait.posOffset;
        g_SpriteEngine->DrawUnrotated(&this->impl->spellPortrait);
        this->impl->spellPortrait.pos = oldPos2;
        g_SpriteEngine->DrawUnrotated(&this->impl->spellPortraitGlow);
        g_SpriteEngine->Draw(&this->impl->spellPortraitFrame);
    }
    if (this->impl->bombName.shown)
    {
        this->impl->bombNameBar.pos = this->impl->bombName.pos;
        g_SpriteEngine->DrawUnrotated(&this->impl->bombNameBar);
        g_SpriteEngine->Draw(&this->impl->bombName);
    }
    if (this->impl->spellName.shown)
    {
        this->impl->spellNameBar.pos = this->impl->spellName.pos;
        g_SpriteEngine->DrawUnrotated(&this->impl->spellNameBar);
        g_SpriteEngine->Draw(&this->impl->spellName);
        g_SpriteEngine->DrawUnrotated(&this->impl->spellScoreBar);
        score = g_SpellCardBonus + g_SpellCardGrazeBonus;
        div2 = 10000000;
        shown = 0;
        catks = &g_GameManager.catks[g_SpellCardNumber];
        if (g_SpellCardCapture == 0)
        {
            score = 0;
        }
        this->impl->spellScoreDigit.pos = this->impl->spellScoreBar.pos;
        this->impl->spellScoreDigit.pos.x -= 40.0f;
        for (iVal = 0; iVal < 8; iVal++)
        {
            digit = score / div2;
            if (digit != 0)
            {
                shown = 1;
            }
            if (shown || div2 == 1)
            {
                this->impl->spellScoreDigit.spriteData = GetSpriteData(digit + 0x84);
                g_SpriteEngine->DrawUnrotated(&this->impl->spellScoreDigit);
            }
            this->impl->spellScoreDigit.pos.x += 7.0f;
            score %= div2;
            div2 /= 10;
        }
        digit = catks->captures[g_GameManager.shotIndex];
        if (digit > 99)
        {
            digit = 99;
        }
        this->impl->spellScoreDigit.pos.x += 36.0f;
        if (digit / 10 != 0)
        {
            this->impl->spellScoreDigit.spriteData = GetSpriteData(digit / 10 + 0x84);
            g_SpriteEngine->DrawUnrotated(&this->impl->spellScoreDigit);
        }
        this->impl->spellScoreDigit.pos.x += 7.0f;
        this->impl->spellScoreDigit.spriteData = GetSpriteData(digit % 10 + 0x84);
        g_SpriteEngine->DrawUnrotated(&this->impl->spellScoreDigit);
        digit = catks->attempts[g_GameManager.shotIndex];
        if (digit > 99)
        {
            digit = 99;
        }
        this->impl->spellScoreDigit.pos.x += 14.0f;
        if (digit / 10 != 0)
        {
            this->impl->spellScoreDigit.spriteData = GetSpriteData(digit / 10 + 0x84);
            g_SpriteEngine->DrawUnrotated(&this->impl->spellScoreDigit);
        }
        this->impl->spellScoreDigit.pos.x += 7.0f;
        this->impl->spellScoreDigit.spriteData = GetSpriteData(digit % 10 + 0x84);
        g_SpriteEngine->DrawUnrotated(&this->impl->spellScoreDigit);
    }
    if (this->impl->loadingIcon.spriteIndex >= 0)
    {
        g_SpriteEngine->DrawUnrotated(&this->impl->loadingIcon);
        g_SpriteEngine->DrawUnrotated(&this->impl->loadingBar);
        if (this->impl->loadingCaption.spriteIndex >= 0)
        {
            this->impl->loadingCaption.pos = D3DXVECTOR3(304.0f, 448.0f, 0.0f);
            g_SpriteEngine->DrawUnrotated(&this->impl->loadingCaption);
        }
    }
    if (this->impl->backgroundTileCount != 0)
    {
        for (iVal = 0; iVal < 0xa8; iVal++)
        {
            g_SpriteEngine->DrawProjected3D(&this->impl->backgroundTiles[iVal]);
            g_SpriteEngine->ClearSpriteData();
        }
    }
    if (this->impl->msg.scriptIndex < 0 && this->bossPresent + this->impl->bossBarState > 0)
    {
        r2[0] = 64.0f;
        r2[1] = 19.0f;
        r2[2] = this->bossHealthShown * 320.0f + 64.0f;
        r2[3] = 23.0f;
        colorA2 = this->bossBarAlpha << 24 | 0xffffff;
        dark = this->bossBarAlpha << 24 | 0x202060;
        pos.x = 48.0f;
        pos.y = 16.0f;
        pos.z = 0.0f;
        DrawGradientRect(r2, colorA2, colorA2, dark, dark);
        for (index2 = 0; index2 < 8; index2++)
        {
            if (this->sectionStarts[index2] == 0.0f)
            {
                continue;
            }
            if (this->sectionEnds[index2] >= this->bossHealthShown)
            {
                continue;
            }
            end = this->sectionStarts[index2];
            if (this->bossHealthShown < end)
            {
                end = this->bossHealthShown;
            }
            r2[0] = this->sectionEnds[index2] * 320.0f + 64.0f;
            r2[1] = 19.0f;
            r2[2] = end * 320.0f + 64.0f;
            r2[3] = 23.0f;
            colorA2 = this->bossBarAlpha << 24 | (this->sectionColors[index2] & 0xffffff);
            dark = this->bossBarAlpha << 24 | (this->sectionColors[index2] >> 2 & 0x3f3f3f);
            DrawGradientRect(r2, colorA2, colorA2, dark, dark);
        }
        g_SpriteEngine->DrawUnrotated(&this->impl->sprites[11]);
        r2[0] = 33.0f;
        r2[1] = 19.0f;
        r2[2] = r2[0] + 3.0f;
        r2[3] = r2[1] + 4.0f;
        total = this->bossLives;
        shrink = this->bossLives <= 5 ? 2 : 1;
        for (index2 = 0; index2 < total; index2++)
        {
            r2[0] = index2 * 26.0f / total + 35.0f;
            r2[2] = (index2 + 1) * 26.0f / total + 35.0f - shrink;
            colorA2 = this->bossBarAlpha << 24 | (0xffffff - index2 * 0xff / 9);
            dark = this->bossBarAlpha << 24 | 0x202020;
            DrawGradientRect(r2, colorA2, colorA2, dark, dark);
        }
        pos = D3DXVECTOR3(384.0f, 16.0f, 0.0f);
        if (this->bossTimer >= 20)
        {
            timerColor = g_BossTimerColors[0];
        }
        else if (this->bossTimer >= 10)
        {
            timerColor = g_BossTimerColors[1];
        }
        else if (this->bossTimer >= 5)
        {
            timerColor = g_BossTimerColors[2];
        }
        else
        {
            timerColor = g_BossTimerColors[3];
        }
        g_AsciiText.SetColor(this->bossBarAlpha << 24 | timerColor);
        total = this->bossTimer > 99 ? 99 : this->bossTimer;
        if (total < 10 && this->lastDrawnBossTimer != this->bossTimer)
        {
            g_SoundPlayer.QueueSound(0x1d, 0);
        }
        g_AsciiText.AddFormat(&pos, "%.2d", total);
        g_AsciiText.color = 0xffffffff;
        this->lastDrawnBossTimer = this->bossTimer;
    }
}

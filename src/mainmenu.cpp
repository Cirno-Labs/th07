// cflags: /Os -/Oi
// The /Os file from 0x4553fa to 0x45c6e0, with the title menu, th06's
// mainmenu.cpp.

#include <direct.h>
#include <stdio.h>

#include "ascii.h"
#include "mainmenu.h"
#include "midi.h"
#include "resultscreen.h"
#include "screeneffect.h"
#include "unsorted.h"

void App::ResetClockDriftCounters()
{
    this->fastClockStreak = 0;
    this->maxFastClockStreak = 0;
    this->clockMeasurementPending = 0;
    this->fastClockCycleCount = 0;
    this->fastClockCycles = 0;
}

inline void MainMenu::SetState(int state)
{
    this->prevState = this->state;
    this->state = state;
    this->stateFrames = 0;
    this->inputLockout = 0;
    this->subState = 0;
    this->unk_b0c0 = 0;
}

int MainMenu::OnUpdate(MainMenu *menu)
{
    int status;

    switch (menu->state)
    {
    case 0:
        status = menu->StartTitle();
        break;
    case 7:
        status = menu->UpdateReplayMenu();
        break;
    case 2:
        status = menu->UpdateOptions();
        break;
    case 3:
        status = menu->UpdateKeyConfig();
        break;
    case 4:
    case 8:
    case 12:
        status = menu->UpdateDifficultyMenu();
        break;
    case 5:
    case 9:
    case 13:
        status = menu->UpdateCharacterMenu();
        break;
    case 6:
    case 10:
    case 14:
        status = menu->UpdateShotTypeMenu();
        break;
    case 11:
        status = menu->UpdateGameStartMenu();
        break;
    }
    g_SpriteEngine->StepSprites(menu->sprites, menu->spriteCount);
    if (menu->helpSprite != NULL)
    {
        g_SpriteEngine->Step(menu->helpSprite);
    }
    return status;
}

int MainMenu::StartTitle()
{
    int i;

    switch (this->subState)
    {
    case 0:
        if (this->prevState == 0 && g_App.prevScene != 5)
        {
            g_App.PlayMidi(8);
        }
        if ((this->prevState == 0 || this->prevState == 4 || this->prevState == 7 || this->prevState == 8 ||
             this->prevState == 12) &&
            g_SpriteEngine->LoadSurface(0, "data/title/title00.jpg") != 0)
        {
            return 0;
        }
        if (this->spriteCount == 0)
        {
            this->spriteCount = 0xa4;
            this->sprites = new Sprite[this->spriteCount];
            g_SpriteEngine->StartSpriteSeries(this->sprites, 0x900, this->spriteCount);
        }
        g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 2);
        for (i = 0; i < 8; i++)
        {
            g_SpriteEngine->SetSprite(&this->sprites[i + 1], this->sprites[i + 1].baseSpriteIndex + 1);
        }
        g_SpriteEngine->SetSprite(&this->sprites[this->cursor + 1], this->sprites[this->cursor + 1].baseSpriteIndex);
        this->subState = 0;
        this->stateFrames = 0;
        this->prevCursor = -1;
        this->subState = 1;
        this->idleFrames = 0;
        if (g_GameManager.unk_93d8Bit3)
        {
            this->SetState(7);
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 13);
            this->helpSprite->SetPendingInterrupt(2);
            g_GameManager.SetUnk93d8Bit3(0);
            return 1;
        }
        if (this->unk_68 != 0)
        {
            this->SetState(8);
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 5);
            this->helpSprite->SetPendingInterrupt(2);
            return 1;
        }
        for (i = 0; i < ARRAY_SIZE(g_TitleHelpTexts); i++)
        {
            g_SpriteEngine->RenderTextCentered(&this->helpSprites[i], 0xfff0e0, 0x300000, g_TitleHelpTexts[i]);
        }
    case 1:
        i = this->MoveMenuCursor(8);
        if (i != 0)
        {
            while (!g_GameManager.AnyShotHasExtraOpen() && this->cursor == 1)
            {
                this->cursor += i;
            }
            for (i = 0; i < 8; i++)
            {
                g_SpriteEngine->SetSprite(&this->sprites[i + 1], this->sprites[i + 1].baseSpriteIndex + 1);
            }
            g_SpriteEngine->SetSprite(&this->sprites[this->cursor + 1],
                                      this->sprites[this->cursor + 1].baseSpriteIndex);
        }
        this->idleFrames++;
        if (g_Buttons != 0)
        {
            this->idleFrames = 0;
        }
        if (this->idleFrames > 900)
        {
            g_GameManager.demoIndex++;
            g_GameManager.demoIndex %= 3;
            strcpy(g_GameManager.replayFile, g_DemoReplayPaths[g_GameManager.demoIndex]);
            this->loadedReplay = (ReplayHeader *)LoadFileData(g_GameManager.replayFile, 0);
            this->loadedReplay = ValidateReplay(this->loadedReplay, g_FileSize);
            if (this->loadedReplay == NULL)
            {
                DebugPrint2("error : Demo Play is not ready\r\n");
                this->idleFrames = 0;
            }
            else
            {
                g_GameManager.SetUnk93d8Bit3(1);
                g_GameManager.unk_93d8 |= 2;
                g_GameManager.demoFrames = 0;
                g_GameManager.difficulty = this->loadedReplay->difficulty;
                g_GameManager.character = this->loadedReplay->shotType / 2;
                g_GameManager.shotType = this->loadedReplay->shotType % 2;
                g_GameManager.shotIndex = this->loadedReplay->shotType;
                i = 0;
                while (this->loadedReplay->stages[i] == NULL)
                {
                    i++;
                }
                g_GameManager.stage = i;
                MemFree(this->loadedReplay);
                this->loadedReplay = NULL;
                g_App.scene = 2;
                g_GameManager.replayMenuChoice = 0;
                return 0;
            }
        }
        if (this->prevCursor != this->cursor)
        {
            this->helpSprite = &this->helpSprites[this->cursor];
            this->helpSprite->SetPendingInterrupt(1);
        }
        this->prevCursor = this->cursor;
        if (this->inputLockout < 10)
        {
            break;
        }
        if (WAS_PRESSED(0x1001))
        {
            g_SoundPlayer.QueueSound(10, 0);
            g_SoundPlayer.PlayQueuedSounds();
            switch (this->cursor)
            {
            case 0:
                g_GameManager.unk_93d8 &= ~1;
                this->cursor = g_App.defaultDifficulty;
                if (this->cursor >= 4)
                {
                    this->cursor = 2;
                }
                this->SetState(4);
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 5);
                this->helpSprite->SetPendingInterrupt(2);
                return 1;
            case 2:
                g_GameManager.unk_93d8 |= 1;
                this->cursor = g_App.defaultDifficulty;
                if (this->cursor >= 4)
                {
                    this->cursor = 2;
                }
                this->SetState(8);
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 5);
                this->helpSprite->SetPendingInterrupt(2);
                return 1;
            case 1:
                if (g_GameManager.AnyShotHasExtraOpen())
                {
                    g_GameManager.unk_93d8 &= ~1;
                    this->cursor = g_App.defaultDifficulty == 5;
                    this->SetState(12);
                    g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 5);
                    this->helpSprite->SetPendingInterrupt(2);
                    return 1;
                }
            case 3:
                g_GameManager.unk_93d8 &= ~1;
                this->SetState(7);
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 13);
                this->helpSprite->SetPendingInterrupt(2);
                return 1;
            case 5:
                g_App.scene = 8;
                this->helpSprite->SetPendingInterrupt(2);
                return 0;
            case 4:
                g_App.scene = 5;
                this->helpSprite->SetPendingInterrupt(2);
                return 0;
            case 6:
                this->subState = 0;
                this->cursor = 0;
                this->inputLockout = 0;
                this->stateFrames = 0;
                this->subState = 3;
                this->stateFrames = 0;
                this->UpdateOptions();
                this->cursor = 0;
                break;
            case 7:
                this->subState = 2;
                this->stateFrames = 0;
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 1);
                if (g_App.musicMode == 2)
                {
                    g_App.midi->Restart(30);
                }
            }
        }
        if (WAS_PRESSED(10))
        {
            g_SpriteEngine->SetSprite(&this->sprites[this->cursor + 1],
                                      this->sprites[this->cursor + 1].baseSpriteIndex + 1);
            this->cursor = 7;
            g_SpriteEngine->SetSprite(&this->sprites[this->cursor + 1],
                                      this->sprites[this->cursor + 1].baseSpriteIndex);
            g_SoundPlayer.QueueSound(11, 0);
            g_SoundPlayer.PlayQueuedSounds();
        }
        break;
    case 2:
        if (this->stateFrames >= 60)
        {
            delete[] this->sprites;
            this->sprites = NULL;
            this->sprites = NULL;
            this->spriteCount = 0;
            this->inputLockout = 0;
            g_App.scene = -1;
            return 0;
        }
        break;
    case 3:
        if (this->stateFrames >= 30)
        {
            this->SetState(2);
            this->cursor = 0;
            this->savedConfig = *(ConfigCopy *)&g_App.controllerMapping;
            return 1;
        }
    }
    this->unk_b0c0++;
    this->stateFrames++;
    this->inputLockout++;
    return 1;
}

int MainMenu::UpdateOptions()
{
    int i;

    switch (this->subState)
    {
    case 0:
        if (this->inputLockout == 0)
        {
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 3);
            for (i = 0; i < 9; i++)
            {
                g_SpriteEngine->SetSprite(&this->sprites[i + 9], this->sprites[i + 9].baseSpriteIndex + 1);
            }
            g_SpriteEngine->SetSprite(&this->sprites[this->cursor + 9],
                                      this->sprites[this->cursor + 9].baseSpriteIndex);
            this->subState = 0;
            this->stateFrames = 0;
            this->prevCursor = -1;
        }
        this->subState = 1;
        for (i = 0; i < ARRAY_SIZE(g_OptionsHelpTexts); i++)
        {
            g_SpriteEngine->RenderTextCentered(&this->helpSprites[i], 0xfff0e0, 0x300000, g_OptionsHelpTexts[i]);
        }
    case 1:
        if (this->MoveMenuCursor(9))
        {
            for (i = 0; i < 9; i++)
            {
                g_SpriteEngine->SetSprite(&this->sprites[i + 9], this->sprites[i + 9].baseSpriteIndex + 1);
            }
            g_SpriteEngine->SetSprite(&this->sprites[this->cursor + 9],
                                      this->sprites[this->cursor + 9].baseSpriteIndex);
        }
        if (this->prevCursor != this->cursor)
        {
            this->helpSprite = &this->helpSprites[this->cursor];
            this->helpSprite->SetPendingInterrupt(1);
        }
        this->prevCursor = this->cursor;
        for (i = 0x12; i <= 0x16; i++)
        {
            g_SpriteEngine->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex + 1);
        }
        i = g_App.lifeCount + 0x12;
        g_SpriteEngine->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex);
        for (i = 0x17; i <= 0x18; i++)
        {
            g_SpriteEngine->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex + 1);
        }
        i = g_App.colorMode16Bits + 0x17;
        g_SpriteEngine->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex);
        for (i = 0x19; i <= 0x1b; i++)
        {
            g_SpriteEngine->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex + 1);
        }
        i = g_App.musicMode + 0x19;
        g_SpriteEngine->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex);
        for (i = 0x1c; i <= 0x1d; i++)
        {
            g_SpriteEngine->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex + 1);
        }
        i = g_App.playSounds + 0x1c;
        g_SpriteEngine->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex);
        for (i = 0x1e; i <= 0x1f; i++)
        {
            g_SpriteEngine->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex + 1);
        }
        i = g_App.windowed + 0x1e;
        g_SpriteEngine->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex);
        for (i = 0x20; i <= 0x21; i++)
        {
            g_SpriteEngine->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex + 1);
        }
        i = g_App.optionsMenuCursor + 0x20;
        g_SpriteEngine->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex);
        if (this->inputLockout < 4)
        {
            break;
        }
        if (WAS_PRESSED_OR_HELD(0x40))
        {
            switch (this->cursor)
            {
            case 0:
                if (g_App.lifeCount == 0)
                {
                    g_App.lifeCount = 4;
                }
                else
                {
                    g_App.lifeCount--;
                }
                break;
            case 1:
                if (g_App.colorMode16Bits == 0)
                {
                    g_App.colorMode16Bits = 1;
                }
                else
                {
                    g_App.colorMode16Bits--;
                }
                break;
            case 2:
                g_App.StopMusic();
                if (g_App.musicMode == 2)
                {
                    g_App.midi->Restart(30);
                }
                if (g_App.musicMode == 0)
                {
                    g_App.musicMode = 2;
                }
                else
                {
                    g_App.musicMode--;
                }
                if (!g_App.flags.preloadBgm && g_App.musicMode == 2)
                {
                    g_SoundPlayer.LoadBgm("thbgm.dat");
                }
                g_App.LoadMidi(8, "bgm/th07_01.mid");
                g_App.PlayMidi(8);
                break;
            case 3:
                if (g_App.playSounds == 0)
                {
                    g_App.playSounds = 1;
                }
                else
                {
                    g_App.playSounds--;
                }
                break;
            case 4:
                if (g_App.windowed == 0)
                {
                    g_App.windowed = 1;
                }
                else
                {
                    g_App.windowed--;
                }
                break;
            case 5:
                if (g_App.optionsMenuCursor == 0)
                {
                    g_App.optionsMenuCursor = 1;
                }
                else
                {
                    g_App.optionsMenuCursor--;
                }
                break;
            default:
                goto right;
            }
            g_SoundPlayer.QueueSound(12, 0);
            g_SoundPlayer.PlayQueuedSounds();
        }
    right:
        if (WAS_PRESSED_OR_HELD(0x80))
        {
            switch (this->cursor)
            {
            case 0:
                if (g_App.lifeCount >= 4)
                {
                    g_App.lifeCount = 0;
                }
                else
                {
                    g_App.lifeCount++;
                }
                break;
            case 1:
                if (g_App.colorMode16Bits >= 1)
                {
                    g_App.colorMode16Bits = 0;
                }
                else
                {
                    g_App.colorMode16Bits++;
                }
                break;
            case 2:
                g_App.StopMusic();
                if (g_App.musicMode >= 2)
                {
                    g_App.musicMode = 0;
                }
                else
                {
                    g_App.musicMode++;
                }
                g_App.LoadMidi(8, "bgm/th07_01.mid");
                g_App.PlayMidi(8);
                break;
            case 3:
                if (g_App.playSounds >= 1)
                {
                    g_App.playSounds = 0;
                }
                else
                {
                    g_App.playSounds++;
                }
                break;
            case 4:
                if (g_App.windowed >= 1)
                {
                    g_App.windowed = 0;
                }
                else
                {
                    g_App.windowed++;
                }
                break;
            case 5:
                if (g_App.optionsMenuCursor >= 1)
                {
                    g_App.optionsMenuCursor = 0;
                }
                else
                {
                    g_App.optionsMenuCursor++;
                }
                break;
            default:
                goto idle;
            }
            g_SoundPlayer.QueueSound(12, 0);
            g_SoundPlayer.PlayQueuedSounds();
        }
    idle:
        if (g_Buttons != 0)
        {
            this->unk_b0c0 = 0;
        }
        if (this->unk_b0c0 >= 3600)
        {
            goto leave;
        }
        if (WAS_PRESSED(0x1001))
        {
            switch (this->cursor)
            {
            case 6:
                g_App.lifeCount = 2;
                g_App.bombCount = 3;
                g_App.musicMode = 1;
                g_App.playSounds = 1;
                g_App.optionsMenuCursor = 0;
                g_SoundPlayer.QueueSound(10, 0);
                g_SoundPlayer.PlayQueuedSounds();
                break;
            case 7:
                this->cursor = 0;
                this->SetState(3);
                g_SoundPlayer.QueueSound(10, 0);
                g_SoundPlayer.PlayQueuedSounds();
                return 1;
            case 8:
            leave:
                this->cursor = 6;
                this->SetState(0);
                g_SoundPlayer.QueueSound(11, 0);
                g_SoundPlayer.PlayQueuedSounds();
                if (this->savedConfig.colorMode16Bits != g_App.colorMode16Bits ||
                    this->savedConfig.windowed != g_App.windowed)
                {
                    return 5;
                }
                return 1;
            }
        }
        if (WAS_PRESSED(10))
        {
            if (this->cursor == 8)
            {
                goto leave;
            }
            g_SpriteEngine->SetSprite(&this->sprites[this->cursor + 9],
                                      this->sprites[this->cursor + 9].baseSpriteIndex + 1);
            this->cursor = 8;
            g_SpriteEngine->SetSprite(&this->sprites[this->cursor + 9],
                                      this->sprites[this->cursor + 9].baseSpriteIndex);
            g_SoundPlayer.QueueSound(11, 0);
            g_SoundPlayer.PlayQueuedSounds();
        }
    }
    this->unk_b0c0++;
    this->stateFrames++;
    this->inputLockout++;
    return 1;
}

void MainMenu::ReassignButton(short button, short replacement, int unk)
{
    if (this->controllerMapping.shootButton == button)
    {
        this->controllerMapping.shootButton = replacement;
    }
    if (this->controllerMapping.bombButton == button)
    {
        this->controllerMapping.bombButton = replacement;
    }
    if (this->controllerMapping.focusButton == button)
    {
        this->controllerMapping.focusButton = replacement;
    }
    if (this->controllerMapping.upButton == button)
    {
        this->controllerMapping.upButton = replacement;
    }
    if (this->controllerMapping.downButton == button)
    {
        this->controllerMapping.downButton = replacement;
    }
    if (this->controllerMapping.leftButton == button)
    {
        this->controllerMapping.leftButton = replacement;
    }
    if (this->controllerMapping.rightButton == button)
    {
        this->controllerMapping.rightButton = replacement;
    }
    if (this->controllerMapping.menuButton == button)
    {
        this->controllerMapping.menuButton = replacement;
    }
    if (this->controllerMapping.skipButton == button)
    {
        this->controllerMapping.skipButton = replacement;
    }
}

int MainMenu::UpdateKeyConfig()
{
    Sprite *digit;
    int i;
    short key;
    BYTE *pad;

    switch (this->subState)
    {
    case 0:
        if (this->inputLockout == 0)
        {
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 4);
            for (i = 0; i < 12; i++)
            {
                g_SpriteEngine->SetSprite(&this->sprites[i + 0x23], this->sprites[i + 0x23].baseSpriteIndex + 1);
            }
            g_SpriteEngine->SetSprite(&this->sprites[this->cursor + 0x23],
                                      this->sprites[this->cursor + 0x23].baseSpriteIndex);
            this->subState = 0;
            this->stateFrames = 0;
            this->controllerMapping = g_App.controllerMapping;
            g_App.controllerMapping.upButton = -1;
            g_App.controllerMapping.downButton = -1;
            digit = &this->sprites[0x2f];
            this->ShowButtonDigits(digit, this->controllerMapping.shootButton);
            digit += 2;
            this->ShowButtonDigits(digit, this->controllerMapping.bombButton);
            digit += 2;
            this->ShowButtonDigits(digit, this->controllerMapping.focusButton);
            digit += 2;
            this->ShowButtonDigits(digit, this->controllerMapping.skipButton);
            digit += 2;
            this->ShowButtonDigits(digit, this->controllerMapping.menuButton);
            digit += 2;
            this->ShowButtonDigits(digit, this->controllerMapping.upButton);
            digit += 2;
            this->ShowButtonDigits(digit, this->controllerMapping.downButton);
            digit += 2;
            this->ShowButtonDigits(digit, this->controllerMapping.leftButton);
            digit += 2;
            this->ShowButtonDigits(digit, this->controllerMapping.rightButton);
            this->prevCursor = -1;
        }
        this->subState = 1;
        for (i = 0; i < ARRAY_SIZE(g_KeyConfigHelpTexts); i++)
        {
            g_SpriteEngine->RenderTextCentered(&this->helpSprites[i], 0xfff0e0, 0x300000, g_KeyConfigHelpTexts[i]);
        }
    case 1:
        if (this->MoveMenuCursor(12))
        {
            for (i = 0; i < 12; i++)
            {
                g_SpriteEngine->SetSprite(&this->sprites[i + 0x23], this->sprites[i + 0x23].baseSpriteIndex + 1);
            }
            g_SpriteEngine->SetSprite(&this->sprites[this->cursor + 0x23],
                                      this->sprites[this->cursor + 0x23].baseSpriteIndex);
        }
        if (this->prevCursor != this->cursor)
        {
            this->helpSprite = &this->helpSprites[this->cursor];
            this->helpSprite->SetPendingInterrupt(1);
        }
        this->prevCursor = this->cursor;
        digit = &this->sprites[0x2f];
        this->ShowButtonDigits(digit, this->controllerMapping.shootButton);
        digit += 2;
        this->ShowButtonDigits(digit, this->controllerMapping.bombButton);
        digit += 2;
        this->ShowButtonDigits(digit, this->controllerMapping.focusButton);
        digit += 2;
        this->ShowButtonDigits(digit, this->controllerMapping.skipButton);
        digit += 2;
        this->ShowButtonDigits(digit, this->controllerMapping.menuButton);
        digit += 2;
        this->ShowButtonDigits(digit, this->controllerMapping.upButton);
        digit += 2;
        this->ShowButtonDigits(digit, this->controllerMapping.downButton);
        digit += 2;
        this->ShowButtonDigits(digit, this->controllerMapping.leftButton);
        digit += 2;
        this->ShowButtonDigits(digit, this->controllerMapping.rightButton);
        for (i = 0x41; i <= 0x42; i++)
        {
            g_SpriteEngine->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex + 1);
        }
        i = g_App.shootFocuses + 0x41;
        g_SpriteEngine->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex);
        pad = GetPadButtons();
        for (key = 0; key < 0x20; key++)
        {
            if (pad[key] & 0x80)
            {
                break;
            }
        }
        if (key < 0x20 && g_LastPressedKey != key)
        {
            switch (this->cursor)
            {
            case 0:
                this->ReassignButton(key, this->controllerMapping.shootButton, 1);
                this->controllerMapping.shootButton = key;
                break;
            case 1:
                this->ReassignButton(key, this->controllerMapping.bombButton, 0);
                this->controllerMapping.bombButton = key;
                break;
            case 2:
                this->ReassignButton(key, this->controllerMapping.focusButton, 1);
                this->controllerMapping.focusButton = key;
                break;
            case 3:
                this->ReassignButton(key, this->controllerMapping.menuButton, 0);
                this->controllerMapping.menuButton = key;
                break;
            case 4:
                this->ReassignButton(key, this->controllerMapping.upButton, 0);
                this->controllerMapping.upButton = key;
                break;
            case 5:
                this->ReassignButton(key, this->controllerMapping.downButton, 0);
                this->controllerMapping.downButton = key;
                break;
            case 6:
                this->ReassignButton(key, this->controllerMapping.leftButton, 0);
                this->controllerMapping.leftButton = key;
                break;
            case 7:
                this->ReassignButton(key, this->controllerMapping.rightButton, 0);
                this->controllerMapping.rightButton = key;
                break;
            case 8:
                this->ReassignButton(key, this->controllerMapping.skipButton, 0);
                this->controllerMapping.skipButton = key;
                break;
            default:
                goto assigned;
            }
            g_SoundPlayer.QueueSound(10, 0);
            g_SoundPlayer.PlayQueuedSounds();
        }
    assigned:
        g_LastPressedKey = key;
        if (WAS_PRESSED(0x40))
        {
            switch (this->cursor)
            {
            case 9:
                g_App.shootFocuses = 1 - g_App.shootFocuses;
            }
        }
        if (WAS_PRESSED(0x80))
        {
            switch (this->cursor)
            {
            case 9:
                g_App.shootFocuses = 1 - g_App.shootFocuses;
            }
        }
        if (g_Buttons != 0)
        {
            this->unk_b0c0 = 0;
        }
        if (this->unk_b0c0 >= 3600)
        {
            goto leave;
        }
        if (WAS_PRESSED(0x1001))
        {
            switch (this->cursor)
            {
            case 10:
                g_SoundPlayer.QueueSound(10, 0);
                g_SoundPlayer.PlayQueuedSounds();
                this->controllerMapping = g_DefaultControllerMapping;
                g_App.shootFocuses = 1;
                break;
            case 11:
            leave:
                g_SoundPlayer.QueueSound(11, 0);
                g_SoundPlayer.PlayQueuedSounds();
                this->SetState(2);
                g_App.controllerMapping = this->controllerMapping;
                this->cursor = 7;
                return 1;
            }
        }
    }
    this->unk_b0c0++;
    this->stateFrames++;
    this->inputLockout++;
    return 1;
}

int MainMenu::ShowButtonDigits(Sprite *vm, short value)
{
    if (value < 0)
    {
        vm[0].flags &= ~2;
        vm[1].flags &= ~2;
    }
    else
    {
        g_SpriteEngine->SetSprite(&vm[0], vm[0].baseSpriteIndex + value / 10 * 2);
        g_SpriteEngine->SetSprite(&vm[1], vm[1].baseSpriteIndex + value % 10 * 2);
        vm[0].flags |= 2;
        vm[1].flags |= 2;
    }
    return 0;
}

int MainMenu::UpdateDifficultyMenu()
{
    int count;
    int i;
    int state;

    switch (this->subState)
    {
    case 0:
        if (this->inputLockout == 0)
        {
            if (this->prevState != 5 && this->prevState != 9 && this->prevState != 13 &&
                g_SpriteEngine->LoadSurface(0, "data/title/select00.jpg") != 0)
            {
                return 0;
            }
            this->cursor = g_App.defaultDifficulty;
            if (this->state != 12)
            {
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 7);
            }
            else if (!g_GameManager.IsPhantasmOpen())
            {
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 12);
                this->cursor = 4;
            }
            else
            {
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 22);
            }
            if (this->state != 12)
            {
                if (this->cursor >= 4)
                {
                    this->cursor = 1;
                }
                for (i = 0; i < 4; i++)
                {
                    g_SpriteEngine->SetSprite(&this->sprites[i + 0x43], this->sprites[i + 0x43].baseSpriteIndex + 1);
                }
                g_SpriteEngine->SetSprite(&this->sprites[this->cursor + 0x43],
                                          this->sprites[this->cursor + 0x43].baseSpriteIndex);
            }
            else
            {
                this->cursor -= 4;
                if (this->cursor < 0)
                {
                    this->cursor = 0;
                }
                for (i = 0; i < 2; i++)
                {
                    g_SpriteEngine->SetSprite(&this->sprites[i + 0xa2], this->sprites[i + 0xa2].baseSpriteIndex + 1);
                }
                g_SpriteEngine->SetSprite(&this->sprites[this->cursor + 0xa2],
                                          this->sprites[this->cursor + 0xa2].baseSpriteIndex);
            }
            this->subState = 0;
            this->stateFrames = 0;
            this->helpSprite = NULL;
        }
        if (this->unk_68 != 0)
        {
            this->SetState(9);
            this->cursor = 0;
            return 1;
        }
        if (this->inputLockout == 30)
        {
            this->subState = 1;
        }
        break;
    case 1:
        count = this->state != 12 ? 4 : (g_GameManager.IsPhantasmOpen() ? 2 : 1);
        if (this->MoveMenuCursor(count))
        {
            if (this->state != 12)
            {
                for (i = 0; i < 4; i++)
                {
                    g_SpriteEngine->SetSprite(&this->sprites[i + 0x43], this->sprites[i + 0x43].baseSpriteIndex + 1);
                }
                g_SpriteEngine->SetSprite(&this->sprites[this->cursor + 0x43],
                                          this->sprites[this->cursor + 0x43].baseSpriteIndex);
            }
            else if (count == 2)
            {
                for (i = 0; i < 2; i++)
                {
                    g_SpriteEngine->SetSprite(&this->sprites[i + 0xa2], this->sprites[i + 0xa2].baseSpriteIndex + 1);
                }
                g_SpriteEngine->SetSprite(&this->sprites[this->cursor + 0xa2],
                                          this->sprites[this->cursor + 0xa2].baseSpriteIndex);
            }
        }
        if (WAS_PRESSED(0x1001))
        {
            if (this->state != 12)
            {
                g_App.defaultDifficulty = this->cursor;
            }
            else
            {
                g_App.defaultDifficulty = this->cursor + 4;
            }
            g_SoundPlayer.QueueSound(10, 0);
            g_SoundPlayer.PlayQueuedSounds();
            if (this->state != 12)
            {
                if (!(g_GameManager.unk_93d8 & 1))
                {
                    this->SetState(5);
                }
                else
                {
                    this->SetState(9);
                }
            }
            else
            {
                this->SetState(13);
            }
            this->cursor = 0;
            return 1;
        }
        if (WAS_PRESSED(0xa))
        {
            if (this->state != 12)
            {
                g_App.defaultDifficulty = this->cursor;
            }
            else
            {
                g_App.defaultDifficulty = this->cursor + 4;
            }
            g_SoundPlayer.QueueSound(11, 0);
            g_SoundPlayer.PlayQueuedSounds();
            this->subState = 3;
            this->stateFrames = 0;
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 6);
        }
        break;
    case 3:
        if (this->stateFrames >= 30)
        {
            state = this->state;
            this->SetState(0);
            if (state != 12)
            {
                if (!(g_GameManager.unk_93d8 & 1))
                {
                    this->cursor = 0;
                }
                else
                {
                    this->cursor = 2;
                }
            }
            else
            {
                this->cursor = 1;
            }
            g_GameManager.unk_93d8 &= ~1;
            return 1;
        }
        break;
    }
    this->stateFrames++;
    this->unk_b0c0++;
    this->inputLockout++;
    return 1;
}

int MainMenu::UpdateCharacterMenu()
{
    switch (this->subState)
    {
    case 0:
        if (this->inputLockout == 0)
        {
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 8);
            if (g_App.defaultDifficulty < 4)
            {
                this->sprites[g_App.defaultDifficulty + 0x43].SetPendingInterrupt(9);
            }
            else if (!g_GameManager.IsPhantasmOpen())
            {
                this->sprites[0xa1].SetPendingInterrupt(9);
            }
            else
            {
                this->sprites[g_App.defaultDifficulty + 0x9e].SetPendingInterrupt(9);
            }
            this->cursor = g_GameManager.character;
            if (g_App.defaultDifficulty == 4)
            {
                while (!g_GameManager.HasExtraOpen(this->cursor * 2) && !g_GameManager.HasExtraOpen(this->cursor * 2 + 1))
                {
                    this->cursor++;
                    if (this->cursor >= 3)
                    {
                        this->cursor -= 3;
                    }
                }
            }
            else if (g_App.defaultDifficulty == 5)
            {
                while (!g_GameManager.HasPhantasmOpen(this->cursor * 2) && !g_GameManager.HasPhantasmOpen(this->cursor * 2 + 1))
                {
                    this->cursor++;
                    if (this->cursor >= 3)
                    {
                        this->cursor -= 3;
                    }
                }
            }
            this->sprites[0x48].flags &= ~2;
            this->sprites[0x49].flags &= ~2;
            this->sprites[0x47].flags &= ~2;
            this->sprites[0x50].flags &= ~2;
            this->sprites[0x53].flags &= ~2;
            this->sprites[0x4b].flags &= ~2;
            this->sprites[0x4c].flags &= ~2;
            this->sprites[0x4a].flags &= ~2;
            this->sprites[0x51].flags &= ~2;
            this->sprites[0x54].flags &= ~2;
            this->sprites[0x4e].flags &= ~2;
            this->sprites[0x4f].flags &= ~2;
            this->sprites[0x4d].flags &= ~2;
            this->sprites[0x52].flags &= ~2;
            this->sprites[0x55].flags &= ~2;
            switch (this->cursor)
            {
            case 0:
                this->sprites[0x48].flags |= 2;
                this->sprites[0x49].flags |= 2;
                this->sprites[0x47].flags |= 2;
                this->sprites[0x50].flags |= 2;
                this->sprites[0x53].flags |= 2;
                break;
            case 1:
                this->sprites[0x4b].flags |= 2;
                this->sprites[0x4c].flags |= 2;
                this->sprites[0x4a].flags |= 2;
                this->sprites[0x51].flags |= 2;
                this->sprites[0x54].flags |= 2;
                break;
            case 2:
                this->sprites[0x4e].flags |= 2;
                this->sprites[0x4f].flags |= 2;
                this->sprites[0x4d].flags |= 2;
                this->sprites[0x52].flags |= 2;
                this->sprites[0x55].flags |= 2;
                break;
            }
            switch (this->cursor)
            {
            case 0:
                this->sprites[0x47].SetPendingInterrupt(9);
                this->sprites[0x4a].SetPendingInterrupt(8);
                this->sprites[0x4d].SetPendingInterrupt(8);
                this->sprites[0x4a].alpha = 0;
                this->sprites[0x4d].alpha = 0;
                this->sprites[0x50].SetPendingInterrupt(9);
                this->sprites[0x51].SetPendingInterrupt(8);
                this->sprites[0x52].SetPendingInterrupt(8);
                this->sprites[0x51].alpha = 0;
                this->sprites[0x52].alpha = 0;
                this->sprites[0x53].SetPendingInterrupt(9);
                this->sprites[0x54].SetPendingInterrupt(8);
                this->sprites[0x55].SetPendingInterrupt(8);
                this->sprites[0x54].alpha = 0;
                this->sprites[0x55].alpha = 0;
                break;
            case 1:
                this->sprites[0x47].SetPendingInterrupt(8);
                this->sprites[0x4a].SetPendingInterrupt(9);
                this->sprites[0x4d].SetPendingInterrupt(8);
                this->sprites[0x47].alpha = 0;
                this->sprites[0x4d].alpha = 0;
                this->sprites[0x50].SetPendingInterrupt(8);
                this->sprites[0x51].SetPendingInterrupt(9);
                this->sprites[0x52].SetPendingInterrupt(8);
                this->sprites[0x50].alpha = 0;
                this->sprites[0x52].alpha = 0;
                this->sprites[0x53].SetPendingInterrupt(8);
                this->sprites[0x54].SetPendingInterrupt(9);
                this->sprites[0x55].SetPendingInterrupt(8);
                this->sprites[0x53].alpha = 0;
                this->sprites[0x55].alpha = 0;
                break;
            case 2:
                this->sprites[0x47].SetPendingInterrupt(8);
                this->sprites[0x4a].SetPendingInterrupt(8);
                this->sprites[0x4d].SetPendingInterrupt(9);
                this->sprites[0x4a].alpha = 0;
                this->sprites[0x47].alpha = 0;
                this->sprites[0x50].SetPendingInterrupt(8);
                this->sprites[0x51].SetPendingInterrupt(8);
                this->sprites[0x52].SetPendingInterrupt(9);
                this->sprites[0x50].alpha = 0;
                this->sprites[0x51].alpha = 0;
                this->sprites[0x53].SetPendingInterrupt(8);
                this->sprites[0x54].SetPendingInterrupt(8);
                this->sprites[0x55].SetPendingInterrupt(9);
                this->sprites[0x53].alpha = 0;
                this->sprites[0x54].alpha = 0;
                break;
            }
            this->subState = 0;
            this->stateFrames = 0;
        }
        if (this->unk_68 != 0)
        {
            this->SetState(10);
            this->cursor = 0;
            return 1;
        }
        if (this->inputLockout == 30)
        {
            this->subState = 1;
        }
        break;
    case 1:
        if (this->MoveMenuCursorLR(3))
        {
            if (g_App.defaultDifficulty == 4)
            {
                while (!g_GameManager.HasExtraOpen(this->cursor * 2) && !g_GameManager.HasExtraOpen(this->cursor * 2 + 1))
                {
                    this->cursor++;
                    if (this->cursor >= 3)
                    {
                        this->cursor -= 3;
                    }
                }
            }
            else if (g_App.defaultDifficulty == 5)
            {
                while (!g_GameManager.HasPhantasmOpen(this->cursor * 2) && !g_GameManager.HasPhantasmOpen(this->cursor * 2 + 1))
                {
                    this->cursor++;
                    if (this->cursor >= 3)
                    {
                        this->cursor -= 3;
                    }
                }
            }
            this->sprites[0x48].flags |= 2;
            this->sprites[0x49].flags |= 2;
            this->sprites[0x47].flags |= 2;
            this->sprites[0x50].flags |= 2;
            this->sprites[0x53].flags |= 2;
            this->sprites[0x4b].flags |= 2;
            this->sprites[0x4c].flags |= 2;
            this->sprites[0x4a].flags |= 2;
            this->sprites[0x51].flags |= 2;
            this->sprites[0x54].flags |= 2;
            this->sprites[0x4e].flags |= 2;
            this->sprites[0x4f].flags |= 2;
            this->sprites[0x4d].flags |= 2;
            this->sprites[0x52].flags |= 2;
            this->sprites[0x55].flags |= 2;
            switch (this->cursor)
            {
            case 0:
                this->sprites[0x47].SetPendingInterrupt(9);
                this->sprites[0x4a].SetPendingInterrupt(8);
                this->sprites[0x4d].SetPendingInterrupt(8);
                this->sprites[0x50].SetPendingInterrupt(9);
                this->sprites[0x51].SetPendingInterrupt(8);
                this->sprites[0x52].SetPendingInterrupt(8);
                this->sprites[0x53].SetPendingInterrupt(9);
                this->sprites[0x54].SetPendingInterrupt(8);
                this->sprites[0x55].SetPendingInterrupt(8);
                break;
            case 1:
                this->sprites[0x47].SetPendingInterrupt(8);
                this->sprites[0x4a].SetPendingInterrupt(9);
                this->sprites[0x4d].SetPendingInterrupt(8);
                this->sprites[0x50].SetPendingInterrupt(8);
                this->sprites[0x51].SetPendingInterrupt(9);
                this->sprites[0x52].SetPendingInterrupt(8);
                this->sprites[0x53].SetPendingInterrupt(8);
                this->sprites[0x54].SetPendingInterrupt(9);
                this->sprites[0x55].SetPendingInterrupt(8);
                break;
            case 2:
                this->sprites[0x47].SetPendingInterrupt(8);
                this->sprites[0x4a].SetPendingInterrupt(8);
                this->sprites[0x4d].SetPendingInterrupt(9);
                this->sprites[0x50].SetPendingInterrupt(8);
                this->sprites[0x51].SetPendingInterrupt(8);
                this->sprites[0x52].SetPendingInterrupt(9);
                this->sprites[0x53].SetPendingInterrupt(8);
                this->sprites[0x54].SetPendingInterrupt(8);
                this->sprites[0x55].SetPendingInterrupt(9);
                break;
            }
        }
        if (WAS_PRESSED(0x1001))
        {
            g_GameManager.character = this->cursor;
            g_SoundPlayer.QueueSound(10, 0);
            g_SoundPlayer.PlayQueuedSounds();
            if (this->state != 13)
            {
                if (!(g_GameManager.unk_93d8 & 1))
                {
                    this->SetState(6);
                }
                else
                {
                    this->SetState(10);
                }
            }
            else
            {
                this->SetState(14);
            }
            this->cursor = 0;
            return 1;
        }
        if (WAS_PRESSED(0xa))
        {
            g_SoundPlayer.QueueSound(11, 0);
            g_SoundPlayer.PlayQueuedSounds();
            g_GameManager.character = this->cursor;
            if (this->state != 13)
            {
                if (!(g_GameManager.unk_93d8 & 1))
                {
                    this->SetState(4);
                }
                else
                {
                    this->SetState(8);
                }
            }
            else
            {
                this->SetState(12);
            }
            this->cursor = 0;
            return 1;
        }
        break;
    }
    this->unk_b0c0++;
    this->stateFrames++;
    this->inputLockout++;
    return 1;
}

int MainMenu::UpdateShotTypeMenu()
{
    switch (this->subState)
    {
    case 0:
        if (this->inputLockout == 0)
        {
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 10);
            if (g_App.defaultDifficulty < 4)
            {
                this->sprites[g_App.defaultDifficulty + 0x43].SetPendingInterrupt(9);
            }
            else if (!g_GameManager.IsPhantasmOpen())
            {
                this->sprites[0xa1].SetPendingInterrupt(9);
            }
            else
            {
                this->sprites[g_App.defaultDifficulty + 0x9e].SetPendingInterrupt(9);
            }
            this->sprites[0x48].flags &= ~2;
            this->sprites[0x49].flags &= ~2;
            this->sprites[0x47].flags &= ~2;
            this->sprites[0x50].flags &= ~2;
            this->sprites[0x53].flags &= ~2;
            this->sprites[0x4b].flags &= ~2;
            this->sprites[0x4c].flags &= ~2;
            this->sprites[0x4a].flags &= ~2;
            this->sprites[0x51].flags &= ~2;
            this->sprites[0x54].flags &= ~2;
            this->sprites[0x4e].flags &= ~2;
            this->sprites[0x4f].flags &= ~2;
            this->sprites[0x4d].flags &= ~2;
            this->sprites[0x52].flags &= ~2;
            this->sprites[0x55].flags &= ~2;
            this->cursor = g_GameManager.shotType;
            if (g_App.defaultDifficulty == 4)
            {
                while (!g_GameManager.HasExtraOpen(this->cursor + g_GameManager.character * 2))
                {
                    this->cursor++;
                    if (this->cursor >= 2)
                    {
                        this->cursor -= 2;
                    }
                }
            }
            else if (g_App.defaultDifficulty == 5)
            {
                while (!g_GameManager.HasPhantasmOpen(this->cursor + g_GameManager.character * 2))
                {
                    this->cursor++;
                    if (this->cursor >= 2)
                    {
                        this->cursor -= 2;
                    }
                }
            }
            switch (g_GameManager.character)
            {
            case 0:
                this->sprites[0x48].flags |= 2;
                this->sprites[0x49].flags |= 2;
                this->sprites[0x47].flags |= 2;
                g_SpriteEngine->SetSprite(&this->sprites[0x49 - this->cursor], this->sprites[0x49 - this->cursor].baseSpriteIndex + 1);
                g_SpriteEngine->SetSprite(&this->sprites[this->cursor + 0x48], this->sprites[this->cursor + 0x48].baseSpriteIndex);
                break;
            case 1:
                this->sprites[0x4b].flags |= 2;
                this->sprites[0x4c].flags |= 2;
                this->sprites[0x4a].flags |= 2;
                g_SpriteEngine->SetSprite(&this->sprites[0x4c - this->cursor], this->sprites[0x4c - this->cursor].baseSpriteIndex + 1);
                g_SpriteEngine->SetSprite(&this->sprites[this->cursor + 0x4b], this->sprites[this->cursor + 0x4b].baseSpriteIndex);
                break;
            case 2:
                this->sprites[0x4e].flags |= 2;
                this->sprites[0x4f].flags |= 2;
                this->sprites[0x4d].flags |= 2;
                g_SpriteEngine->SetSprite(&this->sprites[0x4f - this->cursor], this->sprites[0x4f - this->cursor].baseSpriteIndex + 1);
                g_SpriteEngine->SetSprite(&this->sprites[this->cursor + 0x4e], this->sprites[this->cursor + 0x4e].baseSpriteIndex);
                break;
            }
            this->subState = 0;
            this->stateFrames = 0;
        }
        if (this->unk_68 != 0)
        {
            this->SetState(11);
            this->unk_68 = 0;
            this->cursor = g_GameManager.stage - 1;
            return 1;
        }
        if (this->inputLockout == 30)
        {
            this->subState = 1;
        }
        break;
    case 1:
        if (this->MoveMenuCursor(2))
        {
            if (g_App.defaultDifficulty == 4)
            {
                while (!g_GameManager.HasExtraOpen(this->cursor + g_GameManager.character * 2))
                {
                    this->cursor++;
                    if (this->cursor >= 2)
                    {
                        this->cursor -= 2;
                    }
                }
            }
            else if (g_App.defaultDifficulty == 5)
            {
                while (!g_GameManager.HasPhantasmOpen(this->cursor + g_GameManager.character * 2))
                {
                    this->cursor++;
                    if (this->cursor >= 2)
                    {
                        this->cursor -= 2;
                    }
                }
            }
            switch (g_GameManager.character)
            {
            case 0:
                g_SpriteEngine->SetSprite(&this->sprites[0x49 - this->cursor], this->sprites[0x49 - this->cursor].baseSpriteIndex + 1);
                g_SpriteEngine->SetSprite(&this->sprites[this->cursor + 0x48], this->sprites[this->cursor + 0x48].baseSpriteIndex);
                break;
            case 1:
                g_SpriteEngine->SetSprite(&this->sprites[0x4c - this->cursor], this->sprites[0x4c - this->cursor].baseSpriteIndex + 1);
                g_SpriteEngine->SetSprite(&this->sprites[this->cursor + 0x4b], this->sprites[this->cursor + 0x4b].baseSpriteIndex);
                break;
            case 2:
                g_SpriteEngine->SetSprite(&this->sprites[0x4f - this->cursor], this->sprites[0x4f - this->cursor].baseSpriteIndex + 1);
                g_SpriteEngine->SetSprite(&this->sprites[this->cursor + 0x4e], this->sprites[this->cursor + 0x4e].baseSpriteIndex);
                break;
            }
        }
        if (WAS_PRESSED(0x1001))
        {
            g_GameManager.shotType = this->cursor;
            g_SoundPlayer.QueueSound(10, 0);
            g_SoundPlayer.PlayQueuedSounds();
            if (!(g_GameManager.unk_93d8 & 1))
            {
                g_GameManager.difficulty = g_App.defaultDifficulty;
                if (g_GameManager.difficulty < 4)
                {
                    g_GameManager.stage = 0;
                }
                else
                {
                    g_GameManager.stage = g_GameManager.difficulty + 2;
                }
                g_App.scene = 2;
                g_GameManager.SetUnk93d8Bit3(0);
                g_App.StopMusic();
                while (g_SoundPlayer.PlayQueuedSounds())
                {
                }
                return 0;
            }
            this->cursor = 0;
            this->SetState(11);
            return 2;
        }
        if (WAS_PRESSED(0xa))
        {
            g_SoundPlayer.QueueSound(11, 0);
            g_GameManager.shotType = this->cursor;
            if (this->state != 14)
            {
                if (!(g_GameManager.unk_93d8 & 1))
                {
                    this->SetState(5);
                }
                else
                {
                    this->SetState(9);
                }
            }
            else
            {
                this->SetState(13);
            }
            this->sprites[0x48].flags |= 2;
            this->sprites[0x49].flags |= 2;
            this->sprites[0x47].flags |= 2;
            this->sprites[0x50].flags |= 2;
            this->sprites[0x53].flags |= 2;
            this->sprites[0x4b].flags |= 2;
            this->sprites[0x4c].flags |= 2;
            this->sprites[0x4a].flags |= 2;
            this->sprites[0x51].flags |= 2;
            this->sprites[0x54].flags |= 2;
            this->sprites[0x4e].flags |= 2;
            this->sprites[0x4f].flags |= 2;
            this->sprites[0x4d].flags |= 2;
            this->sprites[0x52].flags |= 2;
            this->sprites[0x55].flags |= 2;
            return 2;
        }
        break;
    }
    this->unk_b0c0++;
    this->stateFrames++;
    this->inputLockout++;
    return 1;
}

int MainMenu::UpdateGameStartMenu()
{
    int count;

    switch (this->subState)
    {
    case 0:
        if (this->inputLockout == 0)
        {
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 18);
            this->sprites[0x48].flags &= ~2;
            this->sprites[0x49].flags &= ~2;
            this->sprites[0x47].flags &= ~2;
            this->sprites[0x50].flags &= ~2;
            this->sprites[0x53].flags &= ~2;
            this->sprites[0x4b].flags &= ~2;
            this->sprites[0x4c].flags &= ~2;
            this->sprites[0x4a].flags &= ~2;
            this->sprites[0x51].flags &= ~2;
            this->sprites[0x54].flags &= ~2;
            this->sprites[0x4e].flags &= ~2;
            this->sprites[0x4f].flags &= ~2;
            this->sprites[0x4d].flags &= ~2;
            this->sprites[0x52].flags &= ~2;
            this->sprites[0x55].flags &= ~2;
            switch (g_GameManager.character)
            {
            case 0:
                this->sprites[0x48].flags |= 2;
                this->sprites[0x49].flags |= 2;
                this->sprites[0x47].flags |= 2;
                break;
            case 1:
                this->sprites[0x4b].flags |= 2;
                this->sprites[0x4c].flags |= 2;
                this->sprites[0x4a].flags |= 2;
                break;
            case 2:
                this->sprites[0x4e].flags |= 2;
                this->sprites[0x4f].flags |= 2;
                this->sprites[0x4d].flags |= 2;
                break;
            }
            this->subState = 0;
            this->stateFrames = 0;
            g_GameManager.unk_93d8 |= 1;
        }
        if (this->inputLockout == 30)
        {
            this->subState = 1;
        }
        break;
    case 1:
        count = g_GameManager.clrds[g_GameManager.character * 2 + g_GameManager.shotType]
                    .stagesCleared[g_App.defaultDifficulty];
        if (count < 0)
        {
            count = 1;
        }
        else if (count >= 99)
        {
            count = 6;
        }
        if (this->cursor >= count)
        {
            this->cursor = 0;
        }
        this->MoveMenuCursor(count);
        if (WAS_PRESSED(0x1001))
        {
            g_SoundPlayer.QueueSound(10, 0);
            g_GameManager.difficulty = g_App.defaultDifficulty;
            g_GameManager.stage = this->cursor;
            g_App.scene = 2;
            g_GameManager.SetUnk93d8Bit3(0);
            g_App.StopMusic();
            while (g_SoundPlayer.PlayQueuedSounds())
            {
            }
            return 0;
        }
        if (WAS_PRESSED(0xa))
        {
            g_SoundPlayer.QueueSound(11, 0);
            this->cursor = g_GameManager.shotType;
            this->SetState(6);
            this->sprites[0x48].flags |= 2;
            this->sprites[0x49].flags |= 2;
            this->sprites[0x47].flags |= 2;
            this->sprites[0x50].flags |= 2;
            this->sprites[0x53].flags |= 2;
            this->sprites[0x4b].flags |= 2;
            this->sprites[0x4c].flags |= 2;
            this->sprites[0x4a].flags |= 2;
            this->sprites[0x51].flags |= 2;
            this->sprites[0x54].flags |= 2;
            this->sprites[0x4e].flags |= 2;
            this->sprites[0x4f].flags |= 2;
            this->sprites[0x4d].flags |= 2;
            this->sprites[0x52].flags |= 2;
            this->sprites[0x55].flags |= 2;
            return 2;
        }
        break;
    }
    this->unk_b0c0++;
    this->stateFrames++;
    this->inputLockout++;
    return 1;
}

int MainMenu::UpdateReplayMenu()
{
    int i;
    HANDLE findHandle;
    int replayCount;
    ReplayHeader *data;
    char path[0x40];
    WIN32_FIND_DATAA fd;

    switch (this->subState)
    {
    case 0:
        if (this->inputLockout == 0)
        {
            if (this->prevState != 7 && g_SpriteEngine->LoadSurface(0, "data/title/select00.jpg") != 0)
            {
                return 0;
            }
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 14);
            this->cursor = 0;
            this->subState = 0;
            this->stateFrames = 0;
            this->helpSprite = NULL;
            replayCount = 0;
            for (i = 0; i < 15; i++)
            {
                sprintf(path, "./replay/th7_%.2d.rpy", i + 1);
                data = (ReplayHeader *)LoadFileData(path, 1);
                if (data == NULL)
                {
                    continue;
                }
                data = ValidateReplay(data, g_FileSize);
                if (data != NULL)
                {
                    this->replays[replayCount] = *data;
                    strcpy(this->replayPaths[replayCount], path);
                    sprintf(this->replayLabels[replayCount], "No.%.2d", i + 1);
                    replayCount++;
                    free(data);
                }
            }
            _mkdir("./replay");
            _chdir("./replay");
            findHandle = FindFirstFileA("th7_ud????.rpy", &fd);
            if (findHandle != INVALID_HANDLE_VALUE)
            {
                for (i = 0; i < 45; i++)
                {
                    data = (ReplayHeader *)LoadFileData(fd.cFileName, 1);
                    if (data == NULL)
                    {
                        continue;
                    }
                    data = ValidateReplay(data, g_FileSize);
                    if (data != NULL)
                    {
                        this->replays[replayCount] = *data;
                        sprintf(this->replayPaths[replayCount], "./replay/%s", fd.cFileName);
                        sprintf(this->replayLabels[replayCount], "User ");
                        free(data);
                        replayCount++;
                    }
                    if (!FindNextFileA(findHandle, &fd))
                    {
                        break;
                    }
                }
            }
            FindClose(findHandle);
            _chdir("../");
            this->replayCount = replayCount;
            this->unk_b0b0 = 0;
        }
        if (this->inputLockout >= 30)
        {
            this->subState = 1;
            this->stateFrames = 0;
        }
        break;
    case 1:
        this->MoveMenuCursor(this->replayCount);
        if (this->replayCount > 15)
        {
            if (WAS_PRESSED_OR_HELD(0x40))
            {
                this->cursor -= 15;
                if (this->cursor < 0)
                {
                    this->cursor += this->replayCount;
                }
                g_SoundPlayer.QueueSound(12, 0);
            }
            if (WAS_PRESSED_OR_HELD(0x80))
            {
                this->cursor += 15;
                if (this->cursor >= this->replayCount)
                {
                    this->cursor -= this->replayCount;
                }
                g_SoundPlayer.QueueSound(12, 0);
            }
        }
        this->replayCursor = this->cursor;
        if (this->stateFrames < 10)
        {
            break;
        }
        if (WAS_PRESSED(0x1001))
        {
            if (this->replayCount == 0)
            {
                break;
            }
            g_SoundPlayer.QueueSound(10, 0);
            this->subState = 2;
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 15);
            this->sprites[this->replayCursor % 15 + 0x87].SetPendingInterrupt(17);
            this->loadedReplay = (ReplayHeader *)LoadFileData(this->replayPaths[this->replayCursor], 1);
            this->loadedReplay = ValidateReplay(this->loadedReplay, g_FileSize);
            for (i = 0; i < 7; i++)
            {
                if (this->loadedReplay->stages[i] != NULL)
                {
                    this->loadedReplay->stages[i] =
                        (ReplayStage *)((int)this->loadedReplay + (int)this->loadedReplay->stages[i]);
                }
            }
            this->cursor = 0;
            while (this->replays[this->replayCursor].stages[this->cursor] == NULL)
            {
                this->cursor++;
                if (this->cursor >= 7)
                {
                    LogError(&g_Log, "リプレイデータが異常\r\n");
                    return 0;
                }
            }
            break;
        }
        if (WAS_PRESSED(0xa))
        {
            g_SoundPlayer.QueueSound(11, 0);
            this->subState = 4;
            this->stateFrames = 0;
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 16);
        }
        break;
    case 2:
        i = this->MoveMenuCursor(7);
        if (i < 0)
        {
            while (this->replays[this->replayCursor].stages[this->cursor] == NULL)
            {
                this->cursor--;
                if (this->cursor < 0)
                {
                    this->cursor = 6;
                }
            }
        }
        else if (i > 0)
        {
            while (this->replays[this->replayCursor].stages[this->cursor] == NULL)
            {
                this->cursor++;
                if (this->cursor >= 7)
                {
                    this->cursor = 0;
                }
            }
        }
        this->replayStage = this->cursor;
        if (WAS_PRESSED(0x1001))
        {
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 19);
            this->sprites[this->replayCursor % 15 + 0x87].SetPendingInterrupt(17);
            this->subState = 3;
            this->cursor = 0;
            this->sprites[0x9e].pendingInterrupt = 21;
            this->sprites[0x9f].pendingInterrupt = 21;
            this->sprites[0xa0].pendingInterrupt = 21;
            this->sprites[this->cursor + 0x9e].pendingInterrupt = 20;
            break;
        }
        if (WAS_PRESSED(0xa))
        {
            MemFree(this->loadedReplay);
            this->loadedReplay = NULL;
            this->subState = 1;
            this->inputLockout = 0;
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 14);
            this->cursor = this->replayCursor;
            break;
        }
        break;
    case 3:
        i = this->MoveMenuCursor(3);
        if (i != 0)
        {
            this->sprites[0x9e].pendingInterrupt = 21;
            this->sprites[0x9f].pendingInterrupt = 21;
            this->sprites[0xa0].pendingInterrupt = 21;
            this->sprites[this->cursor + 0x9e].pendingInterrupt = 20;
        }
        if (WAS_PRESSED(0x1001))
        {
            g_GameManager.SetUnk93d8Bit3(1);
            strcpy(g_GameManager.replayFile, this->replayPaths[this->replayCursor]);
            g_GameManager.difficulty = this->loadedReplay->difficulty;
            g_GameManager.character = this->loadedReplay->shotType / 2;
            g_GameManager.shotType = this->loadedReplay->shotType % 2;
            g_GameManager.shotIndex = this->loadedReplay->shotType;
            MemFree(this->loadedReplay);
            this->loadedReplay = NULL;
            g_GameManager.stage = g_GameManager.difficulty >= 5 ? 7 : this->replayStage;
            g_App.scene = 2;
            g_GameManager.replayMenuChoice = this->cursor;
            g_App.StopMusic();
            while (g_SoundPlayer.PlayQueuedSounds())
            {
            }
            return 0;
        }
        if (WAS_PRESSED(0xa))
        {
            this->subState = 2;
            this->inputLockout = 0;
            this->cursor = this->replayStage;
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 15);
            this->sprites[this->replayCursor % 15 + 0x87].SetPendingInterrupt(17);
            break;
        }
        break;
    case 4:
        if (this->stateFrames >= 30)
        {
            this->SetState(0);
            this->cursor = 3;
            return 1;
        }
        break;
    }
    this->unk_b0c0++;
    this->stateFrames++;
    this->inputLockout++;
    return 1;
}

int MainMenu::DrawReplayMenu()
{
    Sprite *text;
    int top;
    int j;

    text = &this->sprites[0x86];
    g_AsciiText.AddFormat(&text->pos, "No.   Name       Date  Player   Rank");
    j = this->replayCursor - this->replayCursor % 15;
    for (top = j + 15; j < top; j++)
    {
        if (j >= this->replayCount)
        {
            break;
        }
        text++;
        g_AsciiText.SetHighlight(j == this->replayCursor);
        if (j == this->replayCursor)
        {
            g_AsciiText.SetColor(0xffffffff);
        }
        else
        {
            g_AsciiText.SetColor(0xff808080);
        }
        g_AsciiText.AddFormat(&text->pos, "%s %8s  %6s %7s  %8s", this->replayLabels[j], this->replays[j].name,
                              this->replays[j].date, g_MenuShotTypeNames[this->replays[j].shotType],
                              g_MenuDifficultyNames[this->replays[j].difficulty]);
    }
    if ((this->subState == 2 || this->subState == 3) && this->loadedReplay != NULL)
    {
        g_AsciiText.SetColor(0xffffffff);
        g_AsciiText.SetHighlight(0);
        text = &this->sprites[0x85];
        g_AsciiText.AddFormat(&text->pos, "       %2.3f%%", this->loadedReplay->slowdown);
        text = &this->sprites[0x96];
        g_AsciiText.AddFormat(&text->pos, "Stage    LastScore");
        j = this->replayCursor - this->replayCursor % 15;
        for (top = 0; top < 7; top++, j++)
        {
            text++;
            if (this->subState != 3)
            {
                g_AsciiText.SetHighlight(top == this->replayStage);
                if (top == this->replayStage)
                {
                    g_AsciiText.SetColor(0xffffffff);
                }
                else
                {
                    g_AsciiText.SetColor(0xff808080);
                }
            }
            else
            {
                if (top == this->replayStage)
                {
                    g_AsciiText.SetColor(0x60ffffff);
                }
                else
                {
                    g_AsciiText.SetColor(0x60808080);
                }
            }
            if (this->loadedReplay->stages[top] != NULL)
            {
                if (top < 6 || this->loadedReplay->difficulty <= 4)
                {
                    g_AsciiText.AddFormat(&text->pos, "%s %9d0", g_MenuStageNames[top],
                                          this->loadedReplay->stages[top]->score);
                }
                else
                {
                    g_AsciiText.AddFormat(&text->pos, "%s %9d0", g_MenuStageNames[7],
                                          this->loadedReplay->stages[top]->score);
                }
            }
            else
            {
                if (top < 6 || this->loadedReplay->difficulty <= 4)
                {
                    g_AsciiText.AddFormat(&text->pos, "%s ----------", g_MenuStageNames[top]);
                }
                else
                {
                    g_AsciiText.AddFormat(&text->pos, "%s ----------", g_MenuStageNames[7]);
                }
            }
        }
    }
    g_AsciiText.SetColor(0xffffffff);
    g_AsciiText.SetHighlight(0);
    return 1;
}

int MainMenu::DrawPracticeScores()
{
    Sprite *text;
    int i;
    int cleared;
    D3DXVECTOR3 pos;

    g_AsciiText.SetColor(0xffffffff);
    g_AsciiText.SetHighlight(0);
    text = &this->sprites[0x83];
    g_AsciiText.AddFormat(&text->pos, "Stage    HI-Score");
    pos = text->pos;
    pos.y += 16.0f;
    cleared = g_GameManager.clrds[g_GameManager.character * 2 + g_GameManager.shotType]
                  .stagesCleared[g_App.defaultDifficulty];
    if (cleared < 0)
    {
        cleared = 1;
    }
    for (i = 0; i < 6; i++)
    {
        g_AsciiText.SetHighlight(i == this->cursor);
        if (i == this->cursor)
        {
            g_AsciiText.SetColor(0xffffffff);
        }
        else if (i < cleared)
        {
            g_AsciiText.SetColor(0xffa0a0a0);
        }
        else
        {
            g_AsciiText.SetColor(0xff404040);
        }
        g_AsciiText.AddFormat(
            &pos, "%s %9d0 (%3d)", g_PracticeStageNames[i],
            g_GameManager.pscrs[g_GameManager.character * 2 + g_GameManager.shotType][i][g_App.defaultDifficulty].score,
            g_GameManager.pscrs[g_GameManager.character * 2 + g_GameManager.shotType][i][g_App.defaultDifficulty].playCount);
        pos.y += 16.0f;
    }
    g_AsciiText.SetColor(0xffffffff);
    g_AsciiText.SetHighlight(0);
    return 1;
}

int MainMenu::MoveMenuCursor(int count)
{
    if (count == 0)
    {
        return 0;
    }
    if (WAS_PRESSED_OR_HELD(0x10))
    {
        this->cursor--;
        g_SoundPlayer.QueueSound(12, 0);
        if (this->cursor < 0)
        {
            this->cursor = count - 1;
        }
        if (this->cursor >= count)
        {
            this->cursor = 0;
        }
        return -1;
    }
    if (WAS_PRESSED_OR_HELD(0x20))
    {
        this->cursor++;
        g_SoundPlayer.QueueSound(12, 0);
        if (this->cursor < 0)
        {
            this->cursor = count - 1;
        }
        if (this->cursor >= count)
        {
            this->cursor = 0;
        }
        return 1;
    }
    return 0;
}

int MainMenu::MoveMenuCursorLR(int count)
{
    if (count == 0)
    {
        return 0;
    }
    if (WAS_PRESSED_OR_HELD(0x40))
    {
        this->cursor--;
        if (this->cursor < 0)
        {
            this->cursor += count;
        }
        g_SoundPlayer.QueueSound(12, 0);
        return -1;
    }
    if (WAS_PRESSED_OR_HELD(0x80))
    {
        this->cursor++;
        if (this->cursor >= count)
        {
            this->cursor -= count;
        }
        g_SoundPlayer.QueueSound(12, 0);
        return 1;
    }
    return 0;
}

int MainMenu::OnDraw(MainMenu *menu)
{
    int i;
    Sprite *vm;

    g_SpriteEngine->ClearTexture();
    g_SpriteEngine->CopySurfaceToBackBuffer(0, 0, 0, 0, 0);
    switch (menu->state)
    {
    case 7:
        menu->DrawReplayMenu();
        break;
    case 11:
        menu->DrawPracticeScores();
        break;
    }
    vm = menu->sprites;
    for (i = 0; i < menu->spriteCount; i++, vm++)
    {
        if (vm->HasTexture())
        {
            D3DXVECTOR3 savedPos = vm->pos;
            vm->pos += vm->posOffset;
            if (vm->rotationZ != 0.0f)
            {
                g_SpriteEngine->Draw(vm);
            }
            else
            {
                g_SpriteEngine->DrawUnrotated(vm);
            }
            vm->pos = savedPos;
        }
    }
    if (menu->helpSprite != NULL)
    {
        g_SpriteEngine->DrawUnrotated(menu->helpSprite);
    }
    return 1;
}

int MainMenu::InitTitleScene()
{
    ScoreDat *dat;
    int i;

    if (g_GameManager.unk_4 != NULL)
    {
        delete g_GameManager.unk_4;
        g_GameManager.unk_4 = NULL;
    }
    g_GameManager.unk_4 = new Unknown626270Unk4;
    if (g_GameManager.stats != NULL)
    {
        delete g_GameManager.stats;
        g_GameManager.stats = NULL;
    }
    g_GameManager.stats = new Unknown626270Stats;
    g_App.gameSpeed = 1.0f;
    if (g_GameManager.unk_93d8 >> 3 & 1)
    {
        g_GameManager.character = g_GameManager.shotIndex = 0;
    }
    if (g_GameManager.unk_93d8 >> 1 & 1)
    {
        g_GameManager.unk_93d8 &= ~8;
    }
    dat = LoadScoreDat("score.dat");
    LoadClears(dat, g_GameManager.clrds);
    LoadPracticeScores(dat, &g_GameManager.pscrs[0][0][0]);
    LoadSpellCards(dat, g_GameManager.catks);
    ReleaseScoreDat(dat);
    if ((DWORD)g_GameManager.plst.playHours < 7)
    {
        g_GameManager.maxContinues = 3;
    }
    else if ((DWORD)g_GameManager.plst.playHours < 14)
    {
        g_GameManager.maxContinues = 4;
    }
    else
    {
        g_GameManager.maxContinues = 5;
    }
    if (!g_GameManager.phantasmWasOpen && g_GameManager.IsPhantasmOpen())
    {
        i = 0;
        g_SpriteEngine->LoadSurface(0, "data/title/phantasm.jpg");
        while (i < 900)
        {
            g_SpriteEngine->ClearVertexFormat();
            g_SpriteEngine->ClearSpriteData();
            g_SpriteEngine->ClearTexture();
            g_SpriteEngine->ClearColorOp();
            g_SpriteEngine->ClearBlendMode();
            g_SpriteEngine->ClearZWrite();
            g_SpriteEngine->ClearUnk8();
            g_SpriteEngine->ClearUnk2e4d4();
            g_SpriteEngine->ResetUnk0();
            g_App.d3dDevice->BeginScene();
            g_SpriteEngine->CopySurfaceToBackBuffer(0, 0, 0, 0, 0);
            if (i < 60)
            {
                float rect[4];
                ColorBytes fade;

                rect[0] = 0.0f;
                rect[1] = 0.0f;
                rect[2] = 639.0f;
                rect[3] = 479.0f;
                fade.a = (60 - i) * 255 / 60;
                fade.r = fade.g = fade.b = 0;
                DrawColorRect(rect, fade.color);
            }
            else if (i > 840)
            {
                ColorBytes color;
                float area[4];

                area[0] = 0.0f;
                area[1] = 0.0f;
                area[2] = 639.0f;
                area[3] = 479.0f;
                color.a = (i - 840) * 255 / 60;
                color.r = color.g = color.b = 0;
                DrawColorRect(area, color.color);
            }
            g_Buttons = ReadButtons();
            g_App.d3dDevice->EndScene();
            if (g_App.d3dDevice->Present(NULL, NULL, NULL, NULL) < 0)
            {
                g_App.d3dDevice->Reset(&g_App.presentParams);
            }
            if (i >= 120 && i < 840 && WAS_PRESSED(0x1003))
            {
                i = 840;
                g_SoundPlayer.QueueSound(10, 0);
            }
            i++;
            g_SoundPlayer.PlayQueuedSounds();
        }
        g_SpriteEngine->ReleaseSurface(0);
    }
    g_GameManager.phantasmWasOpen = g_GameManager.IsPhantasmOpen();
    this->state = 0;
    g_App.ResetClockDriftCounters();
    switch (g_App.prevScene)
    {
    case 2:
    case 3:
    case 6:
        this->cursor = g_GameManager.difficulty >= 4;
        break;
    case 5:
        this->cursor = 4;
        break;
    case 8:
        this->cursor = 5;
        break;
    default:
        this->cursor = 0;
        break;
    }
    this->unk_68 = 0;
    if (g_GameManager.unk_93d8 & 1)
    {
        this->cursor = 2;
        this->unk_68 = 1;
    }
    g_GameManager.unk_93d8 &= ~1;
    if (g_App.prevScene != 0)
    {
        DrawLoadingScreen();
    }
    if (g_SpriteEngine->LoadAnm(0x20, "data/title01.anm", 0x900) != 0)
    {
        return -1;
    }
    if (!(g_GameManager.unk_93d8 >> 1 & 1))
    {
        if (g_App.prevScene != 5)
        {
            g_App.LoadMidi(8, "bgm/th07_01.mid");
        }
        if (g_App.lastPlayTimeTick == 0)
        {
            ScreenEffect::StartEffect(0, 70, 0xffffff, 0, 0);
        }
        else
        {
            ScreenEffect::StartEffect(0, 70, 0xffffff, 0, 0);
        }
    }
    for (int j = 0; j < 0xe; j++)
    {
        g_SpriteEngine->StartScript(&this->helpSprites[j], 0x706);
        g_SpriteEngine->SetSprite(&this->helpSprites[j], this->helpSprites[j].spriteIndex + j);
    }
    this->helpSprite = this->helpSprites;
    g_GameManager.unk_93d8 &= ~2;
    g_GameManager.demoFrames = 0;
    return 0;
}

int MainMenu::AddedCallback(MainMenu *menu)
{
    return menu->InitTitleScene();
}

int MainMenu::ReleaseTitleScene()
{
    if (this->loadedReplay != NULL)
    {
        MemFree(this->loadedReplay);
        this->loadedReplay = NULL;
    }
    if (this->sprites != NULL)
    {
        delete[] this->sprites;
        this->sprites = NULL;
        this->sprites = NULL;
    }
    return 0;
}

int MainMenu::DeletedCallback(MainMenu *menu)
{
    int i;

    g_App.d3dDevice->ResourceManagerDiscardBytes(0);
    for (i = 0x20; i <= 0x29; i++)
    {
        g_SpriteEngine->ReleaseAnm(i);
    }
    g_SpriteEngine->ReleaseSurface(0);
    g_Chain.Remove(menu->drawChain);
    menu->drawChain = NULL;
    menu->ReleaseTitleScene();
    delete menu;
    menu = NULL;
    return 0;
}

int SetUpTitleScene(int unk)
{
    MainMenu *menu = new MainMenu;

    memset(menu, 0, sizeof(MainMenu));
    g_GameManager.isPaused = 0;
    menu->calcChain = g_Chain.CreateElem((ChainCallback)MainMenu::OnUpdate);
    menu->calcChain->arg = menu;
    menu->calcChain->addedCallback = (ChainCallback)MainMenu::AddedCallback;
    menu->calcChain->deletedCallback = (ChainCallback)MainMenu::DeletedCallback;
    if (g_Chain.AddToCalcChain(menu->calcChain, 3) != 0)
    {
        return -1;
    }
    menu->drawChain = g_Chain.CreateElem((ChainCallback)MainMenu::OnDraw);
    menu->drawChain->arg = menu;
    g_Chain.AddToDrawChain(menu->drawChain, 0);
    return 0;
}

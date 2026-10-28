// cflags: /Os -/Oi
// The /Os file from 0x4553fa to 0x45c6e0, with the title menu, th06's
// mainmenu.cpp.

#include "mainmenu.h"
#include "unsorted.h"

void App::ResetClockDriftCounters()
{
    this->unk_2a4 = 0;
    this->unk_2ac = 0;
    this->unk_2b8 = 0;
    this->unk_2b0 = 0;
    this->unk_2b4 = 0;
}

void MainMenu::SetState(int state)
{
    this->prevState = this->state;
    this->state = state;
    this->stateFrames = 0;
    this->unk_d0fc = 0;
    this->unk_8 = 0;
    this->unk_b0c0 = 0;
}

int MainMenu::OnUpdate(MainMenu *menu)
{
    int status;

    switch (menu->state)
    {
    case 0:
        status = menu->Func4555dd();
        break;
    case 7:
        status = menu->Func45a924();
        break;
    case 2:
        status = menu->Func45624d();
        break;
    case 3:
        status = menu->Func456f6b();
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
        status = menu->Func45a1dd();
        break;
    }
    g_SpriteEngine->Func4550c0(menu->sprites, menu->spriteCount);
    if (menu->unk_b0c8 != NULL)
    {
        g_SpriteEngine->Step(menu->unk_b0c8);
    }
    return status;
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

    switch (this->unk_8)
    {
    case 0:
        if (this->unk_d0fc == 0)
        {
            if (this->prevState != 5 && this->prevState != 9 && this->prevState != 13 &&
                g_SpriteEngine->LoadSurface(0, "data/title/select00.jpg") != 0)
            {
                return 0;
            }
            this->cursor = g_App.defaultDifficulty;
            if (this->state != 12)
            {
                g_SpriteEngine->Func455030(this->sprites, this->spriteCount, 7);
            }
            else if (!g_Unknown626270.Func42f94c())
            {
                g_SpriteEngine->Func455030(this->sprites, this->spriteCount, 12);
                this->cursor = 4;
            }
            else
            {
                g_SpriteEngine->Func455030(this->sprites, this->spriteCount, 22);
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
            this->unk_8 = 0;
            this->stateFrames = 0;
            this->unk_b0c8 = NULL;
        }
        if (this->unk_68 != 0)
        {
            this->SetState(9);
            this->cursor = 0;
            return 1;
        }
        if (this->unk_d0fc == 30)
        {
            this->unk_8 = 1;
        }
        break;
    case 1:
        count = this->state != 12 ? 4 : (g_Unknown626270.Func42f94c() ? 2 : 1);
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
                if (!(g_Unknown626270.unk_93d8 & 1))
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
            this->unk_8 = 3;
            this->stateFrames = 0;
            g_SpriteEngine->Func455030(this->sprites, this->spriteCount, 6);
        }
        break;
    case 3:
        if (this->stateFrames >= 30)
        {
            state = this->state;
            this->SetState(0);
            if (state != 12)
            {
                if (!(g_Unknown626270.unk_93d8 & 1))
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
            g_Unknown626270.unk_93d8 &= ~1;
            return 1;
        }
        break;
    }
    this->stateFrames++;
    this->unk_b0c0++;
    this->unk_d0fc++;
    return 1;
}

int MainMenu::UpdateCharacterMenu()
{
    switch (this->unk_8)
    {
    case 0:
        if (this->unk_d0fc == 0)
        {
            g_SpriteEngine->Func455030(this->sprites, this->spriteCount, 8);
            if (g_App.defaultDifficulty < 4)
            {
                this->sprites[g_App.defaultDifficulty + 0x43].SetPendingInterrupt(9);
            }
            else if (!g_Unknown626270.Func42f94c())
            {
                this->sprites[0xa1].SetPendingInterrupt(9);
            }
            else
            {
                this->sprites[g_App.defaultDifficulty + 0x9e].SetPendingInterrupt(9);
            }
            this->cursor = g_Unknown626270.character;
            if (g_App.defaultDifficulty == 4)
            {
                while (!g_Unknown626270.Func42f7df(this->cursor * 2) && !g_Unknown626270.Func42f7df(this->cursor * 2 + 1))
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
                while (!g_Unknown626270.Func42f853(this->cursor * 2) && !g_Unknown626270.Func42f853(this->cursor * 2 + 1))
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
            this->unk_8 = 0;
            this->stateFrames = 0;
        }
        if (this->unk_68 != 0)
        {
            this->SetState(10);
            this->cursor = 0;
            return 1;
        }
        if (this->unk_d0fc == 30)
        {
            this->unk_8 = 1;
        }
        break;
    case 1:
        if (this->Func45bc63(3))
        {
            if (g_App.defaultDifficulty == 4)
            {
                while (!g_Unknown626270.Func42f7df(this->cursor * 2) && !g_Unknown626270.Func42f7df(this->cursor * 2 + 1))
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
                while (!g_Unknown626270.Func42f853(this->cursor * 2) && !g_Unknown626270.Func42f853(this->cursor * 2 + 1))
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
            g_Unknown626270.character = this->cursor;
            g_SoundPlayer.QueueSound(10, 0);
            g_SoundPlayer.PlayQueuedSounds();
            if (this->state != 13)
            {
                if (!(g_Unknown626270.unk_93d8 & 1))
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
            g_Unknown626270.character = this->cursor;
            if (this->state != 13)
            {
                if (!(g_Unknown626270.unk_93d8 & 1))
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
    this->unk_d0fc++;
    return 1;
}

int MainMenu::UpdateShotTypeMenu()
{
    switch (this->unk_8)
    {
    case 0:
        if (this->unk_d0fc == 0)
        {
            g_SpriteEngine->Func455030(this->sprites, this->spriteCount, 10);
            if (g_App.defaultDifficulty < 4)
            {
                this->sprites[g_App.defaultDifficulty + 0x43].SetPendingInterrupt(9);
            }
            else if (!g_Unknown626270.Func42f94c())
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
            this->cursor = g_Unknown626270.shotType;
            if (g_App.defaultDifficulty == 4)
            {
                while (!g_Unknown626270.Func42f7df(this->cursor + g_Unknown626270.character * 2))
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
                while (!g_Unknown626270.Func42f853(this->cursor + g_Unknown626270.character * 2))
                {
                    this->cursor++;
                    if (this->cursor >= 2)
                    {
                        this->cursor -= 2;
                    }
                }
            }
            switch (g_Unknown626270.character)
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
            this->unk_8 = 0;
            this->stateFrames = 0;
        }
        if (this->unk_68 != 0)
        {
            this->SetState(11);
            this->unk_68 = 0;
            this->cursor = g_Unknown626270.stage - 1;
            return 1;
        }
        if (this->unk_d0fc == 30)
        {
            this->unk_8 = 1;
        }
        break;
    case 1:
        if (this->MoveMenuCursor(2))
        {
            if (g_App.defaultDifficulty == 4)
            {
                while (!g_Unknown626270.Func42f7df(this->cursor + g_Unknown626270.character * 2))
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
                while (!g_Unknown626270.Func42f853(this->cursor + g_Unknown626270.character * 2))
                {
                    this->cursor++;
                    if (this->cursor >= 2)
                    {
                        this->cursor -= 2;
                    }
                }
            }
            switch (g_Unknown626270.character)
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
            g_Unknown626270.shotType = this->cursor;
            g_SoundPlayer.QueueSound(10, 0);
            g_SoundPlayer.PlayQueuedSounds();
            if (!(g_Unknown626270.unk_93d8 & 1))
            {
                g_Unknown626270.difficulty = g_App.defaultDifficulty;
                if (g_Unknown626270.difficulty < 4)
                {
                    g_Unknown626270.stage = 0;
                }
                else
                {
                    g_Unknown626270.stage = g_Unknown626270.difficulty + 2;
                }
                g_App.scene = 2;
                g_Unknown626270.SetUnk93d8Bit3(0);
                g_App.Func43a05f();
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
            g_Unknown626270.shotType = this->cursor;
            if (this->state != 14)
            {
                if (!(g_Unknown626270.unk_93d8 & 1))
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
    this->unk_d0fc++;
    return 1;
}

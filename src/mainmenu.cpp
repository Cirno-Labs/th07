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
        status = menu->Func45798b();
        break;
    case 5:
    case 9:
    case 13:
        status = menu->Func457fe5();
        break;
    case 6:
    case 10:
    case 14:
        status = menu->Func459518();
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

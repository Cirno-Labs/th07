// cflags: /Os
// The file from 0x437903: the App chain callbacks, DirectInput set-up,
// the frame timer, the config and the music, as in th06's
// unsorted42333b.cpp. Built with /Os: its functions are laid out without
// alignment.

#include "chain.h"
#include "unsorted.h"

void DebugPrint2(const char *fmt, ...)
{
}

int App::OnDraw(App *arg)
{
    DrawFps(1);
    return 1;
}

int App::RegisterChain()
{
    App *game = &g_App;
    ChainElem *elem;
    int ret;

    game->curScene = 0;
    game->scene = -1;
    game->frames = 0;
    elem = g_Chain.CreateElem((ChainCallback)App::OnUpdate);
    elem->arg = game;
    elem->addedCallback = (ChainCallback)App::AddedCallback;
    elem->deletedCallback = (ChainCallback)App::DeletedCallback;
    ret = g_Chain.AddToCalcChain(elem, 0);
    if (ret != 0)
    {
        return ret;
    }
    elem = g_Chain.CreateElem((ChainCallback)App::OnDraw);
    elem->arg = game;
    g_Chain.AddToDrawChain(elem, 0xf);
    return 0;
}

BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE *pdidInstance, VOID *pContext)
{
    HRESULT hr;

    if (g_App.joystick == NULL)
    {
        hr = g_App.dinput->CreateDevice(pdidInstance->guidInstance, &g_App.joystick, NULL);
        if (FAILED(hr))
        {
            return DIENUM_CONTINUE;
        }
    }
    return DIENUM_STOP;
}

BOOL CALLBACK EnumAxesCallback(const DIDEVICEOBJECTINSTANCE *pdidoi, VOID *pContext)
{
    HWND hDlg = (HWND)pContext;
    DIPROPRANGE diprg;

    if (pdidoi->dwType & DIDFT_AXIS)
    {
        diprg.diph.dwSize = sizeof(DIPROPRANGE);
        diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        diprg.diph.dwHow = DIPH_BYID;
        diprg.diph.dwObj = pdidoi->dwType;
        diprg.lMin = -1000;
        diprg.lMax = +1000;

        if (FAILED(g_App.joystick->SetProperty(DIPROP_RANGE, &diprg.diph)))
        {
            return DIENUM_STOP;
        }
    }
    return DIENUM_CONTINUE;
}

int InitDirectInput(App *game)
{
    HINSTANCE hInst = (HINSTANCE)GetWindowLongA(game->hwnd, GWL_HINSTANCE);

    if (game->flags.noDirectInput)
    {
        return -1;
    }
    if (FAILED(DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID **)&game->dinput, NULL)))
    {
        game->dinput = NULL;
        LogPrintf(&g_Log, "DirectInput が使用できません\r\n");
        return -1;
    }
    if (FAILED(game->dinput->CreateDevice(GUID_SysKeyboard, &game->keyboard, NULL)))
    {
        if (game->dinput)
        {
            game->dinput->Release();
            game->dinput = NULL;
        }
        LogPrintf(&g_Log, "DirectInput が使用できません\r\n");
        return -1;
    }
    if (FAILED(game->keyboard->SetDataFormat(&c_dfDIKeyboard)))
    {
        if (game->keyboard)
        {
            game->keyboard->Release();
            game->keyboard = NULL;
        }
        if (game->dinput)
        {
            game->dinput->Release();
            game->dinput = NULL;
        }
        LogPrintf(&g_Log, "DirectInput SetDataFormat が使用できません\r\n");
        return -1;
    }
    if (FAILED(game->keyboard->SetCooperativeLevel(game->hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND | DISCL_NOWINKEY)))
    {
        if (game->keyboard)
        {
            game->keyboard->Release();
            game->keyboard = NULL;
        }
        if (game->dinput)
        {
            game->dinput->Release();
            game->dinput = NULL;
        }
        LogPrintf(&g_Log, "DirectInput SetCooperativeLevel が使用できません\r\n");
        return -1;
    }
    game->keyboard->Acquire();
    LogPrintf(&g_Log, "DirectInput は正常に初期化されました\r\n");
    game->dinput->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, NULL, DIEDFL_ATTACHEDONLY);
    if (game->joystick)
    {
        game->joystick->SetDataFormat(&c_dfDIJoystick2);
        game->joystick->SetCooperativeLevel(game->hwnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
        g_App.joystickCaps.dwSize = sizeof(DIDEVCAPS);
        game->joystick->GetCapabilities(&g_App.joystickCaps);
        game->joystick->EnumObjects(EnumAxesCallback, NULL, DIDFT_ALL);
        LogPrintf(&g_Log, "有効なパッドを発見しました\r\n");
    }
    return 0;
}

void Timer::Add(int frames)
{
    if (g_App.unk_18c >> 5 & 1)
    {
        this->current++;
        this->subFrame = 0.0f;
        this->previous = -999;
    }
    if (g_App.unk_178 > 0.99f)
    {
        this->current += frames;
    }
    else if (frames < 0)
    {
        this->Sub(-frames);
    }
    else
    {
        this->previous = this->current;
        this->subFrame += g_App.unk_178 * frames;
        while (this->subFrame >= 1.0f)
        {
            this->current++;
            this->subFrame -= 1.0f;
        }
    }
}

void Timer::Sub(int frames)
{
    if (g_App.unk_18c >> 5 & 1)
    {
        this->current--;
        this->subFrame = 0.0f;
        this->previous = -999;
    }
    if (g_App.unk_178 > 0.99f)
    {
        this->current -= frames;
    }
    else if (frames < 0)
    {
        this->Add(-frames);
    }
    else
    {
        this->previous = this->current;
        this->subFrame -= g_App.unk_178 * frames;
        while (this->subFrame < 0.0f)
        {
            this->current--;
            this->subFrame += 1.0f;
        }
    }
}

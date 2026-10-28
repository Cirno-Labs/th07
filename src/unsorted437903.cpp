// cflags: /Os
// The file from 0x437903: the App chain callbacks, DirectInput set-up,
// the frame timer, the config and the music, as in th06's
// unsorted42333b.cpp. Built with /Os: its functions are laid out without
// alignment.

#include <string.h>

#include "chain.h"
#include "midi.h"
#include "sprite.h"
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

void App::TickTimer(int *frames, float *subFrame)
{
    if (this->unk_178 <= 0.99f)
    {
        *subFrame += this->unk_178;
        if (*subFrame >= 1.0f)
        {
            (*frames)++;
            *subFrame -= 1.0f;
        }
    }
    else
    {
        (*frames)++;
    }
}

int App::LoadMidi(int slot, char *path)
{
    char *dot;
    char wavFile[0x100];

    if (g_App.musicMode == 2)
    {
        if (g_App.midi != NULL)
        {
            g_App.midi->LoadFile(slot, path);
        }
        return 0;
    }
    if (g_App.musicMode == 1)
    {
        strcpy(wavFile, path);
        dot = strrchr(wavFile, '.');
        dot[1] = 'w';
        dot[2] = 'a';
        dot[3] = 'v';
        g_SoundPlayer.Func44d2f0(1, slot, wavFile);
    }
    return 1;
}

int App::PlayMidi(int slot)
{
    if (g_App.musicMode == 2)
    {
        if (g_App.midi != NULL)
        {
            MidiPlayer *midi = g_App.midi;
            midi->Stop();
            midi->ParseFile(slot);
            midi->Play();
        }
        return 0;
    }
    if (g_App.musicMode == 1)
    {
        if (g_App.flags.preloadBgm)
        {
            g_SoundPlayer.Func44d2f0(4, 0, "dummy");
        }
        g_SoundPlayer.Func44d2f0(2, slot, "dummy");
    }
    return 0;
}

int App::PlayMusic(char *path)
{
    char *dot;
    char wavFile[0x100];

    if (g_App.musicMode == 2)
    {
        if (g_App.midi != NULL)
        {
            MidiPlayer *midi = g_App.midi;
            midi->Stop();
            midi->Load(path);
            midi->Play();
        }
    }
    else if (g_App.musicMode == 1)
    {
        strcpy(wavFile, path);
        dot = strrchr(wavFile, '.');
        dot[1] = 'w';
        dot[2] = 'a';
        dot[3] = 'v';
        g_SoundPlayer.Func44d2f0(2, -1, wavFile);
    }
    else
    {
        return -1;
    }
    return 0;
}

int App::StopMusic()
{
    if (g_App.musicMode == 2)
    {
        if (g_App.midi != NULL)
        {
            g_App.midi->Stop();
        }
    }
    else if (g_App.musicMode == 1)
    {
        if (g_App.flags.preloadBgm)
        {
            g_SoundPlayer.Func44d2f0(4, 0, "dummy");
        }
        else
        {
            g_SoundPlayer.Func44d2f0(3, 0, "dummy");
        }
    }
    else
    {
        return -1;
    }
    return 0;
}

int App::FadeOutMusic(float seconds)
{
    float duration;

    if (g_App.musicMode == 2)
    {
        if (g_App.midi != NULL)
        {
            g_App.midi->FadeOut(1000.0f * seconds);
        }
    }
    else if (g_App.musicMode == 1)
    {
        // The wave fade counts frames, so it is stretched while the game
        // runs slow.
        if (this->unk_178 == 0.0f)
        {
            duration = seconds;
        }
        else if (this->unk_178 > 1.0f)
        {
            duration = seconds;
        }
        else
        {
            duration = seconds / this->unk_178;
        }
        g_SoundPlayer.Func44d2f0(5, duration, "");
    }
    else
    {
        return -1;
    }
    return 0;
}

int App::IsScoreDisabled()
{
    return g_Unknown626270.unk_4 != NULL && g_Unknown626270.unk_4->unk_25 != 0;
}

int App::EnableFog()
{
    g_SpriteEngine->Func44f5c0();
    if (this->unk_2bc != 1)
    {
        this->unk_2bc = 1;
        return this->d3dDevice->SetRenderState(D3DRS_FOGENABLE, TRUE);
    }
    return 0;
}

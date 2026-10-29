// cflags: /Os
// The file from 0x437903: the App chain callbacks, DirectInput set-up,
// the frame timer, the config and the music, as in th06's
// unsorted42333b.cpp. Built with /Os: its functions are laid out without
// alignment.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ascii.h"
#include "chain.h"
#include "dsutil.h"
#include "mainmenu.h"
#include "midi.h"
#include "musicscreen.h"
#include "sprite.h"
#include "unsorted.h"

void DebugPrint2(const char *fmt, ...)
{
}

void App::CheckClockDrift()
{
    double counterTime;
    double clockTime;

    if (this->unk_2b8 == 0)
    {
        return;
    }
    QueryPerformanceCounter((LARGE_INTEGER *)this->counter);
    GetLocalTime(&this->time);
    clockTime = this->time.wDay * 24.0 * 60.0 * 60.0 + this->time.wHour * 60 * 60 + this->time.wMinute * 60 +
                this->time.wSecond;
    counterTime = this->startTime.wDay * 24.0 * 60.0 * 60.0 + this->startTime.wHour * 60 * 60 +
                  this->startTime.wMinute * 60 + this->startTime.wSecond;
    // Into the next month.
    if (clockTime < counterTime)
    {
        clockTime = (this->startTime.wDay + 1) * 24.0 * 60.0 * 60.0 + this->time.wHour * 60 * 60 +
                    this->time.wMinute * 60 + this->time.wSecond;
    }
    clockTime -= counterTime;
    clockTime = clockTime * 1000.0 + this->time.wMilliseconds - this->startTime.wMilliseconds;
    clockTime /= 1000.0;
    counterTime = (double)(this->counter[0] - this->startCounter[0]) / (double)this->perfFrequency[0];
    if (counterTime >= 1.0)
    {
        if (clockTime / counterTime > 2.5)
        {
            this->unk_2a4++;
            if (this->unk_2ac < this->unk_2a4)
            {
                this->unk_2ac = this->unk_2a4;
            }
            if (this->unk_2b0 < this->unk_2a4)
            {
                this->unk_2b0 = this->unk_2a4;
            }
            if (this->unk_2b0 >= 10)
            {
                this->unk_2b4++;
                this->unk_2b0 = 0;
            }
            DebugPrint2("alq チェック %f / %f = %f\r\n", clockTime, counterTime, clockTime / counterTime);
        }
        else if (this->unk_2a4 != 0)
        {
            this->unk_2a4--;
        }
        this->unk_2b8 = 0;
    }
    if (this->unk_2ac >= 40 || this->unk_2b4 >= 16)
    {
        this->unk_18c |= 8;
    }
    else
    {
        this->unk_18c &= ~8;
    }
}

void SpriteEngine::ReleaseVertexBuffer()
{
    if (this->vertexBuffer != NULL)
    {
        this->vertexBuffer->Release();
        this->vertexBuffer = NULL;
    }
}

int App::OnUpdate(App *arg)
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
    g_SpriteEngine->shakeY = 0.0f;
    g_SpriteEngine->shakeX = 0.0f;
    g_App.unk_2bc = 0xff;
    if (g_SoundPlayer.bgm != NULL)
    {
        g_SoundPlayer.bgm->UpdateFade();
    }
    if (g_Unknown626270.unk_d == 0)
    {
        g_PrevButtons = g_Buttons;
        g_Buttons = ReadButtons();
        g_ButtonRepeat = 0;
        if (g_PrevButtons == g_Buttons)
        {
            if (g_ButtonFrames >= 30)
            {
                if (g_ButtonFrames % 8 == 0)
                {
                    g_ButtonRepeat = 1;
                }
                if (g_ButtonFrames >= 38)
                {
                    g_ButtonFrames = 30;
                }
            }
            g_ButtonFrames++;
        }
        else
        {
            g_ButtonFrames = 0;
        }
    }
    else
    {
        g_Buttons |= ReadButtons();
    }
    if (arg->curScene != arg->scene)
    {
        arg->prevScene = arg->curScene;
        DebugPrint2("scene %d -> %d\r\n", arg->curScene, arg->scene);
        switch (arg->curScene)
        {
        case 0:
        title:
            arg->scene = 1;
            g_App.d3dDevice->ResourceManagerDiscardBytes(0);
            if (SetUpTitleScene(0) != 0)
            {
                return 4;
            }
            break;
        case 1:
            switch (arg->scene)
            {
            case -1:
                return 4;
            case 2:
                if (RegisterGameManagerChain() != 0)
                {
                    return 4;
                }
                break;
            case 4:
                return 5;
            case 5:
                if (ResultScreen::RegisterChain(0) != 0)
                {
                    return 4;
                }
                break;
            case 8:
                if (MusicScreen::RegisterChain() != 0)
                {
                    return 4;
                }
                break;
            case 9:
                Func42f45d();
                if (Func41e820() != 0)
                {
                    return 4;
                }
                break;
            }
            break;
        case 5:
            switch (arg->scene)
            {
            case -1:
                return 4;
            case 1:
                arg->scene = 0;
                goto title;
            }
            break;
        case 2:
            switch (arg->scene)
            {
            case -1:
                return 4;
            case 1:
                Func42f45d();
                arg->scene = 0;
                SaveReplay(NULL, NULL);
                goto title;
            case 6:
                Func42f45d();
                if (ResultScreen::RegisterChain(1) != 0)
                {
                    return 4;
                }
                break;
            case 10:
                Func42f45d();
                if (!(g_Unknown626270.unk_93d8 & 1) && g_Unknown626270.difficulty < 4)
                {
                    g_Unknown626270.stage = 0;
                }
                else
                {
                    g_Unknown626270.stage--;
                }
                SaveReplay(NULL, NULL);
                if (RegisterGameManagerChain() != 0)
                {
                    return 4;
                }
                arg->scene = 2;
                break;
            case 11:
                g_App.scene = 3;
                Func42f45d();
                g_Unknown626270.stage--;
                if (RegisterGameManagerChain() != 0)
                {
                    return 4;
                }
                arg->scene = 2;
                break;
            case 12:
                g_App.scene = 3;
                Func42f45d();
                if (RegisterGameManagerChain() != 0)
                {
                    return 4;
                }
                arg->scene = 2;
                break;
            case 3:
                Func42f45d();
                if (RegisterGameManagerChain() != 0)
                {
                    return 4;
                }
                arg->scene = 2;
                break;
            case 7:
                Func42f45d();
                arg->scene = 0;
                SaveReplay(NULL, NULL);
                arg->scene = 1;
                g_App.d3dDevice->ResourceManagerDiscardBytes(0);
                if (SetUpTitleScene(1) != 0)
                {
                    return 4;
                }
                break;
            case 9:
                g_Unknown626270.plst.counts[g_Unknown626270.difficulty].clears++;
                Func42f45d();
                if (Func41e820() != 0)
                {
                    return 4;
                }
                break;
            }
            break;
        case 6:
            switch (arg->scene)
            {
            case -1:
                SaveReplay(NULL, NULL);
                return 4;
            case 1:
                arg->scene = 0;
                SaveReplay(NULL, NULL);
                goto title;
            }
            break;
        case 8:
            switch (arg->scene)
            {
            case -1:
                return 4;
            case 1:
                arg->scene = 0;
                goto title;
            }
            break;
        case 9:
            switch (arg->scene)
            {
            case -1:
                return 4;
            case 1:
                arg->scene = 0;
                goto title;
            case 6:
                if (ResultScreen::RegisterChain(1) != 0)
                {
                    return 4;
                }
            }
        }
        g_Buttons = g_PrevButtons = g_ButtonRepeat = 0;
    }
    arg->curScene = arg->scene;
    arg->frames++;
    if (arg->frames % 4000 == 3999)
    {
        if (g_App.CheckVersionFile("0100b", g_App.exeSize, g_App.exeChecksum) != 0)
        {
            return 4;
        }
    }
    return 1;
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

int OpenGameArchive()
{
    char path[0x80];

    if (g_Archive.Open("th07.dat"))
    {
        sprintf(path, "th07_%.4x%c.ver", 0x100, 'b');
        g_App.verFile = LoadFileData(path, 0);
        g_App.verFileSize = g_FileSize;
        if (g_App.verFile == NULL)
        {
            LogError(&g_Log, "error : データのバージョンが違います\r\n");
            return -1;
        }
    }
    else
    {
        LogError(&g_Log, "error : データファイルが存在しません\r\n");
        return -1;
    }
    return 0;
}

int MeasureRefreshRate()
{
    int count;
    int before;
    int ticks;
    float values[30];
    int validCount;
    float fpsRate;
    int interval;
    int nowTime;
    float seconds;
    float sum;
    int k;

    count = 0;
    ticks = 0;
    validCount = 0;
    before = 0;
    timeBeginPeriod(1);
    before = timeGetTime();
    timeEndPeriod(1);
    while (count < 1800 && validCount < 8)
    {
        g_App.d3dDevice->BeginScene();
        g_SpriteEngine->CopySurfaceToBackBuffer(0, 0, 0, 0, 0);
        g_App.d3dDevice->EndScene();
        if (FAILED(g_App.d3dDevice->Present(NULL, NULL, NULL, NULL)))
        {
            g_App.d3dDevice->Reset(&g_App.presentParams);
        }
        count++;
        timeBeginPeriod(1);
        nowTime = timeGetTime();
        timeEndPeriod(1);
        ticks++;
        interval = nowTime - before;
        if (interval >= 700)
        {
            before = nowTime;
            ticks = 0;
        }
        else if (interval >= 500)
        {
            seconds = interval / 1000.0f;
            fpsRate = ticks * 1000.0f / interval;
            if (fpsRate >= 57.0f)
            {
                values[validCount] = fpsRate;
                validCount++;
            }
            before = nowTime;
            ticks = 0;
        }
    }
    if (!g_App.flags.unk14)
    {
        sum = 0.0f;
        if (validCount >= 2)
        {
            for (k = 0; k < validCount; k++)
            {
                sum += values[k];
            }
            sum /= k;
        }
        else
        {
            sum = 1000.0f;
        }
        if (sum > 160.0f)
        {
            LogPrintf(&g_Log, "垂直同期が取れてないか、リフレッシュレートが高すぎます\r\n");
            LogPrintf(&g_Log, "強制６０フレームモードで動作します\r\n");
            g_App.presentImmediate = 1;
        }
        else if (sum >= 65.0f)
        {
            LogPrintf(&g_Log, "垂直同期が取れてないか、リフレッシュレートが高すぎます。\r\n");
            LogPrintf(&g_Log, "強制６０フレームモードで動作します\r\n");
            g_App.presentImmediate = 1;
            return -2;
        }
    }
    return 0;
}

int App::AddedCallback(App *arg)
{
    int i;
    ScoreDat *scoreDat;

    QueryPerformanceFrequency((LARGE_INTEGER *)arg->perfFrequency);
    g_App.d3dDevice->BeginScene();
    g_App.d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0xff000000, 1.0f, 0);
    g_App.d3dDevice->EndScene();
    if (FAILED(g_App.d3dDevice->Present(NULL, NULL, NULL, NULL)))
    {
        g_App.d3dDevice->Reset(&g_App.presentParams);
    }
    g_App.d3dDevice->BeginScene();
    g_App.d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0xff000000, 1.0f, 0);
    g_App.d3dDevice->EndScene();
    if (FAILED(g_App.d3dDevice->Present(NULL, NULL, NULL, NULL)))
    {
        g_App.d3dDevice->Reset(&g_App.presentParams);
    }
    if (OpenGameArchive() != 0)
    {
        return -1;
    }
    // Shows the logo while the refresh rate is measured, or for four
    // frames.
    g_SpriteEngine->LoadSurface(0, "data/title/th07logo.jpg");
    g_App.hideFps = 1;
    if (g_App.presentImmediate == 0)
    {
        if (MeasureRefreshRate() != 0)
        {
            g_SpriteEngine->ReleaseSurface(0);
            return -2;
        }
    }
    else
    {
        i = 0;
        while (i < 4)
        {
            g_App.d3dDevice->BeginScene();
            g_SpriteEngine->CopySurfaceToBackBuffer(0, 0, 0, 0, 0);
            g_App.d3dDevice->EndScene();
            if (FAILED(g_App.d3dDevice->Present(NULL, NULL, NULL, NULL)))
            {
                g_App.d3dDevice->Reset(&g_App.presentParams);
            }
            i++;
        }
    }
    g_SpriteEngine->ReleaseSurface(0);
    arg->hideFps = 0;
    arg->redrawFrames = 0;
    arg->unk_190 = timeGetTime();
    g_Rng.SetSeed(arg->unk_190);
    InitDirectInput(arg);
    if (arg->midi == NULL)
    {
        arg->midi = new MidiPlayer();
    }
    if (arg->midi != NULL)
    {
        arg->midi->LoadFile(0x1e, "bgm/init.mid");
    }
    g_SoundPlayer.LoadSounds();
    if (g_SpriteEngine->LoadAnm(0, "data/text.anm", 0x700) != 0)
    {
        return -1;
    }
    if (AsciiText::RegisterChain() != 0)
    {
        LogPrintf(&g_Log, "error : 文字の初期化に失敗しました\r\n");
        return -1;
    }
    g_SpriteEngine->CreateQuad();
    CreateTextSurface();
    if (g_SoundPlayer.LoadBgmFormat("bgm/thbgm.fmt") != 0)
    {
        LogPrintf(&g_Log, "error : BGM の初期化に失敗しました\r\n");
        return -1;
    }
    if (g_SoundPlayer.unk_39c8 == 0)
    {
        if (!g_App.flags.preloadBgm)
        {
            g_SoundPlayer.LoadBgm("thbgm.dat");
        }
        else
        {
            strcpy(g_SoundPlayer.bgmPath, "thbgm.dat");
        }
    }
    else
    {
        if (!g_App.flags.preloadBgm)
        {
            g_SoundPlayer.LoadBgm("th07.dat");
        }
        else
        {
            strcpy(g_SoundPlayer.bgmPath, "th07.dat");
        }
    }
    scoreDat = LoadScoreDat("score.dat");
    memset(&g_Unknown626270.plst, 0, sizeof(Plst));
    g_Unknown626270.plst.size = sizeof(Plst);
    g_Unknown626270.plst.size2 = g_Unknown626270.plst.size;
    g_Unknown626270.plst.magic = 'TSLP';
    g_Unknown626270.plst.version = 1;
    LoadPlstChunk(scoreDat, &g_Unknown626270.plst);
    ReleaseScoreDat(scoreDat);
    g_App.timer = new Unknown4378b0();
    if (g_App.timer != NULL)
    {
        g_App.timer->StartTimer();
    }
    return 0;
}

inline void FreeData(void *data);

int App::DeletedCallback(App *arg)
{
    if (g_App.verFile != NULL)
    {
        FreeData(g_App.verFile);
        g_App.verFile = NULL;
    }
    g_SpriteEngine->ReleaseVertexBuffer();
    g_SpriteEngine->ReleaseAnm(0);
    AsciiText::RemoveChain();
    g_SoundPlayer.QueueSoundCommand(4, 0, "dummy");
    if (arg->midi != NULL)
    {
        arg->midi->Stop();
        delete arg->midi;
        arg->midi = NULL;
    }
    SaveReplay(NULL, NULL);
    ReleaseTextSurface();
    if (arg->keyboard)
    {
        arg->keyboard->Unacquire();
    }
    if (arg->keyboard)
    {
        arg->keyboard->Release();
        arg->keyboard = NULL;
    }
    if (arg->joystick)
    {
        arg->joystick->Unacquire();
    }
    if (arg->joystick)
    {
        arg->joystick->Release();
        arg->joystick = NULL;
    }
    if (arg->dinput)
    {
        arg->dinput->Release();
        arg->dinput = NULL;
    }
    if (g_Unknown626270.stats != NULL)
    {
        delete g_Unknown626270.stats;
        g_Unknown626270.stats = NULL;
    }
    if (g_Unknown626270.unk_4 != NULL)
    {
        delete g_Unknown626270.unk_4;
        g_Unknown626270.unk_4 = NULL;
    }
    g_Archive.Release();
    if (g_App.timer != NULL)
    {
        g_App.timer->StopTimer();
        delete g_App.timer;
        g_App.timer = NULL;
    }
    return 0;
}

void DrawFps(int draw)
{
    static DWORD frames;
    static char fpsText[0x100];
    static LARGE_INTEGER lastCounter;
    static DWORD measures;
    static char speedText[0x100];
    float fps;
    float duration;

    if (g_Unknown626270.unk_d == 0)
    {
        frames += 1 + g_App.frameSkip;
        if (g_App.perfFrequency[0] == 0)
        {
            static DWORD lastTime = timeGetTime();
            DWORD now = timeGetTime();

            if (now < lastTime)
            {
                lastTime = now;
                frames = 0;
            }
            if (now - lastTime >= 500)
            {
                duration = (now - lastTime) / 1000.0f;
                lastTime = now;
            measured:
                fps = frames / duration;
                frames = 0;
                sprintf(fpsText, "%.02ffps", fps);
                if ((g_Unknown626270.unk_93d8 >> 2 & 1) && draw)
                {
                    float target = 60.0f;

                    g_App.unk_184 += target;
                    if (target * 0.9f < fps)
                    {
                        g_App.unk_180 += target;
                    }
                    else if (target * 0.7f < fps)
                    {
                        g_App.unk_180 += target * 0.8f;
                    }
                    else if (target * 0.5f < fps)
                    {
                        g_App.unk_180 += target * 0.6f;
                    }
                    else
                    {
                        g_App.unk_180 += target * 0.5f;
                    }
                    if (!(g_Unknown626270.unk_93d8 >> 3 & 1))
                    {
                        g_App.unk_188 = fps + 0.5f;
                    }
                    else
                    {
                        sprintf(speedText, "%2d", g_App.unk_188);
                    }
                }
            }
        }
        else
        {
            LARGE_INTEGER counter;

            if (lastCounter.LowPart == 0)
            {
                QueryPerformanceCounter(&lastCounter);
            }
            QueryPerformanceCounter(&counter);
            if (counter.LowPart < lastCounter.LowPart)
            {
                lastCounter = counter;
                frames = 0;
            }
            if (counter.LowPart >= lastCounter.LowPart + (g_App.perfFrequency[0] >> 1))
            {
                duration = (float)(counter.LowPart - lastCounter.LowPart) / (float)g_App.perfFrequency[0];
                lastCounter = counter;
                measures++;
                if (measures % 8 == 0)
                {
                    g_App.CheckClockDrift();
                }
                goto measured;
            }
        }
    }
    if (g_App.hideFps == 0 && draw)
    {
        D3DXVECTOR3 pos;

        pos.x = 512.0f;
        pos.y = 464.0f;
        pos.z = 0.0f;
        g_AsciiText.AddString(&pos, fpsText);
        if ((g_Unknown626270.unk_93d8 >> 3 & 1) && (g_Unknown626270.unk_93d8 >> 2 & 1))
        {
            D3DXVECTOR3 speedPos;

            speedPos.x = 384.0f;
            speedPos.y = 448.0f;
            speedPos.z = 0.0f;
            if (g_App.unk_2a8 != 0)
            {
                g_AsciiText.color = 0xffff4040;
            }
            else
            {
                g_AsciiText.color = 0xffffffd0;
            }
            g_AsciiText.AddString(&speedPos, speedText);
            g_AsciiText.color = 0xffffffff;
        }
    }
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

int App::SaveScreenshot(char *path)
{
    BITMAPFILEHEADER fileHdr;
    BYTE *bits;
    BITMAPINFO *bmi;
    IDirect3DSurface8 *buffer;

    bmi = NULL;
    bits = NULL;
    buffer = NULL;
    this->d3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &buffer);
    memset(&fileHdr, 0, sizeof(fileHdr));
    fileHdr.bfType = *(WORD *)"BM";
    fileHdr.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    fileHdr.bfSize = fileHdr.bfOffBits;
    switch (this->presentParams.BackBufferFormat)
    {
    case D3DFMT_R5G6B5:
        LogPrintf(&g_Log, "16bit は取り込めない\r\n");
        break;
    case D3DFMT_X8R8G8B8:
    {
        int widthBytes;
        int x;
        int y;
        BYTE *dst;
        BYTE *read;
        D3DLOCKED_RECT lockRect;
        int outRow;
        HANDLE file;
        DWORD written;
        int size;

        size = sizeof(BITMAPINFO);
        bmi = (BITMAPINFO *)malloc(size);
        if (bmi == NULL)
        {
            LogPrintf(&g_Log, "snapShotScreen : 確保しくり\r\n");
            break;
        }
        memset(bmi, 0, sizeof(BITMAPINFO));
        widthBytes = 640 * 3;
        bits = (BYTE *)malloc(widthBytes * 480);
        if (bits == NULL)
        {
            LogPrintf(&g_Log, "snapShotScreen : 確保しくり\r\n");
            break;
        }
        fileHdr.bfSize += widthBytes * 480;
        bmi->bmiHeader.biBitCount = 24;
        bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi->bmiHeader.biWidth = 640;
        bmi->bmiHeader.biHeight = 480;
        bmi->bmiHeader.biPlanes = 1;
        bmi->bmiHeader.biCompression = BI_RGB;
        buffer->LockRect(&lockRect, NULL, 0);
        // Bottom-up, as a bitmap's rows go.
        outRow = 0;
        for (y = 479; y > -1; y--, outRow++)
        {
            dst = bits + widthBytes * outRow;
            read = (BYTE *)lockRect.pBits + lockRect.Pitch * y;
            for (x = 0; x < 640; x++)
            {
                *dst = *read;
                read++;
                dst++;
                *dst = *read;
                read++;
                dst++;
                *dst = *read;
                read += 2;
                dst++;
            }
        }
        buffer->UnlockRect();
        file = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (file == INVALID_HANDLE_VALUE)
        {
            break;
        }
        WriteFile(file, &fileHdr, sizeof(BITMAPFILEHEADER), &written, NULL);
        WriteFile(file, bmi, sizeof(BITMAPINFOHEADER), &written, NULL);
        WriteFile(file, bits, widthBytes * 480, &written, NULL);
        CloseHandle(file);
        break;
    }
    default:
        LogPrintf(&g_Log, "error ? mother.cpp\r\n");
        return 1;
    }
    if (buffer != NULL)
    {
        buffer->Release();
        buffer = NULL;
    }
    free(bmi);
    free(bits);
    return 0;
}

// The start of thbgm.dat.
struct BgmDatHeader
{
    // "ZWAV".
    DWORD magic;
    // 1.
    int version;
    // 0x700.
    int unk_8;
    int unk_c;
};

int App::LoadConfig(char *path)
{
    ConfigCopy *file;

    memset(&g_App.controllerMapping, 0, sizeof(ConfigCopy));
    file = (ConfigCopy *)LoadFileData(path, 1);
    if (file == NULL)
    {
        LogPrintf(&g_Log, "コンフィグデータが見つからないので初期化しました\r\n");
    reset:
        g_App.lifeCount = 2;
        g_App.bombCount = 3;
        g_App.colorMode16Bits = 0xff;
        g_App.version = 0x70002;
        g_App.padThresholdX = 600;
        g_App.padThresholdY = 600;
        // Wave music if thbgm.dat is there, with the right header.
        {
            HANDLE bgmFile;
            DWORD read;
            BgmDatHeader bgmHeader;

            bgmFile = CreateFileA("./thbgm.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
            if (bgmFile != INVALID_HANDLE_VALUE)
            {
                ReadFile(bgmFile, &bgmHeader, sizeof(BgmDatHeader), &read, NULL);
                CloseHandle(bgmFile);
                if (bgmHeader.magic != 'VAWZ' || bgmHeader.version != 1 || bgmHeader.unk_8 != 0x700)
                {
                    LogError(&g_Log, "BGM データのバージョンが違います\r\n");
                    return -1;
                }
                g_App.musicMode = 1;
            }
            else
            {
                g_App.musicMode = 2;
                DebugPrint2("wave データが無いので、midi にします\r\n");
            }
        }
        g_App.playSounds = 1;
        g_App.defaultDifficulty = 1;
        g_App.windowed = 0;
        g_App.frameSkip = 0;
        g_App.controllerMapping = g_DefaultControllerMapping;
        g_App.unk_13c = 2;
        g_App.unk_13d = 0;
        g_App.shootFocuses = 1;
    }
    else
    {
        *(ConfigCopy *)&g_App.controllerMapping = *file;
        free(file);
        {
            HANDLE bgmFile;
            DWORD read;
            BgmDatHeader bgmHeader;

            bgmFile = CreateFileA("./thbgm.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
            if (bgmFile != INVALID_HANDLE_VALUE)
            {
                ReadFile(bgmFile, &bgmHeader, sizeof(BgmDatHeader), &read, NULL);
                CloseHandle(bgmFile);
                if (bgmHeader.magic != 'VAWZ' || bgmHeader.version != 1 || bgmHeader.unk_8 != 0x700)
                {
                    LogError(&g_Log, "BGM データのバージョンが違います\r\n");
                    return -1;
                }
            }
        }
        if (g_App.lifeCount >= 5 || g_App.bombCount >= 4 || g_App.colorMode16Bits >= 2 || g_App.musicMode >= 3 ||
            g_App.defaultDifficulty >= 6 || g_App.playSounds >= 2 || g_App.windowed >= 2 || g_App.frameSkip >= 3 ||
            g_App.unk_13c >= 3 || g_App.unk_13d >= 2 || g_App.shootFocuses >= 2 || g_App.version != 0x70002 ||
            g_FileSize != sizeof(ConfigCopy))
        {
            LogPrintf(&g_Log, "コンフィグデータが異常でしたので再初期化しました\r\n");
            goto reset;
        }
        g_DefaultControllerMapping = g_App.controllerMapping;
    }
    g_App.flags.emulateColorAdd = 1;
    if (this->flags.noVertexBuffer)
    {
        LogPrintf(&g_Log, "頂点バッファの使用を抑制します\r\n");
    }
    if (this->flags.noFog)
    {
        LogPrintf(&g_Log, "フォグの使用を抑制します\r\n");
    }
    if (this->flags.force16BitTextures)
    {
        LogPrintf(&g_Log, "16Bit のテクスチャの使用を強制します\r\n");
    }
    if (this->ClearsBackBuffer())
    {
        LogPrintf(&g_Log, "バックバッファの消去を強制します\r\n");
    }
    if (this->flags.noFrameItems)
    {
        LogPrintf(&g_Log, "ゲーム周りのアイテムの描画を抑制します\r\n");
    }
    if (this->flags.noGouraudShading)
    {
        LogPrintf(&g_Log, "グーローシェーディングを抑制します\r\n");
    }
    if (this->flags.noDepthTest)
    {
        LogPrintf(&g_Log, "デプステストを抑制します\r\n");
    }
    // th06's 60 frame mode is gone: the bit is cleared.
    this->presentImmediate = 0;
    this->flags.force60Fps = 0;
    if (this->flags.noColorBlending)
    {
        // Sic: the message ends in "n", not a line break.
        LogPrintf(&g_Log, "テクスチャの色合成を抑制しますn");
    }
    if (this->windowed)
    {
        LogPrintf(&g_Log, "ウィンドウモードで起動します\r\n");
    }
    if (this->flags.referenceRasterizer)
    {
        LogPrintf(&g_Log, "リファレンスラスタライザを強制します\r\n");
    }
    if (this->flags.noDirectInput)
    {
        LogPrintf(&g_Log, "パッド、キーボードの入力に DirectInput を使用しません\r\n");
    }
    if (this->flags.unk12)
    {
        LogPrintf(&g_Log, "画面周りを毎回描画します\r\n");
    }
    if (this->flags.preloadBgm)
    {
        LogPrintf(&g_Log, "ＢＧＭをメモリに読み込みます\r\n");
    }
    if (this->flags.unk14)
    {
        LogPrintf(&g_Log, "垂直同期を取りません\r\n");
        g_App.presentImmediate = 1;
    }
    if (WriteToFile(path, &g_App.controllerMapping, sizeof(ConfigCopy)) != 0)
    {
        LogError(&g_Log, "ファイルが書き出せません %s\r\n", path);
        LogError(&g_Log, "フォルダが書込み禁止属性になっているか、ディスクがいっぱいいっぱいになってませんか？\r\n");
        return -1;
    }
    return 0;
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
        g_SoundPlayer.QueueSoundCommand(1, slot, wavFile);
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
            g_SoundPlayer.QueueSoundCommand(4, 0, "dummy");
        }
        g_SoundPlayer.QueueSoundCommand(2, slot, "dummy");
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
        g_SoundPlayer.QueueSoundCommand(2, -1, wavFile);
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
            g_SoundPlayer.QueueSoundCommand(4, 0, "dummy");
        }
        else
        {
            g_SoundPlayer.QueueSoundCommand(3, 0, "dummy");
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
        g_SoundPlayer.QueueSoundCommand(5, duration, "");
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
    g_SpriteEngine->FlushQuadBatch();
    if (this->unk_2bc != 1)
    {
        this->unk_2bc = 1;
        return this->d3dDevice->SetRenderState(D3DRS_FOGENABLE, TRUE);
    }
    return 0;
}

int App::DisableFog()
{
    g_SpriteEngine->FlushQuadBatch();
    if (this->unk_2bc != 0)
    {
        this->unk_2bc = 0;
        return this->d3dDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
    }
    return 0;
}

int App::SetRenderState(D3DRENDERSTATETYPE state, DWORD value)
{
    g_SpriteEngine->FlushQuadBatch();
    return this->d3dDevice->SetRenderState(state, value);
}

void App::AddPlayTime()
{
    DWORD now = timeGetTime();
    DWORD delta;

    if (now < this->unk_190)
    {
        this->unk_190 = 0;
    }
    delta = now - this->unk_190;
    g_Unknown626270.plst.runHours += delta / 3600000;
    delta %= 3600000;
    g_Unknown626270.plst.runMinutes += delta / 60000;
    delta %= 60000;
    g_Unknown626270.plst.runSeconds += delta / 1000;
    delta %= 1000;
    g_Unknown626270.plst.runMilliseconds += delta;
    if (g_Unknown626270.plst.runMilliseconds >= 1000)
    {
        g_Unknown626270.plst.runSeconds += g_Unknown626270.plst.runMilliseconds / 1000;
        g_Unknown626270.plst.runMilliseconds %= 1000;
    }
    if (g_Unknown626270.plst.runSeconds >= 60)
    {
        g_Unknown626270.plst.runMinutes += g_Unknown626270.plst.runSeconds / 60;
        g_Unknown626270.plst.runSeconds %= 60;
    }
    if (g_Unknown626270.plst.runMinutes >= 60)
    {
        g_Unknown626270.plst.runHours += g_Unknown626270.plst.runMinutes / 60;
        g_Unknown626270.plst.runMinutes %= 60;
    }
    this->unk_190 = now;
}

void App::AddTotalPlayTime()
{
    DWORD now = timeGetTime();
    DWORD delta;

    if (now < this->unk_194)
    {
        this->unk_194 = 0;
    }
    delta = now - this->unk_194;
    g_Unknown626270.plst.playHours += delta / 3600000;
    delta %= 3600000;
    g_Unknown626270.plst.playMinutes += delta / 60000;
    delta %= 60000;
    g_Unknown626270.plst.playSeconds += delta / 1000;
    delta %= 1000;
    g_Unknown626270.plst.playMilliseconds += delta;
    if (g_Unknown626270.plst.playMilliseconds >= 1000)
    {
        g_Unknown626270.plst.playSeconds += g_Unknown626270.plst.playMilliseconds / 1000;
        g_Unknown626270.plst.playMilliseconds %= 1000;
    }
    if (g_Unknown626270.plst.playSeconds >= 60)
    {
        g_Unknown626270.plst.playMinutes += g_Unknown626270.plst.playSeconds / 60;
        g_Unknown626270.plst.playSeconds %= 60;
    }
    if (g_Unknown626270.plst.playMinutes >= 60)
    {
        g_Unknown626270.plst.playHours += g_Unknown626270.plst.playMinutes / 60;
        g_Unknown626270.plst.playMinutes %= 60;
    }
    this->unk_194 = now;
}

inline void FreeData(void *data)
{
    free(data);
}

int App::CheckVersionFile(char *version, DWORD size, DWORD checksum)
{
    DWORD size2;
    char *start;
    DWORD left;
    DWORD checksum2;
    char *p;

    if (this->verFile == NULL)
    {
        return 0;
    }
    p = (char *)this->verFile;
    left = this->verFileSize;
    if (strncmp(version, "debug", 5) == 0)
    {
        return 0;
    }
    if (strcmp("0100b", "debug") == 0)
    {
        return 0;
    }
    while (left > 0)
    {
        if (strncmp(version, p, 5) == 0)
        {
            p += 6;
            sscanf(p, "%d %d", &size2, &checksum2);
            if (size2 == size && checksum2 == checksum)
            {
                return 0;
            }
            return -1;
        }
        start = p;
        p = strchr(p, '\n') + 1;
        left -= p - start;
    }
    return -1;
}

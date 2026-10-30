// The file from 0x433e90: start-up, the window and the display, as in
// th06's unsorted41f230.cpp.

#include <direct.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <windows.h>
#define NO_SHDOCVW_GUIDS
#include <shlguid.h>
#include <shobjidl.h>
#include <winnls32.h>

#include "chain.h"
#include "midi.h"
#include "sprite.h"
#include "unsorted.h"

// The game's data, as the original defines it here. The DLL build uses
// the original's copies instead.
#ifdef TH07_EXE
GameWindow g_GameWindow;
#endif // TH07_EXE

void Log::Flush()
{
    if (this->cursor != this->buffer)
    {
        LogPrintf(this, "---------------------------------------------------------- \r\n");
        if (this->hasError)
        {
            MessageBoxA(NULL, this->buffer, "log", MB_ICONERROR);
        }
        WriteToFile("./log.txt", this->buffer, strlen(this->buffer));
    }
}

inline void SpriteEngine::ReleaseSurfaces()
{
    int i;

    for (i = 0; i < 0x20; i++)
    {
        if (this->surfaces[i] != NULL)
        {
            this->surfaces[i]->Release();
            this->surfaces[i] = NULL;
        }
    }
}

void SpriteEngine::DoPendingCopy()
{
    if (this->pendingCopyTexture >= 0)
    {
        this->CopyBackBufferToTexture(this->pendingCopyTexture, this->pendingCopySrcLeft, this->pendingCopySrcTop,
                                      this->pendingCopySrcWidth, this->pendingCopySrcHeight, this->pendingCopyDstLeft,
                                      this->pendingCopyDstTop, this->pendingCopyDstWidth, this->pendingCopyDstHeight);
        this->pendingCopyTexture = -1;
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    int runResult = 0;
    MSG message;
    HRESULT testResult;
    int j;

    g_App.hInstance = hInstance;
    SystemParametersInfoA(SPI_GETSCREENSAVEACTIVE, 0, &g_GameWindow.screenSaveActive, 0);
    SystemParametersInfoA(SPI_GETLOWPOWERACTIVE, 0, &g_GameWindow.lowPowerActive, 0);
    SystemParametersInfoA(SPI_GETPOWEROFFACTIVE, 0, &g_GameWindow.powerOffActive, 0);
    SystemParametersInfoA(SPI_SETSCREENSAVEACTIVE, 0, NULL, SPIF_SENDCHANGE);
    SystemParametersInfoA(SPI_SETLOWPOWERACTIVE, 0, NULL, SPIF_SENDCHANGE);
    SystemParametersInfoA(SPI_SETPOWEROFFACTIVE, 0, NULL, SPIF_SENDCHANGE);
    if (CheckSingleInstance(hInstance) == -1)
    {
        goto stop;
    }
    if (g_App.LoadConfig("th07.cfg") != 0)
    {
        goto stop;
    }
    ChecksumExecutable();
    QueryPerformanceFrequency((LARGE_INTEGER *)g_GameWindow.perfFrequency);
restart:
    if (CreateD3d() != 0)
    {
        goto stop;
    }
    if (CreateGameWindow(hInstance) != 0)
    {
        goto stop;
    }
    if (InitD3dDevice() != 0)
    {
        goto stop;
    }
    g_SoundPlayer.Initialize(g_GameWindow.hwnd);
    InitJoystick();
    ResetKeyboard();
    g_SpriteRenderer = new SpriteEngine();
    if (!g_App.windowed)
    {
        WINNLSEnableIME(NULL, FALSE);
        ShowCursor(FALSE);
    }
    runResult = App::RegisterChain();
    if (runResult != 0)
    {
        if (runResult == -1)
        {
            goto release;
        }
        runResult = 2;
        goto release;
    }
    runResult = 0;
    g_GameWindow.curFrame = -30;
    while (!g_GameWindow.isClosing)
    {
        if (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
        else
        {
            testResult = g_App.d3dDevice->TestCooperativeLevel();
            if (testResult == D3D_OK)
            {
                runResult = g_GameWindow.RunFrame();
                if (runResult != 0)
                {
                    break;
                }
                g_App.stateFlags &= ~0x10;
            }
            else if (testResult == D3DERR_DEVICENOTRESET)
            {
                g_SpriteRenderer->ReleaseSurfaces();
                if (g_App.d3dDevice->Reset(&g_App.presentParams) != 0)
                {
                    break;
                }
                SetDefaultRenderStates();
                g_App.redrawFrames = 3;
                g_App.stateFlags |= 0x10;
            }
        }
    }
release:
    if (g_GameManager.plst.magic != 0)
    {
        ResultScreen::RegisterChain(2);
    }
    g_Chain.Release();
    while (g_SoundPlayer.PlayQueuedSounds())
    {
    }
stop:
    g_SoundPlayer.Release();
    delete g_SpriteRenderer;
    g_SpriteRenderer = NULL;
    if (g_App.d3dDevice != NULL)
    {
        g_App.d3dDevice->Reset(&g_App.presentParams);
    }
    if (g_App.d3dDevice != NULL)
    {
        g_App.d3dDevice->Release();
        g_App.d3dDevice = NULL;
    }
    if (g_App.d3d != NULL)
    {
        g_App.d3d->Release();
        g_App.d3d = NULL;
    }
    if (g_GameWindow.hwnd != NULL)
    {
        ShowWindow(g_GameWindow.hwnd, SW_HIDE);
        MoveWindow(g_GameWindow.hwnd, 0, 0, 0, 0, FALSE);
        DestroyWindow(g_GameWindow.hwnd);
        g_GameWindow.hwnd = NULL;
    }
    ShowCursor(TRUE);
    if (runResult == 2)
    {
        g_Log.cursor = g_Log.buffer;
        *g_Log.cursor = '\0';
        LogPrintf(&g_Log, "再起動を要するオプションが変更されたので再起動します\r\n");
        if (!g_App.windowed)
        {
            WINNLSEnableIME(NULL, TRUE);
        }
        j = 0;
        while (j < 60)
        {
            if (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&message);
                DispatchMessageA(&message);
            }
            j++;
        }
        goto restart;
    }
    WriteToFile("th07.cfg", &g_App.controllerMapping, 0x38);
    SystemParametersInfoA(SPI_SETSCREENSAVEACTIVE, g_GameWindow.screenSaveActive, NULL, SPIF_SENDCHANGE);
    SystemParametersInfoA(SPI_SETLOWPOWERACTIVE, g_GameWindow.lowPowerActive, NULL, SPIF_SENDCHANGE);
    SystemParametersInfoA(SPI_SETPOWEROFFACTIVE, g_GameWindow.powerOffActive, NULL, SPIF_SENDCHANGE);
    WINNLSEnableIME(NULL, TRUE);
    g_Log.Flush();
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_ERASEBKGND:
        return 1;
    case MM_MOM_DONE:
        if (g_App.midi != NULL)
        {
            g_App.midi->ReleaseHeader((LPMIDIHDR)lParam);
        }
        break;
    case WM_ACTIVATEAPP:
        g_GameWindow.isAppActive = wParam;
        if (g_GameWindow.isAppActive != 0)
        {
            g_GameWindow.isAppInactive = 0;
        }
        else
        {
            g_GameWindow.isAppInactive = 1;
        }
        break;
    case WM_SETCURSOR:
        if (!g_App.windowed)
        {
            if (g_GameWindow.isAppInactive != 0)
            {
                SetCursor(LoadCursorA(NULL, IDC_ARROW));
                ShowCursor(TRUE);
            }
            else
            {
                ShowCursor(FALSE);
                SetCursor(NULL);
            }
        }
        else
        {
            SetCursor(LoadCursorA(NULL, IDC_ARROW));
            ShowCursor(TRUE);
        }
        return 1;
    case WM_CLOSE:
        g_GameWindow.isClosing = 1;
        return 1;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void PresentFrame()
{
    int i;
    char path[0x100];

    if (g_App.d3dDevice->Present(NULL, NULL, NULL, NULL) < 0)
    {
        g_SpriteRenderer->ReleaseSurfaces();
        g_App.d3dDevice->Reset(&g_App.presentParams);
        SetDefaultRenderStates();
        g_App.redrawFrames = 2;
    }
    g_SpriteRenderer->DoPendingCopy();
    if ((g_Buttons & 0x800) && (g_Buttons & 0x800) != (g_LastButtons & 0x800))
    {
        _mkdir("snapshot");
        for (i = 0; i < 1000; i++)
        {
            sprintf(path, "snapshot/th%.3d.bmp", i);
            if (!FileExists(path))
            {
                break;
            }
        }
        if (i < 1000)
        {
            g_App.SaveScreenshot(path);
        }
    }
    if (g_App.redrawFrames != 0)
    {
        g_App.redrawFrames--;
    }
}

int GameWindow::RunFrame()
{
    double delta2;
    double now;
    double elapsed;
    // A LARGE_INTEGER, but only 4-aligned, like GameWindow's.
    DWORD count[2];
    int res;
    static int s_frames;
    static double s_lastFrameTime;
    static LARGE_INTEGER s_lastCount;

    if (this->isAppActive == 0)
    {
        return 0;
    }
    if (this->curFrame == 0)
    {
    loop:
        if (g_App.frameSkip <= this->curFrame)
        {
            g_App.d3dDevice->BeginScene();
            g_SpriteRenderer->BeginQuadBatch();
            g_App.fogState = 0xff;
            g_App.DisableFog();
            g_Chain.RunDrawChain();
            g_SpriteRenderer->FlushQuadBatch();
            g_App.d3dDevice->SetTexture(0, NULL);
            g_App.d3dDevice->EndScene();
        }
        g_SpriteRenderer->FlushQuadBatch();
        g_App.viewport.X = 0;
        g_App.viewport.Y = 0;
        g_App.viewport.Width = 640;
        g_App.viewport.Height = 480;
        g_App.d3dDevice->SetViewport(&g_App.viewport);
        res = g_Chain.RunCalcChain();
        g_SoundPlayer.PlayQueuedSounds();
        if (res == 0)
        {
            return 1;
        }
        if (res == -1)
        {
            return 2;
        }
        this->curFrame++;
    }
    if ((g_App.windowed || g_App.UsesPresentImmediate()) && this->curFrame != 0)
    {
        if (g_GameWindow.perfFrequency[0] != 0)
        {
            QueryPerformanceCounter((LARGE_INTEGER *)count);
            elapsed = (double)(count[0] - s_lastCount.LowPart) / (double)g_GameWindow.perfFrequency[0];
            if (elapsed < 0.0)
            {
                s_lastCount = *(LARGE_INTEGER *)count;
            }
            if (elapsed >= 1.0 / 60.0 || g_GameWindow.forceClockPacing)
            {
                while (elapsed >= 1.0 / 60.0)
                {
                    s_lastCount.LowPart += g_GameWindow.perfFrequency[0] / 60;
                    elapsed -= 1.0 / 60.0;
                }
                if (g_App.frameSkip < this->curFrame)
                {
                    goto present;
                }
                goto loop;
            }
        }
        else
        {
            timeBeginPeriod(1);
            now = timeGetTime();
            if (now < s_lastFrameTime)
            {
                s_lastFrameTime = now;
            }
            delta2 = fabs(now - s_lastFrameTime);
            timeEndPeriod(1);
            if (delta2 >= 1000.0 / 60.0 || g_GameWindow.forceClockPacing)
            {
                while (delta2 >= 1000.0 / 60.0)
                {
                    s_lastFrameTime += 1000.0 / 60.0;
                    delta2 -= 1000.0 / 60.0;
                }
                if (g_App.frameSkip < this->curFrame)
                {
                    goto present;
                }
                goto loop;
            }
        }
    }
    if (!g_App.windowed && !g_App.UsesPresentImmediate())
    {
        if (g_App.frameSkip >= this->curFrame)
        {
            PresentFrame();
            goto loop;
        }
    present:
        PresentFrame();
        this->curFrame = 0;
        s_frames++;
    }
    return 0;
}

int CreateD3d()
{
    g_App.d3d = Direct3DCreate8(D3D_SDK_VERSION);
    if (g_App.d3d == NULL)
    {
        LogError(&g_Log, "Direct3D オブジェクトは何故か作成出来なかった\r\n");
        return 1;
    }
    return 0;
}

int CreateGameWindow(HINSTANCE instance)
{
    WNDCLASSA wc;
    int width;
    int height;

    memset(&wc, 0, sizeof(wc));
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hInstance = instance;
    wc.lpfnWndProc = WindowProc;
    g_GameWindow.isAppActive = 1;
    g_GameWindow.isAppInactive = 0;
    wc.lpszClassName = "BASE";
    RegisterClassA(&wc);
    if (!g_App.windowed)
    {
        width = 640;
        height = 480;
        g_GameWindow.hwnd = CreateWindowExA(0, "BASE", "東方妖々夢　～ Perfect Cherry Blossom. ver 1.00b",
                                            WS_OVERLAPPEDWINDOW, 0, 0, width, height, NULL, NULL, instance, NULL);
    }
    else
    {
        width = GetSystemMetrics(SM_CXDLGFRAME) * 2 + 640;
        height = GetSystemMetrics(SM_CYDLGFRAME) * 2 + 480 + GetSystemMetrics(SM_CYCAPTION);
        g_GameWindow.hwnd = CreateWindowExA(0, "BASE", "東方妖々夢　～ Perfect Cherry Blossom. ver 1.00b",
                                            WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT,
                                            width, height, NULL, NULL, instance, NULL);
    }
    g_App.hwnd = g_GameWindow.hwnd;
    if (g_GameWindow.hwnd == NULL)
    {
        return 1;
    }
    ForceForegroundWindow(g_GameWindow.hwnd);
    return 0;
}

int InitD3dDevice()
{
    char buffer[0x2000];
    float fovAngle;
    float aspectXY;
    float halfWidth;
    float halfHeight4;
    float dist;
    D3DPRESENT_PARAMETERS params3;
    D3DDISPLAYMODE displayMode2;
    bool checkFormat = true;
    int retried;

    memset(&params3, 0, sizeof(params3));
    g_App.d3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode2);
    if (!g_App.windowed)
    {
        if (g_App.flags.force16BitTextures == 1)
        {
            params3.BackBufferFormat = D3DFMT_R5G6B5;
            g_App.colorMode16Bits = 1;
        }
        else if (g_App.colorMode16Bits == 0xff)
        {
            params3.BackBufferFormat = D3DFMT_X8R8G8B8;
            g_App.colorMode16Bits = 0;
            LogPrintf(&g_Log, "初回起動、画面を 32Bits で初期化しました\r\n");
        }
        else if (g_App.colorMode16Bits == 0)
        {
            params3.BackBufferFormat = D3DFMT_X8R8G8B8;
        }
        else
        {
            params3.BackBufferFormat = D3DFMT_R5G6B5;
        }
        if (g_GameWindow.forceClockPacing)
        {
            g_App.presentImmediate = 1;
        }
        if (g_App.presentImmediate == 0)
        {
            params3.FullScreen_RefreshRateInHz = 60;
            params3.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE;
            LogPrintf(&g_Log, "リフレッシュレートを60Hzに変更を試みます\r\n");
            if (g_App.frameSkip == 0)
            {
                params3.SwapEffect = D3DSWAPEFFECT_FLIP;
            }
            else
            {
                params3.SwapEffect = D3DSWAPEFFECT_COPY_VSYNC;
            }
        }
        else
        {
            params3.FullScreen_RefreshRateInHz = 0;
            params3.SwapEffect = D3DSWAPEFFECT_COPY;
            params3.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
            LogPrintf(&g_Log, "VSync非同期可能かどうかを試みます\r\n");
        }
    }
    else
    {
        params3.BackBufferFormat = displayMode2.Format;
        params3.SwapEffect = D3DSWAPEFFECT_COPY;
        params3.Windowed = TRUE;
    }
    params3.BackBufferWidth = 640;
    params3.BackBufferHeight = 480;
    params3.EnableAutoDepthStencil = TRUE;
    params3.AutoDepthStencilFormat = D3DFMT_D16;
    params3.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
    g_App.stateFlags |= 2;
    g_App.wants60Hz = 1;
    retried = 0;
retry:
    if (g_App.flags.referenceRasterizer)
    {
        goto reference;
    }
    if (g_App.d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_GameWindow.hwnd,
                                D3DCREATE_HARDWARE_VERTEXPROCESSING, &params3, &g_App.d3dDevice) < 0)
    {
        if (retried)
        {
            LogPrintf(&g_Log, "T&L HAL は使用できないようです\r\n");
        }
        if (g_App.d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_GameWindow.hwnd,
                                    D3DCREATE_SOFTWARE_VERTEXPROCESSING, &params3, &g_App.d3dDevice) < 0)
        {
            if (retried)
            {
                LogPrintf(&g_Log, "HAL も使用できないようです\r\n");
            }
        reference:
            if (g_App.d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, g_GameWindow.hwnd,
                                        D3DCREATE_SOFTWARE_VERTEXPROCESSING, &params3, &g_App.d3dDevice) < 0)
            {
                if (g_App.presentImmediate == 0)
                {
                    LogPrintf(&g_Log, "リフレッシュレートが変更できません\r\n");
                    params3.FullScreen_RefreshRateInHz = 0;
                    g_App.wants60Hz = 0;
                    retried = 1;
                    goto retry;
                }
                if (params3.FullScreen_PresentationInterval == D3DPRESENT_INTERVAL_IMMEDIATE)
                {
                    LogPrintf(&g_Log, "非同期更新も行えません。一番汚いモードに変更します\r\n");
                    LogError(&g_Log, "*** リフレッシュレートを60Hzに変更することを推奨します ***\r\n");
                    params3.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE;
                    params3.SwapEffect = D3DSWAPEFFECT_COPY;
                    goto retry;
                }
                LogError(&g_Log, "Direct3D の初期化に失敗、これではゲームは出来ません\r\n");
                if (g_App.d3d != NULL)
                {
                    g_App.d3d->Release();
                    g_App.d3d = NULL;
                }
                return 1;
            }
            else
            {
                LogPrintf(&g_Log, "REF で動作しますが、重すぎて恐らくゲームになりません...\r\n");
                g_App.stateFlags &= ~1;
                checkFormat = false;
            }
        }
        else
        {
            LogPrintf(&g_Log, "HAL で動作します\r\n");
            g_App.stateFlags &= ~1;
        }
    }
    else
    {
        LogPrintf(&g_Log, "T&L HAL で動作しま～す\r\n");
        g_App.stateFlags |= 1;
    }
    memcpy(&g_App.presentParams, &params3, sizeof(params3));
    halfWidth = 320.0f;
    halfHeight4 = 240.0f;
    aspectXY = 640.0f / 480.0f;
    fovAngle = 0.5235988f;
    dist = halfHeight4 / tanf(fovAngle / 2.0f);
    {
        D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
        D3DXVECTOR3 lookAt(halfWidth, -halfHeight4, 0.0f);
        D3DXVECTOR3 from(halfWidth, -halfHeight4, -dist);
        D3DXMatrixLookAtLH(&g_App.viewMatrix, &from, &lookAt, &up);
        D3DXMatrixPerspectiveFovLH(&g_App.projectionMatrix, fovAngle, aspectXY, 100.0f, 10000.0f);
    }
    g_App.d3dDevice->SetTransform(D3DTS_VIEW, &g_App.viewMatrix);
    g_App.d3dDevice->SetTransform(D3DTS_PROJECTION, &g_App.projectionMatrix);
    g_App.d3dDevice->GetViewport(&g_App.viewport);
    g_App.d3dDevice->GetDeviceCaps(&g_App.caps);
    if (g_App.flags.emulateColorAdd == 0 && (g_App.caps.TextureOpCaps & D3DTEXOPCAPS_ADD) == 0)
    {
        LogPrintf(&g_Log, "D3DTEXOPCAPS_ADD をサポートしていません、色加算エミュレートモードで動作します\r\n");
        g_App.flags.emulateColorAdd = 1;
    }
    if (g_App.caps.MaxTextureWidth <= 256)
    {
        LogPrintf(&g_Log, "512 以上のテクスチャをサポートしていません。殆どの絵がボケて表示されます。\r\n");
    }
    FormatCapsReport(&g_App.caps, buffer);
    LogPrintf(&g_Log, buffer);
    if (g_App.flags.force16BitTextures == 0 && checkFormat)
    {
        if (g_App.d3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, params3.BackBufferFormat, 0,
                                         D3DRTYPE_TEXTURE, D3DFMT_A8R8G8B8) == D3D_OK)
        {
            g_App.stateFlags |= 4;
        }
        else
        {
            g_App.stateFlags &= ~4;
            g_App.flags.force16BitTextures = 1;
            LogPrintf(&g_Log, "D3DFMT_A8R8G8B8 をサポートしていません、減色モードで動作します\r\n");
        }
    }
    SetDefaultRenderStates();
    ClearViewport(0xff000000);
    g_GameWindow.isClosing = 0;
    g_App.clearedOnD3dInit = 0;
    return 0;
}

char *AppendCapsFlagText(char *name, DWORD caps, DWORD flag, char *out)
{
    out += sprintf(out, name);
    if ((caps & flag) == 0)
    {
        out += sprintf(out, "不可\r\n");
    }
    else
    {
        out += sprintf(out, "可\r\n");
    }
    return out;
}

void FormatCapsReport(D3DCAPS8 *caps, char *buffer)
{
    char *p = buffer;

    p += sprintf(p, "現在のビデオカード、及びドライバの能力詳細\r\n");
    p = AppendCapsFlagText("　走査線取得能力 : ", caps->Caps, 0x20000, p);
    p = AppendCapsFlagText("　ウィンドウモードのレンダリング : ", caps->Caps2, 0x80000, p);
    p = AppendCapsFlagText("　プレゼンテーション間隔（直接）: ", caps->PresentationIntervals, 0x80000000, p);
    p = AppendCapsFlagText("　プレゼンテーション間隔（垂直同期）: ", caps->PresentationIntervals, 0x1, p);
    p += sprintf(p, "　-- デバイス能力 ------------------------------\r\n");
    p = AppendCapsFlagText("　System -> 非ローカルVRAMブリット : ", caps->DevCaps, 0x20000, p);
    p = AppendCapsFlagText("　ハードウェア T&L : ", caps->DevCaps, 0x10000, p);
    p = AppendCapsFlagText("　非ローカルVRAMからテクスチャ取得 : ", caps->DevCaps, 0x1000, p);
    p = AppendCapsFlagText("　システムメモリからテクスチャ取得 : ", caps->DevCaps, 0x100, p);
    p = AppendCapsFlagText("　VRAM からテクスチャ取得 : ", caps->DevCaps, 0x200, p);
    p = AppendCapsFlagText("　頂点バッファにシステムメモリを使用 : ", caps->DevCaps, 0x40, p);
    p = AppendCapsFlagText("　頂点バッファにビデオメモリを使用 : ", caps->DevCaps, 0x80, p);
    p += sprintf(p, "　-- プリミティブ能力 ---------------------------\r\n");
    p = AppendCapsFlagText("　半透明処理 : ", caps->PrimitiveMiscCaps, 0x800, p);
    p = AppendCapsFlagText("　ポイントのクリッピング処理 : ", caps->PrimitiveMiscCaps, 0x100, p);
    p = AppendCapsFlagText("　プリミティブのクリッピング処理 : ", caps->PrimitiveMiscCaps, 0x200, p);
    p = AppendCapsFlagText("　法線クリップ（反時計周り） : ", caps->PrimitiveMiscCaps, 0x40, p);
    p = AppendCapsFlagText("　法線クリップ（時計周り） : ", caps->PrimitiveMiscCaps, 0x20, p);
    p = AppendCapsFlagText("　法線クリップ無し : ", caps->PrimitiveMiscCaps, 0x10, p);
    p = AppendCapsFlagText("　デプステストON/OFF切り替え : ", caps->PrimitiveMiscCaps, 0x2, p);
    p += sprintf(p, "　-- ラスタ能力 --------------------------------\r\n");
    p = AppendCapsFlagText("　異方性フィルタリング : ", caps->RasterCaps, 0x20000, p);
    p = AppendCapsFlagText("　アンチエイリアシング : ", caps->RasterCaps, 0x1000, p);
    p = AppendCapsFlagText("　ディザ処理 : ", caps->RasterCaps, 0x1, p);
    p = AppendCapsFlagText("　範囲ベースのフォグ : ", caps->RasterCaps, 0x10000, p);
    p = AppendCapsFlagText("　Zベースのフォグ : ", caps->RasterCaps, 0x200000, p);
    p = AppendCapsFlagText("　テーブルフォグ : ", caps->RasterCaps, 0x100, p);
    p = AppendCapsFlagText("　頂点フォグ : ", caps->RasterCaps, 0x80, p);
    p = AppendCapsFlagText("　デプステスト : ", caps->RasterCaps, 0x10, p);
    p += sprintf(p, "　-- シェーディング能力 -----------------------\r\n");
    p = AppendCapsFlagText("　グーローシェーディング : ", caps->ShadeCaps, 0x8, p);
    p = AppendCapsFlagText("　α成分のグーローシェーディング : ", caps->ShadeCaps, 0x4000, p);
    p = AppendCapsFlagText("　グーローシェーディングでフォグ : ", caps->ShadeCaps, 0x80000, p);
    p += sprintf(p, "　-- テクスチャ能力 ---------------------------\r\n");
    p += sprintf(p, "　最大テクスチャサイズ : (%d, %d)\r\n", caps->MaxTextureWidth, caps->MaxTextureHeight);
    p = AppendCapsFlagText("　α付きテクスチャ : ", caps->TextureCaps, 0x4, p);
    p = AppendCapsFlagText("　テクスチャトランスフォーム : ", caps->TextureCaps, 0x400, p);
    p = AppendCapsFlagText("　バイリニア補間（拡大） : ", caps->TextureFilterCaps, 0x2000000, p);
    p = AppendCapsFlagText("　バイリニア補間（縮小） : ", caps->TextureFilterCaps, 0x200, p);
    p += sprintf(p, "--------------------------------------------\r\n");
}

void SetDefaultRenderStates()
{
    float fogDist;
    float fogDensity;

    if (g_App.flags.noDepthTest == 0)
    {
        g_App.d3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
    }
    else
    {
        g_App.d3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
    }
    g_App.d3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    g_App.d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    g_App.d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    if (g_App.flags.noGouraudShading == 0)
    {
        g_App.d3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
    }
    else
    {
        g_App.d3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
    }
    g_App.d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    g_App.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    g_App.d3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
    g_App.d3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    g_App.d3dDevice->SetRenderState(D3DRS_ALPHAREF, 4);
    g_App.d3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
    if (g_App.flags.noFog == 0)
    {
        g_App.d3dDevice->SetRenderState(D3DRS_FOGENABLE, TRUE);
    }
    else
    {
        g_App.d3dDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
    }
    fogDensity = 1.0f;
    g_App.d3dDevice->SetRenderState(D3DRS_FOGDENSITY, *(DWORD *)&fogDensity);
    g_App.d3dDevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
    g_App.d3dDevice->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
    g_App.d3dDevice->SetRenderState(D3DRS_FOGCOLOR, 0xffa0a0a0);
    fogDist = 1000.0f;
    g_App.d3dDevice->SetRenderState(D3DRS_FOGSTART, *(DWORD *)&fogDist);
    fogDist = 5000.0f;
    g_App.d3dDevice->SetRenderState(D3DRS_FOGEND, *(DWORD *)&fogDist);
    // Always true: | where & was meant.
    if (g_App.caps.RasterCaps | D3DPRASTERCAPS_ANTIALIASEDGES)
    {
        g_App.d3dDevice->SetRenderState(D3DRS_EDGEANTIALIAS, FALSE);
    }
    g_App.d3dDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);
    if (g_App.flags.noColorBlending == 0)
    {
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    }
    else
    {
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    }
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    if (g_App.flags.noVertexBuffer == 0)
    {
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
    }
    else
    {
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    }
    if (g_App.flags.noColorBlending == 0)
    {
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    }
    else
    {
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    }
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    if (g_App.flags.noVertexBuffer == 0)
    {
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
    }
    else
    {
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    }
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_NONE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
    if (g_SpriteRenderer != NULL)
    {
        g_SpriteRenderer->ClearBlendMode();
        g_SpriteRenderer->ClearColorOp();
        g_SpriteRenderer->ClearVertexFormat();
        g_SpriteRenderer->ClearTexture();
        g_SpriteRenderer->ClearLastCameraSetupFlag();
    }
    g_Stage.fogColorChanged = 1;
}

int CheckSingleInstance(HINSTANCE instance)
{
    char exePath[0x105];
    char consoleTitle[0x105];
    char *extension;
    // A static here in the original: it sits among the function-local
    // statics. Keeps a second instance from starting.
    static HANDLE mutex;

    mutex = CreateMutexA(NULL, TRUE, "Touhou YouYouMu App");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        LogError(&g_Log, "二つは起動できません\r\n");
        return -1;
    }
    STARTUPINFOA startup = {sizeof(STARTUPINFOA)};
    GetModuleFileNameA(NULL, exePath, 0x105);
    GetConsoleTitleA(consoleTitle, 0x105);
    GetStartupInfoA(&startup);
    if (startup.lpTitle != NULL)
    {
        extension = strrchr(startup.lpTitle, '.');
        if (FileExists(startup.lpTitle) && extension != NULL)
        {
            if (strcmpi(extension, ".lnk") == 0)
            {
                do
                {
                    ResolveShortcut(startup.lpTitle, consoleTitle, 0x104);
                    extension = strrchr(consoleTitle, '.');
                } while (strcmpi(extension, ".lnk") == 0);
            }
            else
            {
                strcpy(consoleTitle, startup.lpTitle);
            }
            if (strcmp(exePath, consoleTitle) != 0)
            {
                g_GameWindow.forceClockPacing = 1;
            }
        }
    }
    if (mutex == NULL)
    {
        return -1;
    }
    return 0;
}

void ForceForegroundWindow(HWND hwnd)
{
    DWORD activeThread;
    DWORD thread;
    DWORD lockTime;

    activeThread = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
    thread = GetWindowThreadProcessId(hwnd, NULL);
    AttachThreadInput(thread, activeThread, TRUE);
    SystemParametersInfoA(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &lockTime, 0);
    SystemParametersInfoA(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, NULL, 0);
    SetActiveWindow(hwnd);
    SystemParametersInfoA(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, &lockTime, 0);
    AttachThreadInput(thread, activeThread, FALSE);
}

int ChecksumExecutable()
{
    char exePath[0x105];
    int sum;
    int *data;
    DWORD idx;
    int *words;

    if (GetModuleFileNameA(NULL, exePath, sizeof(exePath)) != 0)
    {
        sum = 0;
        data = words = (int *)LoadFileData(exePath, 1);
        if (words == NULL)
        {
            return -1;
        }
        for (idx = 0; idx < (DWORD)g_LastFileSize / 4 - 1; idx++, words++)
        {
            sum += *words;
        }
        DebugPrint("main sum %d\r\n", sum);
        free(data);
        g_App.exeChecksum = sum;
        g_App.exeSize = g_LastFileSize;
        return sum;
    }
    return -1;
}

int ResolveShortcut(char *path, char *out, int size)
{
    HRESULT res;
    int ok;
    IShellLinkA *shellLink;
    IPersistFile *storage;
    WCHAR *buf;
    WIN32_FIND_DATAA findData;

    if (out == NULL)
    {
        return 0;
    }
    ok = 0;
    CoInitialize(NULL);
    res = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkA, (void **)&shellLink);
    if (SUCCEEDED(res))
    {
        res = shellLink->QueryInterface(IID_IPersistFile, (void **)&storage);
        if (SUCCEEDED(res))
        {
            buf = new WCHAR[size];
            if (SUCCEEDED(res))
            {
                MultiByteToWideChar(CP_ACP, 0, path, -1, buf, size);
                res = storage->Load(buf, STGM_READ);
                if (SUCCEEDED(res))
                {
                    res = shellLink->GetPath(out, size, &findData, 0);
                    if (SUCCEEDED(res))
                    {
                        ok = 1;
                    }
                }
            }
            delete buf;
            storage->Release();
        }
        shellLink->Release();
    }
    CoUninitialize();
    return ok;
}

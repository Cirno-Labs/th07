// The file from 0x433e90: start-up, the window and the display, as in
// th06's unsorted41f230.cpp.

#include <direct.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <windows.h>
#include <winnls32.h>

#include "chain.h"
#include "midi.h"
#include "sprite.h"
#include "unsorted.h"

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
    if (Func435bd0(hInstance) == -1)
    {
        goto stop;
    }
    if (g_App.LoadConfig("th07.cfg") != 0)
    {
        goto stop;
    }
    Func435ec0();
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
    g_SpriteEngine = new SpriteEngine();
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
                g_App.unk_18c &= ~0x10;
            }
            else if (testResult == D3DERR_DEVICENOTRESET)
            {
                g_SpriteEngine->ReleaseSurfaces();
                if (g_App.d3dDevice->Reset(&g_App.presentParams) != 0)
                {
                    break;
                }
                SetDefaultRenderStates();
                g_App.redrawFrames = 3;
                g_App.unk_18c |= 0x10;
            }
        }
    }
release:
    if (g_Unknown626270.unk_9270 != 0)
    {
        Func44a302(2);
    }
    g_Chain.Release();
    while (g_SoundPlayer.Func44c9c0())
    {
    }
stop:
    g_SoundPlayer.Release();
    delete g_SpriteEngine;
    g_SpriteEngine = NULL;
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

void PresentFrame()
{
    int i;
    char path[0x100];

    if (g_App.d3dDevice->Present(NULL, NULL, NULL, NULL) < 0)
    {
        g_SpriteEngine->ReleaseSurfaces();
        g_App.d3dDevice->Reset(&g_App.presentParams);
        SetDefaultRenderStates();
        g_App.redrawFrames = 2;
    }
    g_SpriteEngine->DoPendingCopy();
    if ((g_Buttons & 0x800) && (g_Buttons & 0x800) != (g_PrevButtons & 0x800))
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
            g_App.Func4395fb(path);
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
            g_SpriteEngine->Func44f580();
            g_App.unk_2bc = 0xff;
            g_App.Func43a207();
            g_Chain.RunDrawChain();
            g_SpriteEngine->Func44f5c0();
            g_App.d3dDevice->SetTexture(0, NULL);
            g_App.d3dDevice->EndScene();
        }
        g_SpriteEngine->Func44f5c0();
        g_App.viewport.X = 0;
        g_App.viewport.Y = 0;
        g_App.viewport.Width = 640;
        g_App.viewport.Height = 480;
        g_App.d3dDevice->SetViewport(&g_App.viewport);
        res = g_Chain.RunCalcChain();
        g_SoundPlayer.Func44c9c0();
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
            if (elapsed >= 1.0 / 60.0 || g_GameWindow.unk_1c)
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
            if (delta2 >= 1000.0 / 60.0 || g_GameWindow.unk_1c)
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
    Func435e30(g_GameWindow.hwnd);
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
    if (g_SpriteEngine != NULL)
    {
        g_SpriteEngine->ClearBlendMode();
        g_SpriteEngine->ClearColorOp();
        g_SpriteEngine->ClearVertexFormat();
        g_SpriteEngine->ClearTexture();
        g_SpriteEngine->ClearUnk2e4d4();
    }
    g_Unknown1348010 = 1;
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

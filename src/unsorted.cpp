#include <math.h>
#include <stdio.h>
#include <stdarg.h>

#include "unsorted.h"

// The game's data, as the original defines it here. The DLL build uses
// the original's copies instead.
#ifdef TH07_EXE
ControllerMapping g_DefaultControllerMapping = {
    0, 1, 2, 4, -1, -1, -1, -1, 3,
};
JOYCAPSA g_JoyCaps;
WORD g_ShotButtonHeldLen;
Rng g_Rng;
#endif // TH07_EXE

WORD InitJoystick()
{
    JOYINFOEX joyInfo;

    joyInfo.dwSize = sizeof(joyInfo);
    joyInfo.dwFlags = JOY_RETURNALL;
    if (joyGetPosEx(JOYSTICKID1, &joyInfo) != JOYERR_NOERROR)
    {
        LogPrintf(&g_Log, "使えるパッドが存在しないようです、残念\r\n");
        return 1;
    }
    joyGetDevCapsA(JOYSTICKID1, &g_JoyCaps, sizeof(g_JoyCaps));
    return 0;
}

int SetButtonFromDirectInput(WORD *buttons, short button, WORD bit, BYTE *rgbButtons)
{
    if (button < 0)
    {
        return 0;
    }
    *buttons |= (rgbButtons[button] & 0x80) ? bit : 0;
    return (rgbButtons[button] & 0x80) ? bit : 0;
}

int SetButtonFromJoystick(WORD *buttons, short button, WORD bit, DWORD joyButtons)
{
    if (button < 0)
    {
        return 0;
    }
    DWORD mask = 1 << button;
    *buttons |= (joyButtons & mask) ? bit : 0;
    return (joyButtons & mask) ? bit : 0;
}

WORD ReadJoystick(WORD buttons)
{
    JOYINFOEX joyInfo;
    DWORD margin;
    int shotPressed;
    int diFire;
    HRESULT hr;
    DIJOYSTATE2 state;

    if (g_App.joystick == NULL)
    {
        memset(&joyInfo, 0, sizeof(joyInfo));
        joyInfo.dwSize = sizeof(joyInfo);
        joyInfo.dwFlags = JOY_RETURNALL;
        if (joyGetPosEx(JOYSTICKID1, &joyInfo) != JOYERR_NOERROR)
        {
            return buttons;
        }
        shotPressed = SetButtonFromJoystick(&buttons, g_App.controllerMapping.shootButton, 1, joyInfo.dwButtons);
        if (g_App.shootFocuses)
        {
            if (shotPressed)
            {
                if (g_ShotButtonHeldLen < 20)
                {
                    g_ShotButtonHeldLen++;
                }
                if (g_ShotButtonHeldLen >= 10)
                {
                    buttons |= 4;
                }
            }
            else if (g_ShotButtonHeldLen > 10)
            {
                g_ShotButtonHeldLen -= 10;
                buttons |= 4;
            }
            else
            {
                g_ShotButtonHeldLen = 0;
            }
        }
        SetButtonFromJoystick(&buttons, g_App.controllerMapping.bombButton, 2, joyInfo.dwButtons);
        SetButtonFromJoystick(&buttons, g_App.controllerMapping.focusButton, 4, joyInfo.dwButtons);
        SetButtonFromJoystick(&buttons, g_App.controllerMapping.menuButton, 8, joyInfo.dwButtons);
        SetButtonFromJoystick(&buttons, g_App.controllerMapping.upButton, 0x10, joyInfo.dwButtons);
        SetButtonFromJoystick(&buttons, g_App.controllerMapping.downButton, 0x20, joyInfo.dwButtons);
        SetButtonFromJoystick(&buttons, g_App.controllerMapping.leftButton, 0x40, joyInfo.dwButtons);
        SetButtonFromJoystick(&buttons, g_App.controllerMapping.rightButton, 0x80, joyInfo.dwButtons);
        SetButtonFromJoystick(&buttons, g_App.controllerMapping.skipButton, 0x100, joyInfo.dwButtons);
        margin = (g_JoyCaps.wXmax - g_JoyCaps.wXmin) / 2 / 2;
        buttons |= ((g_JoyCaps.wXmin + g_JoyCaps.wXmax) / 2 + margin < joyInfo.dwXpos) ? 0x80 : 0;
        buttons |= (joyInfo.dwXpos < (g_JoyCaps.wXmin + g_JoyCaps.wXmax) / 2 - margin) ? 0x40 : 0;
        margin = (g_JoyCaps.wYmax - g_JoyCaps.wYmin) / 2 / 2;
        buttons |= ((g_JoyCaps.wYmin + g_JoyCaps.wYmax) / 2 + margin < joyInfo.dwYpos) ? 0x20 : 0;
        buttons |= (joyInfo.dwYpos < (g_JoyCaps.wYmin + g_JoyCaps.wYmax) / 2 - margin) ? 0x10 : 0;
        return buttons;
    }
    else
    {
        hr = g_App.joystick->Poll();
        if (FAILED(hr))
        {
            int retries = 0;

            DebugPrint("error : DIERR_INPUTLOST\r\n");
            hr = g_App.joystick->Acquire();
            while (hr == DIERR_INPUTLOST)
            {
                hr = g_App.joystick->Acquire();
                DebugPrint("error : DIERR_INPUTLOST %d\r\n", retries);
                retries++;
                if (retries >= 400)
                {
                    return buttons;
                }
            }
            return buttons;
        }
        memset(&state, 0, sizeof(state));
        hr = g_App.joystick->GetDeviceState(sizeof(DIJOYSTATE2), &state);
        if (FAILED(hr))
        {
            return buttons;
        }
        diFire = SetButtonFromDirectInput(&buttons, g_App.controllerMapping.shootButton, 1, state.rgbButtons);
        if (g_App.shootFocuses)
        {
            if (diFire)
            {
                if (g_ShotButtonHeldLen < 20)
                {
                    g_ShotButtonHeldLen++;
                }
                if (g_ShotButtonHeldLen >= 10)
                {
                    buttons |= 4;
                }
            }
            else if (g_ShotButtonHeldLen > 10)
            {
                g_ShotButtonHeldLen -= 10;
                buttons |= 4;
            }
            else
            {
                g_ShotButtonHeldLen = 0;
            }
        }
        SetButtonFromDirectInput(&buttons, g_App.controllerMapping.bombButton, 2, state.rgbButtons);
        SetButtonFromDirectInput(&buttons, g_App.controllerMapping.focusButton, 4, state.rgbButtons);
        SetButtonFromDirectInput(&buttons, g_App.controllerMapping.menuButton, 8, state.rgbButtons);
        SetButtonFromDirectInput(&buttons, g_App.controllerMapping.upButton, 0x10, state.rgbButtons);
        SetButtonFromDirectInput(&buttons, g_App.controllerMapping.downButton, 0x20, state.rgbButtons);
        SetButtonFromDirectInput(&buttons, g_App.controllerMapping.leftButton, 0x40, state.rgbButtons);
        SetButtonFromDirectInput(&buttons, g_App.controllerMapping.rightButton, 0x80, state.rgbButtons);
        SetButtonFromDirectInput(&buttons, g_App.controllerMapping.skipButton, 0x100, state.rgbButtons);
        SetButtonFromDirectInput(&buttons, 7, 0x2000, state.rgbButtons);
        buttons |= (state.lX > g_App.padThresholdX) ? 0x80 : 0;
        buttons |= (state.lX < -g_App.padThresholdX) ? 0x40 : 0;
        buttons |= (state.lY > g_App.padThresholdY) ? 0x20 : 0;
        buttons |= (state.lY < -g_App.padThresholdY) ? 0x10 : 0;
        return buttons;
    }
}

BYTE *GetPadButtons()
{
#ifdef TH07_EXE
    static BYTE g_PadButtons[0x80];
#endif // TH07_EXE
    JOYINFOEX joyInfo;
    DWORD mask;
    DWORD button;
    HRESULT hr;
    DIJOYSTATE2 state;

    memset(g_PadButtons, 0, sizeof(g_PadButtons));
    if (g_App.joystick == NULL)
    {
        memset(&joyInfo, 0, sizeof(joyInfo));
        joyInfo.dwSize = sizeof(joyInfo);
        joyInfo.dwFlags = JOY_RETURNALL;
        if (joyGetPosEx(JOYSTICKID1, &joyInfo) != JOYERR_NOERROR)
        {
            return g_PadButtons;
        }
        mask = joyInfo.dwButtons;
        for (button = 0; button < 32; button++, mask >>= 1)
        {
            if (mask & 1)
            {
                g_PadButtons[button] = 0x80;
            }
        }
        return g_PadButtons;
    }
    else
    {
        hr = g_App.joystick->Poll();
        if (FAILED(hr))
        {
            int retries = 0;

            DebugPrint("error : DIERR_INPUTLOST\r\n");
            hr = g_App.joystick->Acquire();
            while (hr == DIERR_INPUTLOST)
            {
                hr = g_App.joystick->Acquire();
                retries++;
                if (retries >= 400)
                {
                    DebugPrint("error : DIERR_INPUTLOST %d\r\n", retries);
                    return g_PadButtons;
                }
            }
            return g_PadButtons;
        }
        g_App.joystick->GetDeviceState(sizeof(DIJOYSTATE2), &state);
        if (FAILED(hr))
        {
            return g_PadButtons;
        }
        memcpy(g_PadButtons, state.rgbButtons, sizeof(g_PadButtons));
        return g_PadButtons;
    }
}

// Sets `bit` in `buttons` while key `key` of `keyState` is down.
#define KEY(key, bit) buttons |= ((keyState[key] & 0x80) ? (bit) : 0)

WORD ReadButtons()
{
    WORD buttons = 0;
    BYTE keyState[0x100];

    if (g_App.keyboard == NULL)
    {
        GetKeyboardState(keyState);
        KEY(VK_UP, 0x10);
        KEY(VK_DOWN, 0x20);
        KEY(VK_LEFT, 0x40);
        KEY(VK_RIGHT, 0x80);
        KEY(VK_NUMPAD8, 0x10);
        KEY(VK_NUMPAD2, 0x20);
        KEY(VK_NUMPAD4, 0x40);
        KEY(VK_NUMPAD6, 0x80);
        KEY(VK_NUMPAD7, 0x50);
        KEY(VK_NUMPAD9, 0x90);
        KEY(VK_NUMPAD1, 0x60);
        KEY(VK_NUMPAD3, 0xa0);
        KEY(VK_HOME, 0x800);
        KEY('D', 0x2000);
        KEY('Z', 0x1);
        KEY('X', 0x2);
        KEY(VK_SHIFT, 0x4);
        KEY(VK_ESCAPE, 0x8);
        KEY(VK_CONTROL, 0x100);
        KEY('Q', 0x200);
        KEY('S', 0x400);
        KEY('R', 0x4000);
        KEY(VK_RETURN, 0x1000);
    }
    else
    {
        HRESULT hr = g_App.keyboard->GetDeviceState(sizeof(keyState), keyState);
        buttons = 0;
        if (hr == DIERR_INPUTLOST)
        {
            g_App.keyboard->Acquire();
            return ReadJoystick(buttons);
        }
        KEY(DIK_UP, 0x10);
        KEY(DIK_DOWN, 0x20);
        KEY(DIK_LEFT, 0x40);
        KEY(DIK_RIGHT, 0x80);
        KEY(DIK_NUMPAD8, 0x10);
        KEY(DIK_NUMPAD2, 0x20);
        KEY(DIK_NUMPAD4, 0x40);
        KEY(DIK_NUMPAD6, 0x80);
        KEY(DIK_NUMPAD7, 0x50);
        KEY(DIK_NUMPAD9, 0x90);
        KEY(DIK_NUMPAD1, 0x60);
        KEY(DIK_NUMPAD3, 0xa0);
        KEY(DIK_HOME, 0x800);
        KEY(DIK_D, 0x2000);
        KEY(DIK_Z, 0x1);
        KEY(DIK_X, 0x2);
        KEY(DIK_LSHIFT, 0x4);
        KEY(DIK_RSHIFT, 0x4);
        KEY(DIK_ESCAPE, 0x8);
        KEY(DIK_LCONTROL, 0x100);
        KEY(DIK_RCONTROL, 0x100);
        KEY(DIK_Q, 0x200);
        KEY(DIK_S, 0x400);
        KEY(DIK_RETURN, 0x1000);
        KEY(DIK_R, 0x4000);
    }
    return ReadJoystick(buttons);
}

void ResetKeyboard()
{
    BYTE keyState[0x100];
    int i;

    GetKeyboardState(keyState);
    for (i = 0; i < 0x100; i++)
    {
        keyState[i] &= 0x7f;
    }
    SetKeyboardState(keyState);
}


void *LoadFileData(char *path, int isExternal)
{
    int idx = -1;
    char *name;
    DWORD dataSize;
    void *buffer;
    HANDLE handle;

    if (isExternal == 0)
    {
        name = strrchr(path, '\\');
        if (name == NULL)
        {
            name = path;
        }
        else
        {
            name++;
        }
        name = strrchr(name, '/');
        if (name == NULL)
        {
            name = path;
        }
        else
        {
            name++;
        }
        dataSize = g_Archive.GetEntrySize(name);
        g_FileSize = dataSize;
        if (dataSize == 0)
        {
            LogError(&g_Log, "error : %s is not found in arcfile.\r\n", name);
            return NULL;
        }
        if (dataSize != 0)
        {
            DebugPrint("%s Decode ... \r\n", name);
            buffer = MemAlloc(dataSize);
            if (buffer == NULL)
            {
                return NULL;
            }
            g_Archive.Decode(name, buffer);
            return buffer;
        }
    }
    DebugPrint("%s Load ... \r\n", path);
    handle = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (handle == INVALID_HANDLE_VALUE)
    {
        DebugPrint("error : %s is not found.\r\n", path);
        return NULL;
    }
    dataSize = GetFileSize(handle, NULL);
    buffer = MemAlloc(dataSize);
    if (buffer == NULL)
    {
        CloseHandle(handle);
        return NULL;
    }
    ReadFile(handle, buffer, dataSize, &dataSize, NULL);
    g_FileSize = dataSize;
    CloseHandle(handle);
    return buffer;
}

int FileExists(char *path)
{
    HANDLE file;

    file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (file != INVALID_HANDLE_VALUE)
    {
        CloseHandle(file);
        return 1;
    }
    return 0;
}

int WriteToFile(char *path, void *data, int size)
{
    HANDLE file;
    DWORD written;

    file = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE)
    {
        DebugPrint("error : %s write error\r\n", path);
        return -1;
    }
    WriteFile(file, data, size, &written, NULL);
    if (size != written)
    {
        CloseHandle(file);
        DebugPrint("error : %s write error\r\n", path);
        return -2;
    }
    CloseHandle(file);
    DebugPrint("%s write ...\r\n", path);
    return 0;
}

int LogPrintf(Log *log, const char *fmt, ...)
{
    int len2;
    char buf[0x2000];
    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    len2 = strlen(buf);
    if (log->cursor + len2 < log->buffer + sizeof(log->buffer) - 1)
    {
        strcpy(log->cursor, buf);
        log->cursor += len2;
        *log->cursor = '\0';
    }
    va_end(args);
    return (int)fmt;
}

int LogError(Log *log, const char *fmt, ...)
{
    int len2;
    char buf[0x200];
    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    len2 = strlen(buf);
    if (log->cursor + len2 < log->buffer + sizeof(log->buffer) - 1)
    {
        strcpy(log->cursor, buf);
        log->cursor += len2;
        *log->cursor = '\0';
    }
    va_end(args);
    log->hasError = 1;
    return (int)fmt;
}

WORD Rng::GetRandomU16()
{
    WORD a;

    a = (this->seed ^ 0x9630) - 0x6553;
    this->seed = (((a & 0xc000) >> 14) + a * 4) & 0xffff;
    this->generated++;
    return this->seed;
}

DWORD Rng::GetRandomU32()
{
    return (this->GetRandomU16() << 16) | this->GetRandomU16();
}

float Rng::GetRandomF32ZeroToOne()
{
    return this->GetRandomU32() / 4294967296.0f;
}

float AddNormalizeAngle(float a, float b)
{
    int i = 0;

    a += b;
    while (a > 3.1415927f)
    {
        a -= 6.2831855f;
        if (i++ > 16)
        {
            break;
        }
    }
    while (a < -3.1415927f)
    {
        a += 6.2831855f;
        if (i++ > 16)
        {
            break;
        }
    }
    return a;
}

void Rotate2D(D3DXVECTOR3 *out, D3DXVECTOR3 *in, float angle)
{
    float s = sinf(angle);
    float c = cosf(angle);

    out->x = c * in->x + s * in->y;
    out->y = c * in->y - s * in->x;
}

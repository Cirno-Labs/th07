// Classes and functions not yet assigned to a source file. Names are
// placeholders carrying an address until behavior shows what they are.

#pragma once

#include <windows.h>
#include <mmsystem.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include "common.h"

// The application: window, Direct3D, configuration and the main loop.
// Only the fields known so far. Its start matches th06's App: the same
// fields are used at the same offsets.
// The buttons each action is mapped to, as pad button numbers; negative
// when unmapped. The same as th06's.
struct ControllerMapping
{
    short shootButton;
    short bombButton;
    short focusButton;
    short menuButton;
    short upButton;
    short downButton;
    short leftButton;
    short rightButton;
    short skipButton;
};
ASSERT_SIZE(ControllerMapping, 0x12);

class App
{
public:
    unsigned char unk_0[0x10];
    // The keyboard's DirectInput device; NULL without one.
    IDirectInputDevice8 *keyboard;
    // The first joystick found; NULL without one.
    IDirectInputDevice8 *joystick;
    unsigned char unk_18[0x2c];
    HWND hwnd;
    unsigned char unk_48[0xd0];
    ControllerMapping controllerMapping;
    unsigned char unk_12a[0x6];
    // How far a DirectInput pad axis goes before it counts as a
    // direction.
    short padThresholdX;
    short padThresholdY;
    unsigned char unk_134[0xa];
    // Whether holding shoot also focuses: ReadJoystick reports focus
    // once shoot has been held for 10 frames, and for a few frames after
    // it is let go.
    BYTE shootFocuses;
};

extern App g_App;

// Its only known caller is Func45db60.
class Unknown45e1f0
{
public:
    int GetField2c();

    char unk0[0x2c];
    int field2c;
};

// Debug output, compiled out of the release build: an empty function
// that the game still calls with messages.
void DebugPrint(const char *fmt, ...);

// Loads a whole file into a malloc'd buffer; NULL on failure. As in
// th06, where the second argument says whether the file is read from
// disk rather than from the archives.
void *LoadFileData(char *path, int isExternal);

// The object error messages go to, e.g. "error : MIDI File が読み込めない".
// Its layout is not known yet.
class Log;
extern Log g_Log;

// Logs a printf-style message to a log object.
int LogPrintf(Log *log, const char *fmt, ...);

// Ors `bit` into `buttons` while button `button` (a bit index into
// joyGetPosEx's dwButtons) is down, and returns what it ored in. A
// negative button is unmapped.
int SetButtonFromJoystick(WORD *buttons, short button, WORD bit, DWORD joyButtons);

// Like SetButtonFromJoystick, for a DirectInput state's rgbButtons:
// a button is down when its byte has bit 0x80 set.
int SetButtonFromDirectInput(WORD *buttons, short button, WORD bit, BYTE *rgbButtons);

// The joystick's capabilities, from joyGetDevCaps.
extern JOYCAPSA g_JoyCaps;
// How many frames shoot has been held, for App::shootFocuses; at most
// 20.
extern WORD g_ShotHeldFrames;

// Adds the joystick's buttons and directions to `buttons` and returns
// them: through DirectInput if there is a device, else joyGetPosEx.
WORD ReadJoystick(WORD buttons);

// The pad's buttons as GetPadButtons last read them: 0x80 when down.
extern BYTE g_PadButtons[0x80];
// Reads which pad buttons are down into g_PadButtons and returns it.
BYTE *GetPadButtons();

// Reads the keyboard and the joystick into button bits: through
// DirectInput if there is a keyboard device, else GetKeyboardState.
WORD ReadButtons();

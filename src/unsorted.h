// Classes and functions not yet assigned to a source file. Names are
// placeholders carrying an address until behavior shows what they are.

#pragma once

#include <windows.h>

// The application: window, Direct3D, configuration and the main loop.
// Only the fields known so far. Its start matches th06's App: the same
// fields are used at the same offsets.
class App
{
public:
    unsigned char unk_0[0x44];
    HWND hwnd;
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

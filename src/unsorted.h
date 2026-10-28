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

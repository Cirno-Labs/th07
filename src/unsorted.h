// Classes and functions not yet assigned to a source file. Names are
// placeholders carrying an address until behavior shows what they are.

#pragma once

#include <windows.h>
#include <mmsystem.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <dsound.h>
#include <d3dx8math.h>

#include "app.h"
#include "archive.h"
#include "common.h"
#include "resultscreen.h"
#include "timer.h"

// Scene set-up and tear-down functions App::OnUpdate calls, not named
// yet. The set-up ones return nonzero on failure.
// th06's GameManager::RegisterChain and RemoveChain.
int RegisterGameManagerChain();
void RemoveGameManagerChain();

// Measures the frame rate every half second, with the performance
// counter when there is one (calling g_App.CheckClockDrift every eighth
// time), and adds up the frames there should have been and those there
// were while bit 2 of g_GameManager.stateFlags is set and `draw` is.
// Shows the rate when `draw` is set and hideFps is not, with
// g_App.roundedFps below it while bits 2 and 3 are set.
void DrawFps(int draw);

// Sets up DirectInput as the DirectX 8 SDK's Keyboard and Joystick
// samples: the keyboard, then the first joystick found, if any. Returns
// 0, or -1 if DirectInput is off or the keyboard fails.
int InitDirectInput(App *game);

// Opens th07.dat as g_Archive and loads its version file,
// "th07_0100b.ver", into g_App.verFile. Returns 0, or -1 after logging
// an error if either is missing. Not in th06.
int OpenGameArchive();

// Measures the refresh rate by presenting surface 0, up to 1800 times
// or until it has eight rates of 57 Hz or more, each over half a second
// or so. Unless g_App.flags.noVsync is set, it turns presentImmediate on
// when their average is over 160 Hz (or there are fewer than two), and
// also when it is 65 Hz or more, returning -2 then. Otherwise returns
// 0. Not in th06.
int MeasureRefreshRate();

// IDirectInput8::EnumDevices's callback for joysticks, from the DirectX
// 8 SDK's Joystick sample: creates a device for the first joystick that
// works, unless there already is one.
BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE *pdidInstance, VOID *pContext);

// IDirectInputDevice8::EnumObjects's callback for the joystick's axes,
// from the same sample without its dialog updates: sets each axis's
// range to -1000 to 1000.
BOOL CALLBACK EnumAxesCallback(const DIDEVICEOBJECTINSTANCE *pdidoi, VOID *pContext);

inline void Timer::Tick()
{
    this->previous = this->current;
    g_App.TickTimer(&this->current, &this->subFrame);
}

// The game window (at 0x575c20), as in th06 with more fields.
struct GameWindow
{
    // Zeroes the window. Inline: the original runs it as g_GameWindow's
    // static initializer.
    GameWindow()
    {
        memset(this, 0, sizeof(GameWindow));
    }

    // Runs one frame of the main loop. Returns 0 to go on, else what
    // App::RegisterChain's result means.
    int RunFrame();

    HWND hwnd;
    // Set when the window is being closed: the main loop ends.
    int isClosing;
    WPARAM isAppActive;
    int isAppInactive;
    char curFrame;
    unsigned char pad_11[0x3];
    // QueryPerformanceFrequency's result.
    DWORD perfFrequency[2];
    // Set when the game was started from a shortcut or another path;
    // paces every frame by the clock, even ones that are early.
    BYTE forceClockPacing;
    unsigned char pad_1d[0x3];
    // The screen saver and power settings, put back on exit.
    int screenSaveActive;
    int lowPowerActive;
    int powerOffActive;
};
extern GameWindow g_GameWindow;

// Hands finished MIDI long messages back to g_App.midi, keeps the
// cursor shown only in a window or while inactive, swallows
// WM_ERASEBKGND, and has WM_CLOSE end the main loop.
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Presents the frame, resetting the device if that fails; does the
// pending texture copy; saves a screenshot to snapshot/thNNN.bmp when
// button 0x800 is pressed.
void PresentFrame();

// Creates g_App.d3d. Returns 0, or 1 on failure.
int CreateD3d();
// Creates the game window. Returns 0, or nonzero on failure.
int CreateGameWindow(HINSTANCE instance);
// Brings `hwnd` to the foreground, lifting the foreground lock timeout
// for the time. Not in th06.
void ForceForegroundWindow(HWND hwnd);
// Clears the back buffer and the depth buffer to `color` and presents,
// twice, resetting the device when a present fails.
void ClearScreen(D3DCOLOR color);
// Draws the batched quads, then clears the whole back buffer to `color`.
void ClearViewport(D3DCOLOR color);
// Fills `rect` (left, top, right, bottom) with `color`, blended by its
// alpha, after drawing the batched quads.
void DrawColorRect(float *rect, D3DCOLOR color);
// A color by its bytes.
union ColorBytes
{
    D3DCOLOR color;
    struct
    {
        BYTE b;
        BYTE g;
        BYTE r;
        BYTE a;
    };
};
// DrawColorRect with a color for each corner, blended across. Not in
// th06.
void DrawGradientRect(float *rect, D3DCOLOR topLeft, D3DCOLOR topRight, D3DCOLOR bottomLeft, D3DCOLOR bottomRight);
// Creates the Direct3D device. Returns 0, or nonzero on failure.
int InitD3dDevice();
// Prints `name` and then 可 or 不可 (yes or no), by whether `caps` has
// any of `flag`, at `out`; returns the end of the text. Not in th06.
char *AppendCapsFlagText(char *name, DWORD caps, DWORD flag, char *out);
// Writes a report of the device capabilities `caps` that matter to the
// game into `buffer`, in Japanese. Not in th06.
void FormatCapsReport(D3DCAPS8 *caps, char *buffer);
// Sets the device's render and texture stage states, as the config
// flags pick them, and forgets SpriteEngine's cached states.
void SetDefaultRenderStates();
// Creates a mutex and returns -1 if another instance holds it, as
// th06's CheckSingleInstance. Also sets g_GameWindow.forceClockPacing when the
// game was started through a shortcut or a path other than its own.
int CheckSingleInstance(HINSTANCE instance);
// Resolves the shortcut at `path` into `out`, `size` bytes long. Not in
// th06.
int ResolveShortcut(char *path, char *out, int size);
// Sums the executable's dwords. Not in th06.
int ChecksumExecutable();

class CSoundManager;
class CStreamingSound;

#include "sound.h"

// The object error messages go to, e.g. "error : MIDI File が読み込めない".
// Larger than th06's: the text buffer holds 0x2000 bytes, not 0x800.
class Log
{
public:
    // Empties the log and logs its header line. Inline: the original
    // runs it as g_Logger's static initializer.
    Log();
    ~Log()
    {
    }

    char buffer[0x2000];
    // The end of the text in `buffer`.
    char *cursor;
    // Set by LogError, the error logger.
    char hasError;

    // Writes the log to ./log.txt, if anything was logged, after a
    // closing line; shows it in a message box first if it holds an
    // error.
    void Flush();
};
extern Log g_Logger;

// Logs a printf-style message to a log object.
int LogPrintf(Log *log, const char *fmt, ...);
inline Log::Log()
{
    this->cursor = this->buffer;
    *this->cursor = '\0';
    this->hasError = 0;
    LogPrintf(this, "東方動作記録 --------------------------------------------- \r\n");
}

// Like LogPrintf, and marks the log as holding an error.
int LogError(Log *log, const char *fmt, ...);

// Ors `bit` into `buttons` while button `button` (a bit index into
// joyGetPosEx's dwButtons) is down, and returns what it ored in. A
// negative button is unmapped.
int SetButtonFromJoystick(WORD *buttons, short button, WORD bit, DWORD joyButtons);

// Like SetButtonFromJoystick, for a DirectInput state's rgbButtons:
// a button is down when its byte has bit 0x80 set.
int SetButtonFromDirectInput(WORD *buttons, short button, WORD bit, BYTE *rgbButtons);

// How many frames shoot has been held, for App::shootFocuses; at most
// 20. Named, and declared before g_JoyDeviceCaps, to come between g_JoyDeviceCaps
// and g_Rng in .bss as in the original: the exe sorts an object's
// globals by name_hash(name) % 1024 (tools/locals.py), and of two equal
// ones puts the name first seen later first.
extern WORD g_ShotHeldFrames;
// The joystick's capabilities, from joyGetDevCaps.
extern JOYCAPSA g_JoyDeviceCaps;

// Adds the joystick's buttons and directions to `buttons` and returns
// them: through DirectInput if there is a device, else joyGetPosEx.
WORD ReadJoystick(WORD buttons);

// Checks that joystick 1 is there and reads its capabilities into
// g_JoyDeviceCaps. Returns 1 when there is none.
WORD InitJoystick();

// The pad's buttons as GetPadButtons last read them: 0x80 when down.
extern BYTE g_PadButtons[0x80];
// Reads which pad buttons are down into g_PadButtons and returns it.
BYTE *GetPadButtons();

// Reads the keyboard and the joystick into button bits: through
// DirectInput if there is a keyboard device, else GetKeyboardState.
WORD ReadButtons();
// Not named yet; the title menu calls it when it comes back from
// another scene.
void DrawLoadingScreen();

// The buttons held this frame and the frame before, as bits.
extern WORD g_Buttons;
extern WORD g_PreviousButtons;
// The buttons the game plays by this frame: g_Buttons, or a replay's.
extern WORD g_PlayButtons;
// g_PlayButtons the frame before.
extern WORD g_PreviousPlayButtons;
// 1 on every 8th frame once the buttons have been held unchanged for 30
// frames, for key repeat.
extern WORD g_RepeatButtons;
// How many frames the buttons have been held unchanged, cycling from 38
// back to 30.
extern WORD g_ButtonHeldFrames;

// Whether a button in `mask` went down this frame.
#define WAS_PRESSED(mask) ((g_Buttons & (mask)) && (g_Buttons & (mask)) != (g_PreviousButtons & (mask)))
// Also while they are held, once the repeat delay has passed.
#define WAS_PRESSED_OR_HELD(mask) (WAS_PRESSED(mask) || ((g_Buttons & (mask)) && g_RepeatButtons))

// Clears the down bit (0x80) of every key in the thread's keyboard
// state.
void ResetKeyboard();

// Whether the file at `path` exists and can be opened for reading: 1 or
// 0. Not in th06.
int FileExists(char *path);

// Writes `size` bytes of `data` to the file at `path`. Returns 0, -1 if
// the file cannot be created, -2 if writing fails.
int WriteToFile(char *path, void *data, int size);

// The game's random number generator: a 16-bit state stepped with a xor,
// a subtraction and a rotation, as in th06.
class Rng
{
public:
    // Advances the generator and returns 16 random bits.
    WORD GetRandomU16();
    // `generated`, read through a temporary.
    DWORD GetGenerated()
    {
        return this->generated;
    }
    // Unlike th06's, leaves `generated` alone.
    void SetSeed(WORD seed)
    {
        this->seed = seed;
    }
    // 32 random bits, from two GetRandomU16 calls.
    DWORD GetRandomU32();
    WORD GetRandomU16InRange(WORD range)
    {
        return range != 0 ? this->GetRandomU16() % range : 0;
    }
    DWORD GetRandomU32InRange(DWORD range)
    {
        return range != 0 ? this->GetRandomU32() % range : 0;
    }
    // A random float from 0 up to 1.
    float GetRandomF32ZeroToOne();
    // A random float from 0 up to `range`.
    float GetRandomF32InRange(float range)
    {
        return this->GetRandomF32ZeroToOne() * range;
    }

    WORD seed;
    // How many times the generator was advanced.
    DWORD generated;
};
extern Rng g_Rng;

// a + b, brought back into [-pi, pi] (giving up after 17 turns).
float AddNormalizeAngle(float a, float b);

// Rotates `in` by `angle` radians in the xy plane into `out` (z is left
// alone).
void Rotate2D(D3DXVECTOR3 *out, D3DXVECTOR3 *in, float angle);
// Sets *sinOut and *cosOut to the sine and cosine of `angle`, with one
// fsincos, as th06's. The compiler keeps an out-of-line copy (0x417af0).
inline void SinCos(float *sinOut, float *cosOut, float angle)
{
    __asm
    {
        fld angle
        fsincos
        mov eax, cosOut
        fstp [eax]
        mov eax, sinOut
        fstp [eax]
    }
}

#include "gdisurface.h"

// A 2D vector with a polar setter, out of line in the item file. Its
// callers use it on fields of several objects. Named for what it is;
// th06 has no counterpart.
struct Vector2
{
    // Sets x and y to `radius` along `angle`, with one fsincos.
    void SetPolar(float angle, float radius);

    float x;
    float y;
};

#include "gamemanager.h"

#include "gui.h"

#include "enemyscript.h"

#include "stage.h"

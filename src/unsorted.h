// Classes and functions not yet assigned to a source file. Names are
// placeholders carrying an address until behavior shows what they are.

#pragma once

#include <windows.h>
#include <mmsystem.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <d3dx8math.h>

#include "common.h"
#include "resultscreen.h"

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

class MidiPlayer;

// The config's option bits, in th06's order; the ones used so far test
// the same bits as th06's.
struct GameConfigFlags
{
    unsigned int emulateColorAdd : 1;
    unsigned int noVertexBuffer : 1;
    unsigned int force16BitTextures : 1;
    unsigned int clearBackBuffer : 1;
    unsigned int noFrameItems : 1;
    unsigned int noGouraudShading : 1;
    unsigned int noDepthTest : 1;
    unsigned int force60Fps : 1;
    unsigned int noColorBlending : 1;
    unsigned int referenceRasterizer : 1;
    unsigned int noFog : 1;
    unsigned int noDirectInput : 1;
    unsigned int unk12 : 20;
};

class App
{
public:
    // Reads the config file at `path` into the config. Returns 0, or
    // nonzero to quit.
    int LoadConfig(char *path);
    // Adds the application's chain elements. Returns 0, -1 to quit, or
    // anything else to restart.
    static int RegisterChain();
    // Adds the time since the last call to the play time in
    // g_Unknown626270.plst. Not in th06.
    void Func43a27f();
    // Saves the back buffer as a bitmap at `path`. Not in th06.
    int Func4395fb(char *path);
    // Flushes the sprites and turns fog off if unk_2bc is set; returns
    // 0 when it was not. Not in th06.
    int Func43a207();

    BOOL UsesPresentImmediate()
    {
        return this->presentImmediate;
    }

    HINSTANCE hInstance;
    IDirect3D8 *d3d;
    IDirect3DDevice8 *d3dDevice;
    unsigned char unk_c[0x4];
    // The keyboard's DirectInput device; NULL without one.
    IDirectInputDevice8 *keyboard;
    // The first joystick found; NULL without one.
    IDirectInputDevice8 *joystick;
    unsigned char unk_18[0x2c];
    HWND hwnd;
    D3DXMATRIX viewMatrix;
    D3DXMATRIX projectionMatrix;
    D3DVIEWPORT8 viewport;
    D3DPRESENT_PARAMETERS presentParams;
    unsigned char unk_114[0x4];
    // The start of the config, which is 0x38 bytes long, as in th06.
    ControllerMapping controllerMapping;
    unsigned char unk_12a[0x6];
    // How far a DirectInput pad axis goes before it counts as a
    // direction.
    short padThresholdX;
    short padThresholdY;
    unsigned char unk_134[0x2];
    // 0 for a 32-bit display, 1 for 16 bits, 0xff before the first run
    // picks one.
    BYTE colorMode16Bits;
    unsigned char unk_137[0x3];
    // Played in a window rather than full screen.
    BYTE windowed;
    // How many frames to skip drawing between drawn ones.
    BYTE frameSkip;
    unsigned char unk_13c[0x2];
    // Whether holding shoot also focuses: ReadJoystick reports focus
    // once shoot has been held for 10 frames, and for a few frames after
    // it is let go.
    BYTE shootFocuses;
    unsigned char unk_13f[0xd];
    GameConfigFlags flags;
    unsigned char unk_150[0x14];
    // How many more frames to draw in full, after the device was reset.
    int redrawFrames;
    unsigned char unk_168[0x4];
    // Frames are paced by the clock rather than by the presents.
    int presentImmediate;
    // Set while the device is created asking for 60 Hz.
    int unk_170;
    // Cleared by InitD3dDevice.
    int unk_174;
    unsigned char unk_178[0x4];
    // The MIDI player; its long messages come back to WindowProc.
    MidiPlayer *midi;
    unsigned char unk_180[0xc];
    // Bit 1: the device does T&L in hardware; 2: the back buffer is
    // lockable; 4: A8R8G8B8 textures work; 0x10: the device was reset and
    // no frame has been drawn since.
    DWORD unk_18c;
    unsigned char unk_190[0x8];
    D3DCAPS8 caps;
    unsigned char unk_26c[0x50];
    // Set to 0xff before each frame is drawn; Func43a207 clears it.
    DWORD unk_2bc;
    // The sum of the executable's dwords and its size, as read at
    // startup.
    DWORD exeChecksum;
    DWORD exeSize;
};

extern App g_App;

// The game window (at 0x575c20), as in th06 with more fields.
struct GameWindow
{
    // Runs one frame of the main loop. Returns 0 to go on, else what
    // App::RegisterChain's result means.
    int RunFrame();

    HWND hwnd;
    // Set when the window is being closed: the main loop ends.
    int isClosing;
    WPARAM isAppActive;
    int isAppInactive;
    char curFrame;
    unsigned char unk_11[0x3];
    // QueryPerformanceFrequency's result.
    DWORD perfFrequency[2];
    // Set when the game was started from a shortcut or another path;
    // paces every frame by the clock, even ones that are early.
    BYTE unk_1c;
    unsigned char unk_1d[0x3];
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
void Func435e30(HWND hwnd);
// Clears the whole back buffer to `color`.
void ClearViewport(D3DCOLOR color);
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
// Set by SetDefaultRenderStates; th06 sets g_Stage.fogColorChanged
// there.
extern BYTE g_Unknown1348010;
// Creates g_AppMutex and returns -1 if another instance holds it, as
// th06's CheckSingleInstance. Also sets g_GameWindow.unk_1c when the
// game was started through a shortcut or a path other than its own.
int CheckSingleInstance(HINSTANCE instance);
// The mutex that keeps a second instance from starting.
extern HANDLE g_AppMutex;
// Resolves the shortcut at `path` into `out`, `size` bytes long. Not in
// th06.
int Func435fc0(char *path, char *out, int size);
// Sums the executable's dwords. Not in th06.
int Func435ec0();

// Plays the sound effects and music (at 0x4ba0d8). Only what is known
// so far.
class SoundPlayer
{
public:
    // Creates the DirectSound objects for `hwnd`. Returns 0, or -1 on
    // failure.
    int Initialize(HWND hwnd);
    // Releases the sounds and DirectSound.
    int Release();
    // Not in th06. WinMain calls it until it returns 0.
    int Func44c9c0();
    // Queues sound effect `idx` to start on the next update, unless
    // it is already queued or five are.
    void QueueSound(int idx, int unk);
};
extern SoundPlayer g_SoundPlayer;

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
// The size of the file LoadFileData loaded last.
extern int g_FileSize;

// The game's data archive, at 0x626258: files are looked up by name
// (without the directory). Its layout is not known yet.
class Archive
{
public:
    // The decoded size of entry `name`; 0 if there is none.
    DWORD GetEntrySize(char *name);
    // Decodes entry `name` into `buffer`, which must hold GetEntrySize
    // bytes.
    void *Decode(char *name, void *buffer);
};
extern Archive g_Archive;

// The object error messages go to, e.g. "error : MIDI File が読み込めない".
// Larger than th06's: the text buffer holds 0x2000 bytes, not 0x800.
class Log
{
public:
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
extern Log g_Log;

// Logs a printf-style message to a log object.
int LogPrintf(Log *log, const char *fmt, ...);
// Like LogPrintf, and marks the log as holding an error.
int LogError(Log *log, const char *fmt, ...);

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

// Checks that joystick 1 is there and reads its capabilities into
// g_JoyCaps. Returns 1 when there is none.
WORD InitJoystick();

// The pad's buttons as GetPadButtons last read them: 0x80 when down.
extern BYTE g_PadButtons[0x80];
// Reads which pad buttons are down into g_PadButtons and returns it.
BYTE *GetPadButtons();

// Reads the keyboard and the joystick into button bits: through
// DirectInput if there is a keyboard device, else GetKeyboardState.
WORD ReadButtons();

// The buttons held this frame and the frame before, as bits.
extern WORD g_Buttons;
extern WORD g_PrevButtons;
// 1 on every 8th frame once the buttons have been held unchanged for 30
// frames, for key repeat.
extern WORD g_ButtonRepeat;

// Whether a button in `mask` went down this frame.
#define WAS_PRESSED(mask) ((g_Buttons & (mask)) && (g_Buttons & (mask)) != (g_PrevButtons & (mask)))
// Also while they are held, once the repeat delay has passed.
#define WAS_PRESSED_OR_HELD(mask) (WAS_PRESSED(mask) || ((g_Buttons & (mask)) && g_ButtonRepeat))

// Clears the down bit (0x80) of every key in the thread's keyboard
// state.
void ResetKeyboard();

// Whether the file at `path` exists and can be opened for reading: 1 or
// 0. Not in th06.
int FileExists(char *path);

// Writes `size` bytes of `data` to the file at `path`. Returns 0, -1 if
// the file cannot be created, -2 if writing fails.
int WriteToFile(char *path, void *data, int size);

// Compresses `size` bytes at `data` into a new GlobalAlloc block, which
// it returns, and stores the compressed size in `outSize`.
void *Func45ead0(BYTE *data, int size, int *outSize);

// The game's random number generator: a 16-bit state stepped with a xor,
// a subtraction and a rotation, as in th06.
class Rng
{
public:
    // Advances the generator and returns 16 random bits.
    WORD GetRandomU16();
    // 32 random bits, from two GetRandomU16 calls.
    DWORD GetRandomU32();
    WORD GetRandomU16InRange(WORD range)
    {
        return range != 0 ? this->GetRandomU16() % range : 0;
    }
    // A random float from 0 up to 1.
    float GetRandomF32ZeroToOne();

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

// How a D3DFORMAT's pixels are laid out, for a DIB section.
struct FormatInfo
{
    // A D3DFORMAT; -1 ends the table.
    int format;
    int bitCount;
    DWORD alphaMask;
    DWORD redMask;
    DWORD greenMask;
    DWORD blueMask;
};

// A pixel in D3DFMT_A1R5G5B5.
struct A1R5G5B5
{
    WORD b : 5;
    WORD g : 5;
    WORD r : 5;
    WORD a : 1;
};

// A GDI DIB section text is drawn into before it is copied to a Direct3D
// surface; `format` is the D3DFORMAT its pixels follow. As in th06.
class GdiSurface
{
public:
    GdiSurface();
    ~GdiSurface();
    // Selects the old object back into `hdc`, then deletes the DC and the
    // bitmap. Returns whether there was a DC.
    bool Release();
    // Creates the DIB section in `format`, or failing that in a 32-bit
    // format with the same channels. Returns whether it was created.
    bool Create(int width, int height, int format);
    // Creates the DIB section and its DC, after releasing any old ones.
    bool CreateInFormat(int width, int height, int format);
    // The entry of g_FormatInfos for `format`, or the end of the table
    // when it is missing; NULL for -1.
    FormatInfo *FindFormat(int format);
    // Copies the pixels into `surface` when it has the same format.
    // Returns false when there is no bitmap or the surface can't be
    // locked.
    bool CopyToSurface(IDirect3DSurface8 *surface);
    // Flips the alpha of the pixels in rows `y` on (GDI leaves alpha at
    // 0), taking `width` and `height` in 16-bit pixels. In A1R5G5B5 it
    // also shades the colors darker towards the bottom, a quarter as
    // much when `lightShading` is set, and makes the uncovered pixels
    // black. Returns false for other formats.
    bool InvertAlpha(int x, int y, int width, int height, int lightShading);

    void *GetBits()
    {
        return this->bits;
    }
    bool IsCreated()
    {
        return this->bitmap != NULL;
    }
    int GetWidth()
    {
        return this->width;
    }
    int GetHeight()
    {
        return this->height;
    }
    int GetFormat()
    {
        return this->format;
    }

    int format;
    int width;
    int height;
    DWORD imageSize;
    int widthBytes;
    HDC hdc;
    HGDIOBJ oldObject;
    HBITMAP bitmap;
    void *bits;
};
ASSERT_SIZE(GdiSurface, 0x24);
// The formats GdiSurface supports: X8R8G8B8, A8R8G8B8, X1R5G5B5, R5G6B5,
// A1R5G5B5 and A4R4G4B4.
extern FormatInfo g_FormatInfos[];

// The 1024 by 64 surface text is drawn on with GDI before it is copied
// into a texture (640 by 64 in th06).
extern IDirect3DSurface8 *g_TextSurface;

// Creates g_TextSurface.
void CreateTextSurface();
// Releases g_TextSurface, if there is one.
void ReleaseTextSurface();

// Draws `text` at `x` in "ＭＳ ゴシック" `fontHeight` high, with a black
// outline when `shadowColor` is -1 and a wider black shadow otherwise, on
// g_TextSurface at twice the size, then scales `width` by `fontHeight` of
// it into `texture`, `unk6` rows high at row `y`.
void RenderTextToTexture(int x, int y, int width, int unk4, int fontHeight, int unk6, COLORREF textColor,
                         COLORREF shadowColor, char *text, IDirect3DTexture8 *texture);

// A 2D vector with a polar setter, out of line in the item file. Its
// callers use it on fields of several objects. Placeholder name.
struct Unknown4325c0
{
    // Sets x and y to `radius` along `angle`, with one fsincos.
    void SetPolar(float angle, float radius);

    float x;
    float y;
};

// The player's numbers, behind Unknown626270::stats. Only the fields
// known so far.
struct Unknown626270Stats
{
    unsigned char unk_0[0x7c];
    // The shot power, up to 128.
    float power;
};

// Holds the player's numbers (at 0x626270), with checksums against
// tampering. Placeholder name.
class Unknown626270
{
public:
    // Whether the checksummed fields of `stats` no longer agree.
    int Func404fe0();
    // Rerolls the checksum keys and recomputes the checksums.
    void Func4012b0();
    // Adds `power` to stats->power and recomputes the checksums; wipes
    // g_App with 0xff first if the checksums were already wrong.
    void AddPower(int power);

    unsigned char unk_0[0x8];
    Unknown626270Stats *stats;
    unsigned char unk_c[0xc];
    // The spell card chunks of the score file, by card.
    Catk catks[0x8d];
    unsigned char unk_4230[0x4218];
    // The clear chunks, by shot type.
    Clrd clrds[6];
    // The practice scores, by shot type, stage and difficulty.
    Pscr pscrs[6][6][4];
    // The "PLST" chunk. When its magic is set on exit, WinMain calls
    // Func44a302(2).
    Plst plst;
    unsigned char unk_93d0[0x224];
    // The play area: x, y, width and height. Its neighbours (a 0x200
    // byte replay file name, the stage) are th06's GameManager's, so
    // this is probably th07's GameManager.
    float arena[4];
};
extern Unknown626270 g_Unknown626270;

// Not identified yet; WinMain calls it with 2 on exit when
// g_Unknown626270.plst.magic is set.
int Func44a302(int arg);

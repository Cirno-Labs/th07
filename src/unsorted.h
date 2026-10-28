// Classes and functions not yet assigned to a source file. Names are
// placeholders carrying an address until behavior shows what they are.

#pragma once

#include <windows.h>
#include <mmsystem.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <dsound.h>
#include <d3dx8math.h>

#include "archive.h"
#include "common.h"
#include "resultscreen.h"
#include "timer.h"

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
    unsigned int unk12 : 1;
    // Each track of the wave music is read into memory before it plays,
    // rather than streamed from thbgm.dat. Not in th06.
    unsigned int preloadBgm : 1;
    unsigned int unk14 : 18;
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
    // Whether g_Unknown626270.unk_4 is set and its byte at 0x25 is not
    // 0. The result screen records no score then. Not in th06.
    int Func43a18d();
    // Saves the back buffer as a bitmap at `path`. Not in th06.
    int Func4395fb(char *path);
    // Flushes the sprites and turns fog off if unk_2bc is set; returns
    // 0 when it was not. Not in th06.
    int Func43a207();
    // Advances `frames` by one frame, or by a fraction of one into
    // `subFrame` while the game runs slow.
    void TickTimer(int *frames, float *subFrame);
    // Clears unk_2a4 and unk_2ac to unk_2b8. Called by the title menu's
    // added callback. Not in th06.
    void ResetClockDriftCounters();

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
    // 0 for no music, 1 for wave music, as th06's musicMode.
    BYTE musicMode;
    // Whether sound effects are played.
    BYTE playSounds;
    // The difficulty the difficulty select starts on: 0 to 3, or 4 and
    // 5 for Extra and Phantasm.
    BYTE defaultDifficulty;
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
    unsigned char unk_150[0x8];
    // The scene the game is in or switches to; the result screen sets it
    // to 1 to leave.
    int scene;
    unsigned char unk_15c[0x8];
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
    // Their ratio is the game's speed, which the score entry records
    // as slowdown.
    float unk_180;
    float unk_184;
    unsigned char unk_188[0x4];
    // Bit 1: the device does T&L in hardware; 2: the back buffer is
    // lockable; 4: A8R8G8B8 textures work; 8: no score is recorded;
    // 0x10: the device was reset and no frame has been drawn since.
    DWORD unk_18c;
    unsigned char unk_190[0x8];
    D3DCAPS8 caps;
    unsigned char unk_26c[0x38];
    DWORD unk_2a4;
    unsigned char unk_2a8[0x4];
    DWORD unk_2ac;
    DWORD unk_2b0;
    DWORD unk_2b4;
    DWORD unk_2b8;
    // Set to 0xff before each frame is drawn; Func43a207 clears it.
    DWORD unk_2bc;
    // The sum of the executable's dwords and its size, as read at
    // startup.
    DWORD exeChecksum;
    DWORD exeSize;
};

extern App g_App;

inline void Timer::Tick()
{
    this->previous = this->current;
    g_App.TickTimer(&this->current, &this->subFrame);
}

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
// Clears the back buffer and the depth buffer to `color` and presents,
// twice, resetting the device when a present fails.
void ClearScreen(D3DCOLOR color);
// Draws the batched quads, then clears the whole back buffer to `color`.
void ClearViewport(D3DCOLOR color);
// Fills `rect` (left, top, right, bottom) with `color`, blended by its
// alpha, after drawing the batched quads.
void DrawColorRect(float *rect, D3DCOLOR color);
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

class CSoundManager;
class CStreamingSound;

// A command PlayQueuedSounds carries out over several frames, for the
// streaming wave music. Not in th06.
struct SoundCommand
{
    // 0 ends the queue.
    int type;
    int arg;
    // How far the command has got.
    int stage;
    char path[0x100];
};
ASSERT_SIZE(SoundCommand, 0x10c);

// Plays the sound effects and music (at 0x4ba0d8). It starts as th06's;
// the wave music now streams from one file of tracks (thbgm.dat), with
// a queue of commands.
// An entry of thbgm.fmt: where a track is in thbgm.dat and its format.
// Not in th06.
struct BgmFormat
{
    // The track's file name; an empty one ends the list.
    char name[0x10];
    DWORD offset;
    DWORD size;
    // As th06's CWaveFile fields: CWaveFile::ResetFile limits the track
    // to `loopEnd` bytes and, when looping, skips its first `loopStart`.
    // SoundPlayer::PlayQueuedSounds fills the buffer with Loops() as the repeat
    // flag.
    int loopStart;
    int loopEnd;
    WAVEFORMATEX format;

    BOOL Loops()
    {
        return this->loopEnd != 0;
    }
};
ASSERT_SIZE(BgmFormat, 0x34);

class SoundPlayer
{
public:
    // Zeroes everything; no sound is loaded.
    SoundPlayer();
    // Creates the DirectSound objects for `hwnd`. Returns 0, or -1 on
    // failure.
    int Initialize(HWND hwnd);
    // Releases the sounds and DirectSound.
    int Release();
    // Stops the wave music: ends the notification thread and releases
    // the stream.
    void StopBgm();
    // The index in bgmFormat of the track named as the file name of
    // `path`, or 0 if there is none. Not in th06.
    int FindTrackIndex(char *path);
    // Loads the wave file at `path` into soundBuffers[`index`]. Returns
    // 0, also when there is no sound, or -1 on failure.
    int LoadSound(int index, char *path);
    // Loads the track list at `path` (thbgm.fmt) into bgmFormat. Returns
    // 0 or -1. Not in th06.
    int LoadBgmFormat(char *path);
    // Starts streaming the wave music from the file at `path`
    // (thbgm.dat), as bgmFormat's first track, after stopping what
    // plays, and starts the thread that streams it. Returns 0 or -1.
    int LoadBgm(char *path);
    // Switches the wave music to the track named as the file name of
    // `path`. Returns 0, or -1 when no music is loaded. Not in th06.
    int SwitchBgmTrack(char *path);
    // Reads the track named as the file name of `path` out of bgmPath
    // into preload slot `slot`, unless it is there already. Does nothing
    // unless g_App.flags.preloadBgm is set. Returns 0 or -1. Not in th06.
    int PreloadBgm(int slot, char *path);
    // Starts the wave music from preload slot `slot`, as LoadBgm does,
    // or switches the stream to its track without g_App.flags.preloadBgm.
    // Returns 0 or -1. Not in th06.
    int PlayPreloadedBgm(int slot);
    // Loads the sound effects ("data/wav/se_plst00.wav", ...).
    int LoadSounds();
    // Carries out the commands' next stages and plays the queued sound
    // effects. Returns the first command's type: WinMain calls it until
    // it returns 0. Not in th06.
    int PlayQueuedSounds();
    // Fades the wave music out over `seconds`, counted in frames at 60
    // per second. Inline in the sound file, unlike th06's.
    void FadeOutBgm(float seconds);
    // Queues sound effect `idx` to start on the next update, unless
    // it is already queued or five are.
    void QueueSound(int idx, int unk);

    IDirectSound8 *dsound;
    DWORD unk_004;
    // The loaded sound files' buffers.
    IDirectSoundBuffer *soundBuffers[0x80];
    // Each sound effect's own buffer.
    IDirectSoundBuffer *sounds[0x80];
    // Set when a sound is queued; -1 at first.
    int unk_408[0x80];
    // A silent buffer, looping from start-up.
    IDirectSoundBuffer *silence;
    // The window the sound timer posts to.
    HWND hwnd;
    // NULL until Initialize succeeds.
    CSoundManager *manager;
    // The thread streaming the wave music and its ID.
    DWORD notifyThreadId;
    HANDLE notifyThread;
    DWORD unk_61c;
    // The sound effects to play, or -1.
    int queue[5];
    // The thbgm.fmt entries of the preloaded tracks, their data, and
    // its size.
    BgmFormat *bgmEntries[0x10];
    void *bgmPreloadData[0x10];
    void *bgmPreloadCursor[0x10];
    DWORD bgmPreloadSize[0x10];
    // The preloaded track playing.
    int bgmPlaying;
    // The contents of thbgm.fmt.
    BgmFormat *bgmFormat;
    // The commands for the wave music, in order.
    SoundCommand commands[0x20];
    // The file names of the preloaded tracks.
    char bgmNames[0x10][0x100];
    // The file the wave music streams from.
    char bgmPath[0x100];
    // The wave music playing; NULL when stopped.
    CStreamingSound *bgm;
    // Signalled as the wave music plays.
    HANDLE notifyEvent;
    DWORD unk_39c4;
    // Added to every BgmFormat offset by CWaveFile::ResetFile.
    DWORD unk_39c8;
};
ASSERT_SIZE(SoundPlayer, 0x39cc);
extern SoundPlayer g_SoundPlayer;

// The thread LoadBgm starts, with the main window: streams the wave music
// each time notifyEvent is signalled, until WM_QUIT or the music is gone.
DWORD WINAPI BgmNotificationProc(LPVOID lpParameter);

// Each sound effect's sound buffer and volume, as in th06.
struct SoundInfo
{
    int buffer;
    short volume;
    short unk_6;
};
extern SoundInfo g_SoundInfos[0x26];
// The sound files LoadSounds loads, by index.
extern char *g_SoundPaths[];

// Debug output, compiled out of the release build: an empty function
// that the game still calls with messages.
void DebugPrint(const char *fmt, ...);
// Another such function.
void DebugPrint2(const char *fmt, ...);

// Loads a whole file into a malloc'd buffer; NULL on failure. As in
// th06, where the second argument says whether the file is read from
// disk rather than from the archives.
void *LoadFileData(char *path, int isExternal);
// The size of the file LoadFileData loaded last.
extern int g_FileSize;

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
    DWORD GetRandomU32InRange(DWORD range)
    {
        return range != 0 ? this->GetRandomU32() % range : 0;
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
    // The score shown, and the score.
    DWORD guiScore;
    DWORD score;
    unsigned char unk_8[0x10];
    int grazeTotal;
    int spellCardsCaptured;
    BYTE continuesUsed;
    unsigned char unk_21[0x7];
    int pointItemsTotal;
    unsigned char unk_2c[0x24];
    float deaths;
    unsigned char unk_54[0x18];
    float bombsUsed;
    unsigned char unk_70[0xc];
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
    // Whether Phantasm is open: a shot type cleared Extra (or captured
    // 60 spell cards, which marks those that cleared Extra as such).
    int Func42f94c();
    // Whether shot type `shot` (character * 2 + shot) cleared one of
    // the four main difficulties, which opens Extra for it.
    int Func42f7df(int shot);
    // Whether shot type `shot` has Phantasm open (Func42f94c for one
    // shot type).
    int Func42f853(int shot);
    // Rerolls the checksum keys and recomputes the checksums.
    void Func4012b0();
    // Adds `power` to stats->power and recomputes the checksums; wipes
    // g_App with 0xff first if the checksums were already wrong.
    void AddPower(int power);

    unsigned char unk_0[0x8];
    Unknown626270Stats *stats;
    // Set while the game is paused: the screen shake holds still. As
    // th06's GameManager::isFrozen, probably.
    char isFrozen;
    unsigned char unk_d[0x3];
    int difficulty;
    unsigned char unk_14[0x4];
    // The spell card chunks of the score file, by card.
    Catk catks[0x8d];
    // A copy of catks, put back when no score is recorded.
    Catk savedCatks[0x8d];
    // The clear chunks, by shot type.
    Clrd clrds[6];
    // The practice scores, by shot type, stage and difficulty.
    Pscr pscrs[6][6][4];
    // The "PLST" chunk. When its magic is set on exit, WinMain calls
    // ResultScreen::RegisterChain(2) to save the score file.
    Plst plst;
    unsigned char unk_93d0[0x5];
    BYTE character;
    BYTE shotType;
    unsigned char unk_93d7;
    // Bit 1: the game is a practice; 0x10: it was cleared.
    DWORD unk_93d8;
    unsigned char unk_93dc[0x20c];
    // The screen shake ends at once while this is 1 or less.
    int unk_95e8;
    int stage;
    unsigned char unk_95f0[0x4];
    // The play area: x, y, width and height. Its neighbours (a 0x200
    // byte replay file name, the stage) are th06's GameManager's, so
    // this is probably th07's GameManager.
    float arena[4];
    unsigned char unk_9604[0x24];
    // The frames played, which the result screen shows as progress.
    int progressFrames;
};
extern Unknown626270 g_Unknown626270;

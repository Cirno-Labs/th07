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
class Unknown4378b0;

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
    // Draws the items around the game every frame ("画面周りを毎回描画
    // します"). Not in th06.
    unsigned int unk12 : 1;
    // Each track of the wave music is read into memory before it plays,
    // rather than streamed from thbgm.dat. Not in th06.
    unsigned int preloadBgm : 1;
    // Not waiting for vsync ("垂直同期を取りません"): LoadConfig turns
    // presentImmediate on, and MeasureRefreshRate skips its measurement. Not in
    // th06.
    unsigned int unk14 : 1;
    unsigned int unk15 : 17;
};

class App
{
public:
    // Reads the config file at `path` into the config, resetting it to
    // the defaults if the file is missing or invalid, checks the header
    // of thbgm.dat, logs the options in effect and writes the config
    // back. Returns 0, or -1 if thbgm.dat has the wrong version or
    // writing fails.
    int LoadConfig(char *path);
    // Resets the scene and adds OnUpdate at priority 0, with its added
    // and deleted callbacks, and OnDraw at 0xf. Returns 0, -1 to quit,
    // or anything else to restart: what adding OnUpdate returned.
    static int RegisterChain();
    // The main update chain callback and its added and deleted
    // callbacks.
    static int OnUpdate(App *arg);
    static int AddedCallback(App *arg);
    static int DeletedCallback(App *arg);
    // The draw chain callback: draws the frame rate. Returns 1.
    static int OnDraw(App *arg);
    // Adds the time since the last call to the play time in
    // g_Unknown626270.plst. Not in th06.
    void AddPlayTime();
    // Unless unk_2b8 is clear, compares the time the local clock and
    // the performance counter say passed since startTime and
    // startCounter, counting in unk_2a4 to unk_2b4 how often the clock
    // ran over 2.5 times faster, then clears unk_2b8. Sets bit 0x8 of
    // unk_18c (no score is recorded) once unk_2ac reaches 40 or unk_2b4
    // 16, and clears it otherwise. Called by DrawFps. Not in th06.
    void CheckClockDrift();
    // Checks the version file against `version` ("0100b") and the
    // executable's size and checksum: finds the line starting with the
    // version and reads "%d %d" after it. Returns 0 without a version
    // file, for a "debug" version or when both numbers match, otherwise
    // -1, which makes OnUpdate quit. Not in th06.
    int CheckVersionFile(char *version, DWORD size, DWORD checksum);
    // Adds the time since the last call to the time played in
    // g_Unknown626270.plst. Not in th06.
    void AddTotalPlayTime();
    // Whether g_Unknown626270.unk_4 is set and its byte at 0x25 is not
    // 0. The result screen records no score then. Not in th06.
    int IsScoreDisabled();
    // Saves the back buffer as a 24-bit 640x480 bitmap at `path`, from an
    // X8R8G8B8 back buffer only (a 16-bit one is logged and skipped).
    // Returns 1 for any other format, otherwise 0. Not in th06.
    int SaveScreenshot(char *path);
    // Flushes the sprites and turns fog off if unk_2bc is set; returns
    // 0 when it was not. Not in th06.
    // Flushes the sprites and turns fog on unless unk_2bc is 1 already;
    // returns 0 when it was. Not in th06.
    int EnableFog();
    int DisableFog();
    // Flushes the sprites and sets render state `state` to `value`.
    // Returns what SetRenderState returned. Not in th06.
    int SetRenderState(D3DRENDERSTATETYPE state, DWORD value);
    // Advances `frames` by one frame, or by a fraction of one into
    // `subFrame` while the game runs slow.
    void TickTimer(int *frames, float *subFrame);
    // Clears unk_2a4 and unk_2ac to unk_2b8. Called by the title menu's
    // added callback. Not in th06.
    void ResetClockDriftCounters();
    // th06's StopMusic: stops the MIDI player in MIDI mode; in wave mode
    // it queues sound command 3, or 4 with g_App.flags.preloadBgm.
    // Returns -1 if the music mode is neither, otherwise 0.
    int StopMusic();
    // th06's LoadMidi: loads the MIDI file `path` into slot `slot` of
    // `midi` in MIDI mode, and returns 0. In wave mode it queues sound
    // command 1 for the .wav of the same name instead. Returns 1 outside
    // MIDI mode.
    int LoadMidi(int slot, char *path);
    // th06's PlayMidi: plays MIDI slot `slot` from the start in MIDI
    // mode. In wave mode it queues sound command 2 for the slot, after
    // command 4 with g_App.flags.preloadBgm. Returns 0.
    int PlayMidi(int slot);
    // Plays the MIDI file at `path` from the start in MIDI mode. In wave
    // mode it queues sound command 2 for the .wav of the same name.
    // Returns -1 if the music mode is neither, otherwise 0.
    int PlayMusic(char *path);
    // Fades the music out over `seconds`: in wave mode by queueing sound
    // command 5 with the whole seconds, stretched by unk_178 while the
    // game runs slow. Returns -1 if the music mode is neither wave nor
    // MIDI, otherwise 0.
    int FadeOutMusic(float seconds);

    // Whether the back buffer is cleared every frame: forced, or because
    // the items around the game are not drawn. Inline; the operands are
    // evaluated right to left.
    int ClearsBackBuffer()
    {
        return this->flags.clearBackBuffer | this->flags.noFrameItems;
    }

    BOOL UsesPresentImmediate()
    {
        return this->presentImmediate;
    }

    HINSTANCE hInstance;
    IDirect3D8 *d3d;
    IDirect3DDevice8 *d3dDevice;
    // Created by DirectInput8Create.
    IDirectInput8 *dinput;
    // The keyboard's DirectInput device; NULL without one.
    IDirectInputDevice8 *keyboard;
    // The first joystick found; NULL without one.
    IDirectInputDevice8 *joystick;
    // The joystick's capabilities.
    DIDEVCAPS joystickCaps;
    HWND hwnd;
    D3DXMATRIX viewMatrix;
    D3DXMATRIX projectionMatrix;
    D3DVIEWPORT8 viewport;
    D3DPRESENT_PARAMETERS presentParams;
    // Started by AddedCallback, stopped by DeletedCallback.
    Unknown4378b0 *timer;
    // The start of the config, which is 0x38 bytes long, as in th06.
    ControllerMapping controllerMapping;
    // 0x70002; LoadConfig resets a config with any other.
    DWORD version;
    // How far a DirectInput pad axis goes before it counts as a
    // direction.
    short padThresholdX;
    short padThresholdY;
    // The lives and bombs a game starts with.
    BYTE lifeCount;
    BYTE bombCount;
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
    // 0 to 2, 2 by default.
    BYTE unk_13c;
    // The options screen's last entry: 0 or 1.
    BYTE unk_13d;
    // Whether holding shoot also focuses: ReadJoystick reports focus
    // once shoot has been held for 10 frames, and for a few frames after
    // it is let go.
    BYTE shootFocuses;
    unsigned char unk_13f[0xd];
    GameConfigFlags flags;
    // Counts OnUpdate's calls.
    int frames;
    // The scene set up; OnUpdate switches it to `scene` when they
    // differ.
    int curScene;
    // The scene the game is in or switches to; the result screen sets it
    // to 1 to leave.
    int scene;
    // The scene the game came from.
    int prevScene;
    unsigned char unk_160[0x4];
    // How many more frames to draw in full, after the device was reset.
    int redrawFrames;
    // DrawFps shows the frame rate while it is 0; AddedCallback sets it
    // while the refresh rate is measured.
    int hideFps;
    // Frames are paced by the clock rather than by the presents.
    int presentImmediate;
    // Set while the device is created asking for 60 Hz.
    int unk_170;
    // Cleared by InitD3dDevice.
    int unk_174;
    // Set to 1 by the title menu.
    float unk_178;
    // The MIDI player; its long messages come back to WindowProc.
    MidiPlayer *midi;
    // Their ratio is the game's speed, which the score entry records
    // as slowdown.
    float unk_180;
    float unk_184;
    // The frame rate rounded, as DrawFps shows it with bit 3 of
    // g_Unknown626270.unk_93d8.
    short unk_188;
    unsigned char unk_18a[0x2];
    // Bit 1: the device does T&L in hardware; 2: the back buffer is
    // lockable; 4: A8R8G8B8 textures work; 8: no score is recorded;
    // 0x10: the device was reset and no frame has been drawn since.
    DWORD unk_18c;
    // timeGetTime() when AddPlayTime last added to the run time.
    DWORD unk_190;
    // timeGetTime() when AddTotalPlayTime last added to the play time.
    DWORD unk_194;
    D3DCAPS8 caps;
    // QueryPerformanceFrequency's result, as AddedCallback read it. Only
    // 4-aligned, so not a LARGE_INTEGER.
    DWORD perfFrequency[2];
    // The performance counter and the local time at the start of
    // CheckClockDrift's measurement, and at its end.
    DWORD startCounter[2];
    DWORD counter[2];
    SYSTEMTIME startTime;
    SYSTEMTIME time;
    // How many measurements in a row found the clock running more than
    // 2.5 times faster than the performance counter, one fewer for each
    // that did not.
    int unk_2a4;
    // DrawFps shows the rate in red when it is set.
    int unk_2a8;
    // The highest unk_2a4 so far.
    int unk_2ac;
    // The same, cleared after reaching 10, which adds one to unk_2b4.
    int unk_2b0;
    int unk_2b4;
    // Set while a measurement is pending; CheckClockDrift clears it.
    int unk_2b8;
    // Set to 0xff before each frame is drawn; DisableFog clears it.
    DWORD unk_2bc;
    // The sum of the executable's dwords and its size, as read at
    // startup.
    DWORD exeChecksum;
    DWORD exeSize;
    // The version file OpenGameArchive loads from th07.dat, and its size.
    int verFileSize;
    void *verFile;
};

extern App g_App;

// Scene set-up and tear-down functions App::OnUpdate calls, not named
// yet. The set-up ones return nonzero on failure.
// th06's GameManager::RegisterChain and RemoveChain.
int Func42f3c5();
void Func42f45d();
// th06's Ending::RegisterChain.
int Func41e820();

// Measures the frame rate every half second, with the performance
// counter when there is one (calling g_App.CheckClockDrift every eighth
// time), and adds up the frames there should have been and those there
// were while bit 2 of g_Unknown626270.unk_93d8 is set and `draw` is.
// Shows the rate when `draw` is set and hideFps is not, with
// g_App.unk_188 below it while bits 2 and 3 are set.
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
// or so. Unless g_App.flags.unk14 is set, it turns presentImmediate on
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
int ResolveShortcut(char *path, char *out, int size);
// Sums the executable's dwords. Not in th06.
int ChecksumExecutable();

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
    // Queues a command of `type` with `arg` and `path` in the first free
    // entry of `commands`. Not in th06.
    void Func44d2f0(int type, int arg, char *path);

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
// Not named yet; the title menu calls it when it comes back from
// another scene.
void Func42e1f8();

// The buttons held this frame and the frame before, as bits.
extern WORD g_Buttons;
extern WORD g_PrevButtons;
// The buttons the game plays by this frame: g_Buttons, or a replay's.
extern WORD g_Unknown4b9e50;
// 1 on every 8th frame once the buttons have been held unchanged for 30
// frames, for key repeat.
extern WORD g_ButtonRepeat;
// How many frames the buttons have been held unchanged, cycling from 38
// back to 30.
extern WORD g_ButtonFrames;

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
    unsigned char unk_8[0xc];
    // The stage's grazes, up to 9999, and the game's, up to 999999.
    int graze;
    int grazeTotal;
    int spellCardsCaptured;
    BYTE continuesUsed;
    unsigned char unk_21[0x7];
    int pointItemsTotal;
    unsigned char unk_2c[0x8];
    // Set to 6543 plus a random number under 100000 by
    // Unknown626270::RerollLifeChecksumKeys.
    DWORD unk_34[5];
    unsigned char unk_48[0x8];
    float deaths;
    unsigned char unk_54[0x8];
    // The lives left, a float behind checksums, as the other counts.
    float lives;
    unsigned char unk_60[0x8];
    // The bombs left, the same way.
    float bombs;
    float bombsUsed;
    // Set to 6543 plus a random float under 100000 by
    // Unknown626270::RerollLifeChecksumKeys.
    float unk_70[3];
    // The shot power, up to 128.
    float power;
    unsigned char unk_80[0x8];
    // Player::Graze subtracts it from Unknown626270::unk_961c.
    int unk_88;
    unsigned char unk_8c[0x3c];
};
ASSERT_SIZE(Unknown626270Stats, 0xc8);

// What Unknown626270::unk_4 points at.
struct Unknown626270Unk4
{
    unsigned char unk_0[0x25];
    // App::IsScoreDisabled tests it.
    BYTE unk_25;
    unsigned char unk_26[0x12];
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
    // Whether a shot type has Extra open (Func42f7df for any of them).
    int Func42f8de();
    // Whether shot type `shot` (character * 2 + shot) cleared one of
    // the four main difficulties, which opens Extra for it.
    int Func42f7df(int shot);
    // Whether shot type `shot` has Phantasm open (Func42f94c for one
    // shot type).
    int Func42f853(int shot);
    // Rerolls the checksum keys and recomputes the checksums.
    void Func4012b0();
    // Adds `amount` hundredths to the rank, as th06's.
    void AddSubRank(int amount);
    // Add `amount` to unk_9618 and to unk_961c, each up to a limit.
    void Func42f789(int amount);
    void Func42f736(int amount);
    // Takes `amount` from unk_961c, down to stats->unk_88.
    void SubUnk961c(int amount)
    {
        if (this->unk_961c - this->stats->unk_88 >= amount)
        {
            this->unk_961c -= amount;
        }
        else
        {
            this->unk_961c = this->stats->unk_88;
        }
    }
    // Adds `points` to stats->score, which counts in tens.
    void AddScore(int points)
    {
        this->stats->score += points / 10;
    }
    // Whether a `width` by `height` box centred on (`x`, `y`) touches the
    // play area, as th06's GameManager::IsInPlayArea.
    BOOL IsInPlayArea(float x, float y, float width, float height);
    // Adds `power` to stats->power and recomputes the checksums; wipes
    // g_App with 0xff first if the checksums were already wrong.
    void AddPower(int power);
    // Sets stats->unk_34 and unk_70 to random numbers from 6543 up.
    // Not in th06.
    void RerollLifeChecksumKeys();
    // Adds `lives` to stats->lives, checked like AddPower.
    void AddLives(int lives);
    // Adds `bombs` to stats->bombs, checked like AddPower.
    void AddBombs(int bombs);
    // Sets stats->bombs to `bombs` and updates the fields that follow
    // from it.
    void SetBombs(int bombs);
    // Takes `amount` hundredths from the rank, as th06's SubSubRank.
    void SubSubRank(int amount);
    // Adds `deaths` to stats->deaths, checked like AddPower.
    void AddDeaths(int deaths);
    // Adds `bombs` to stats->bombsUsed, checked like AddPower.
    void AddBombsUsed(int bombs);
    // Sets bit 3 (0x8) of unk_93d8 to `value`.
    void SetUnk93d8Bit3(int value)
    {
        this->unk_93d8Bit3 = value;
    }

    unsigned char unk_0[0x4];
    Unknown626270Unk4 *unk_4;
    Unknown626270Stats *stats;
    // Set while the game is paused: the screen shake holds still. As
    // th06's GameManager::isFrozen, probably.
    char isFrozen;
    // DrawFps measures nothing while it is set.
    char unk_d;
    unsigned char unk_e[0x2];
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
    unsigned char unk_93d0[0x4];
    // Cleared by Player's UpdateDyingState when the player dies.
    BYTE unk_93d4;
    BYTE character;
    BYTE shotType;
    // character * 2 + shotType, as a replay stores it.
    BYTE unk_93d7;
    // Bit 1: the game is a practice; 0x10: it was cleared.
    union
    {
        DWORD unk_93d8;
        struct
        {
            unsigned int unk_93d8Bits0 : 3;
            unsigned int unk_93d8Bit3 : 1;
        };
    };
    // Cleared by SetUpTitleScene.
    BYTE unk_93dc;
    // Set while the continue menu shows, as th06's.
    BYTE isInContinueMenu;
    // The demo replay played last, 0 to 2.
    BYTE demoIndex;
    // Set from the replay menu's last choice when a replay starts.
    BYTE unk_93df;
    DWORD unk_93e0;
    // The replay to play.
    char replayFile[0x200];
    unsigned char unk_95e4[0x4];
    // The screen shake ends at once while this is 1 or less.
    int unk_95e8;
    int stage;
    unsigned char unk_95f0[0x4];
    // The play area: x, y, width and height. Its neighbours (a 0x200
    // byte replay file name, the stage) are th06's GameManager's, so
    // this is probably th07's GameManager.
    float arena[4];
    unsigned char unk_9604[0x18];
    // Added to (up to unk_9618) by Func42f736.
    int unk_961c;
    // UpdateInvulnerability sets it while Player::playerState is 4.
    int unk_9620;
    // Whether Phantasm was open when the title menu last came up.
    int unk_9624;
    // The frames played, which the result screen shows as progress.
    int progressFrames;
    unsigned char unk_962c[0x4];
    // Set by the title menu from the play time: 3 under 7 hours, 4
    // under 14, else 5.
    int unk_9630;
};
extern Unknown626270 g_Unknown626270;

// The object at 0x49fbf0. Placeholder name; only what is known so far.
class Unknown49fbf0
{
public:
    // Whether the pointer at +8 is set and the int at its +0x1fbac is
    // not negative, or is -2.
    int Func42ad66();
    // Player::EndBombState calls it with the points it scores and 4.
    void Func427c81(int points, int unk);

    unsigned char unk_0[0x4];
    // Set to 2 when they change, as th06's GuiFlags.
    unsigned int livesChanged : 2;
    unsigned int bombsChanged : 2;
    unsigned int powerChanged : 2;
    unsigned int grazeChanged : 2;
    unsigned int unk_4_8 : 2;
};
extern Unknown49fbf0 g_Unknown49fbf0;

// Grows by 2500 plus 20 for every 1500 of Unknown626270::unk_961c above
// stats->unk_88 with each graze. Probably a field of a larger object.
extern int g_Unknown12fe0d0;
// Cleared by Player::StopBombState, next to g_Unknown12fe0d0.
extern int g_Unknown12fe0c4;
extern int g_Unknown12fe0cc;
// Player's UpdateBombState copies the first into the second when a bomb
// starts.
extern int g_Unknown12fe0c8;
extern int g_Unknown12fe0dc;

// The object at 0x1347b00. Placeholder name; only what is known so far.
class Unknown1347b00
{
public:
    // Sets the color at +0x52ac to `color`, or to the average of the two
    // while its alpha byte is set.
    void Func406930(D3DCOLOR color);
};
extern Unknown1347b00 g_Unknown1347b00;

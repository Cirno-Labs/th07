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
    // As th06's: whether color blending is off or color add emulated.
    // AsciiText::AddString only keeps a string's highlight then.
    int HasFlag0Or8()
    {
        return this->flags.noColorBlending | this->flags.emulateColorAdd;
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
    // GameManager::DeletedCallback clears it.
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
int RegisterGameManagerChain();
void RemoveGameManagerChain();

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
    void QueueSoundCommand(int type, int arg, char *path);

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
void DrawLoadingScreen();

// The buttons held this frame and the frame before, as bits.
extern WORD g_Buttons;
extern WORD g_PrevButtons;
// The buttons the game plays by this frame: g_Buttons, or a replay's.
extern WORD g_Unknown4b9e50;
// g_Unknown4b9e50 the frame before.
extern WORD g_Unknown4b9e58;
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

// The player's numbers, behind GameManager::stats. Only the fields
// known so far.
struct Unknown626270Stats
{
    // The score shown, and the score.
    DWORD guiScore;
    DWORD score;
    unsigned char unk_8[0x4];
    // The high score to beat, from the score file.
    DWORD highScore;
    // What LoadHighScores stores from the best entry; cleared in a
    // practice.
    BYTE unk_10;
    unsigned char unk_11[0x3];
    // The stage's grazes, up to 9999, and the game's, up to 999999.
    int graze;
    int grazeTotal;
    int spellCardsCaptured;
    BYTE continuesUsed;
    unsigned char unk_21[0x7];
    int pointItemsTotal;
    int unk_2c;
    int unk_30;
    // Set to 6543 plus a random number under 100000 by
    // GameManager::RerollLifeChecksumKeys.
    DWORD unk_34[7];
    float deaths;
    // Set to random floats from 6543 up by RerollStatsChecksumKeys, as unk_60,
    // unk_70 and unk_80.
    float unk_54[2];
    // The lives left, a float behind checksums, as the other counts.
    float lives;
    float unk_60[2];
    // The bombs left, the same way.
    float bombs;
    float bombsUsed;
    // Set to 6543 plus a random float under 100000 by
    // GameManager::RerollLifeChecksumKeys.
    float unk_70[3];
    // The shot power, up to 128.
    float power;
    float unk_80[2];
    // Player::Graze subtracts it from GameManager::unk_961c.
    int unk_88;
    int unk_8c[8];
    // ChecksumBytes adds unk_b4[2] to it for every byte it sums.
    int unk_ac;
    // The checksum, from ComputeChecksum.
    int unk_b0;
    DWORD unk_b4[5];
};
ASSERT_SIZE(Unknown626270Stats, 0xc8);

// What GameManager::unk_4 points at.
struct Unknown626270Unk4
{
    unsigned char unk_0[0x1c];
    // The lives a game starts with, as App's lifeCount (+0x1c of the
    // config).
    BYTE lifeCount;
    unsigned char unk_1d[0x8];
    // App::IsScoreDisabled tests it.
    BYTE unk_25;
    unsigned char unk_26[0x12];
};

// Holds the player's numbers (at 0x626270), with checksums against
// tampering. Placeholder name.
class GameManager
{
public:
    GameManager();
    // The update chain callback and its added and deleted callbacks,
    // as th06's GameManager's.
    static int OnUpdate(GameManager *mgr);
    static int AddedCallback(GameManager *mgr);
    static int DeletedCallback(GameManager *mgr);
    // The draw chain callback, as th06's: moves unk_93dc on from 1 to
    // 2. Returns 1.
    static int OnDraw(GameManager *mgr);
    // Whether the checksummed fields of `stats` no longer agree.
    int ChecksumMismatch();
    // Whether Phantasm is open: a shot type cleared Extra (or captured
    // 60 spell cards, which marks those that cleared Extra as such).
    int IsPhantasmOpen();
    // Whether a shot type has Extra open (HasExtraOpen for any of them).
    int AnyShotHasExtraOpen();
    // Whether shot type `shot` (character * 2 + shot) cleared one of
    // the four main difficulties, which opens Extra for it.
    int HasExtraOpen(int shot);
    // Whether shot type `shot` has Phantasm open (IsPhantasmOpen for one
    // shot type).
    int HasPhantasmOpen(int shot);
    // The checksum: ChecksumBytes over stats from unk_34 to unk_ac, over
    // stats->unk_b4, over what unk_4 points at and over g_App's config
    // (0x38 bytes each).
    int ComputeChecksum();
    // Sets the rank and its bounds from g_Ranks for the difficulty.
    void SetRankForDifficulty();
    // Rerolls the checksum keys and recomputes the checksums.
    void RerollChecksumKeys();
    // Adds `amount` hundredths to the rank, as th06's.
    void AddSubRank(int amount);
    // Add `amount` to unk_9618 and to unk_961c, each up to a limit.
    void IncreaseBonusCap(int amount);
    void AddBonusPointsCapped(int amount);
    // Adds `amount` to unk_961c, up to unk_9618. A gain outside
    // Player::unk_240d also adds to unk_9620 and, once that reaches
    // stats->unk_88 + 50000, caps it there and calls StartBombState. When
    // unk_961c reaches unk_9618 and changed, calls Gui's
    // Func427c81 with unk_961c less stats->unk_88 and 3.
    void AddBonusPoints(int amount);
    // The same without unk_9620.
    void AddBonusPointsNoTrack(int amount);
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
    // Sets arena to th06's play area (32, 16, 384, 448) and unk_9604
    // to (8, 16, 368, 416).
    void SetPlayArea();
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
    // Gives a life, or a bomb when the lives are full (8). Either one
    // plays sound 0x1c, adds 200 to the subrank and marks the Gui's
    // count changed. Called for the 1up items.
    void AddLifeOrBomb();
    // Sets stats->bombs to `bombs` and updates the fields that follow
    // from it.
    void SetBombs(int bombs);
    // Set stats->lives and stats->power, without the checksums.
    void SetLives(int lives)
    {
        this->stats->lives = lives;
    }
    void SetPower(int power)
    {
        this->stats->power = power;
    }
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
    // ReplayManager::RecordOnUpdate2 marks 0x100 in the frame's replay bits
    // while it is set, and clears it.
    int unk_93d0;
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
    // Cleared by SetUpTitleScene. OnDraw moves it on to 2, as th06's
    // isPaused.
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
    // A replay stage keeps it.
    WORD unk_95e4;
    unsigned char unk_95e6[0x2];
    // The screen shake ends at once while this is 1 or less. Cleared
    // by RegisterGameManagerChain, as th06's stageFrames.
    int unk_95e8;
    int stage;
    unsigned char unk_95f0[0x4];
    // The play area: x, y, width and height. Its neighbours (a 0x200
    // byte replay file name, the stage) are th06's GameManager's, so
    // this is probably th07's GameManager.
    float arena[4];
    // A second rectangle, set with arena by SetPlayArea: x, y, width
    // and height.
    float unk_9604[4];
    // RerollStatsChecksumKeys sets it to the checksum plus stats->unk_8c[3].
    float unk_9614;
    int unk_9618;
    // Added to (up to unk_9618) by AddBonusPointsCapped.
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
    // A replay stage keeps it. The rank, as th06's: SetRankForDifficulty sets it
    // and its bounds from g_Ranks.
    int unk_9634;
    int maxRank;
    int minRank;
    // Hundredths of a rank, moved by AddSubRank and SubSubRank.
    int subRank;
};
ASSERT_SIZE(GameManager, 0x9644);
extern GameManager g_Unknown626270;

// The update and draw chain elements of g_Unknown626270, after it.
extern ChainElem g_Unknown626270CalcChain;
extern ChainElem g_Unknown626270DrawChain;

// The rank a game starts at and its bounds, as th06's.
struct RankSettings
{
    int rank;
    int minRank;
    int maxRank;
};
// By difficulty.
extern RankSettings g_Ranks[6];

// The sum of the `size` bytes at `data`. Adds
// g_Unknown626270.stats->unk_b4[2] to its unk_ac for every byte.
int ChecksumBytes(BYTE *data, int size);
// Fills the checksum keys in g_Unknown626270.stats with random numbers
// from 6543 up, then stores the checksum. Not in th06.
void RerollStatsChecksumKeys();
// th06's score loading from GameManager::AddedCallback: saves the score
// file (ResultScreen::RegisterChain(2)), resets the spell cards, then
// loads the high score, the spell cards, the clears and the practice
// scores from "score.dat" (a practice takes its high score from the
// practice scores and adds one to its unk_0c) and keeps a copy of the
// spell cards in savedCatks. Returns 0, or -1 if the file cannot be
// read. Not in th06 as a function of its own.
int LoadScoreFile();
// Adds one to `*count` while it is under 999999. Its callers pass
// 999999 as `max`, which it ignores.
void IncrementStatCounter(DWORD *count, DWORD max);

// A loaded message file: a count, then the offsets of its scripts, made
// pointers once loaded, as th06's.
struct MsgFile
{
    int count;
    void *scripts[1];
};

// One instruction of a message script.
struct MsgInstr;

// The dialogue (th06's GuiMsg), GuiImpl+0x1fba4, 0xe10 bytes. Only what
// is known so far.
struct GuiMsg
{
    // The message file loaded.
    MsgFile *msgFile;
    // The next instruction of the running script.
    MsgInstr *instr;
    // The running script; negative when none runs.
    int scriptIndex;
    Timer timer;
    int unk_18;
    // The player's and the boss's portraits, and the two lines of text,
    // as th06's.
    Sprite portraits[2];
    Sprite text[2];
    Sprite unk_94c[2];
    // The text colors and their shadows, by speaker.
    D3DCOLOR textColors[4];
    D3DCOLOR shadowColors[4];
    // The text's height in pixels.
    int fontSize;
    // While it is above 0, the enemy timeline's wait for the dialogue
    // ends, as th06's.
    DWORD timelineRelease;
    // Lets the skip button fast-forward the script, as th06's.
    BYTE skippable;
    unsigned char unk_e0d[0x3];
};
ASSERT_SIZE(GuiMsg, 0xe10);

// A message Gui shows for a while at a fixed place, with a
// number, as th06's GuiPopup.
struct GuiPopup
{
    D3DXVECTOR3 pos;
    int value;
    // Nonzero while it shows.
    int shown;
    Timer timer;
};

// The bulk of Gui (th06's GuiImpl), 0x20a30 bytes. Only what
// is known so far.
struct GuiImpl
{
    Sprite sprites[0x21];
    // The boss's bar (sprites[11]): 0 hidden, 1 appearing, 2 shown, 3
    // leaving, as th06's.
    BYTE bossBarState;
    unsigned char unk_4bcd[0x3];
    Sprite unk_4bd0[5];
    Sprite bombPortrait;
    Sprite spellPortrait;
    Sprite unk_5be4;
    Sprite unk_5e30;
    Sprite unk_607c;
    Sprite unk_62c8;
    Sprite bombName;
    Sprite spellName;
    Sprite bombNameBar;
    Sprite spellNameBar;
    Sprite unk_6e44;
    Sprite unk_7090;
    Sprite unk_72dc;
    Sprite unk_7528;
    Sprite unk_7774;
    Sprite unk_79c0[0xa8];
    int unk_1fba0;
    // Starts script `index` of msg.msgFile, as th06's.
    void StartMsgScript(int index);
    // Runs the dialogue for a frame, as th06's.
    int RunMsg();

    // Draws the dialogue, as th06's.
    int DrawMsg();

    GuiMsg msg;
    // Set once the stage is cleared: OnDraw then shows the bonus,
    // stageClearTotal (in tens).
    int stageClearShown;
    int stageClearTotal;
    // When set, OnUpdate clears it and sets g_App.scene to 3.
    int unk_209bc;
    // The bonus for bullets cleared (th06's bonusPopup), a popup that
    // also shows what kind it is in `shown`, and the spell card's bonus.
    GuiPopup bonusPopup;
    GuiPopup unk_209e0;
    GuiPopup spellBonusPopup;
    unsigned char unk_20a20[0x4];
    // The stage clear bonus's point items, cherry and grazes.
    int stageClearPoints;
    int stageClearCherry;
    int stageClearGraze;
};
ASSERT_SIZE(GuiImpl, 0x20a30);

// The object at 0x49fbf0, th06's Gui. Placeholder name; only what is
// known so far.
class Gui
{
public:
    // The update element's callback: unless g_Unknown626270.isFrozen is
    // set, sets g_App.scene to 3 if impl->unk_209bc is set, runs
    // UpdateInterface and impl->RunMsg and counts unk_0 up; plays MIDI
    // slot 0 on frame 300 of stage 6, and holds g_App.redrawFrames at 8
    // or more while button 0x100 is held. Returns 1.
    static int OnUpdate(Gui *gui);
    // The draw element's callback: stops the screen shake, then draws the
    // stage clear bonus once impl->stageClearShown is set, the dialogue,
    // DrawBanners, DrawHud and the popups. Returns 1.
    static int OnDraw(Gui *gui);
    // Shows the player's bomb, spell card `name`, with sprite `sprite`
    // in bombPortrait (script 0x4a1), scripts 0x4a4 and 0x4a6 with
    // sprite 0x4ac, and bombName (script 0x704); sends bombNameBar
    // interrupt 1, plays sound 14 and sets g_App.redrawFrames to 2.
    void ShowBombName(int sprite, char *name);
    // Shows a boss's spell card, `text`: unless `sprite` is negative,
    // sprite `sprite` + 0x4ad in spellPortrait (script 0x4a3), moved left
    // by 288 or 112 when it is wider than 256 or 128; scripts 0x4a5 and
    // 0x4a7 with sprite 0x4ac, and spellName (script 0x705); sends
    // spellNameBar and unk_7774 interrupt 1, plays sound 14 and sets
    // g_App.redrawFrames to 2.
    void ShowSpellCardName(int sprite, char *text);
    // Loads the message file at `path` into impl->msg.msgFile, turning
    // its offsets into pointers. Returns 0, or -1 after logging
    // "メッセージファイル %s が読み込めませんでした".
    int LoadMsg(char *path);
    // Frees impl->msg.msgFile, if there is one.
    void FreeMsgFile();
    // Runs impl->StartMsgScript(`index`). th06 also set
    // g_App.redrawFrames.
    void StartMsg(int index);
    // Whether impl is set, a dialogue script runs and
    // impl->msg.timelineRelease is 0.
    BOOL MsgHoldsTimeline();
    // As th06's DrawBanners and DrawHud.
    void DrawBanners();
    void DrawHud();
    // The update: while no dialogue runs, the boss bar (sprites[11],
    // interrupt 1 to show it, 2 to hide it, its flag 13 when done) and
    // its life; then the interface's sprites, unk_6e44 and unk_72dc
    // (hidden when done), the unk_79c0 sprites while impl->unk_1fba0
    // is set (counting those still running into it), the popups, and
    // once the stage is cleared, the bonus (stage, grazes, point items,
    // cherry and, on the last stages, lives and bombs, by difficulty
    // and starting lives), added to the score in ten parts.
    void UpdateInterface();
    // Loads the interface's sprites and the stage's portraits and
    // messages, as th06's.
    int LoadResources();
    // The update element's added callback: runs LoadResources.
    static int AddedCallback(Gui *gui);
    // Frees ANM slots 0x18 and 0x1c to 0x1f and the message file and,
    // unless the scene is 3, 0xb or 0xc, slots 0x15 to 0x17 and 0x19 to
    // 0x1b and impl. Returns 0.
    static int DeletedCallback(Gui *gui);
    // Unless the scene is 3, 0xb or 0xc, zeroes the interface and
    // allocates impl; then adds the update element (priority 13) and the
    // draw element (priority 12). Returns 0, or -1 if adding fails.
    static int RegisterChain();
    // Removes the update element and the draw element.
    static void RemoveChain();
    // Sets bombName's pendingInterrupt to 1 and bombNameBar's to 2, as
    // the bomb ends. th06 interrupted bombName only.
    void EndBombName();
    // Sets spellName's pendingInterrupt to 1 and spellNameBar's and
    // unk_7774's to 2, as the spell card ends.
    void EndSpellCardName();
    // Hides unk_6e44, unk_7090 and unk_72dc (spriteIndex -1) and clears
    // impl->unk_1fba0. Not in th06.
    void HideInterface();
    // Whether impl->unk_6e44 shows a sprite with flag bit 13 set, as
    // th06's IsLoadingShown.
    BOOL IsLoadingShown();

    // Whether impl is set and impl->msg.scriptIndex is not negative, or
    // is -2. th06's IsMsgRunning.
    int Func42ad66();
    // Returns impl->msg.skippable, as th06's IsMsgSkippable.
    int Func427be2();
    // Shows `bonus` in bonusPopup, at (416, 48), and sets
    // g_App.redrawFrames to 2.
    void ShowBonusPopup(int bonus);
    // th06's ShowFullPowerPopup: shows `points` in impl->unk_209e0, at
    // (416, 168), with `unk` as its shown value, and sets
    // g_App.redrawFrames to 2. Player::EndBombState calls it with the
    // points it scores and 4.
    void Func427c81(int points, int unk);
    // Shows `value` in spellBonusPopup, at (224, 16), and sets
    // g_App.redrawFrames to 2.
    void ShowSpellCardBonus(int value);
    // Sets bossTimer, as th06's.
    void SetBossTimer(int seconds)
    {
        this->bossTimer = seconds;
    }

    // Counts OnUpdate's frames.
    int unk_0;
    // Set to 2 when they change, as th06's GuiFlags.
    unsigned int livesChanged : 2;
    unsigned int bombsChanged : 2;
    unsigned int powerChanged : 2;
    unsigned int grazeChanged : 2;
    unsigned int unk_4_8 : 2;
    GuiImpl *impl;
    // The width of the text ShowBombName shows, from its length.
    float bombNameWidth;
    // The same for ShowSpellCardName.
    float spellNameWidth;
    // The boss bar's alpha, raised by 4 a frame up to 0xff while a boss
    // fight runs.
    DWORD bossBarAlpha;
    unsigned char unk_18[0x4];
    // The seconds left on the boss's timer.
    int bossTimer;
    unsigned char unk_20[0x4];
    // Set while a boss fight runs.
    BYTE bossPresent;
    unsigned char unk_25[0x3];
    // The boss's life as a fraction of its maximum, and what the bar
    // shows, following it.
    float bossHealth;
    float bossHealthShown;
    unsigned char unk_30[0x64];
};
ASSERT_SIZE(Gui, 0x94);
extern Gui g_Unknown49fbf0;
extern ChainElem g_Unknown49fbf0CalcChain;
extern ChainElem g_Unknown49fbf0DrawChain;
// Copies the rectangle of sprite `sprite` in texture 0x16 into sprite
// 0x609's in texture 0x15 (SpriteEngine::CopyTextureRect). Not in th06.
void CopySpriteToTexture(int sprite);

// Grows by 2500 plus 20 for every 1500 of GameManager::unk_961c above
// stats->unk_88 with each graze. Probably a field of a larger object.
extern int g_Unknown12fe0d0;
// Cleared by Player::StopBombState, next to g_Unknown12fe0d0.
extern int g_Unknown12fe0c4;
extern int g_Unknown12fe0cc;
// Player's UpdateBombState copies the first into the second when a bomb
// starts.
extern int g_Unknown12fe0c8;
extern int g_Unknown12fe0dc;
// Set as a spell card starts: the card's number, and its bonus over
// ten plus its seconds.
extern int g_Unknown12fe0d8;
extern int g_Unknown12fe0d4;
// The bonus of each spell card, by number.
extern int g_SpellCardBonuses[];

struct Enemy;
struct EnemyScriptState;

// One argument of a script instruction.
union EnemyScriptArg
{
    int i;
    float f;
};

// One instruction of an enemy script, th06's with a mask of the
// arguments that are variables.
struct EnemyScriptInstr
{
    // The frame it runs on.
    int time;
    short opcode;
    // The size of the instruction, arguments included.
    short size;
    BYTE unk_8;
    BYTE unk_9;
    // Bit n: argument n names a variable.
    WORD paramMask;
    EnemyScriptArg args[1];
};

// The head of an enemy script file: the subroutine count, then the
// offsets of sixteen timelines, then those of the subroutines.
struct EnemyScriptHeader
{
    short subCount;
    short unk_2;
    void *timelines[16];
};

// The object at 0x1347938, where th06 has its EnemyScripts.
// Placeholder name; only what is known so far.
class EnemyScripts
{
public:
    // Loads the script file at `path` and turns its offsets into
    // pointers, as th06's Load. Returns 0, or -1 if loading fails.
    int Load(char *path);
    // Starts subroutine `sub` in `ctx`, from its first instruction with
    // both timers at 0, as th06's StartSub. Returns 0.
    int StartSub(EnemyScriptState *ctx, short sub);
    // Runs `enemy`'s script for a frame, as th06's RunEnemy: -1 once
    // it ends.
    int RunEnemy(Enemy *enemy);
    // Frees the script file, as th06's Release.
    void Release();

    EnemyScriptHeader *file;
    void **subs;
    unsigned char unk_8[0x160];
    // The scripts' shared variables, 10037 to 10040 and 10041 to 10044.
    int globalInts[4];
    float globalFloats[4];
};
extern EnemyScripts g_Unknown1347938;

// The value of script operand `id` for `enemy`: a variable (floats
// truncated) or another value the scripts read by number from 10000;
// any other number is its own value.
int GetIntOperand(Enemy *enemy, int id);
// The int variable script operand `operand` names, as th06's
// ResolveOperand, or `operand` itself if it names none or if `bit` is
// not negative and clear in `mask`.
int *ResolveOperand(Enemy *enemy, int *operand, WORD mask, int bit);
// GetIntOperand for float operands: the value of script operand `id`
// for `enemy` as a float, with random floats for 10055, 10056 and
// 10060.
float GetFloatOperand(Enemy *enemy, float id);
// ResolveOperand for float variables.
float *ResolveFloatOperand(Enemy *enemy, float *operand, WORD mask, int bit);
// Starts a timed move at an angle (argument 2, normalized), as th06's
// ScriptMoveAngle: moveDelta is argument 3 times argument 0, the
// duration; argument 1 picks the easing. Mirrored with flags.mirrored.
void ScriptMoveAngle(Enemy *enemy, EnemyScriptInstr *instr);
// Starts a timed move to arguments 2 to 4, as th06's ScriptMovePos,
// with ScriptMoveAngle's duration, easing and mirroring; stops the
// velocity.
void ScriptMovePos(Enemy *enemy, EnemyScriptInstr *instr);
// Sets the float variable argument 8 names to argument 4 eased
// linearly towards argument 5 by `t`, 0 to 1.
void ScriptMoveLinear(Enemy *enemy, EnemyScriptInstr *instr, float t);
// ScriptMoveLinear along a cubic Hermite curve from argument 4 to argument
// 5, with tangents arguments 6 and 7.
void ScriptMoveHermite(Enemy *enemy, EnemyScriptInstr *instr, float t);
// Starts a spell card, as th06's RunEnemy did: shows the name
// (arguments 1 on, xor 0xaa) with sprite argument 0, clears the
// bullets, starts the stage's spell card background, sets the bonus
// from g_SpellCardBonuses[index] (the high word of argument 0), resets
// the rank values, and starts effect 0x19 in reserved slot 1 as
// effects[24], with its scale and then 0.125 twice in unk_218 and
// timerCallbackTime in its unk_84[4] timer. Unless bit 3 of
// g_Unknown626270.unk_93d8 is set, counts the attempt in the card's
// Catk (by unk_93d7 and in all), first clearing its counts if their
// checksum (unk_2a, with the name's) fails.
void StartSpellCard(Enemy *enemy, EnemyScriptInstr *instr);
// Ends the spell card, as th06's RunEnemy did: hides the name; if it
// was active (1), clears the bullets and the other enemies for bonuses
// and, if it was captured, scores its bonus and, unless bit 3 of
// g_Unknown626270.unk_93d8 is set, keeps the best bonus and counts the
// capture in its Catk as StartSpellCard does the attempt. Then releases
// the bosses' effects[24] and plays sound 0xf. Always resets the
// stage's spellCardState.
void EndSpellCard(Enemy *enemy, EnemyScriptInstr *instr);

// The functions of the table at 0x49f158, th06's g_EnemyScriptFuncs,
// in its order.

// Puts `enemy` where boss argument 1 is, with its velocity and angle,
// and sets flags.unk_18.
void WarpToBoss(Enemy *enemy, EnemyScriptInstr *instr);
// Shakes and flashes the screen, and starts the flying bullets with
// sprites that no call has turned yet (only color 8 for argument 1 = 1,
// color 4 for 2) turning by a small random angle a frame, its sign by
// color, slowed to 0.3.
void ShakeAndSpinBullets(Enemy *enemy, EnemyScriptInstr *instr);
// Each flying color 2 bullet with a sprite within a range of the enemy
// (128, 192, 256 or 999 by argument 1; 0 also shakes and flashes the
// screen) fires two color 6 bullets and is removed (FreeBullet).
void SpawnFansFromNearbyBullets(Enemy *enemy, EnemyScriptInstr *instr);
// Does nothing.
void NoOp(Enemy *enemy, EnemyScriptInstr *instr);
// Finds the first flying bullet with a sprite at least 60 high, keeps
// its position in floats[0] and [1] (floats[0] is -999 if none),
// starts effect 2 there and removes the bullet.
void ReplaceTallBulletWithEffect(Enemy *enemy, EnemyScriptInstr *instr);
// Copies boss 0's position into moveStart, and its unk_2b6c and
// unk_2b60; then switches on its ints2[0] with no cases.
void CopyBossMoveStart(Enemy *enemy, EnemyScriptInstr *instr);
// Each flying bullet with a sprite of color 6, 15 or 2 (argument 1 = 0,
// 1 or 2) fires three fans back the way it came and is removed.
void FireBackwardFans(Enemy *enemy, EnemyScriptInstr *instr);
// For the even or odd lasers by callbackTimer, those not ending: the
// flying bullets with sprites inside one count unk_c08 down; at 0 they
// slow by 0.1 (above 0.5), turn square to the laser on the side their
// velocity points to, take typeSprites[5]'s sprites, and wait 10 more.
void UpdateLaserEdgeBullets(Enemy *enemy, EnemyScriptInstr *instr);
// Shakes the screen for 0x50 frames and starts the type 30 effects
// falling (StartPetalsFalling).
void ShakeAndStartFallingPetals(Enemy *enemy, EnemyScriptInstr *instr);
// Slows the game to 1 / argument 1 (App::unk_178), sends interrupt 2 to
// the stage's first two unk_528 sprites, and scales the velocity of
// every bullet by it, keeping each sprite in baseSpriteIndex and
// showing sprites 0x260 to 0x26f as 0x26f.
void StartSlowdown(Enemy *enemy, EnemyScriptInstr *instr);
// Undoes StartSlowdown: scales the bullets' velocities back up, restores
// sprites 0x260 to 0x26f from baseSpriteIndex, sets bit 5 of
// App::unk_18c if 1 / argument 1 is below 1, and puts the game speed
// and the stage sprites' interrupts back to 1.
void EndSlowdown(Enemy *enemy, EnemyScriptInstr *instr);
// Flashes the screen 0xd0cfcfff for argument 1 frames.
void FlashScreen(Enemy *enemy, EnemyScriptInstr *instr);
// Fades the music out over 3 seconds.
void ScriptFadeOutMusic(Enemy *enemy, EnemyScriptInstr *instr);
// Plays music slot 2 (App::PlayMidi) and, if that returns nonzero,
// bgm/th07_13b.mid.
void ScriptPlayMusic(Enemy *enemy, EnemyScriptInstr *instr);
// Whether `point`, turned about `origin` by the angle with sine
// `sinAngle` and cosine `cosAngle`, lies in the `size` box centred on
// `center` (x and y only).
int PointInRotatedBox(D3DXVECTOR3 *point, D3DXVECTOR3 *center, D3DXVECTOR3 *size, D3DXVECTOR3 *origin,
               float sinAngle, float cosAngle);

// Set to 9 as the dialogue of stages 6 to 8 starts.
extern int g_Unknown9a9ab8;

// A run of quads in the stage data, ended by one with a negative type,
// as th06's.
struct StageQuad
{
    short type;
    // The offset of the next quad.
    short size;
    short script;
    // Set by Load: the quad's Sprite in Stage::sprites.
    short spriteIndex;
    D3DXVECTOR3 pos;
    float width;
    float height;
};

// An instruction of the stage script: at `frame`, `opcode` acts on
// `args`. As th06's, with more opcodes.
struct StageInstr
{
    int frame;
    short opcode;
    short unk_06;
    D3DXVECTOR3 args;
};

// D3DRS_FOGSTART, D3DRS_FOGEND and D3DRS_FOGCOLOR, as th06's.
struct StageFog
{
    float nearPlane;
    float farPlane;
    D3DCOLOR color;
};

// An object of the stage data: its quads start at +0x1c. As th06's.
struct StageObject
{
    BYTE unk_00[0x2];
    // The background layer it is drawn on.
    char layer;
    // Bit 0: its sprites still run scripts. Bit 1: DrawLayer drew it.
    char flags;
    D3DXVECTOR3 pos;
    D3DXVECTOR3 size;
    StageQuad quads[1];
};

// A placement of an object; a negative `id` ends the list.
struct StageInstance
{
    short id;
    short unk_02;
    D3DXVECTOR3 pos;
};

// The start of the stage data file, as th06's.
struct StageData
{
    short objectCount;
    short quadCount;
    int instancesOffset;
    int scriptOffset;
    DWORD unk_0c;
    char name[0x80];
    char songNames[4][0x80];
    char songPaths[4][0x80];
    // Then, at 0x490, the objects' offsets.
};

// A camera of the stage script: where it is, where it looks (relative to
// `eye`) and its field of view. Not in th06.
struct StageCamera
{
    D3DXVECTOR3 eye;
    D3DXVECTOR3 at;
    D3DXVECTOR3 up;
    // `at`, normalized.
    D3DXVECTOR3 dir;
    D3DXVECTOR3 unk_30;
    float fov;
};

// The cubic Hermite curve from p0 to p1, with tangents m0 and m1, at `t`
// (0 to 1). Not in th06.
float HermiteInterpolate(float p0, float p1, float m0, float m1, float t);

class Stage;
// Moves `out` from `from` to `to` over stage->unk_5238[i] frames, on
// stage->unk_5248[i], eased by stage->unk_5278[i]: 1 to 3 slow down, 4
// to 6 speed up, 7 follows a Hermite curve with tangents m0 and m1.
// Clears the frame count once the timer reaches it. Not in th06.
void UpdateEasedVector(Stage *stage, int i, D3DXVECTOR3 *out, D3DXVECTOR3 *from, D3DXVECTOR3 *to, D3DXVECTOR3 *m0, D3DXVECTOR3 *m1);

// The object at 0x1347b00: th06's Stage, the background. Placeholder
// name.
class Stage
{
public:
    // Zeroes the object, then puts the camera at (0, 0, 1000) looking
    // nowhere, with up along y and a field of view of pi / 6, and copies
    // it to unk_50f8 and unk_5138.
    Stage();

    // Zeroes the stage, remembers `stage` and adds the update element
    // (priority 7) and the draw elements (priorities 3 and 4), as th06's.
    // Returns 0, or -1 if adding fails.
    static int RegisterChain(int stage);
    // Removes the update element and the two draw elements.
    static void RemoveChain();
    // Runs the stage script up to the current frame, as th06's, with
    // th07's opcodes: 3 waits for unk_50f4 (which first restarts the
    // script after the opcode 31 with that argument), 4 jumps, 5 to 28
    // set and fade the cameras' eye, at, up and field of view (fade
    // modes as UpdateEasedVector's), 29 and 30 start unk_4's and unk_250's
    // scripts. Then fades the cameras and the fog and runs the sprites.
    // Returns 1, or 4 when ChecksumMismatch finds the player's numbers
    // tampered with (checked every 500 frames).
    static int OnUpdate(Stage *stage);
    // Resets the SpriteEngine's render state, clears the play area to
    // black once if unk_51c is set, makes the SpriteEngine modulate by
    // unk_52ac if its alpha is set (and resets it to gray), draws unk_4
    // and unk_250, clears the viewport to unk_4d8, sets the fog (its
    // color modulated as the SpriteEngine's) and draws layers 0 and 1
    // under the same conditions as OnDraw2. Returns 1.
    static int OnDraw(Stage *stage);
    // Draws layers 2 and 3 unless spellCardState is above 1 or the Gui's
    // loading sprite shows, with a rectangle fading in while
    // spellCardState is 1, as th06's; then unk_528's first unk_520
    // sprites, the fixed camera (SetupFixedCamera) and the fog at 1000 to
    // 2000. Resets the SpriteEngine's color unless unk_52b0 was set,
    // and clears it. Returns 1.
    static int OnDraw2(Stage *stage);
    // Resets the script, loads the stage's background ANMs (five for
    // stage 4) and its data from g_StageDataFiles, and sets the fog and
    // the cameras up as the constructor. Returns 0, or -1 if loading
    // fails.
    static int AddedCallback(Stage *stage);
    // Loads the stage data from `dataPath` and starts the background
    // sprites' scripts, as th06's (the ANMs are loaded by AddedCallback
    // here). Returns 0, or -1 on failure.
    int Load(char *dataPath);
    // Draws the objects on `layer` near enough the camera, as th06's
    // DrawLayer with a different test. Returns 0.
    int DrawLayer(int layer);
    // Runs one frame of the scripts of each animated object's sprites,
    // as th06's. Returns 0.
    int StepObjects();
    // Releases the background ANMs (slots 5 to 9) and frees the sprites
    // and the data. Returns 0.
    static int DeletedCallback(Stage *stage);
    // Sets the color at +0x52ac to `color`, or to the average of the two
    // while its alpha byte is set.
    void SetTintColor(D3DCOLOR color);
    // One of the two set-ups SetRenderStateForSprite switches between
    // on flag bit 14, before it sets the viewport. This one is th06's
    // SetupFixedCamera without the camera globals: it looks at the
    // viewport's centre along z, with y down, a field of view of pi / 10
    // and planes at 1 and 10000.
    void SetupFixedCamera();
    // The other: looks from camera.eye along camera.at, with a field of
    // view of camera.fov and planes at 30 and 1800, then sets
    // camera.unk_30 to the unit vector across at and up.
    void SetupCamera();

    // One Sprite per object sprite in the data, malloc'd.
    Sprite *sprites;
    Sprite unk_4;
    Sprite unk_250;
    StageData *data;
    int quadCount;
    int objectCount;
    StageObject **objects;
    StageInstance *instances;
    StageInstr *script;
    Timer scriptTime;
    int scriptIndex;
    int unk_4c4;
    // RegisterChain's argument.
    int stage;
    D3DXVECTOR3 cameraPos;
    D3DCOLOR unk_4d8;
    StageFog fog;
    StageFog fogStart;
    StageFog fogTarget;
    int fogFadeFrames;
    Timer fogFadeTime;
    BYTE unk_510;
    unsigned char unk_511[0x3];
    int spellCardState;
    int spellCardFrames;
    int unk_51c;
    // Set as the dialogue of stages 6 to 8 starts.
    int unk_520;
    int unk_524;
    Sprite unk_528[0x20];
    Sprite unk_4ea8;
    int unk_50f4;
    StageCamera unk_50f8;
    StageCamera unk_5138;
    StageCamera unk_5178;
    StageCamera unk_51b8;
    StageCamera camera;
    int unk_5238[4];
    Timer unk_5248[4];
    int unk_5278[4];
    D3DXVECTOR3 nextKeyPos;
    int nextKeyFrame;
    D3DXVECTOR3 keyPos;
    int keyFrame;
    BYTE unk_52a8;
    unsigned char unk_52a9[0x3];
    ColorBytes unk_52ac;
    int unk_52b0;
};
ASSERT_SIZE(Stage, 0x52b4);
extern Stage g_Unknown1347b00;
// The stage data file of each stage; stage 0 has a "dummy" one.
extern char *g_StageDataFiles[];
// Stage's statically allocated chain elements.
extern ChainElem g_Unknown1347b00CalcChain;
extern ChainElem g_Unknown1347b00DrawChain;
extern ChainElem g_Unknown1347b00DrawChain2;

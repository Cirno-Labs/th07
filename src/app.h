// The application: window, Direct3D, configuration and the main loop,
// apart from the rest of unsorted.h so that a file can define functions
// using App ahead of sprite.h's inline functions (ascii.cpp's
// Timer::NextTick).

#pragma once

#include <windows.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <d3d8.h>
#include <d3dx8math.h>

#include "common.h"

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
class MidiTimer;

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
    unsigned int redrawHudEveryFrame : 1;
    // Each track of the wave music is read into memory before it plays,
    // rather than streamed from thbgm.dat. Not in th06.
    unsigned int preloadBgm : 1;
    // Not waiting for vsync ("垂直同期を取りません"): LoadConfig turns
    // presentImmediate on, and MeasureRefreshRate skips its measurement. Not in
    // th06.
    unsigned int noVsync : 1;
    unsigned int unusedBits : 17;
};

// The application: window, Direct3D, configuration and the main loop.
// Only the fields known so far. Its start matches th06's App: the same
// fields are used at the same offsets.
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
    // g_GameManager.plst. Not in th06.
    void AddPlayTime();
    // Unless clockMeasurementPending is clear, compares the time the local clock and
    // the performance counter say passed since startTime and
    // startCounter, counting in fastClockStreak to fastClockCycles how often the clock
    // ran over 2.5 times faster, then clears clockMeasurementPending. Sets bit 0x8 of
    // stateFlags (no score is recorded) once maxFastClockStreak reaches 40 or fastClockCycles
    // 16, and clears it otherwise. Called by DrawFps. Not in th06.
    void CheckClockDrift();
    // Checks the version file against `version` ("0100b") and the
    // executable's size and checksum: finds the line starting with the
    // version and reads "%d %d" after it. Returns 0 without a version
    // file, for a "debug" version or when both numbers match, otherwise
    // -1, which makes OnUpdate quit. Not in th06.
    int CheckVersionFile(char *version, DWORD size, DWORD checksum);
    // Adds the time since the last call to the time played in
    // g_GameManager.plst. Not in th06.
    void AddTotalPlayTime();
    // Whether g_GameManager.savedConfig is set and its byte at 0x25 is not
    // 0. The result screen records no score then. Not in th06.
    int IsScoreDisabled();
    // Saves the back buffer as a 24-bit 640x480 bitmap at `path`, from an
    // X8R8G8B8 back buffer only (a 16-bit one is logged and skipped).
    // Returns 1 for any other format, otherwise 0. Not in th06.
    int SaveScreenshot(char *path);
    // Flushes the sprites and turns fog off if fogState is set; returns
    // 0 when it was not. Not in th06.
    // Flushes the sprites and turns fog on unless fogState is 1 already;
    // returns 0 when it was. Not in th06.
    int EnableFog();
    int DisableFog();
    // Flushes the sprites and sets render state `state` to `value`.
    // Returns what SetRenderState returned. Not in th06.
    int SetRenderState(D3DRENDERSTATETYPE state, DWORD value);
    // Advances `frames` by one frame, or by a fraction of one into
    // `subFrame` while the game runs slow.
    void TickTimer(int *frames, float *subFrame);
    // Clears fastClockStreak and maxFastClockStreak to clockMeasurementPending. Called by the title menu's
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
    // command 5 with the whole seconds, stretched by gameSpeed while the
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
    MidiTimer *timer;
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
    BYTE effectsQuality;
    // The options screen's last entry: 0 or 1.
    BYTE optionsMenuCursor;
    // Whether holding shoot also focuses: ReadJoystick reports focus
    // once shoot has been held for 10 frames, and for a few frames after
    // it is let go.
    BYTE shootFocuses;
    unsigned char pad_13f[0xd];
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
    unsigned char pad_160[0x4];
    // How many more frames to draw in full, after the device was reset.
    int redrawFrames;
    // DrawFps shows the frame rate while it is 0; AddedCallback sets it
    // while the refresh rate is measured.
    int hideFps;
    // Frames are paced by the clock rather than by the presents.
    int presentImmediate;
    // Set while the device is created asking for 60 Hz.
    int wants60Hz;
    // Cleared by InitD3dDevice.
    int clearedOnD3dInit;
    // Set to 1 by the title menu.
    float gameSpeed;
    // The MIDI player; its long messages come back to WindowProc.
    MidiPlayer *midi;
    // Their ratio is the game's speed, which the score entry records
    // as slowdown.
    float achievedFrames;
    float expectedFrames;
    // The frame rate rounded, as DrawFps shows it with bit 3 of
    // g_GameManager.stateFlags.
    short roundedFps;
    unsigned char pad_18a[0x2];
    // Bit 1: the device does T&L in hardware; 2: the back buffer is
    // lockable; 4: A8R8G8B8 textures work; 8: no score is recorded;
    // 0x10: the device was reset and no frame has been drawn since.
    DWORD stateFlags;
    // timeGetTime() when AddPlayTime last added to the run time.
    DWORD lastPlayTimeTick;
    // timeGetTime() when AddTotalPlayTime last added to the play time.
    // GameManager::DeletedCallback clears it.
    DWORD lastTotalPlayTimeTick;
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
    int fastClockStreak;
    // DrawFps shows the rate in red when it is set.
    int fpsWarning;
    // The highest fastClockStreak so far.
    int maxFastClockStreak;
    // The same, cleared after reaching 10, which adds one to fastClockCycles.
    int fastClockCycleCount;
    int fastClockCycles;
    // Set while a measurement is pending; CheckClockDrift clears it.
    int clockMeasurementPending;
    // Set to 0xff before each frame is drawn; DisableFog clears it.
    DWORD fogState;
    // The sum of the executable's dwords and its size, as read at
    // startup.
    DWORD exeChecksum;
    DWORD exeSize;
    // The version file OpenGameArchive loads from th07.dat, and its size.
    int verFileSize;
    void *verFile;
};

extern App g_App;

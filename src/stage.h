// The stage background: its geometry, scripts and camera, as th06's.
// Included by unsorted.h where these declarations were; it relies on what
// unsorted.h declares before it.

#pragma once

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
    short pad_06;
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
    BYTE pad_00[0x2];
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
    short pad_02;
    D3DXVECTOR3 pos;
};

// The start of the stage data file, as th06's.
struct StageData
{
    short objectCount;
    short quadCount;
    int instancesOffset;
    int scriptOffset;
    DWORD pad_0c;
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
    // ProjectSprite projects a sprite's position plus this vector to
    // get the size it draws the sprite at.
    D3DXVECTOR3 cameraRight;
    float fov;
};

// The cubic Hermite curve from p0 to p1, with tangents m0 and m1, at `t`
// (0 to 1). Not in th06.
float HermiteInterpolate(float p0, float p1, float m0, float m1, float t);

class Stage;
// Moves `out` from `from` to `to` over stage->cameraFadeFrames[i] frames, on
// stage->cameraFadeTimer[i], eased by stage->cameraFadeMode[i]: 1 to 3 slow down, 4
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
    // it to cameraFrom and cameraTo.
    Stage();

    // Zeroes the stage, remembers `stage` and adds the update element
    // (priority 7) and the draw elements (priorities 3 and 4), as th06's.
    // Returns 0, or -1 if adding fails.
    static int RegisterChain(int stage);
    // Removes the update element and the two draw elements.
    static void RemoveChain();
    // Runs the stage script up to the current frame, as th06's, with
    // th07's opcodes: 3 waits for waitLabel (which first restarts the
    // script after the opcode 31 with that argument), 4 jumps, 5 to 28
    // set and fade the cameras' eye, at, up and field of view (fade
    // modes as UpdateEasedVector's), 29 and 30 start backgroundLayer0's and backgroundLayer1's
    // scripts. Then fades the cameras and the fog and runs the sprites.
    // Returns 1, or 4 when ChecksumMismatch finds the player's numbers
    // tampered with (checked every 500 frames).
    static int OnUpdate(Stage *stage);
    // Resets the SpriteEngine's render state, clears the play area to
    // black once if clearScreenOnce is set, makes the SpriteEngine modulate by
    // tintColor if its alpha is set (and resets it to gray), draws backgroundLayer0
    // and backgroundLayer1, clears the viewport to clearColor, sets the fog (its
    // color modulated as the SpriteEngine's) and draws layers 0 and 1
    // under the same conditions as OnDraw2. Returns 1.
    static int OnDraw(Stage *stage);
    // Draws layers 2 and 3 unless spellCardState is above 1 or the Gui's
    // loading sprite shows, with a rectangle fading in while
    // spellCardState is 1, as th06's; then dialogueSprites's first dialogueSpriteCount
    // sprites, the fixed camera (SetupFixedCamera) and the fog at 1000 to
    // 2000. Resets the SpriteEngine's color unless keepTintColor was set,
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
    // camera.cameraRight to the unit vector across at and up.
    void SetupCamera();

    // One Sprite per object sprite in the data, malloc'd.
    Sprite *sprites;
    Sprite backgroundLayer0;
    Sprite backgroundLayer1;
    StageData *data;
    int quadCount;
    int objectCount;
    StageObject **objects;
    StageInstance *instances;
    StageInstr *script;
    Timer scriptTime;
    int scriptIndex;
    int checksumCheckFrames;
    // RegisterChain's argument.
    int stage;
    D3DXVECTOR3 cameraPos;
    D3DCOLOR clearColor;
    StageFog fog;
    StageFog fogStart;
    StageFog fogTarget;
    int fogFadeFrames;
    Timer fogFadeTime;
    // Set by SetDefaultRenderStates, as th06's.
    BYTE fogColorChanged;
    unsigned char pad_511[0x3];
    int spellCardState;
    int spellCardFrames;
    int clearScreenOnce;
    // Set to 1 or 2 by stage in EffectManager::AddedCallback, which
    // clears dialogueScriptOffset, and again as the dialogue of stages 6
    // to 8 starts.
    int dialogueSpriteCount;
    int dialogueScriptOffset;
    Sprite dialogueSprites[0x20];
    Sprite pad_4ea8;
    int waitLabel;
    StageCamera cameraFrom;
    StageCamera cameraTo;
    StageCamera cameraFadeTarget;
    StageCamera cameraFadeStart;
    StageCamera camera;
    int cameraFadeFrames[4];
    Timer cameraFadeTimer[4];
    int cameraFadeMode[4];
    D3DXVECTOR3 nextKeyPos;
    int nextKeyFrame;
    D3DXVECTOR3 keyPos;
    int keyFrame;
    BYTE cameraOffsetChanged;
    unsigned char pad_52a9[0x3];
    ColorBytes tintColor;
    int keepTintColor;
};
ASSERT_SIZE(Stage, 0x52b4);
extern Stage g_StageManager;
// The stage data file of each stage; stage 0 has a "dummy" one.
extern char *g_StageDataFiles[];
// Stage's statically allocated chain elements.
extern ChainElem g_StageBgCalcElem;
extern ChainElem g_StageDrawElem;
extern ChainElem g_StageDrawElem2;

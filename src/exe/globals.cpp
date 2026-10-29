// The game's global objects. Built into the standalone exe only: the DLL
// build uses the original's, which its own static initializers construct.
//
// The objects with constructors are defined in the order the original
// constructs them (its initializers run file by file in link order, at
// 0x48c600), so constructors and the destructors registered with atexit
// run in the same order. The original also constructs a few globals that
// no game code names (four ChainElems, a 0xc-byte struct set to -999 at
// 0x4bdac8, an array of 20 DatArchives at 0x4b9e68 and a pointer at
// 0x626268); they are left out. The plain globals follow, by original
// address.

#include "archive.h"
#include "ascii.h"
#include "bullet.h"
#include "chain.h"
#include "effect.h"
#include "enemy.h"
#include "item.h"
#include "lzss.h"
#include "midi.h"
#include "player.h"
#include "replay.h"
#include "sprite.h"
#include "unsorted.h"

// ascii.cpp
AsciiText g_AsciiText;
ChainElem g_AsciiCalcChain;
ChainElem g_AsciiDrawChain;
ChainElem g_AsciiDrawChain2;

// stage.cpp
Stage g_Stage;
ChainElem g_StageCalcChain;
ChainElem g_StageDrawChain;
ChainElem g_StageDrawChain2;

// enemyscript.cpp
EnemyScripts g_EnemyScripts;

// effect.cpp
EffectManager g_EffectManager;
ChainElem g_EffectCalcChain;
ChainElem g_EffectDrawChain;

// enemy.cpp
ChainElem g_EnemyManagerCalcChain;
ChainElem g_EnemyManagerDrawChain;
ChainElem g_EnemyManagerDrawChain2;
EnemyManager g_EnemyManager;

// bullet.cpp
ChainElem g_BulletCalcChain;
ChainElem g_BulletDrawChain;
BulletManager g_BulletManager;

// gui.cpp
ChainElem g_GuiCalcChain;
ChainElem g_GuiDrawChain;

// gamemanager.cpp
GameManager g_GameManager;
ChainElem g_GameManagerCalcChain;
ChainElem g_GameManagerDrawChain;

// chain.cpp, the archive
Chain g_Chain;
DatArchive g_Archive;

// The log. The original's Log has an inline constructor: it empties the
// buffer and logs this header line first.
Log g_Log;

static int InitLog()
{
    g_Log.cursor = g_Log.buffer;
    *g_Log.cursor = '\0';
    g_Log.hasError = 0;
    LogPrintf(&g_Log, "東方動作記録 --------------------------------------------- \r\n");
    return 0;
}
static int s_LogInitialized = InitLog();

// item.cpp
ItemManager g_ItemManager;

// The main window and the game object (zeroed by inline constructors in
// the original, as static storage is anyway).
GameWindow g_GameWindow;
App g_App;

// player.cpp, sound.cpp
Player g_Player;
SoundPlayer g_SoundPlayer;

// spriteengine.cpp (empty inline constructors in the original)
VertexTex1Xyzrwh g_Quad[4];
VertexTex1DiffuseXyzrwh g_DiffuseQuad[4];
VertexTex1DiffuseXyz g_DiffuseQuadXyz[4];

// Plain globals, by original address.
Gui g_Gui;
JOYCAPSA g_JoyCaps;
WORD g_ShotHeldFrames;
Rng g_Rng;
IDirect3DSurface8 *g_TextSurface;
LzssNode g_LzssTree[0x2001];
BYTE g_LzssWindow[0x2000];
SpriteEngine *g_SpriteEngine;
ReplayManager *g_ReplayManager;
WORD g_Buttons;
WORD g_GameButtons;
WORD g_PrevButtons;
WORD g_PrevGameButtons;
WORD g_ButtonRepeat;
WORD g_ButtonFrames;
int g_FileSize;
int g_ClearedBulletItem;
int g_SpellCardCapture;
int g_SpellCardActive;
int g_SpellCardBonus;
int g_SpellCardGrazeBonus;
int g_SpellCardBonusStep;
int g_SpellCardNumber;
int g_SpellCardActiveAtBomb;
BYTE g_FogColorChanged;
int g_EffectStyle;
int g_EffectManagerInitFlag;
D3DXVECTOR3 g_CameraEye;
D3DXVECTOR3 g_CameraAt;
D3DXVECTOR3 g_CameraDir;
D3DXVECTOR3 g_CameraSizeOffset;
HANDLE g_AppMutex;
LARGE_INTEGER g_MidiTimerLastCounter;
BYTE g_PadButtons[0x80];

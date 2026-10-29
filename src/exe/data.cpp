// The game's initialized data: tables of strings, script numbers, colors,
// shot patterns, effects and sounds. Built into the standalone exe only:
// the DLL build uses the original's copies, which the original code
// shares. Values are the 1.00b exe's initial bytes.

#include <d3d8.h>

#include "archive.h"
#include "bullet.h"
#include "effect.h"
#include "ending.h"
#include "enemy.h"
#include "item.h"
#include "mainmenu.h"
#include "player.h"
#include "replay.h"
#include "resultscreen.h"
#include "sprite.h"
#include "unsorted.h"

float g_Half = 0.5f;

// .data in the original.
char *g_FileModes[2] = {
    "r",
    "w",
};
char *g_DemoReplayPaths[3] = {
    "data/demo/demorpy0.rpy",
    "data/demo/demorpy1.rpy",
    "data/demo/demorpy2.rpy",
};
SoundInfo g_SoundInfos[0x26] = {
    {0, -2000, 0},
    {0, -2500, 0},
    {1, -1200, 5},
    {1, -1500, 5},
    {2, -1000, 100},
    {3, -400, 100},
    {4, -400, 100},
    {5, -1500, 50},
    {6, -1700, 50},
    {7, -1900, 50},
    {8, -1000, 100},
    {9, -1000, 100},
    {10, -1700, 10},
    {11, -1200, 10},
    {12, -900, 100},
    {5, -1500, 50},
    {13, -900, 50},
    {14, -900, 50},
    {15, -900, 100},
    {16, -200, 100},
    {17, -1400, 0},
    {18, -1300, 0},
    {5, -100, 20},
    {6, -1800, 20},
    {7, -1800, 20},
    {19, -800, 50},
    {20, -1000, 50},
    {21, -1300, 50},
    {22, -300, 140},
    {23, -900, 100},
    {24, -900, 20},
    {25, -500, 90},
    {26, -300, 100},
    {27, -300, 100},
    {24, -300, 20},
    {19, 0, 50},
    {28, -300, 100},
    {29, -300, 100},
};
char *g_SoundPaths[30] = {
    "data/wav/se_plst00.wav",
    "data/wav/se_enep00.wav",
    "data/wav/se_pldead00.wav",
    "data/wav/se_power0.wav",
    "data/wav/se_power1.wav",
    "data/wav/se_tan00.wav",
    "data/wav/se_tan01.wav",
    "data/wav/se_tan02.wav",
    "data/wav/se_ok00.wav",
    "data/wav/se_cancel00.wav",
    "data/wav/se_select00.wav",
    "data/wav/se_gun00.wav",
    "data/wav/se_cat00.wav",
    "data/wav/se_lazer00.wav",
    "data/wav/se_lazer01.wav",
    "data/wav/se_enep01.wav",
    "data/wav/se_nep00.wav",
    "data/wav/se_damage00.wav",
    "data/wav/se_item00.wav",
    "data/wav/se_kira00.wav",
    "data/wav/se_kira01.wav",
    "data/wav/se_kira02.wav",
    "data/wav/se_extend.wav",
    "data/wav/se_timeout.wav",
    "data/wav/se_graze.wav",
    "data/wav/se_powerup.wav",
    "data/wav/se_border.wav",
    "data/wav/se_bonus.wav",
    "data/wav/se_bonus2.wav",
    "data/wav/se_pause.wav",
};
char *g_NameAlphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ.,:;_@abcdefghijklmnopqrstuvwxyz+-/*=%0123456789#!?'\"$(){}[]<>&\\|~^ --";
char *g_ShotNames[7] = {
    "博麗 霊夢 (霊)　",
    "博麗 霊夢 (夢)　",
    "霧雨 魔理沙 (魔)",
    "霧雨 魔理沙 (恋)",
    "十六夜 咲夜 (幻)",
    "十六夜 咲夜 (時)",
    "全主人公合計  　",
};
BombData g_Bombs[6] = {
    {UpdateReimuABomb, DrawReimuABomb, UpdateReimuABombFocused, DrawReimuABombFocused},
    {UpdateReimuBBomb, DrawReimuBBomb, UpdateReimuBBombFocused, DrawReimuBBombFocused},
    {UpdateMarisaABomb, DrawMarisaABomb, UpdateMarisaABombFocused, DrawMarisaABombFocused},
    {UpdateMarisaBBomb, DrawMarisaBBomb, UpdateMarisaBBombFocused, DrawMarisaBBombFocused},
    {UpdateSakuyaABomb, DrawSakuyaABomb, UpdateSakuyaABombFocused, DrawSakuyaABombFocused},
    {UpdateSakuyaBBomb, DrawSakuyaBBomb, UpdateSakuyaBBombFocused, DrawSakuyaBBombFocused},
};
ShotFireFunc g_ShotFireFuncs[6] = {
    NULL,
    TryFireShot,
    FireLaser,
    FireTrailLaser,
    FireHomingShot,
    FireAimedShot,
};
ShotUpdateFunc g_ShotUpdateFuncs[6] = {
    NULL,
    UpdateHomingOrb,
    UpdateHomingOrbFast,
    UpdateAcceleratingBullet,
    UpdateLaserBullet,
    UpdateTrailLaserBullet,
};
ShotUpdateFunc g_ShotDrawFuncs[2] = {
    NULL,
    DrawTrailLaserBullet,
};
ShotHitFunc g_ShotHitFuncs[4] = {
    NULL,
    UpdateBombShot,
    BurstHitEffect,
    (ShotHitFunc)1,
};
int g_PowerItemBonus[31] = {
    10, 20, 30, 40, 50, 60, 70, 80,
    90, 100, 200, 300, 400, 500, 600, 700,
    800, 900, 1000, 2000, 3000, 4000, 5000, 6000,
    7000, 8000, 9000, 10000, 11000, 12000, 51200,
};
int g_PowerLevels[9] = {
    8, 16, 32, 48, 64, 80, 96, 128,
    999,
};
FormatInfo g_FormatInfos[7] = {
    {22, 32, 0x00000000, 0x00ff0000, 0x0000ff00, 0x000000ff},
    {21, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff},
    {24, 16, 0x00000000, 0x00007c00, 0x000003e0, 0x0000001f},
    {23, 16, 0x00000000, 0x0000f800, 0x000007e0, 0x0000001f},
    {25, 16, 0x00008000, 0x00007c00, 0x000003e0, 0x0000001f},
    {26, 16, 0x0000f000, 0x00000f00, 0x000000f0, 0x0000000f},
    {-1, 0, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
};
ControllerMapping g_DefaultControllerMapping = {
    0, 1, 2, 4, -1, -1, -1, -1, 3,
};
// The colors g_EffectColors points at until BulletManager::RegisterChain
// switches it to g_EffectColorsFull.
DWORD g_EffectColorsSolid[0x1c] = {
    0xff000000, 0xff303030, 0xff606060, 0xff500000,
    0xff900000, 0xffff2020, 0xff400040, 0xff800080,
    0xffff30ff, 0xff000050, 0xff000090, 0xff2020ff,
    0xff203060, 0xff304090, 0xff3080ff, 0xff005000,
    0xff009000, 0xff20ff20, 0xff206000, 0xff409010,
    0xff80ff20, 0xff505000, 0xff909000, 0xffffff20,
    0xff603000, 0xff904010, 0xfff08020, 0xffffffff,
};
DWORD g_EffectColorsFull[0x1c] = {
    0xfff0f0f0, 0xfff0f0f0, 0xffffffff, 0xffffe0e0,
    0xffffe0e0, 0xffffe0e0, 0xffffe0ff, 0xffffe0ff,
    0xffffe0ff, 0xffe0e0ff, 0xffe0e0ff, 0xffe0e0ff,
    0xffe0ffff, 0xffe0ffff, 0xffe0ffff, 0xffe0ffe0,
    0xffe0ffe0, 0xffe0ffe0, 0xffe0ffe0, 0xffe0ffe0,
    0xffe0ffe0, 0xffffffe0, 0xffffffe0, 0xffffffe0,
    0xffffe0e0, 0xffffe0e0, 0xffffe0e0, 0xffffffff,
};
DWORD *g_EffectColors = g_EffectColorsSolid;
int g_BulletColorOffsets16[16] = {
    0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 0,
};
int g_BulletColorOffsets32[8] = {
    0, 1, 1, 2, 2, 3, 4, 0,
};
BYTE g_EnemyItemDrops[0x20] = {
    0, 0, 1, 0, 1, 0, 0, 7, 1, 1, 0, 0, 7, 1, 1, 0,
    1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 7, 1, 1, 1, 0, 2,
};
EffectInfo g_EffectInfos[34] = {
    {0x000002ab, NULL, NULL},
    {0x000002ac, NULL, NULL},
    {0x000002ad, NULL, NULL},
    {0x000002ae, EffectUpdateMove, EffectInitDriftFast},
    {0x000002b3, EffectUpdateMove, EffectInitDrift},
    {0x000002b4, EffectUpdateMove, EffectInitDrift},
    {0x000002b5, EffectUpdateMove, EffectInitDrift},
    {0x000002b6, EffectUpdateMove, EffectInitDrift},
    {0x000002b7, EffectUpdateMove, EffectInitDrift},
    {0x000002b8, EffectUpdateMove, EffectInitDrift},
    {0x000002b9, EffectUpdateMove, EffectInitDrift},
    {0x000002ba, EffectUpdateMove, EffectInitDrift},
    {0x000002bb, NULL, NULL},
    {0x000002bc, EffectUpdateOrbit, EffectInitForeground},
    {0x000002bc, EffectUpdateOrbit, EffectInitForeground},
    {0x000002bc, EffectUpdateOrbit, EffectInitForeground},
    {0x000002dc, NULL, NULL},
    {0x000002af, EffectUpdateConverge, EffectInitConverge},
    {0x000002b0, EffectUpdateConvergeSlow, EffectInitConverge},
    {0x000002bd, EffectUpdateNone, NULL},
    {0x000002bf, EffectUpdatePetal, EffectInitPetal20},
    {0x000002c3, NULL, NULL},
    {0x000002c0, EffectUpdateDivergeEased, EffectInitDiverge},
    {0x00000304, EffectUpdateFixedPos, NULL},
    {0x000002c2, EffectUpdateFollowPlayer, NULL},
    {0x000002da, EffectUpdateNone, NULL},
    {0x000002bf, EffectUpdatePetal, EffectInitPetal26},
    {0x000002bf, EffectUpdatePetal, EffectInitPetal27},
    {0x000002db, EffectUpdateNone, NULL},
    {0x000002b2, EffectUpdateDivergeLinear, EffectInitConverge},
    {0x000002bf, EffectUpdatePetal, EffectInitPetal30},
    {0x000002bf, EffectUpdatePetal, EffectInitPetal31},
    {0x000002c1, EffectUpdateDivergeEased, EffectInitDiverge},
    {0x000002b1, EffectUpdateConverge, EffectInitConverge},
};
EnemyScriptFunc g_EnemyScriptFuncs[24] = {
    WarpToBoss,
    ShakeAndSpinBullets,
    SpawnFansFromNearbyBullets,
    NoOp,
    ReplaceTallBulletWithEffect,
    CopyBossMoveStart,
    FireBackwardFans,
    UpdateLaserEdgeBullets,
    DeflectBulletsOffLasers,
    ShakeAndStartFallingPetals,
    StartSlowdown,
    EndSlowdown,
    BurstBulletsByDifficulty,
    TurnNearbyBullets,
    AimTurnedBulletsAtPlayer,
    FlashScreen,
    FireBackFromUnturned,
    ReplaceBulletsWithEnemies,
    CountUnturnedBullets,
    ScriptFadeOutMusic,
    ScriptPlayMusic,
    BurstBulletsFixedCount,
    FireAimedBulletPattern1,
    FireAimedBulletPattern2,
};
int g_SpellCardBonuses[141] = {
    2000000, 2000000, 2200000, 2200000, 2400000, 2400000, 2400000, 2400000,
    2400000, 2400000, 2600000, 2600000, 2600000, 2600000, 2600000, 2600000,
    2600000, 2600000, 2600000, 2600000, 2600000, 2600000, 2600000, 2600000,
    2600000, 2600000, 3000000, 3000000, 3000000, 3000000, 3000000, 3000000,
    3000000, 3000000, 3000000, 3000000, 3000000, 3000000, 3000000, 3000000,
    3000000, 3000000, 3000000, 3000000, 3500000, 3500000, 3500000, 3500000,
    3500000, 3500000, 3500000, 3500000, 3500000, 3500000, 3500000, 3500000,
    3500000, 3500000, 3500000, 3500000, 3500000, 3500000, 3500000, 3500000,
    3500000, 3500000, 3500000, 3500000, 4000000, 4000000, 4000000, 4000000,
    4000000, 4000000, 4000000, 4000000, 4000000, 4000000, 4000000, 4000000,
    4000000, 4000000, 4000000, 4000000, 4000000, 4000000, 4000000, 4000000,
    5000000, 5000000, 5000000, 5000000, 5000000, 5000000, 5000000, 5000000,
    5000000, 5000000, 5000000, 5000000, 5000000, 5000000, 5000000, 5000000,
    5000000, 5000000, 5000000, 5000000, 5000000, 5000000, 5000000, 5000000,
    3000000, 3000000, 3000000, 3000000, 6000000, 6000000, 7000000, 7000000,
    7000000, 7000000, 7000000, 7000000, 7000000, 7000000, 4000000, 7000000,
    7000000, 7000000, 8000000, 8000000, 8000000, 8000000, 8000000, 8000000,
    8000000, 8000000, 4000000, 8000000, 4000000,
};
EnemyScriptInterp g_EnemyScriptInterps[8] = {
    ScriptMoveLinear,
    ScriptMoveLinear,
    ScriptMoveLinear,
    ScriptMoveLinear,
    ScriptMoveLinear,
    ScriptMoveLinear,
    ScriptMoveLinear,
    ScriptMoveHermite,
};
char *g_PracticeStageNames[6] = {
    "Stage1",
    "Stage2",
    "Stage3",
    "Stage4",
    "Stage5",
    "Stage6",
};
char *g_MenuStageNames[8] = {
    "Stage1  ",
    "Stage2  ",
    "Stage3  ",
    "Stage4  ",
    "Stage5  ",
    "Stage6  ",
    "Extra   ",
    "Phantasm",
};
char *g_MenuDifficultyNames[6] = {
    "Easy    ",
    "Normal  ",
    "Hard    ",
    "Lunatic ",
    "Extra   ",
    "Phantasm",
};
char *g_MenuShotTypeNames[6] = {
    "ReimuA ",
    "ReimuB ",
    "MarisaA",
    "MarisaB",
    "SakuyaA",
    "SakuyaB",
};
short g_LastPressedKey = 32;
char *g_KeyConfigHelpTexts[12] = {
    "ショット、決定ボタンを設定します",
    "ボム、キャンセルボタンを設定します",
    "低速移動ボタンを設定します",
    "メッセージスキップボタンを設定します",
    "ポーズボタンを設定します",
    "上移動ボタンを設定します",
    "下移動ボタンを設定します",
    "左移動ボタンを設定します",
    "右移動ボタンを設定します",
    "ショット押しっぱなしで低速移動になるようにします",
    "初期設定に戻します",
    "おおよそ終了します",
};
char *g_OptionsHelpTexts[9] = {
    "プレイヤーの初期数を変更します。（初期設定　３）",
    "画面の色数を変更します。３２ＢＩＴだと最も綺麗に表示されます。",
    "ＢＧＭの再生方法を変更します。（初期設定　ＷＡＶ）",
    "効果音を再生するか選択します",
    "ウィンドウかフルスクリーンか選択します",
    "弾が多い場面でわざと処理落ちさせます(スコア、リプレイ記録不可)",
    "全て初期設定にします",
    "パッド操作のボタン配置を変更します",
    "おいそれと終了します",
};
char *g_TitleHelpTexts[8] = {
    "ゲームを開始します",
    "エキストラステージを開始します",
    "ステージを選択し、練習を開始します",
    "リプレイを鑑賞できます",
    "過去のスコアやスペルカードの取得歴を見られます",
    "音楽を聴けます",
    "各種設定できます",
    "いろいろと終了します",
};
char *g_ShotTypeNames[6] = {
    "ReimuA ",
    "ReimuB ",
    "MarisaA",
    "MarisaB",
    "SakuyaA",
    "SakuyaB",
};
float g_SpellCardRatings[5] = {
    1.0f, 1.5f, 1.5f, 2.0f, 2.5f,
};
char *g_DifficultyNames[6] = {
    "      Easy",
    "    Normal",
    "      Hard",
    "   Lunatic",
    "     Extra",
    "  Phantasm",
};
char *g_ShotFiles[6] = {
    "data/ply00a.sht",
    "data/ply00b.sht",
    "data/ply01a.sht",
    "data/ply01b.sht",
    "data/ply02a.sht",
    "data/ply02b.sht",
};
char *g_ShotFilesFocused[6] = {
    "data/ply00as.sht",
    "data/ply00bs.sht",
    "data/ply01as.sht",
    "data/ply01bs.sht",
    "data/ply02as.sht",
    "data/ply02bs.sht",
};
char *g_EclPaths[10] = {
    "dummy",
    "data/ecldata1.ecl",
    "data/ecldata2.ecl",
    "data/ecldata3.ecl",
    "data/ecldata4.ecl",
    "data/ecldata5.ecl",
    "data/ecldata6.ecl",
    "data/ecldata7.ecl",
    "data/ecldata8.ecl",
    NULL,
};
EnemyAnmPaths g_EnemyAnmPaths[9] = {
    {"dummy", "dummy"},
    {"data/stg1enm.anm", NULL},
    {"data/stg2enm.anm", NULL},
    {"data/stg3enm.anm", NULL},
    {"data/stg4enm.anm", NULL},
    {"data/stg5enm.anm", NULL},
    {"data/stg6enm.anm", NULL},
    {"data/stg7enm.anm", NULL},
    {"data/stg8enm.anm", NULL},
};
RankSettings g_Ranks[6] = {
    {16, 12, 20},
    {16, 10, 32},
    {16, 10, 32},
    {16, 10, 32},
    {16, 15, 16},
    {16, 15, 16},
};
D3DCOLOR g_BossTimerColors[4] = {
    0x00a0d0ff, 0x00a080ff, 0x00e080c0, 0x00ff4040,
};
char *g_BadEndingFiles[3] = {
    "data/end00b.end",
    "data/end10b.end",
    "data/end20b.end",
};
char *g_EndingFiles[6] = {
    "data/end00.end",
    "data/end01.end",
    "data/end10.end",
    "data/end11.end",
    "data/end20.end",
    "data/end21.end",
};
char *g_StageDataFiles[9] = {
    "dummy",
    "data/stage1.std",
    "data/stage2.std",
    "data/stage3.std",
    "data/stage4.std",
    "data/stage5.std",
    "data/stage6.std",
    "data/stage7.std",
    "data/stage8.std",
};

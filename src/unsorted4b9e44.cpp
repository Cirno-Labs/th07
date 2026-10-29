// Data only. The original's uninitialized globals come object by object
// in link order, these after lzss.obj's, and then its constructed ones
// in reverse link order, these (the array) first; the array also has
// the last static initializer (0x48cb70). So this is the last object
// linked, after archive.obj, which has no globals of its own.

#include "archive.h"
#include "replay.h"
#include "sprite.h"
#include "unsorted.h"

#ifdef TH07_EXE
SpriteEngine *g_SpriteEngine;
ReplayManager *g_ReplayManager;
WORD g_Buttons;
WORD g_GameButtons;
WORD g_PrevButtons;
WORD g_PrevGameButtons;
WORD g_ButtonRepeat;
WORD g_ButtonFrames;
int g_FileSize;
// Constructed and destroyed, never used.
DatArchive g_Archives4b9e68[20];
#endif // TH07_EXE

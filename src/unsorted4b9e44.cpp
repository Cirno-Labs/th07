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
// The linker lays out an object's uninitialized globals by a hash of
// their names (tools/locals.py name_hash % 1024, ascending), so these
// names were picked to hash in the original's order, which is the order
// below. The original's names are unknown.
//
// Unused, a stand-in (name and type invented): the original has four
// unreferenced bytes before g_SpriteRenderer.
DWORD g_UnusedGlobal;
SpriteEngine *g_SpriteRenderer;
ReplayManager *g_ReplaySystem;
WORD g_Buttons;
WORD g_GameInputs;
WORD g_LastButtons;
WORD g_PreviousGameInputs;
WORD g_RepeatButtons;
WORD g_ButtonHeldFrames;
int g_LastFileSize;
// Constructed and destroyed, never used.
DatArchive g_Archives4b9e68[20];
#endif // TH07_EXE

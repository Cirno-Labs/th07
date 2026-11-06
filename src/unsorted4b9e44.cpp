// Data only. The original's uninitialized globals come object by object
// in link order, these after lzss.obj's, and then its constructed ones
// in reverse link order, these (the array) first; the array also has
// the last static initializer (0x48cb70). The renderer, replay, input and
// file-size data-only units follow this one without adding constructors.

#include "archive.h"
#include "replay.h"
#include "sprite.h"
#include "unsorted.h"

#ifdef TH07_EXE
// Unused, a stand-in (name and type invented): the original has four
// unreferenced bytes before g_SpriteEngine.
DWORD g_UnusedGlobal;
// Constructed and destroyed, never used.
DatArchive g_Archives4b9e68[20];
#endif // TH07_EXE

#ifdef TH07_EXE
// Preserve the original legacy-name library contributor. The pointer is
// discarded by /OPT:REF; strcmpi already requires its implementation.
#ifdef stricmp
#undef stricmp
#endif
extern "C" int __cdecl stricmp(const char *, const char *);
extern __declspec(selectany) int (__cdecl *const g_UnusedStringCompare)(const char *, const char *) = stricmp;
#endif

#ifdef TH07_EXE
extern void *__pMyUnhandledExceptionFilter;
extern __declspec(selectany) void **const g_UnusedFilterReference = &__pMyUnhandledExceptionFilter;
#endif // TH07_EXE

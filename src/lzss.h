// The LZSS coder of the score file and the data archives: a 0x2000-byte
// window, 13-bit offsets and 4-bit lengths (3 to 18 bytes), matches found
// through a binary search tree over the window. Not in th06, whose
// archives decode inside DatArchive::Decode.

#pragma once

#include <windows.h>

#include "common.h"

// The last 0x2000 bytes coded.
extern BYTE g_LzssWindow[0x2000];

// Compresses `size` bytes at `data` into a new GlobalAlloc block, which
// it returns, and stores the compressed size in `outSize`.
void *LzssCompress(BYTE *data, int size, int *outSize);
// Clears the window and the tree.
void Func45f2c0();
// Makes window position `pos` the tree's only node.
void Func45f270(int pos);
// Adds the string at window position `pos` to the tree. Returns the
// length of the longest match found and stores its position in
// `matchPosition`.
int Func45f340(int pos, int *matchPosition);
// Removes the string at window position `pos` from the tree.
void Func45f460(int pos);

// The game's files: a file interface and its Win32 implementation (from
// 0x45e500), and the data archives (from 0x45f680). Unlike th06's, built
// without optimization.

#pragma once

#include <windows.h>

#include "common.h"

// An abstract file, the interface File implements. Not in th06.
class Unknown45e500
{
public:
    virtual bool Open(char *path, char *mode) = 0;
    virtual void Close() = 0;
    virtual DWORD Read(void *buffer, DWORD size) = 0;
    virtual bool Write(void *buffer, DWORD size) = 0;
    virtual DWORD Tell() = 0;
    virtual DWORD GetSize() = 0;
    virtual bool Seek(LONG offset, DWORD origin) = 0;
    virtual ~Unknown45e500()
    {
    }
};

// A Win32 file.
class File : public Unknown45e500
{
public:
    File();
    ~File();

    // Opens `path`, relative to the game's directory unless it has a
    // drive, in `mode` ("r", "w" or "a"). Returns true on success.
    virtual bool Open(char *path, char *mode);
    virtual void Close();
    // Returns the bytes read.
    virtual DWORD Read(void *buffer, DWORD size);
    // Returns true if all `size` bytes were written.
    virtual bool Write(void *buffer, DWORD size);
    virtual DWORD Tell();
    virtual DWORD GetSize();
    virtual bool Seek(LONG offset, DWORD origin);
    // Reads the whole file, if it is at most `maxSize` bytes, into a
    // GlobalAlloc'd buffer, and seeks back to where it was.
    virtual void *ReadAll(DWORD maxSize);

    HANDLE handle;
    // GENERIC_READ or GENERIC_WRITE.
    DWORD access;
};
ASSERT_SIZE(File, 0xc);

// FILE_BEGIN, FILE_CURRENT and FILE_END, in .rdata. Only the first is used.
extern const DWORD g_SeekOrigins[3];

// Writes `path` to `dst` (MAX_PATH bytes) as a full path: in the game's
// directory unless it has a drive. Not in th06.
void MakeFullPath(char *dst, char *path);

struct DatArchiveEntry;

// The game's data archive (th07.dat, magic "PBG4"), at 0x626258: files
// are looked up by name (without the directory). Rewritten since th06:
// the table is LZSS-compressed and so is each entry.
class DatArchive
{
public:
    DatArchive();

    // The decoded size of entry `name`; 0 if there is none.
    DWORD GetEntrySize(char *name);
    // Decodes entry `name` into `buffer`, which must hold GetEntrySize
    // bytes.
    void *Decode(char *name, void *buffer);

    DatArchiveEntry *entries;
    int numEntries;
    // The archive's path, GlobalAlloc'd.
    char *path;
    File *file;
};
ASSERT_SIZE(DatArchive, 0x10);
extern DatArchive g_Archive;

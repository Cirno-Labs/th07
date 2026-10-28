// The game's files: a file interface and its Win32 implementation (from
// 0x45e500). Unlike th06's, built without optimization.

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
void Func45e9d0(char *dst, char *path);

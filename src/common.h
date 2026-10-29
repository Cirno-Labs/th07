// Shared helpers for the decompiled code.

#pragma once

#include <stdlib.h>
#include <string.h>

// Fails to compile unless sizeof(type) == size. Generates no code.
#define ASSERT_SIZE(type, size) typedef char assert_size_##type[sizeof(type) == (size) ? 1 : -1]

// The number of elements of array `a`, as an unsigned size_t.
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

// Marks a constructor the original defines inline. /Ob1 can't expand
// it (it has a loop or member arrays), so it is still called, but the
// static initializer that calls it reserves a frame for its locals. The
// DLL build defines it out of line: nothing there constructs the
// global, and an inline function nobody calls isn't compiled.
#ifdef TH07_EXE
#define EXE_INLINE inline
#else
#define EXE_INLINE
#endif

// Every free() in the game passes through a temporary, the trace an
// inlined wrapper leaves under /Ob1.
inline void MemFree(void *p)
{
    free(p);
}

// Like MemFree: malloc() through a temporary, even for a constant size.
inline void *MemAlloc(int size)
{
    return malloc(size);
}

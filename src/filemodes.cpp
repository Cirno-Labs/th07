// File::Open modes. Kept in a separate translation unit immediately
// after file.cpp: the original's read-only pool has these three strings
// before File's vtables, and its mode pointers occupy the same data slot.

#ifdef TH07_EXE
char *g_FileModes[3] = {
    "r",
    "w",
    "a",
};
#endif // TH07_EXE

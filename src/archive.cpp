// The data archive, from 0x45f680.

#include "archive.h"
#include "lzss.h"
#include "unsorted.h"

// The game's data, as the original defines it here. The DLL build uses
// the original's copies instead.
#ifdef TH07_EXE
const DWORD g_SeekOrigins[3] = {
    0, 1, 2,
};
#endif // TH07_EXE

DatArchive::DatArchive()
{
    this->entries = NULL;
    this->numEntries = 0;
    this->path = NULL;
    this->file = NULL;
}

DatArchive::~DatArchive()
{
    this->Release();
}

bool DatArchive::Open(char *path)
{
    this->Release();
    DebugPrint("info : %s open arcfile\r\n", path);
    this->file = new File();
    if (this->file == NULL)
    {
        return false;
    }
    if (this->ReadTable(path))
    {
        this->path = this->DuplicateString(path);
        if (this->path != NULL)
        {
            return true;
        }
    }
    DebugPrint("info : %s not found\r\n", path);
    this->Release();
    return false;
}

void DatArchive::Release()
{
    DebugPrint("info : %s close arcfile\r\n", this->path);
    if (this->path != NULL)
    {
        GlobalFree(this->path);
        this->path = NULL;
    }
    if (this->entries != NULL)
    {
        delete[] this->entries;
        this->entries = NULL;
    }
    if (this->file != NULL)
    {
        delete this->file;
        this->file = NULL;
    }
    this->numEntries = 0;
}

void *DatArchive::Decode(char *name, void *buffer)
{
    DatArchiveEntry *found;
    DWORD decodedSize;
    void *decoded;
    HGLOBAL packed;
    DWORD packedSize;

    packed = NULL;
    if (this->file == NULL)
    {
        return NULL;
    }
    found = this->FindEntry(name);
    if (found == NULL)
    {
        goto fail;
    }
    if (!this->file->Open(this->path, g_FileModes[0]))
    {
        goto fail;
    }
    packedSize = found[1].offset - found->offset;
    decodedSize = found->size;
    packed = GlobalAlloc(GMEM_FIXED, packedSize);
    if (packed == NULL)
    {
        goto fail;
    }
    if (!this->file->Seek(found->offset, g_SeekOrigins[0]))
    {
        goto fail;
    }
    if (this->file->Read(packed, packedSize) == 0)
    {
        goto fail;
    }
    decoded = LzssDecompress((BYTE *)packed, packedSize, (BYTE *)buffer, decodedSize);
    if (packed != NULL)
    {
        GlobalFree(packed);
        packed = NULL;
    }
    return decoded;

fail:
    DebugPrint("info : %s error\r\n", this->path);
    if (packed != NULL)
    {
        GlobalFree(packed);
        packed = NULL;
    }
    return NULL;
}

DWORD DatArchive::GetEntrySize(char *name)
{
    DatArchiveEntry *entry;

    entry = this->FindEntry(name);
    if (entry != NULL)
    {
        return entry->size;
    }
    return 0;
}

DatArchiveEntry *DatArchive::FindEntry(char *name)
{
    DatArchiveEntry *cur;
    int left;

    if (this->entries == NULL)
    {
        return NULL;
    }
    for (cur = this->entries, left = this->numEntries; left > 0; left--, cur++)
    {
        if (strcmpi(name, cur->name) == 0)
        {
            return cur;
        }
    }
    return NULL;
}

bool DatArchive::ReadTable(char *path)
{
    BYTE *unpacked;
    DWORD decodedSize;
    DWORD magic;
    DWORD size;
    DWORD tableOffset;
    HGLOBAL packed;

    packed = NULL;
    unpacked = NULL;
    if (this->file == NULL)
    {
        return false;
    }
    if (!this->file->Open(path, g_FileModes[0]))
    {
        goto fail;
    }
    if (!this->file->ReadDword(&magic))
    {
        goto fail;
    }
    if (magic != 0x34474250)
    {
        goto fail;
    }
    if (!this->file->ReadDword(&this->numEntries))
    {
        goto fail;
    }
    if (this->numEntries <= 0)
    {
        goto fail;
    }
    size = this->file->GetSize();
    if (!this->file->ReadDword(&tableOffset))
    {
        goto fail;
    }
    if (tableOffset >= size)
    {
        goto fail;
    }
    size -= tableOffset;
    if (!this->file->ReadDword(&decodedSize))
    {
        goto fail;
    }
    this->file->Seek(tableOffset, g_SeekOrigins[0]);
    packed = GlobalAlloc(GMEM_FIXED, size);
    if (packed == NULL)
    {
        goto fail;
    }
    if (this->file->Read(packed, size) == 0)
    {
        goto fail;
    }
    unpacked = (BYTE *)LzssDecompress((BYTE *)packed, size, NULL, decodedSize);
    if (unpacked == NULL)
    {
        goto fail;
    }
    this->entries = this->BuildEntryTable(unpacked, this->numEntries, tableOffset);
    if (this->entries == NULL)
    {
        goto fail;
    }
    if (packed != NULL)
    {
        GlobalFree(packed);
        packed = NULL;
    }
    if (unpacked != NULL)
    {
        GlobalFree(unpacked);
        unpacked = NULL;
    }
    return true;

fail:
    if (packed != NULL)
    {
        GlobalFree(packed);
        packed = NULL;
    }
    if (unpacked != NULL)
    {
        GlobalFree(unpacked);
        unpacked = NULL;
    }
    if (this->file != NULL)
    {
        delete this->file;
        this->file = NULL;
    }
    DebugPrint("ファイル %s のオープン中にエラーが発生しました\r\n", path);
    while (0)
    {
    }
    return false;
}

DatArchiveEntry *DatArchive::BuildEntryTable(BYTE *table, int count, DWORD tableOffset)
{
    DatArchiveEntry *entries;
    int i;
    BYTE *p;

    entries = NULL;
    entries = new DatArchiveEntry[count + 1];
    if (entries == NULL)
    {
        goto fail;
    }
    p = table;
    for (i = 0; i < count; i++)
    {
        entries[i].name = this->DuplicateString((char *)p);
        p += strlen((char *)p) + 1;
        entries[i].offset = *(DWORD *)p;
        p += 4;
        entries[i].size = *(DWORD *)p;
        p += 4;
        entries[i].checksum = *(DWORD *)p;
        p += 4;
    }
    entries[count].offset = tableOffset;
    entries[count].size = 0;
    return entries;

fail:
    if (entries != NULL)
    {
        delete[] entries;
        entries = NULL;
    }
    return NULL;
}

char *DatArchive::DuplicateString(char *str)
{
    char *copy;

    copy = (char *)GlobalAlloc(GMEM_FIXED, strlen(str) + 1);
    if (copy != NULL)
    {
        strcpy(copy, str);
    }
    return copy;
}

#ifdef TH07_EXE
// Unused: the original also mentioned five more KERNEL32 functions after
// file.obj's, from code /LTCG discarded. The Rich header counts their
// import objects and the IAT order needs five more KERNEL32 entries in
// the linker's import sort (the dropped ones are still sorted) after
// DeleteFileA. Which five is unknown; th06's file helpers stand in.
BOOL UnusedReplaceFile(char *path, char *backup, char *tempDir)
{
    if (GetTempPathA(MAX_PATH, tempDir) == 0)
    {
        return FALSE;
    }
    if (GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES)
    {
        return FALSE;
    }
    SetFileAttributesA(path, FILE_ATTRIBUTE_NORMAL);
    if (!CopyFileA(path, backup, FALSE))
    {
        return FALSE;
    }
    return MoveFileA(backup, path);
}
#endif // TH07_EXE

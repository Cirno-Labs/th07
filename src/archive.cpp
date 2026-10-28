// The data archive, from 0x45f680.

#include "archive.h"

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

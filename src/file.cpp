// The file class, from 0x45e500.

#include "archive.h"

File::File()
{
    this->handle = INVALID_HANDLE_VALUE;
    this->access = 0;
}

File::~File()
{
    this->Close();
}

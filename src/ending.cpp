// The ending's file, 0x41d180 to 0x41e915, as th06's ending.cpp.

#include <stdlib.h>
#include <string.h>

#include "ending.h"
#include "unsorted.h"

int Ending::ReadInt()
{
    int value;

    value = atoi(this->cursor);
    while (*this->cursor != '\0')
    {
        this->cursor++;
    }
    while (*this->cursor == '\0')
    {
        this->cursor++;
    }
    return value;
}

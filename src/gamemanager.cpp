// cflags: /Os
// The /Os file with Unknown626270, probably th07's GameManager, from
// 0x42d560 to the Chain code at 0x42fa60.

#include <string.h>

#include "unsorted.h"

void Unknown626270::AddLives(int lives)
{
    if (this->Func404fe0())
    {
        memset(&g_App, 0xff, 0x2d0);
    }
    this->stats->lives += lives;
    this->Func4012b0();
}

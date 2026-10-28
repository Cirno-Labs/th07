// cflags: /Os -/Oi
// The /Os file with the result screen: the score file and its sorted
// lists, th06's resultscreen.cpp.

#include <string.h>

#include "resultscreen.h"

int LoadPracticeScores(void *scoreDat, Pscr *pscrs)
{
    Pscr *p;
    Pscr *chunk;
    int i3;
    int j0;
    int rest3;
    int k1;
    ScoreDat *dat10 = (ScoreDat *)scoreDat;

    if (pscrs == NULL)
    {
        return -1;
    }
    p = pscrs;
    for (i3 = 0; i3 < 6; i3++)
    {
        for (j0 = 0; j0 < 6; j0++)
        {
            for (k1 = 0; k1 < 4; k1++, p++)
            {
                memset(p, 0, sizeof(Pscr));
                p->magic = 0x52435350;
                p->size2 = sizeof(Pscr);
                p->size = sizeof(Pscr);
                p->version = 1;
                p->shot = i3;
                p->difficulty = k1;
                p->stage = j0;
                p->score = 0;
            }
        }
    }
    chunk = (Pscr *)((BYTE *)dat10 + dat10->dataOffset);
    rest3 = dat10->fileSize - dat10->dataOffset;
    while (rest3 > 0)
    {
        if (chunk->magic == 0x52435350 && chunk->version == 1)
        {
            p = chunk;
            if (p->shot >= 6 || p->difficulty >= 5 || p->stage >= 7)
            {
                break;
            }
            pscrs[p->shot * 6 * 4 + p->stage * 4 + p->difficulty] = *p;
        }
        rest3 -= chunk->size;
        chunk = (Pscr *)((BYTE *)chunk + chunk->size);
    }
    return 0;
}

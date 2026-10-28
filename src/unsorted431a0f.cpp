// cflags: /Os -/Oi
// The file starting at 0x431a0f, th06's unsorted41e940.cpp. It is built
// with /Os, as in th06: functions end with `leave` instead of
// `mov esp, ebp; pop ebp`.

#include "unsorted.h"

GdiSurface::GdiSurface()
{
    this->format = -1;
    this->width = 0;
    this->height = 0;
    this->hdc = NULL;
    this->bitmap = NULL;
    this->oldObject = NULL;
    this->bits = NULL;
}

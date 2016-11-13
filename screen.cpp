#include "screen.h"
#include <U8glib.h>

Screen::Screen(int a, int b, int c) : screen(U8GLIB_ST7920_128X64(a, b, c, U8G_PIN_NONE)) { }

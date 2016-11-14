#include "screen.h"
#include <U8glib.h>

Screen::Screen(int a, int b, int c) : screen(U8GLIB_ST7920_128X64(a, b, c, U8G_PIN_NONE)) {
}
void Screen::clear() {
}
void Screen::write_text(int x, int y, const char* text) {
}
void Screen::draw_image(const unsigned char* image) {
}

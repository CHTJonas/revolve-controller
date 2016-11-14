#include "screen.h"
#include <U8glib.h>
#include <stdint.h>

Screen::Screen(uint8_t sck, uint8_t mosi, uint8_t cs) : screen(U8GLIB_ST7920_128X64(sck, mosi, cs)) {
}
void Screen::clear() {
}
void Screen::write_text(int x, int y, const char* text) {
}
void Screen::draw_image(const unsigned char* image) {
}

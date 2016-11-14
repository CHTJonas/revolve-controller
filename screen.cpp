#include "screen.h"
#include <U8glib.h>
#include <stdint.h>

Screen::Screen(uint8_t sck, uint8_t mosi, uint8_t cs) : screen(U8GLIB_ST7920_128X64(sck, mosi, cs)) {
	screen.begin();
	screen.setFont(u8g_font_unifont);
}

void Screen::clear() {
	screen.begin();
}

void Screen::write_text(int x, int y, const char* text) {
	screen.drawStr(x * 8, (y + 1) * screen.getFontLineSpacing(), text);
}

void Screen::draw_image(const unsigned char* image) {
	screen.drawBitmap(0, 0, 128, 64, image);
}

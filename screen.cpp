#include "screen.h"
#include <Arduino.h>
#include <U8glib.h>
#include <stdint.h>

Screen::Screen(State *state, uint8_t sck, uint8_t mosi, uint8_t cs) : screen(U8GLIB_ST7920_128X64(sck, mosi, cs)), mosi(mosi), state(state) {
}

int i = 0;

void Screen::clear() {
	Serial.write("\033[2J");
	Serial.write("\033[10;0H");
	char buffer[16];
	snprintf(buffer, 16, "...%i...%i...", state->get_state(), i++);
	Serial.write(buffer);
	screen.setColorIndex(1);
	screen.begin();
	screen.setFont(u8g_font_unifont);
}

void Screen::write_text(int x, int y, const char* text) {
	int sid;
	switch (mosi) {
	case 24:
		sid = 0;
		break;
	case 25:
		sid = 2;
		break;
	case 34:
		sid = 1;
		break;
	}
	char buffer[16];
	snprintf(buffer, 16, "\033[%i;%iH", y+1, x + sid * 16);
	Serial.write(buffer);
	Serial.write(text);
	screen.drawStr(x * 8, (y + 1) * screen.getFontLineSpacing(), text);
}

void Screen::draw_image(const unsigned char* image) {
	screen.drawBitmap(0, 0, 128, 64, image);
}

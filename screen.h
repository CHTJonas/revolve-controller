#pragma once
#include <U8glib.h>
#include <stdint.h>

class Screen {
public:
	Screen(uint8_t sck, uint8_t mosi, uint8_t cs);

	void clear();
	void write_text(int x, int y, const char* text);
	void draw_image(const unsigned char image[]);

private:
	U8GLIB_ST7920_128X64 screen;
};

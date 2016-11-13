#pragma once
#include <U8glib.h>

class Screen {
public:
	Screen(int a, int b, int c);

	void clear();
	void write_text(int x, int y, const char* text);
	void draw_image(const unsigned char image[]);

private:
	U8GLIB_ST7920_128X64 screen;
};

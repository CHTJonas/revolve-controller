#pragma once
#include <U8glib.h>

class Screen {
public:
	Screen(int a, int b, int c);

	void draw_image(unsigned char image[]);

private:
	U8GLIB_ST7920_128X64 screen;
};

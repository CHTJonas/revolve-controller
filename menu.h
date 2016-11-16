#pragma once
#include <U8glib.h>
#include "interface.h"

class Menu {
public:
	Menu(const char* menuOptionStrings[], int numberOfOptions, U8GLIB_ST7920_128X64& screen, Interface& interface);
    ~Menu();
    void draw() const;

private:
    U8GLIB_ST7920_128X64& m_screen;
    Interface& m_interface;
    int m_numberOfOptions;
    const char** m_menuOptionStrings;
    void drawStrCentre(int y, const char* text) const;
    void drawStrCentre(int y, char text);

    const u8g_fntpgm_uint8_t* extra_large_font = u8g_font_profont22;  // 16px high row
	const u8g_fntpgm_uint8_t* large_font = u8g_font_profont15;  // 10px high row
    const u8g_fntpgm_uint8_t* standard_font = u8g_font_profont11;  // 8px high row
    const u8g_fntpgm_uint8_t* small_font = u8g_font_5x7;  // 7px high row
};

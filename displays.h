#pragma once
#include "interface.h"
#include "menu.h"
#include "revolve.h"
#include "state.h"
#include "strings.h"
#include <U8glib.h>

class Displays {
public:
	Displays(
	    State* state,
	    U8GLIB_ST7920_128X64& cue,
	    U8GLIB_ST7920_128X64& menu,
	    U8GLIB_ST7920_128X64& info,
	    Adafruit_NeoPixel& ringLeds,
	    Revolve& inner,
	    Revolve& outer,
	    Keypad& keypad,
	    Interface& interface,
	    Cuestack& cuestack);
	void setup();
	void loop();

	// Initialisation
	void setMode();

	// Screen Drawing
	static void drawStrCentre(U8GLIB_ST7920_128X64& lcd, int y, const char* text);
	static void drawStrCentre(U8GLIB_ST7920_128X64& lcd, int y, char text);
	void drawCueLayout(U8GLIB_ST7920_128X64& lcd, int values[], int cursorEnable) const;
	void drawParamsLayout(U8GLIB_ST7920_128X64& lcd, int cursorEnable) const;
	void drawCuelistLayout(U8GLIB_ST7920_128X64& lcd, int index, int cursorEnable) const;
	void drawLeftDisplay() const;
	void drawCentreDisplay() const;
	void drawRightDisplay() const;
	void updateRingLeds();
	void updateDisplays(int cue1, int menu, int info, int ringLeds);
	void forceUpdateDisplays(int cue1, int menu, int info, int ringLeds);

	void drawWheelCueDetails(
		U8GLIB_ST7920_128X64& lcd,
		int values[],
		int cursorEnable,
		int menu_pos,
		int menu_pos_offset,
		int yOffset, const char* revolveName) const;

	int update;
	int ledOuter;
	int ledInner;

private:
	// define the menu strings
	const char* menu_strings[4] = { MENU_OPTION_1, MENU_OPTION_2, MENU_OPTION_3, MENU_OPTION_4 };
	const char* settings_strings[9] = { SETTINGS_OPTION_1, SETTINGS_OPTION_2, SETTINGS_OPTION_3,
		                            SETTINGS_OPTION_4, SETTINGS_OPTION_5, SETTINGS_OPTION_6,
		                            SETTINGS_OPTION_7, SETTINGS_OPTION_8, SETTINGS_OPTION_9 };
	const char* enc_settings_strings[4] = {
		ENCODER_OPTION_1, ENCODER_OPTION_2, ENCODER_OPTION_3, ENCODER_OPTION_4
	};
	const char* led_settings_strings[4] = { LED_OPTION_1, LED_OPTION_2, LED_OPTION_3, LED_OPTION_4 };
	const char* kp_settings_strings[6] = { PID_OPTION_1, PID_OPTION_2, PID_OPTION_3,
		                               PID_OPTION_4, PID_OPTION_5, PID_OPTION_6 };
	const char* program_strings[4] = { PROGRAM_OPTION_1, PROGRAM_OPTION_2, PROGRAM_OPTION_3, PROGRAM_OPTION_4 };
	const char* param_strings[3] = { PARAM_OPTION_1, PARAM_OPTION_2, PARAM_OPTION_3 };

	// define the actual menus
	const Menu mainMenu = Menu(menu_strings, 4, displayCentre, interface);
	// const Menu settingsMenu = Menu(settings_strings, 8);
	// const Menu encoderMenu = Menu(enc_settings_strings, 4);
	// const Menu ledMenu = Menu(led_settings_strings, 4);
	// const Menu pidMenu = Menu(kp_settings_strings, 6);
	// const Menu programMenu = Menu(program_strings, 4);
	// const Menu paramMenu = Menu(param_strings, 3);

	const u8g_fntpgm_uint8_t* font = u8g_font_profont11;  // 8px high row
	const u8g_fntpgm_uint8_t* xlarge_font = u8g_font_profont22;  // 16px high row
	const u8g_fntpgm_uint8_t* large_font = u8g_font_profont15;  // 10px high row
	const u8g_fntpgm_uint8_t* small_font = u8g_font_5x7;  // 7px high row

	State* state;

	U8GLIB_ST7920_128X64& displayLeft;
	U8GLIB_ST7920_128X64& displayCentre;
	U8GLIB_ST7920_128X64& displayRight;

	Adafruit_NeoPixel& ringLeds;

	Revolve& inner;
	Revolve& outer;

	Keypad& keypad;

	Interface& interface;

	Cuestack& cuestack;
};

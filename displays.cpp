#include "displays.h"
#include "logo.h"
#include "strings.h"

// Constructor
Displays::Displays(
    U8GLIB_ST7920_128X64& cue,
    U8GLIB_ST7920_128X64& menu,
    U8GLIB_ST7920_128X64& info,
    Adafruit_NeoPixel& ringLeds,
    Revolve& inner,
    Revolve& outer,
    Keypad& keypad,
    Interface& interface,
    Cuestack& cuestack)
      : cue(cue),
        menu(menu),
        info(info),
        ringLeds(ringLeds),
        inner(inner),
        outer(outer),
        keypad(keypad),
        interface(interface),
        cuestack(cuestack) {
	mode = STARTUP;
}

void Displays::step() {
}

void Displays::begin() {
	cue.begin();
	cue.setColorIndex(1);

	menu.begin();
	menu.setColorIndex(1);

	info.begin();
	info.setColorIndex(1);

	mode = STARTUP;
	interface.menu_pos = 0;
	updateDisplays(1, 1, 1, 1);
}

void Displays::setMode(int newMode) {
	// Reset input encoder in case value has accrued
	interface.input.getInputEncoder();
	mode = newMode;
	forceUpdateDisplays(1, 1, 1, 1);
}

void Displays::drawStrCenter(U8GLIB_ST7920_128X64& lcd, int y, const char* text) {
	int width = lcd.getStrWidth(text);
	auto x = (SCREEN_WIDTH - width) / 2;
	lcd.setPrintPos(x, y);
	lcd.print(text);
}

void Displays::drawStrCenter(U8GLIB_ST7920_128X64& lcd, int y, char text) {
	const char string[] = { text, '\0' };
	int width = lcd.getStrWidth(string);
	auto x = (SCREEN_WIDTH - width) / 2;
	lcd.setPrintPos(x, y);
	lcd.print(text);
}

void Displays::drawCueLayout(U8GLIB_ST7920_128X64& lcd, int (&values)[10], int cursorEnable) const {
	lcd.setFont(font);

	auto menu_pos_shift = interface.menu_pos;
	if (mode != MAN && interface.cueParams[1] == 0) {  // Shift menu_pos by 5 if inner disabled
		menu_pos_shift += 5;
	}

	// Only draw if enabled
	if (mode == MAN || (mode != MAN && interface.cueParams[1] == 1)) {
		lcd.drawHLine(0, 0, SCREEN_WIDTH);
		// Inner label
		lcd.drawBox(0, 1, lcd.getStrWidth("INNER") + 4, 10);
		lcd.setDefaultBackgroundColor();
		lcd.drawStr(2, 9, "INNER");
		lcd.setDefaultForegroundColor();

		// Position
		lcd.drawStr(lcd.getStrWidth("INNER") + 6, 9, "Position: ");
		lcd.setPrintPos(lcd.getStrWidth("INNERPosition: ") + 4, 9);

		if (cursorEnable && interface.menu_pos == 0) {
			lcd.drawBox(lcd.getStrWidth("INNERPosition: ") + 3, 1, 19, 9);
			lcd.setDefaultBackgroundColor();
		}
		lcd.print(values[0]);
		lcd.setDefaultForegroundColor();

		// Speed
		lcd.drawStr(2, 19, "Speed: ");
		lcd.setPrintPos(lcd.getStrWidth("Speed: "), 19);

		if (cursorEnable && interface.menu_pos == 1) {
			lcd.drawBox(lcd.getStrWidth("Speed: ") - 1, 11, 19, 9);
			lcd.setDefaultBackgroundColor();
		}
		lcd.print(values[1]);
		lcd.setDefaultForegroundColor();

		// Acceleration
		lcd.drawStr(70, 19, "Acc:  ");
		lcd.setPrintPos(lcd.getStrWidth("Acc:  ") + 70, 19);

		if (cursorEnable && interface.menu_pos == 2) {
			lcd.drawBox(lcd.getStrWidth("Acc:  ") + 69, 11, 19, 9);
			lcd.setDefaultBackgroundColor();
		}
		lcd.print(values[2]);
		lcd.setDefaultForegroundColor();

		// Direction
		lcd.drawStr(2, 29, "Dir: ");
		lcd.setPrintPos(lcd.getStrWidth("Speed: "), 29);

		if (cursorEnable && interface.menu_pos == 3) {
			lcd.drawBox(lcd.getStrWidth("Speed: ") - 1, 21, 19, 9);
			lcd.setDefaultBackgroundColor();
		}
		if (values[3]) {
			lcd.print("ACW");
		} else {
			lcd.print("CW");
		}
		lcd.setDefaultForegroundColor();

		// Extra Revolutions
		lcd.drawStr(70, 29, "Revs: ");
		lcd.setPrintPos(lcd.getStrWidth("Acc:  ") + 70, 29);

		if (cursorEnable && interface.menu_pos == 4) {
			lcd.drawBox(lcd.getStrWidth("Acc:  ") + 69, 21, 13, 9);
			lcd.setDefaultBackgroundColor();
		}
		lcd.print(values[4]);
		lcd.setDefaultForegroundColor();
	}

	// Only draw if enabled
	if (mode == MAN || (mode != MAN && interface.cueParams[2] == 1)) {
		// Outer
		lcd.drawHLine(0, 32, SCREEN_WIDTH);

		// Outer label
		lcd.drawBox(0, 33, lcd.getStrWidth("OUTER") + 4, 10);
		lcd.setDefaultBackgroundColor();
		lcd.drawStr(2, 41, "OUTER");
		lcd.setDefaultForegroundColor();

		// Position
		lcd.drawStr(lcd.getStrWidth("OUTER") + 6, 41, "Position: ");
		lcd.setPrintPos(lcd.getStrWidth("INNERPosition: ") + 4, 41);

		if (cursorEnable && menu_pos_shift == 5) {
			lcd.drawBox(lcd.getStrWidth("INNERPosition: ") + 3, 33, 19, 9);
			lcd.setDefaultBackgroundColor();
		}
		lcd.print(values[5]);
		lcd.setDefaultForegroundColor();

		// Speed
		lcd.drawStr(2, 51, "Speed: ");
		lcd.setPrintPos(lcd.getStrWidth("Speed: "), 51);

		if (cursorEnable && menu_pos_shift == 6) {
			lcd.drawBox(lcd.getStrWidth("Speed: ") - 1, 43, 19, 9);
			lcd.setDefaultBackgroundColor();
		}
		lcd.print(values[6]);
		lcd.setDefaultForegroundColor();

		// Acceleration
		lcd.drawStr(70, 51, "Acc:  ");
		lcd.setPrintPos(lcd.getStrWidth("Acc:  ") + 70, 51);

		if (cursorEnable && menu_pos_shift == 7) {
			lcd.drawBox(lcd.getStrWidth("Acc:  ") + 69, 43, 19, 9);
			lcd.setDefaultBackgroundColor();
		}
		lcd.print(values[7]);
		lcd.setDefaultForegroundColor();

		// Direction
		lcd.drawStr(2, 61, "Dir: ");
		lcd.setPrintPos(lcd.getStrWidth("Speed: "), 61);

		if (cursorEnable && menu_pos_shift == 8) {
			lcd.drawBox(lcd.getStrWidth("Speed: ") - 1, 53, 19, 9);
			lcd.setDefaultBackgroundColor();
		}
		if (values[8]) {
			lcd.print("ACW");
		} else {
			lcd.print("CW");
		}
		lcd.setDefaultForegroundColor();

		// Extra Revolutions
		lcd.drawStr(70, 61, "Revs: ");
		lcd.setPrintPos(lcd.getStrWidth("Acc:  ") + 70, 61);

		if (cursorEnable && menu_pos_shift == 9) {
			lcd.drawBox(lcd.getStrWidth("Acc:  ") + 69, 53, 13, 9);
			lcd.setDefaultBackgroundColor();
		}
		lcd.print(values[9]);
		lcd.setDefaultForegroundColor();
	}
}

void Displays::drawParamsLayout(U8GLIB_ST7920_128X64& lcd, int cursorEnable) const {
	menu.setFont(font);
	menu.drawStr(4, 10, "Cue Number:");
	menu.setPrintPos(90, 10);
	if (interface.menu_pos == 0 && cursorEnable) {
		menu.drawBox(88, 1, 27, 11);
		menu.setDefaultBackgroundColor();
	}

	if (interface.cueNumber - floor(interface.cueNumber) ==
	    0)  // Don't display something like 1.0 (but do display 2.4)
		menu.print(interface.cueNumber, 0);
	else
		menu.print(interface.cueNumber, 1);
	menu.setDefaultForegroundColor();

	for (int i = 0; i < 3; i++) {
		menu.drawStr(4, ((i + 2) * 10), param_strings[i]);
		menu.setPrintPos(90, ((i + 2) * 10));

		if ((interface.menu_pos - 1) == i && cursorEnable) {
			menu.drawBox(88, 1 + (i + 1) * 10, 21, 11);
			menu.setDefaultBackgroundColor();
		}

		if (interface.cueParams[i] == 0)
			menu.print("NO");
		else
			menu.print("YES");
		menu.setDefaultForegroundColor();
	}

	if (interface.menu_pos == 4 && cursorEnable) {
		menu.drawBox(0, 41, SCREEN_WIDTH, 11);
		menu.setDefaultBackgroundColor();
	}
	drawStrCenter(menu, 50, "Add Cue");
	menu.setDefaultForegroundColor();

	if (interface.menu_pos == 5 && cursorEnable) {
		menu.drawBox(0, 51, SCREEN_WIDTH, 11);
		menu.setDefaultBackgroundColor();
	}
	drawStrCenter(menu, 60, "Delete Cue");
	menu.setDefaultForegroundColor();
}

void Displays::drawCuelistLayout(U8GLIB_ST7920_128X64& lcd, int index, int cursorEnable) const {
	// Set font and row height
	lcd.setFont(small_font);

	// Work out pagination
	int cueListPage = index / 7;

	// Header highlight
	lcd.drawBox(0, 0, SCREEN_WIDTH, 8);
	lcd.setDefaultBackgroundColor();

	// Draw header row
	lcd.drawStr(5, 7, "#");
	lcd.drawStr(30, 7, "Inn.");
	lcd.drawStr(55, 7, "R");
	lcd.drawStr(75, 7, "Out.");
	lcd.drawStr(100, 7, "R");
	lcd.drawStr(115, 7, "AF");
	lcd.setDefaultForegroundColor();

	// Draw correct page of cues
	for (auto i = (cueListPage * 7); i < (cueListPage + 1) * 7; i++) {
		if (cuestack.stack[i].active) {
			auto iPos =
			    (i - (cueListPage * 7));  // Offset page number from i used to position elements on page

			// Draw currentCue box
			if (cuestack.currentCue == i) {
				lcd.setDefaultForegroundColor();
				lcd.drawFrame(0, (iPos + 1) * 8, SCREEN_WIDTH, 8);
			}

			// Draw highlight box if cue selected
			if (index == i && cursorEnable) {
				lcd.drawBox(0, (iPos + 1) * 8, SCREEN_WIDTH, 8);
				lcd.setDefaultBackgroundColor();
			}

			// Number
			lcd.setPrintPos(2, (8 * (iPos + 2)) - 1);
			lcd.print(cuestack.stack[i].num / 10);

			// Print decimal part if any
			if (cuestack.stack[i].num % 10 != 0) {
				lcd.print(".");
				lcd.print(cuestack.stack[i].num % 10);
			}

			if (cuestack.stack[i].en_i) {
				lcd.setPrintPos(30, (8 * (iPos + 2)) - 1);
				lcd.print(cuestack.stack[i].pos_i);
				if (cuestack.stack[i].revs_i) {
					lcd.setPrintPos(55, (8 * (iPos + 2)) - 1);
					lcd.print(cuestack.stack[i].revs_i);
				}
			}

			if (cuestack.stack[i].en_o) {
				lcd.setPrintPos(75, (8 * (iPos + 2)) - 1);
				lcd.print(cuestack.stack[i].pos_o);
				if (cuestack.stack[i].revs_o) {
					lcd.setPrintPos(100, (8 * (iPos + 2)) - 1);
					lcd.print(cuestack.stack[i].revs_o);
				}
			}

			// Auto follow arrow
			if (cuestack.stack[i].auto_follow) {
				lcd.setFont(u8g_font_m2icon_7);
				lcd.drawStr180(120, (8 * (iPos + 1)), "\x62");
				lcd.setFont(small_font);
			}

			lcd.setDefaultForegroundColor();
		}
	}
	lcd.setFont(font);
}

void Displays::drawCue() const {
	cue.setFont(font);

	switch (mode) {
	case STARTUP:
	case HOMING:
		cue.setFont(large_font);
		drawStrCenter(cue, 15, TITLE);
		drawStrCenter(cue, 30, SUBTITLE);
		cue.setFont(font);
		drawStrCenter(cue, 45, NAME);
		cue.setFont(small_font);
		drawStrCenter(cue, 55, DATE);
		break;

	case SHOW:
		drawCueLayout(cue, interface.cueMovements, 0);
		break;

	case PROGRAM:
	case PROGRAM_PARAMS:
	case PROGRAM_CUELIST:
	case PROGRAM_DELETE:
	case PROGRAM_GOTOCUE:
	case PROGRAM_SAVED:
		// Add box if screen selected in PROGRAM
		if (interface.menu_pos == 0 && mode == PROGRAM) {
			cue.setDefaultForegroundColor();
			cue.drawFrame(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			cue.drawFrame(1, 1, SCREEN_WIDTH - 2, SCREEN_HEIGHT - 2);
			cue.setDefaultForegroundColor();
		}
		drawCueLayout(cue, interface.cueMovements, 0);
		break;

	case PROGRAM_MOVEMENTS:
		drawCueLayout(cue, interface.cueMovements, 1);
		break;

	case ESTOP:
		cue.setFont(xlarge_font);
		drawStrCenter(cue, 16, "ESTOP");
		cue.setFont(large_font);
		drawStrCenter(cue, 32, "Reset all Estops");
		drawStrCenter(cue, 48, "to continue");
		break;

	default:
		menu.drawXBMP(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, screen_logo);
		break;
	}
}

void Displays::drawMenu() const {
	menu.setFont(font);

	switch (mode) {
	case STARTUP:
	case HOMING:
		menu.drawXBMP(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, screen_logo);
		break;
	case NORMAL:
		menu.setFont(large_font);

		for (int i = 0; i < 4; i++) {
			if (i == interface.menu_pos) {
				// Position highlight box from top left corner
				menu.drawBox(0, (interface.menu_pos * 16), SCREEN_WIDTH, 16);
				menu.setDefaultBackgroundColor();
				drawStrCenter(menu, (i * 16) + 12, menu_strings[i]);
			}
			drawStrCenter(menu, (i * 16) + 12, menu_strings[i]);
			menu.setDefaultForegroundColor();
		}
		break;

	case MAN:
		drawCueLayout(menu, interface.currentMovements, 1);
		break;

	case SHOW:
		drawCuelistLayout(menu, interface.menu_pos, 1);
		break;

	case PROGRAM:
	case PROGRAM_MOVEMENTS:
	case PROGRAM_CUELIST:
	case PROGRAM_GOTOCUE:
		// Add box if screen selected in PROGRAM
		if (interface.menu_pos == 1 && mode == PROGRAM) {
			menu.setDefaultForegroundColor();
			menu.drawFrame(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			menu.drawFrame(1, 1, SCREEN_WIDTH - 2, SCREEN_HEIGHT - 2);
			menu.setDefaultForegroundColor();
		}
		drawParamsLayout(menu, 0);
		break;

	case PROGRAM_DELETE:
		menu.setFont(large_font);
		drawStrCenter(menu, 35, "Delete Cue?");
		menu.setFont(font);
		break;

	case PROGRAM_SAVED:
		menu.setFont(large_font);
		drawStrCenter(menu, 35, "Cuestack Saved");
		menu.setFont(font);
		break;

	case PROGRAM_PARAMS:
		drawParamsLayout(menu, 1);
		break;

	case SETTINGS:
		menu.setFont(large_font);
		if (interface.menu_pos < 4) {
			for (int i = 0; i < 4; i++) {
				if (i == interface.menu_pos) {
					// Position highlight box from top left corner
					menu.drawBox(0, (interface.menu_pos * 16), SCREEN_WIDTH, 16);
					menu.setDefaultBackgroundColor();
					drawStrCenter(menu, (i * 16) + 12, settings_strings[i]);
				}
				drawStrCenter(menu, (i * 16) + 12, settings_strings[i]);
				menu.setDefaultForegroundColor();
			}
		} else if (interface.menu_pos < 8) {
			for (int i = 0; i < 4; i++) {
				if ((i + 4) == interface.menu_pos) {
					// Position highlight box from top left corner
					menu.drawBox(0, (i * 16), SCREEN_WIDTH, 16);
					menu.setDefaultBackgroundColor();
					drawStrCenter(menu, (i * 16) + 12, settings_strings[i + 4]);
				}
				drawStrCenter(menu, (i * 16) + 12, settings_strings[i + 4]);
				menu.setDefaultForegroundColor();
			}
		}

		break;

	case HARDWARETEST:
		menu.setFont(large_font);
		drawStrCenter(menu, 16, "Hardware Test");
		menu.setFont(font);
		drawStrCenter(menu, 40, "Hold Go and");
		drawStrCenter(menu, 48, "Pause to exit");
		break;

	case BRIGHTNESS:
		menu.setFont(large_font);

		for (int i = 0; i < 4; i++) {
			menu.drawStr(0, 13 + (i * 16), led_settings_strings[i]);
			menu.setPrintPos(90, 13 + (i * 16));

			if (interface.menu_pos == i) {
				menu.drawBox(88, i * 16, 24, 16);
				menu.setDefaultBackgroundColor();
			}

			menu.print(interface.leds.ledSettings[i]);
			menu.setDefaultForegroundColor();
		}
		menu.setFont(font);
		break;

	case ENCSETTINGS:
		menu.setFont(large_font);

		for (int i = 0; i < 4; i++) {
			menu.drawStr(0, 13 + (i * 16), enc_settings_strings[i]);
			menu.setPrintPos(90, 13 + (i * 16));

			if (interface.menu_pos == i) {
				menu.drawBox(88, i * 16, 40, 16);
				menu.setDefaultBackgroundColor();
			}

			if (interface.encSettings[i] == 0 && (i == 0 || i == 1))
				menu.print("FWD");
			else if (interface.encSettings[i] == 1 && (i == 0 || i == 1))
				menu.print("REV");
			else
				menu.print(interface.encSettings[i]);
			menu.setDefaultForegroundColor();
		}
		menu.setFont(font);
		break;

	case ESTOP:
		menu.setFont(xlarge_font);
		drawStrCenter(menu, 16, "ESTOP");
		menu.setFont(large_font);
		drawStrCenter(menu, 32, "Reset all Estops");
		drawStrCenter(menu, 48, "to continue");
		break;

	case DEFAULTVALUES:
		drawCueLayout(menu, interface.defaultValues, 0);
		break;

	case KPSETTINGS:
		menu.setFont(font);

		for (int i = 0; i < 6; i++) {
			menu.drawStr(0, 10 + (i * 10), kp_settings_strings[i]);
			menu.setPrintPos(90, 10 + (i * 10));

			if (interface.menu_pos == i) {
				menu.drawBox(88, (i * 10) + 2, 34, 10);
				menu.setDefaultBackgroundColor();
			}

			menu.print(interface.kpSettings[i], 3);
			menu.setDefaultForegroundColor();
		}
		menu.setFont(font);
		break;

	case RESET_CUESTACK:
		menu.setFont(large_font);
		drawStrCenter(menu, 19, "Reset");
		drawStrCenter(menu, 35, "ENTIRE CUESTACK?");
		menu.setFont(font);
		drawStrCenter(menu, 50, "Press Go, Pause and");
		drawStrCenter(menu, 60, "Select to Continue");
		break;
	default:
		break;
	}
}

void Displays::drawInfo() const {
	info.setFont(font);

	switch (mode) {
	case STARTUP:
		info.setFont(large_font);
		drawStrCenter(info, 20, "READY TO HOME");
		info.setFont(font);
		drawStrCenter(info, 40, "ENSURE REVOLVE CLEAR");
		drawStrCenter(info, 50, "Press GO to home");
		break;
	case HOMING:
		info.setFont(xlarge_font);
		drawStrCenter(info, 20, "HOMING");
		info.setFont(font);
		drawStrCenter(info, 40, "Please wait");
		break;
	case HARDWARETEST:
		info.setFont(xlarge_font);

		if (digitalRead(GO) == LOW) {
			drawStrCenter(info, 40, "GO");
		} else if (digitalRead(PAUSE) == LOW) {
			drawStrCenter(info, 40, "PAUSE");
		} else if (digitalRead(BACK) == HIGH) {
			drawStrCenter(info, 40, "BACK");
		} else if (digitalRead(SELECT) == LOW) {
			drawStrCenter(info, 40, "SELECT");
		} else if (interface.input.currentKey) {
			drawStrCenter(info, 40, interface.input.currentKey);
		}
		info.setFont(font);
		break;

	case ESTOP:
		info.setFont(xlarge_font);
		drawStrCenter(info, 16, "ESTOP");
		info.setFont(large_font);
		drawStrCenter(info, 32, "Reset all Estops");
		drawStrCenter(info, 48, "to continue");
		break;

	case MAN:
	case SHOW:
	case PROGRAM_GOTOCUE:
		info.setFont(large_font);
		info.drawStr(0, 10, "Inner");
		info.setFont(font);
		info.drawStr(0, 20, "Position: ");
		info.setPrintPos(info.getStrWidth("Position: "), 20);
		info.print(inner.displayPos());
		info.drawStr(0, 28, "Speed: ");
		info.setPrintPos(info.getStrWidth("Speed: "), 28);
		info.print(inner.getSpeed());

		info.setFont(large_font);
		info.drawStr(0, 42, "Outer");
		info.setFont(font);
		info.drawStr(0, 52, "Position: ");
		info.setPrintPos(info.getStrWidth("Position: "), 52);
		info.print(outer.displayPos());
		info.drawStr(0, 60, "Speed: ");
		info.setPrintPos(info.getStrWidth("Speed: "), 60);
		info.print(outer.getSpeed());
		break;

	case PROGRAM:
	case PROGRAM_MOVEMENTS:
	case PROGRAM_PARAMS:
	case PROGRAM_DELETE:
	case PROGRAM_SAVED:
		// Add box if screen selected in PROGRAM
		if (interface.menu_pos == 2 && mode == PROGRAM) {
			info.setDefaultForegroundColor();
			info.drawFrame(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			info.drawFrame(1, 1, SCREEN_WIDTH - 2, SCREEN_HEIGHT - 2);
			info.setDefaultForegroundColor();
		}
		drawCuelistLayout(info, cuestack.currentCue, 1);
		break;

	case PROGRAM_CUELIST:
		drawCuelistLayout(info, interface.menu_pos, 1);
		break;

	default:
		info.drawXBMP(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, screen_logo);
		break;
	}
}

void Displays::updateRingLeds() {

	switch (mode) {
	case STARTUP:
	case HOMING:
	case RESET_CUESTACK:
		for (int i = 0; i < 24; i++) {
			ringLeds.setPixelColor(i, 255, 0, 0);
		}
		ringLeds.show();
		break;

	case HARDWARETEST:
		for (int i = 0; i < 24; i++) {
			ringLeds.setPixelColor(i, 255, 255, 255);
		}
		ringLeds.show();
		break;
	case HOMED:
	case PROGRAM_SAVED:
		for (int i = 0; i < 24; i++) {
			ringLeds.setPixelColor(i, 0, 255, 0);
		}
		ringLeds.show();
		break;

	case ESTOP:
		for (int i = 0; i < 24; i++) {
			ringLeds.setPixelColor(i, 255, 0, 0);
		}
		ringLeds.show();
		break;

	case MAN:
	case SHOW:
	case PROGRAM_GOTOCUE:
		ledOuter = outer.displayPos();
		ledInner = inner.displayPos();

		// Get position in terms of 12ths of a circle
		ledOuter = (ledOuter + 15) / 30;
		ledInner = (ledInner + 15) / 30;

		// Flip as LEDS in reverse order
		ledOuter = 12 - ledOuter;
		ledInner = 12 - ledInner;

		// Add 9 then % 12 as LEDS offset by -3
		ledOuter = (ledOuter + 9) % 12;
		ledInner = (ledInner + 9) % 12;

		// Inner is 12 pixels further on
		ledInner += 12;

		// Blank first
		for (int i = 0; i < 24; i++) {
			ringLeds.setPixelColor(i, 50, 0, 0);
		}

		// Set green LEDS
		ringLeds.setPixelColor(ledOuter, 0, 255, 0);
		ringLeds.setPixelColor(ledInner, 0, 255, 0);

		if (ledOuter == 0) {
			ringLeds.setPixelColor(11, 100, 75, 0);
			ringLeds.setPixelColor(1, 100, 75, 0);
		} else if (ledOuter == 11) {
			ringLeds.setPixelColor(0, 100, 75, 0);
			ringLeds.setPixelColor(10, 100, 75, 0);
		} else {
			ringLeds.setPixelColor(ledOuter - 1, 100, 75, 0);
			ringLeds.setPixelColor(ledOuter + 1, 100, 75, 0);
		}

		if (ledInner == 12) {
			ringLeds.setPixelColor(13, 100, 75, 0);
			ringLeds.setPixelColor(23, 100, 75, 0);
		} else if (ledInner == 23) {
			ringLeds.setPixelColor(12, 100, 75, 0);
			ringLeds.setPixelColor(22, 100, 75, 0);
		} else {
			ringLeds.setPixelColor(ledInner - 1, 100, 75, 0);
			ringLeds.setPixelColor(ledInner + 1, 100, 75, 0);
		}

		ringLeds.show();
		break;

	default:
		interface.leds.ringLedsColor(200, 0, 255);
		break;
	}
}

void Displays::updateDisplays(int cue1, int menu1, int info1, int ringLeds1) {
	if (update) {
		if (cue1) {
			cue.firstPage();
			do {
				drawCue();
			} while (cue.nextPage());
		}

		if (menu1) {
			menu.firstPage();
			do {
				drawMenu();
			} while (menu.nextPage());
		}

		if (info1) {
			info.firstPage();
			do {
				drawInfo();
			} while (info.nextPage());
		}

		if (ringLeds1) {
			updateRingLeds();
		}

		update = 0;
	}
}

void Displays::forceUpdateDisplays(int cue1, int menu1, int info1, int ringLeds1) {
	update++;
	updateDisplays(cue1, menu1, info1, ringLeds1);
}

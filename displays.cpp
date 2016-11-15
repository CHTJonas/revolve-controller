#include "displays.h"
#include "logo.h"
#include "state_machine.h"
#include "strings.h"

// Constructor
Displays::Displays(
    State* state,
    U8GLIB_ST7920_128X64& cue,
    U8GLIB_ST7920_128X64& menu,
    U8GLIB_ST7920_128X64& info,
    Adafruit_NeoPixel& ringLeds,
    Revolve& inner,
    Revolve& outer,
    Keypad& keypad,
    Interface& interface,
    Cuestack& cuestack)
      : state(state),
        displayLeft(cue),
        displayCenter(menu),
        displayRight(info),
        ringLeds(ringLeds),
        inner(inner),
        outer(outer),
        keypad(keypad),
        interface(interface),
        cuestack(cuestack) {
}

void Displays::step() {
}

void Displays::begin() {
	displayLeft.begin();
	displayLeft.setColorIndex(1);

	displayCenter.begin();
	displayCenter.setColorIndex(1);

	displayRight.begin();
	displayRight.setColorIndex(1);

	interface.menu_pos = 0;
	updateDisplays(1, 1, 1, 1);
}

void Displays::setMode() {
	// Reset input encoder in case value has accrued
	interface.input.getInputEncoder();
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
	drawStrCenter(lcd, y, string);
}

void Displays::drawWheelCueDetails(
    U8GLIB_ST7920_128X64& lcd, int(values)[5], int cursorEnable, int menu_pos, int yOffset, const char* revolveName)
    const {
	lcd.drawHLine(0, yOffset, SCREEN_WIDTH);

	// Label
	lcd.drawBox(0, yOffset + 1, lcd.getStrWidth(revolveName) + 4, 10);
	lcd.setDefaultBackgroundColor();
	lcd.drawStr(2, yOffset + 9, revolveName);
	lcd.setDefaultForegroundColor();

	// Position
	lcd.drawStr(lcd.getStrWidth(revolveName) + 6, yOffset + 9, "Position: ");
	lcd.setPrintPos(lcd.getStrWidth("INNERPosition:") + 4, yOffset + 9);

	if (cursorEnable && menu_pos == 0) {
		lcd.drawBox(lcd.getStrWidth("INNERPosition: ") + 3, yOffset + 1, 19, 9);
		lcd.setDefaultBackgroundColor();
	}
	lcd.print(values[0]);
	lcd.setDefaultForegroundColor();

	// Speed
	lcd.drawStr(2, yOffset + 19, "Speed: ");
	lcd.setPrintPos(lcd.getStrWidth("Speed: "), yOffset + 19);

	if (cursorEnable && menu_pos == 1) {
		lcd.drawBox(lcd.getStrWidth("Speed: ") - 1, yOffset + 11, 19, 9);
		lcd.setDefaultBackgroundColor();
	}
	lcd.print(values[1]);
	lcd.setDefaultForegroundColor();

	// Acceleration
	lcd.drawStr(70, 51, "Acc:  ");
	lcd.setPrintPos(lcd.getStrWidth("Acc:  ") + 70, yOffset + 19);

	if (cursorEnable && menu_pos == 2) {
		lcd.drawBox(lcd.getStrWidth("Acc:  ") + 69, yOffset + 11, 19, 9);
		lcd.setDefaultBackgroundColor();
	}
	lcd.print(values[2]);
	lcd.setDefaultForegroundColor();

	// Direction
	lcd.drawStr(2, yOffset + 29, "Dir: ");
	lcd.setPrintPos(lcd.getStrWidth("Speed: "), yOffset + 29);

	if (cursorEnable && menu_pos == 3) {
		lcd.drawBox(lcd.getStrWidth("Speed: ") - 1, yOffset + 29, 19, 9);
		lcd.setDefaultBackgroundColor();
	}
	lcd.print(values[3] ? "CCW" : "CW");
	lcd.setDefaultForegroundColor();

	// Extra Revolutions
	lcd.drawStr(70, yOffset + 29, "Revs: ");
	lcd.setPrintPos(lcd.getStrWidth("Acc:  ") + 70, yOffset + 29);

	if (cursorEnable && menu_pos == 4) {
		lcd.drawBox(lcd.getStrWidth("Acc:  ") + 69, yOffset + 21, 13, 9);
		lcd.setDefaultBackgroundColor();
	}
	lcd.print(values[4]);
	lcd.setDefaultForegroundColor();
}

void Displays::drawCueLayout(U8GLIB_ST7920_128X64& lcd, int(values)[10], int cursorEnable) const {
	lcd.setFont(font);

	auto menu_pos_shift = interface.menu_pos;
	if (state->state != STATE_MANUAL && interface.cueParams[1] == 0) {
		// Shift menu_pos by 5 if inner disabled
		menu_pos_shift += 5;
	}

	// Only draw if enabled
	if (state->state == STATE_MANUAL || (state->state != MAN && interface.cueParams[1] == 1)) {
		drawWheelCueDetails(lcd, &(values[0]), cursorEnable, menu_pos_shift, 0, "INNER");
	}

	// Only draw if enabled
	if (state->state == STATE_MANUAL || (state->state != STATE_MANUAL && interface.cueParams[2] == 1)) {
		drawWheelCueDetails(lcd, &(values[5]), cursorEnable, menu_pos_shift, 32, "OUTER");
	}
}

void Displays::drawParamsLayout(U8GLIB_ST7920_128X64& lcd, int cursorEnable) const {
	displayCenter.setFont(font);
	displayCenter.drawStr(4, 10, "Cue Number:");
	displayCenter.setPrintPos(90, 10);
	if (interface.menu_pos == 0 && cursorEnable) {
		displayCenter.drawBox(88, 1, 27, 11);
		displayCenter.setDefaultBackgroundColor();
	}

	if (interface.cueNumber - floor(interface.cueNumber) ==
	    0)  // Don't display something like 1.0 (but do display 2.4)
		displayCenter.print(interface.cueNumber, 0);
	else
		displayCenter.print(interface.cueNumber, 1);
	displayCenter.setDefaultForegroundColor();

	for (int i = 0; i < 3; i++) {
		displayCenter.drawStr(4, ((i + 2) * 10), param_strings[i]);
		displayCenter.setPrintPos(90, ((i + 2) * 10));

		if ((interface.menu_pos - 1) == i && cursorEnable) {
			displayCenter.drawBox(88, 1 + (i + 1) * 10, 21, 11);
			displayCenter.setDefaultBackgroundColor();
		}

		if (interface.cueParams[i] == 0)
			displayCenter.print("NO");
		else
			displayCenter.print("YES");
		displayCenter.setDefaultForegroundColor();
	}

	if (interface.menu_pos == 4 && cursorEnable) {
		displayCenter.drawBox(0, 41, SCREEN_WIDTH, 11);
		displayCenter.setDefaultBackgroundColor();
	}
	drawStrCenter(displayCenter, 50, "Add Cue");
	displayCenter.setDefaultForegroundColor();

	if (interface.menu_pos == 5 && cursorEnable) {
		displayCenter.drawBox(0, 51, SCREEN_WIDTH, 11);
		displayCenter.setDefaultBackgroundColor();
	}
	drawStrCenter(displayCenter, 60, "Delete Cue");
	displayCenter.setDefaultForegroundColor();
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

void Displays::drawLeftDisplay() const {
	displayLeft.setFont(font);

	switch (state->state) {
	case STATE_HOMING_INPROGRESS:
		displayLeft.setFont(large_font);
		drawStrCenter(displayLeft, 15, TITLE);
		drawStrCenter(displayLeft, 30, SUBTITLE);
		displayLeft.setFont(font);
		drawStrCenter(displayLeft, 45, NAME);
		displayLeft.setFont(small_font);
		drawStrCenter(displayLeft, 55, DATE);
		break;

	case STATE_SHOW:
		drawCueLayout(displayLeft, interface.cueMovements, 0);
		break;

	case STATE_PROGRAM_MAIN:
	case STATE_PROGRAM_PARAMS:
	case STATE_PROGRAM_CUELIST:
	case STATE_PROGRAM_DELETE:
	case STATE_PROGRAM_GOTOCUE:
	case STATE_PROGRAM_SAVED:
		// Add box if screen selected in PROGRAM
		if (interface.menu_pos == 0 && state->state == STATE_PROGRAM_MAIN) {
			displayLeft.setDefaultForegroundColor();
			displayLeft.drawFrame(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			displayLeft.drawFrame(1, 1, SCREEN_WIDTH - 2, SCREEN_HEIGHT - 2);
			displayLeft.setDefaultForegroundColor();
		}
		drawCueLayout(displayLeft, interface.cueMovements, 0);
		break;

	case STATE_PROGRAM_MOVEMENTS:
		drawCueLayout(displayLeft, interface.cueMovements, 1);
		break;

	case STATE_ESTOP:
		displayLeft.setFont(xlarge_font);
		drawStrCenter(displayLeft, 16, "ESTOP");
		displayLeft.setFont(large_font);
		drawStrCenter(displayLeft, 32, "Reset all Estops");
		drawStrCenter(displayLeft, 48, "to continue");
		break;

	default:
		displayCenter.drawXBMP(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, screen_logo);
		break;
	}
}

void Displays::drawCenterDisplay() const {
	displayCenter.setFont(font);

	switch (state->state) {
	case STATE_HOMING_INPROGRESS:
		displayCenter.drawXBMP(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, screen_logo);
		break;
	case STATE_MAINMENU:
		displayCenter.setFont(large_font);

		for (int i = 0; i < 4; i++) {
			if (i == interface.menu_pos) {
				// Position highlight box from top left corner
				displayCenter.drawBox(0, (interface.menu_pos * 16), SCREEN_WIDTH, 16);
				displayCenter.setDefaultBackgroundColor();
				drawStrCenter(displayCenter, (i * 16) + 12, menu_strings[i]);
			}
			drawStrCenter(displayCenter, (i * 16) + 12, menu_strings[i]);
			displayCenter.setDefaultForegroundColor();
		}
		break;

	case STATE_MANUAL:
		drawCueLayout(displayCenter, interface.currentMovements, 1);
		break;

	case STATE_SHOW:
		drawCuelistLayout(displayCenter, interface.menu_pos, 1);
		break;

	case STATE_PROGRAM_MAIN:
	case STATE_PROGRAM_MOVEMENTS:
	case STATE_PROGRAM_CUELIST:
	case STATE_PROGRAM_GOTOCUE:
		// Add box if screen selected in PROGRAM
		if (interface.menu_pos == 1 && state->state == STATE_PROGRAM_MAIN) {
			displayCenter.setDefaultForegroundColor();
			displayCenter.drawFrame(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			displayCenter.drawFrame(1, 1, SCREEN_WIDTH - 2, SCREEN_HEIGHT - 2);
			displayCenter.setDefaultForegroundColor();
		}
		drawParamsLayout(displayCenter, 0);
		break;

	case STATE_PROGRAM_DELETE:
		displayCenter.setFont(large_font);
		drawStrCenter(displayCenter, 35, "Delete Cue?");
		displayCenter.setFont(font);
		break;

	case STATE_PROGRAM_SAVED:
		displayCenter.setFont(large_font);
		drawStrCenter(displayCenter, 35, "Cuestack Saved");
		displayCenter.setFont(font);
		break;

	case STATE_PROGRAM_PARAMS:
		drawParamsLayout(displayCenter, 1);
		break;

	case STATE_SETTINGS:
		displayCenter.setFont(large_font);
		if (interface.menu_pos < 4) {
			for (int i = 0; i < 4; i++) {
				if (i == interface.menu_pos) {
					// Position highlight box from top left corner
					displayCenter.drawBox(0, (interface.menu_pos * 16), SCREEN_WIDTH, 16);
					displayCenter.setDefaultBackgroundColor();
					drawStrCenter(displayCenter, (i * 16) + 12, settings_strings[i]);
				}
				drawStrCenter(displayCenter, (i * 16) + 12, settings_strings[i]);
				displayCenter.setDefaultForegroundColor();
			}
		} else if (interface.menu_pos < 8) {
			for (int i = 0; i < 4; i++) {
				if ((i + 4) == interface.menu_pos) {
					// Position highlight box from top left corner
					displayCenter.drawBox(0, (i * 16), SCREEN_WIDTH, 16);
					displayCenter.setDefaultBackgroundColor();
					drawStrCenter(displayCenter, (i * 16) + 12, settings_strings[i + 4]);
				}
				drawStrCenter(displayCenter, (i * 16) + 12, settings_strings[i + 4]);
				displayCenter.setDefaultForegroundColor();
			}
		}

		break;

	case STATE_HARDWARETEST:
		displayCenter.setFont(large_font);
		drawStrCenter(displayCenter, 16, "Hardware Test");
		displayCenter.setFont(font);
		drawStrCenter(displayCenter, 40, "Hold Go and");
		drawStrCenter(displayCenter, 48, "Pause to exit");
		break;

	case STATE_BRIGHTNESS:
		displayCenter.setFont(large_font);

		for (int i = 0; i < 4; i++) {
			displayCenter.drawStr(0, 13 + (i * 16), led_settings_strings[i]);
			displayCenter.setPrintPos(90, 13 + (i * 16));

			if (interface.menu_pos == i) {
				displayCenter.drawBox(88, i * 16, 24, 16);
				displayCenter.setDefaultBackgroundColor();
			}

			displayCenter.print(interface.leds.ledSettings[i]);
			displayCenter.setDefaultForegroundColor();
		}
		displayCenter.setFont(font);
		break;

	case STATE_ENCSETTINGS:
		displayCenter.setFont(large_font);

		for (int i = 0; i < 4; i++) {
			displayCenter.drawStr(0, 13 + (i * 16), enc_settings_strings[i]);
			displayCenter.setPrintPos(90, 13 + (i * 16));

			if (interface.menu_pos == i) {
				displayCenter.drawBox(88, i * 16, 40, 16);
				displayCenter.setDefaultBackgroundColor();
			}

			if (interface.encSettings[i] == 0 && (i == 0 || i == 1))
				displayCenter.print("FWD");
			else if (interface.encSettings[i] == 1 && (i == 0 || i == 1))
				displayCenter.print("REV");
			else
				displayCenter.print(interface.encSettings[i]);
			displayCenter.setDefaultForegroundColor();
		}
		displayCenter.setFont(font);
		break;

	case STATE_ESTOP:
		displayCenter.setFont(xlarge_font);
		drawStrCenter(displayCenter, 16, "ESTOP");
		displayCenter.setFont(large_font);
		drawStrCenter(displayCenter, 32, "Reset all Estops");
		drawStrCenter(displayCenter, 48, "to continue");
		break;

	case STATE_DEFAULTVALUES:
		drawCueLayout(displayCenter, interface.defaultValues, 0);
		break;

	case STATE_KPSETTINGS:
		displayCenter.setFont(font);

		for (int i = 0; i < 6; i++) {
			displayCenter.drawStr(0, 10 + (i * 10), kp_settings_strings[i]);
			displayCenter.setPrintPos(90, 10 + (i * 10));

			if (interface.menu_pos == i) {
				displayCenter.drawBox(88, (i * 10) + 2, 34, 10);
				displayCenter.setDefaultBackgroundColor();
			}

			displayCenter.print(interface.kpSettings[i], 3);
			displayCenter.setDefaultForegroundColor();
		}
		displayCenter.setFont(font);
		break;

	case STATE_RESET_CUESTACK:
		displayCenter.setFont(large_font);
		drawStrCenter(displayCenter, 19, "Reset");
		drawStrCenter(displayCenter, 35, "ENTIRE CUESTACK?");
		displayCenter.setFont(font);
		drawStrCenter(displayCenter, 50, "Press Go, Pause and");
		drawStrCenter(displayCenter, 60, "Select to Continue");
		break;
	default:
		break;
	}
}

void Displays::drawRightDisplay() const {
	displayRight.setFont(font);

	switch (state->state) {
	case STATE_HOMING_INPROGRESS:
		displayRight.setFont(xlarge_font);
		drawStrCenter(displayRight, 20, "HOMING");
		displayRight.setFont(font);
		drawStrCenter(displayRight, 40, "Please wait");
		break;
	case STATE_HARDWARETEST:
		displayRight.setFont(xlarge_font);

		if (InputButtonsInterface::goEngaged()) {
			drawStrCenter(displayRight, 40, "GO");
		} else if (InputButtonsInterface::dmhEngaged()) {
			drawStrCenter(displayRight, 40, "DMH");
		} else if (digitalRead(BACK) == HIGH) {
			drawStrCenter(displayRight, 40, "BACK");
		} else if (digitalRead(SELECT) == LOW) {
			drawStrCenter(displayRight, 40, "SELECT");
		} else if (interface.input.currentKey) {
			drawStrCenter(displayRight, 40, interface.input.currentKey);
		}
		displayRight.setFont(font);
		break;

	case STATE_ESTOP:
		displayRight.setFont(xlarge_font);
		drawStrCenter(displayRight, 16, "ESTOP");
		displayRight.setFont(large_font);
		drawStrCenter(displayRight, 32, "Reset all Estops");
		drawStrCenter(displayRight, 48, "to continue");
		break;

	case STATE_MANUAL:
	case STATE_SHOW:
	case STATE_PROGRAM_GOTOCUE:
		displayRight.setFont(large_font);
		displayRight.drawStr(0, 10, "Inner");
		displayRight.setFont(font);
		displayRight.drawStr(0, 20, "Position: ");
		displayRight.setPrintPos(displayRight.getStrWidth("Position: "), 20);
		displayRight.print(inner.displayPos());
		displayRight.drawStr(0, 28, "Speed: ");
		displayRight.setPrintPos(displayRight.getStrWidth("Speed: "), 28);
		displayRight.print(inner.getSpeed());

		displayRight.setFont(large_font);
		displayRight.drawStr(0, 42, "Outer");
		displayRight.setFont(font);
		displayRight.drawStr(0, 52, "Position: ");
		displayRight.setPrintPos(displayRight.getStrWidth("Position: "), 52);
		displayRight.print(outer.displayPos());
		displayRight.drawStr(0, 60, "Speed: ");
		displayRight.setPrintPos(displayRight.getStrWidth("Speed: "), 60);
		displayRight.print(outer.getSpeed());
		break;

	case STATE_PROGRAM_MAIN:
	case STATE_PROGRAM_MOVEMENTS:
	case STATE_PROGRAM_PARAMS:
	case STATE_PROGRAM_DELETE:
	case STATE_PROGRAM_SAVED:
		// Add box if screen selected in PROGRAM
		if (interface.menu_pos == 2 && state->state == STATE_PROGRAM_MAIN) {
			displayRight.setDefaultForegroundColor();
			displayRight.drawFrame(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			displayRight.drawFrame(1, 1, SCREEN_WIDTH - 2, SCREEN_HEIGHT - 2);
			displayRight.setDefaultForegroundColor();
		}
		drawCuelistLayout(displayRight, cuestack.currentCue, 1);
		break;

	case STATE_PROGRAM_CUELIST:
		drawCuelistLayout(displayRight, interface.menu_pos, 1);
		break;

	default:
		displayRight.drawXBMP(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, screen_logo);
		break;
	}
}

void Displays::updateRingLeds() {

	switch (state->state) {
	case STATE_HOMING_INPROGRESS:
	case STATE_RESET_CUESTACK:
		interface.leds.ringLedsColor(255, 0, 0);
		break;

	case STATE_HARDWARETEST:
		interface.leds.ringLedsColor(255, 255, 255);
		break;

	case STATE_HOMING_COMPLETE:
	case STATE_PROGRAM_SAVED:
		interface.leds.ringLedsColor(0, 255, 0);
		break;

	case STATE_ESTOP:
		interface.leds.ringLedsColor(255, 0, 0);
		break;

	case STATE_MANUAL:
	case STATE_SHOW:
	case STATE_PROGRAM_GOTOCUE:
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
			displayLeft.firstPage();
			do {
				drawLeftDisplay();
			} while (displayLeft.nextPage());
		}

		if (menu1) {
			displayCenter.firstPage();
			do {
				drawCenterDisplay();
			} while (displayCenter.nextPage());
		}

		if (info1) {
			displayRight.firstPage();
			do {
				drawRightDisplay();
			} while (displayRight.nextPage());
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

#include "displays.h"
#include "logo.h"
#include "pins.h"
#include "state.h"
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
	displayCentre(menu),
	displayRight(info),
	ringLeds(ringLeds),
	inner(inner),
	outer(outer),
	keypad(keypad),
	interface(interface),
	cuestack(cuestack) {
}

void Displays::setup() {
	displayLeft.begin();
	displayLeft.setColorIndex(1);

	displayCentre.begin();
	displayCentre.setColorIndex(1);

	displayRight.begin();
	displayRight.setColorIndex(1);

	interface.menu_pos = 0;
	updateDisplays(1, 1, 1, 1);
}

void Displays::loop() {
	updateDisplays(true, true, true, true);
}

void Displays::setMode() {
	// Reset input encoder in case value has accrued
	interface.input.getInputEncoder();
	forceUpdateDisplays(1, 1, 1, 1);
}

void Displays::drawStrCentre(U8GLIB_ST7920_128X64& lcd, int y, const char* text) {
	int width = lcd.getStrWidth(text);
	int x = (SCREEN_WIDTH - width) / 2;
	lcd.setPrintPos(x, y);
	lcd.print(text);
}

void Displays::drawStrCentre(U8GLIB_ST7920_128X64& lcd, int y, char text) {
	const char string[] = { text, '\0' };
	drawStrCentre(lcd, y, string);
}

void Displays::drawWheelCueDetails(
	U8GLIB_ST7920_128X64& lcd, int(values)[5], int cursorEnable, int menu_pos, int menu_pos_offset, int yOffset, const char* revolveName)
	const {
	lcd.drawHLine(0, yOffset, SCREEN_WIDTH);

	// Label
	lcd.drawBox(0, yOffset + 1, lcd.getStrWidth(revolveName) + 4, 10);
	lcd.setDefaultBackgroundColor();
	lcd.drawStr(2, yOffset + 9, revolveName);
	lcd.setDefaultForegroundColor();

	// Position
	lcd.drawStr(lcd.getStrWidth(revolveName) + 6, yOffset + 9, "Position: ");
	lcd.setPrintPos(lcd.getStrWidth("INNERPosition: ") + 4, yOffset + 9);

	if (cursorEnable && menu_pos == 0 + menu_pos_offset) {
		lcd.drawBox(lcd.getStrWidth("INNERPosition: ") + 3, yOffset + 1, 19, 9);
		lcd.setDefaultBackgroundColor();
	}
	lcd.print(values[0]);
	lcd.setDefaultForegroundColor();

	// Speed
	lcd.drawStr(2, yOffset + 19, "Speed: ");
	lcd.setPrintPos(lcd.getStrWidth("Speed: "), yOffset + 19);

	if (cursorEnable && menu_pos == 1 + menu_pos_offset) {
		lcd.drawBox(lcd.getStrWidth("Speed: ") - 1, yOffset + 11, 19, 9);
		lcd.setDefaultBackgroundColor();
	}
	lcd.print(values[1]);
	lcd.setDefaultForegroundColor();

	// Acceleration
	lcd.drawStr(70, 51, "Acc:  ");
	lcd.setPrintPos(lcd.getStrWidth("Acc:  ") + 70, yOffset + 19);

	if (cursorEnable && menu_pos == 2 + menu_pos_offset) {
		lcd.drawBox(lcd.getStrWidth("Acc:  ") + 69, yOffset + 11, 19, 9);
		lcd.setDefaultBackgroundColor();
	}
	lcd.print(values[2]);
	lcd.setDefaultForegroundColor();

	// Direction
	lcd.drawStr(2, yOffset + 29, "Dir: ");
	lcd.setPrintPos(lcd.getStrWidth("Speed: "), yOffset + 29);

	if (cursorEnable && menu_pos == 3 + menu_pos_offset) {
		lcd.drawBox(lcd.getStrWidth("Speed: ") - 1, yOffset + 21, 19, 9);
		lcd.setDefaultBackgroundColor();
	}
	lcd.print(values[3] ? "CCW" : "CW");
	lcd.setDefaultForegroundColor();

	// Extra Revolutions
	lcd.drawStr(70, yOffset + 29, "Revs: ");
	lcd.setPrintPos(lcd.getStrWidth("Acc:  ") + 70, yOffset + 29);

	if (cursorEnable && menu_pos == 4 + menu_pos_offset) {
		lcd.drawBox(lcd.getStrWidth("Acc:  ") + 69, yOffset + 21, 13, 9);
		lcd.setDefaultBackgroundColor();
	}
	lcd.print(values[4]);
	lcd.setDefaultForegroundColor();
}

void Displays::drawCueLayout(U8GLIB_ST7920_128X64& lcd, int(values)[10], int cursorEnable) const {
	lcd.setFont(font);

	auto menu_pos_shift = interface.menu_pos;
	if (state->state != STATE_MANUAL_READY && interface.cueParams[1] == 0) {
		// Shift menu_pos by 5 if inner disabled
		menu_pos_shift += 5;
	}

	// Only draw if enabled
	if (state->state == STATE_MANUAL_READY || (state->state != MAN && interface.cueParams[1] == 1)) {
		drawWheelCueDetails(lcd, &(values[0]), cursorEnable, menu_pos_shift, 0, 0, "INNER");
	}

	// Only draw if enabled
	if (state->state == STATE_MANUAL_READY || (state->state != STATE_MANUAL_READY && interface.cueParams[2] == 1)) {
		drawWheelCueDetails(lcd, &(values[5]), cursorEnable, menu_pos_shift, 5, 32, "OUTER");
	}
}

void Displays::drawParamsLayout(U8GLIB_ST7920_128X64& lcd, int cursorEnable) const {
	displayCentre.setFont(font);
	displayCentre.drawStr(4, 10, "Cue Number:");
	displayCentre.setPrintPos(90, 10);
	if (interface.menu_pos == 0 && cursorEnable) {
		displayCentre.drawBox(88, 1, 27, 11);
		displayCentre.setDefaultBackgroundColor();
	}

	if (interface.cueNumber - floor(interface.cueNumber) ==
		0)  // Don't display something like 1.0 (but do display 2.4)
		displayCentre.print(interface.cueNumber, 0);
	else
		displayCentre.print(interface.cueNumber, 1);
	displayCentre.setDefaultForegroundColor();

	for (int i = 0; i < 3; i++) {
		displayCentre.drawStr(4, ((i + 2) * 10), param_strings[i]);
		displayCentre.setPrintPos(90, ((i + 2) * 10));

		if ((interface.menu_pos - 1) == i && cursorEnable) {
			displayCentre.drawBox(88, 1 + (i + 1) * 10, 21, 11);
			displayCentre.setDefaultBackgroundColor();
		}

		if (interface.cueParams[i] == 0)
			displayCentre.print("NO");
		else
			displayCentre.print("YES");
		displayCentre.setDefaultForegroundColor();
	}

	if (interface.menu_pos == 4 && cursorEnable) {
		displayCentre.drawBox(0, 41, SCREEN_WIDTH, 11);
		displayCentre.setDefaultBackgroundColor();
	}
	drawStrCentre(displayCentre, 50, "Add Cue");
	displayCentre.setDefaultForegroundColor();

	if (interface.menu_pos == 5 && cursorEnable) {
		displayCentre.drawBox(0, 51, SCREEN_WIDTH, 11);
		displayCentre.setDefaultBackgroundColor();
	}
	drawStrCentre(displayCentre, 60, "Delete Cue");
	displayCentre.setDefaultForegroundColor();
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
	case STATE_DEBUG:
		displayLeft.drawStr(0, 10, "Left");
		break;
	case STATE_HOMING_INPROGRESS:
		displayLeft.setFont(large_font);
		drawStrCentre(displayLeft, 15, TITLE);
		drawStrCentre(displayLeft, 30, SUBTITLE);
		displayLeft.setFont(font);
		drawStrCentre(displayLeft, 45, NAME);
		displayLeft.setFont(small_font);
		drawStrCentre(displayLeft, 55, DATE);
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
		drawStrCentre(displayLeft, 16, "ESTOP");
		displayLeft.setFont(large_font);
		drawStrCentre(displayLeft, 32, "Reset all Estops");
		drawStrCentre(displayLeft, 48, "to continue");
		break;

	default:
		displayCentre.drawXBMP(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, screen_logo);
		break;
	}
}

void Displays::drawCentreDisplay() const {
	displayCentre.setFont(font);

	switch (state->state) {
	case STATE_DEBUG:
		displayCentre.drawStr(0, 10, "Centre");
		break;
	case STATE_HOMING_INPROGRESS:
		displayCentre.drawXBMP(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, screen_logo);
		break;
	case STATE_MAINMENU:
		displayCentre.setFont(large_font);

		for (int i = 0; i < 4; i++) {
			if (i == interface.menu_pos) {
				// Position highlight box from top left corner
				displayCentre.drawBox(0, (interface.menu_pos * 16), SCREEN_WIDTH, 16);
				displayCentre.setDefaultBackgroundColor();
				drawStrCentre(displayCentre, (i * 16) + 12, menu_strings[i]);
			}
			drawStrCentre(displayCentre, (i * 16) + 12, menu_strings[i]);
			displayCentre.setDefaultForegroundColor();
		}
		break;

	case STATE_MANUAL_READY:
		drawCueLayout(displayCentre, interface.currentMovements, 1);
		break;

	case STATE_SHOW:
		drawCuelistLayout(displayCentre, interface.menu_pos, 1);
		break;

	case STATE_PROGRAM_MAIN:
	case STATE_PROGRAM_MOVEMENTS:
	case STATE_PROGRAM_CUELIST:
	case STATE_PROGRAM_GOTOCUE:
		// Add box if screen selected in PROGRAM
		if (interface.menu_pos == 1 && state->state == STATE_PROGRAM_MAIN) {
			displayCentre.setDefaultForegroundColor();
			displayCentre.drawFrame(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			displayCentre.drawFrame(1, 1, SCREEN_WIDTH - 2, SCREEN_HEIGHT - 2);
			displayCentre.setDefaultForegroundColor();
		}
		drawParamsLayout(displayCentre, 0);
		break;

	case STATE_PROGRAM_DELETE:
		displayCentre.setFont(large_font);
		drawStrCentre(displayCentre, 35, "Delete Cue?");
		displayCentre.setFont(font);
		break;

	case STATE_PROGRAM_SAVED:
		displayCentre.setFont(large_font);
		drawStrCentre(displayCentre, 35, "Cuestack Saved");
		displayCentre.setFont(font);
		break;

	case STATE_PROGRAM_PARAMS:
		drawParamsLayout(displayCentre, 1);
		break;

	case STATE_SETTINGS:
		displayCentre.setFont(large_font);
		if (interface.menu_pos < 4) {
			for (int i = 0; i < 4; i++) {
				if (i == interface.menu_pos) {
					// Position highlight box from top left corner
					displayCentre.drawBox(0, (interface.menu_pos * 16), SCREEN_WIDTH, 16);
					displayCentre.setDefaultBackgroundColor();
					drawStrCentre(displayCentre, (i * 16) + 12, settings_strings[i]);
				}
				drawStrCentre(displayCentre, (i * 16) + 12, settings_strings[i]);
				displayCentre.setDefaultForegroundColor();
			}
		}
		else if (interface.menu_pos < 8) {
			for (int i = 0; i < 4; i++) {
				if ((i + 4) == interface.menu_pos) {
					// Position highlight box from top left corner
					displayCentre.drawBox(0, (i * 16), SCREEN_WIDTH, 16);
					displayCentre.setDefaultBackgroundColor();
					drawStrCentre(displayCentre, (i * 16) + 12, settings_strings[i + 4]);
				}
				drawStrCentre(displayCentre, (i * 16) + 12, settings_strings[i + 4]);
				displayCentre.setDefaultForegroundColor();
			}
		}

		break;

	case STATE_HARDWARETEST:
		displayCentre.setFont(large_font);
		drawStrCentre(displayCentre, 16, "Hardware Test");
		displayCentre.setFont(font);
		drawStrCentre(displayCentre, 40, "Hold Go and");
		drawStrCentre(displayCentre, 48, "Pause to exit");
		break;

	case STATE_BRIGHTNESS:
		displayCentre.setFont(large_font);

		for (int i = 0; i < 4; i++) {
			displayCentre.drawStr(0, 13 + (i * 16), led_settings_strings[i]);
			displayCentre.setPrintPos(90, 13 + (i * 16));

			if (interface.menu_pos == i) {
				displayCentre.drawBox(88, i * 16, 24, 16);
				displayCentre.setDefaultBackgroundColor();
			}

			displayCentre.print(interface.leds.ledSettings[i]);
			displayCentre.setDefaultForegroundColor();
		}
		displayCentre.setFont(font);
		break;

	case STATE_ENCSETTINGS:
		displayCentre.setFont(large_font);

		for (int i = 0; i < 4; i++) {
			displayCentre.drawStr(0, 13 + (i * 16), enc_settings_strings[i]);
			displayCentre.setPrintPos(90, 13 + (i * 16));

			if (interface.menu_pos == i) {
				displayCentre.drawBox(88, i * 16, 40, 16);
				displayCentre.setDefaultBackgroundColor();
			}

			if (interface.encSettings[i] == 0 && (i == 0 || i == 1))
				displayCentre.print("FWD");
			else if (interface.encSettings[i] == 1 && (i == 0 || i == 1))
				displayCentre.print("REV");
			else
				displayCentre.print(interface.encSettings[i]);
			displayCentre.setDefaultForegroundColor();
		}
		displayCentre.setFont(font);
		break;

	case STATE_ESTOP:
		displayCentre.setFont(xlarge_font);
		drawStrCentre(displayCentre, 16, "ESTOP");
		displayCentre.setFont(large_font);
		drawStrCentre(displayCentre, 32, "Reset all Estops");
		drawStrCentre(displayCentre, 48, "to continue");
		break;

	case STATE_DEFAULTVALUES:
		drawCueLayout(displayCentre, interface.defaultValues, 0);
		break;

	case STATE_KPSETTINGS:
		displayCentre.setFont(font);

		for (int i = 0; i < 6; i++) {
			displayCentre.drawStr(0, 10 + (i * 10), kp_settings_strings[i]);
			displayCentre.setPrintPos(90, 10 + (i * 10));

			if (interface.menu_pos == i) {
				displayCentre.drawBox(88, (i * 10) + 2, 34, 10);
				displayCentre.setDefaultBackgroundColor();
			}

			displayCentre.print(interface.kpSettings[i], 3);
			displayCentre.setDefaultForegroundColor();
		}
		displayCentre.setFont(font);
		break;

	case STATE_RESET_CUESTACK:
		displayCentre.setFont(large_font);
		drawStrCentre(displayCentre, 19, "Reset");
		drawStrCentre(displayCentre, 35, "ENTIRE CUESTACK?");
		displayCentre.setFont(font);
		drawStrCentre(displayCentre, 50, "Press Go, Pause and");
		drawStrCentre(displayCentre, 60, "Select to Continue");
		break;
	default:
		break;
	}
}

void Displays::drawRightDisplay() const {
	displayRight.setFont(font);

	switch (state->state) {
	case STATE_DEBUG:
		displayRight.drawStr(0, 10, "Right");
		char buffer[16];
		snprintf(
			buffer,
			16,
			"%i%i%i%i%i%i%i%i%i",
			Buttons::go.engaged(),
			Buttons::dmh.engaged(),
			Buttons::select.engaged(),
			Buttons::back.engaged(),
			Buttons::e_stop.engaged(),
			Buttons::e_stop.nc1_b.engaged(),
			Buttons::e_stop.nc2_b.engaged(),
			Buttons::e_stop.nc3_b.engaged(),
			Buttons::e_stop.no_b.engaged());
		displayRight.drawStr(0, 20, buffer);
		snprintf(buffer, 16, "%i", interface.input.currentKey);
		displayRight.drawStr(0, 30, buffer);
		break;
	case STATE_HOMING_INPROGRESS:
		displayRight.setFont(xlarge_font);
		drawStrCentre(displayRight, 20, "HOMING");
		displayRight.setFont(font);
		drawStrCentre(displayRight, 40, "Please wait");
		break;
	case STATE_HARDWARETEST:
		displayRight.setFont(xlarge_font);

		if (Buttons::go.engaged()) {
			drawStrCentre(displayRight, 40, "GO");
		}
		else if (Buttons::dmh.engaged()) {
			drawStrCentre(displayRight, 40, "DMH");
		}
		else if (digitalRead(BACK) == HIGH) {
			drawStrCentre(displayRight, 40, "BACK");
		}
		else if (digitalRead(SELECT) == LOW) {
			drawStrCentre(displayRight, 40, "SELECT");
		}
		else if (interface.input.currentKey) {
			drawStrCentre(displayRight, 40, interface.input.currentKey);
		}
		displayRight.setFont(font);
		break;

	case STATE_ESTOP:
		displayRight.setFont(xlarge_font);
		drawStrCentre(displayRight, 16, "ESTOP");
		displayRight.setFont(large_font);
		drawStrCentre(displayRight, 32, "Reset all Estops");
		drawStrCentre(displayRight, 48, "to continue");
		break;

	case STATE_MANUAL_READY:
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

	case STATE_MANUAL_READY:
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
		}
		else if (ledOuter == 11) {
			ringLeds.setPixelColor(0, 100, 75, 0);
			ringLeds.setPixelColor(10, 100, 75, 0);
		}
		else {
			ringLeds.setPixelColor(ledOuter - 1, 100, 75, 0);
			ringLeds.setPixelColor(ledOuter + 1, 100, 75, 0);
		}

		if (ledInner == 12) {
			ringLeds.setPixelColor(13, 100, 75, 0);
			ringLeds.setPixelColor(23, 100, 75, 0);
		}
		else if (ledInner == 23) {
			ringLeds.setPixelColor(12, 100, 75, 0);
			ringLeds.setPixelColor(22, 100, 75, 0);
		}
		else {
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
			displayCentre.firstPage();
			do {
				drawCentreDisplay();
			} while (displayCentre.nextPage());
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

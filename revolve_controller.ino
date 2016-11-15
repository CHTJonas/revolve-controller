#include "navigation.h"
#include "revolve_controller.h"
#include "stage.h"
#include "state.h"
#include <EEPROM.h>
#include <TimerOne.h>

char keys[4][3] = { { '1', '2', '3' }, { '4', '5', '6' }, { '7', '8', '9' }, { '*', '0', '#' } };
byte ROWS[4] = { KEY1, KEY2, KEY3, KEY4 };
byte COLS[3] = { KEY5, KEY6, KEY7 };
Keypad keypad = Keypad(makeKeymap(keys), ROWS, COLS, 4, 3);

Adafruit_NeoPixel pauseLeds = Adafruit_NeoPixel(2, PAUSELEDS, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ringLeds = Adafruit_NeoPixel(24, RINGLEDS, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel keypadLeds = Adafruit_NeoPixel(4, KEYPADLEDS, NEO_GRB + NEO_KHZ800);

Encoder enc_inner(INNERENC1, INNERENC2);
Encoder enc_outer(OUTERENC1, OUTERENC2);
Encoder enc_input(INPUTENC1, INPUTENC2);

U8GLIB_ST7920_128X64 cue1(22, 24, 26, U8G_PIN_NONE);
U8GLIB_ST7920_128X64 menu(23, 25, 27, U8G_PIN_NONE);
U8GLIB_ST7920_128X64 info(32, 34, 36, U8G_PIN_NONE);

State state = State{.state = STATE_MAINMENU, .data = {.mainmenu = {} } };
Revolve inner(4, 5, 6, enc_inner);
Revolve outer(11, 10, 9, enc_outer);
Cuestack cuestack;
Interface interface(cuestack, enc_input, keypad, ringLeds, pauseLeds, keypadLeds);
Displays displays(&state, cue1, menu, info, ringLeds, inner, outer, keypad, interface, cuestack);
Stage stage(&state, &inner, &outer, &displays, &interface, &ringLeds);
Navigation navigation(&state, &cuestack, &interface, &displays, &stage);

void setup();
void loop();

char* encodeCue(Cue cue);
void updateFlags();
void goToCurrentCue(int target_mode);
void updateSetting(void (*settingLimiter)(void), int mode);
void brightnessLimiter();
void encoderLimiter();
void eepromLimiter();
void kpLimiter();
void manualLimiter();
void movementLimiter();
void setup();
void loop();

char* encodeCue(Cue cue);
void updateFlags();
void goToCurrentCue(int target_mode);
void updateSetting(void (*settingLimiter)(void), int mode);
void brightnessLimiter();
void encoderLimiter();
void eepromLimiter();
void kpLimiter();
void manualLimiter();
void movementLimiter();

void setup() {
	Timer1.initialize(100000);
	Timer1.attachInterrupt(updateFlags);

	Serial.begin(1200);

	interface.leds.setup();
	interface.setup();
	cuestack.setup();
	displays.setup();
	navigation.setup();
}

void loop() {
	stage.loop();
	displays.loop();
	navigation.loop();
}

void goToCurrentCue(int target_mode) {
	// Goto current cue if Go and Pause pressed
	if (Buttons::dmh.engaged() && Buttons::go.engaged()) {
		// Update displays to show realtime position
		state.state = STATE_PROGRAM_GOTOCUE;
		state.data.program_gotocue = {};
		displays.setMode();

		// Move - both enabled
		if (interface.cueParams[1] && interface.cueParams[2]) {
			// stage.gotoPos();
		}
		// Move - inner disabled
		else if (interface.cueParams[1] == 0 && interface.cueParams[2]) {
			// stage.gotoPos();
		}
		// Move - outer disabled
		else if (interface.cueParams[1] && interface.cueParams[2] == 0) {
			// stage.gotoPos();
		}

		// Reset mode
		// displays.setMode(target_mode); FIXME
	}
}

void updateSetting(void (*settingLimiter)(void), int mode) {

	if (Buttons::select.engaged()) {
		interface.editing = 1;
		interface.input.resetKeypad();

		while (interface.editing) {
			if (interface.editVars(mode)) {
				if (!interface.input.usingKeypad) {
					(*settingLimiter)();
				}
				displays.forceUpdateDisplays(
				    0,
				    1,
				    0,
				    1);  // the parameters originally varied but since we're redoing everything...
			}

			// If select pressed to confirm value, exit editing mode
			if (Buttons::select.engaged()) {
				(*settingLimiter)();
				interface.editing = 0;
			}
		}
	}
}

void brightnessLimiter() {
	keypadLeds.setBrightness(interface.leds.ledSettings[0]);
	interface.leds.keypadLedsColor(
	    interface.leds.ledSettings[1], interface.leds.ledSettings[2], interface.leds.ledSettings[3]);
	ringLeds.setBrightness(interface.leds.ledSettings[0]);
	ringLeds.show();
	displays.forceUpdateDisplays(0, 1, 0, 1);
}

void encoderLimiter() {
	interface.limitEncSettings();
	displays.forceUpdateDisplays(0, 1, 0, 0);
}

void eepromLimiter() {
	interface.limitMovements(interface.defaultValues);
	displays.forceUpdateDisplays(0, 1, 0, 0);
}

void kpLimiter() {
	interface.limitKpSettings();
	displays.forceUpdateDisplays(0, 1, 0, 0);
}

void manualLimiter() {
	interface.limitMovements(interface.currentMovements);
	displays.forceUpdateDisplays(0, 1, 0, 0);
}

void movementLimiter() {
	interface.limitMovements(interface.cueMovements);
	cuestack.setMovements(interface.cueMovements);
	displays.forceUpdateDisplays(1, 0, 1, 0);
}

void updateFlags() {
	inner.tenths = 1;
	outer.tenths = 1;
	displays.update = 1;
}

char* encodeCue(Cue cue) {
	auto cue_bytes = reinterpret_cast<char*>(&cue);
	auto encoded = new char[(sizeof cue) * 2 + 1];
	for (size_t i = 0; i < sizeof cue; i++) {
		encoded[2 * i] = cue_bytes[i] | 0x0f;
		encoded[2 * i + 1] = cue_bytes[i] | 0xf0;
	}

	encoded[(sizeof cue) * 2] = '\0';
	return encoded;
}

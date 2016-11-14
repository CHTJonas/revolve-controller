#include "buttons.h"
#include "cuestack.h"
#include "interface.h"
#include "revolve.h"
#include "stage.h"
#include "state_machine.h"
#include <Keypad.h>
#include <PID_v1.h>
#include <TimerOne.h>

char keys[4][3] = { { '1', '2', '3' }, { '4', '5', '6' }, { '7', '8', '9' }, { '*', '0', '#' } };
byte ROWS[4] = { KEY1, KEY2, KEY3, KEY4 };
byte COLS[3] = { KEY5, KEY6, KEY7 };
Keypad keypad = Keypad(makeKeymap(keys), ROWS, COLS, 4, 3);

Adafruit_NeoPixel pauseLeds = Adafruit_NeoPixel(2, PAUSELEDS, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ringLeds = Adafruit_NeoPixel(24, RINGLEDS, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel keypadLeds = Adafruit_NeoPixel(4, KEYPADLEDS, NEO_GRB + NEO_KHZ800);

Buttons buttons;

Encoder enc_inner(INNERENC1, INNERENC2);
Encoder enc_outer(OUTERENC1, OUTERENC2);
Encoder enc_input(INPUTENC1, INPUTENC2);

Screen left(22, 24, 26);
Screen centre(22, 25, 27);
Screen right(32, 34, 36);

State state = State{.state = STATE_MAINMENU, .data = {} };
Revolve inner(4, 5, 6, enc_inner);
Revolve outer(11, 10, 9, enc_outer);
Cuestack cuestack;
Interface interface(&cuestack, &enc_input, &keypad, &ringLeds, &pauseLeds, &keypadLeds, &buttons);
Displays displays(&state, &left, &right, &centre, buttons);
Stage stage(&state, &inner, &outer, &interface, &ringLeds, &buttons);

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
	// Timer1.initialize(100000);
	// Timer1.attachInterrupt(updateFlags);

	// Serial.begin(1200);

	// interface.leds.setupLeds();
	// interface.setupSwitches();

	// cuestack.loadCuestack();
	// cuestack.loadExampleCues();
	// displays.begin();
	// displays.setMode(NORMAL);
}

void loop() {
	stage.step();
	displays.step();
	buttons.step();
}

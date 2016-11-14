#include "buttons.h"
#include "cuestack.h"
#include "interface.h"
#include "revolve.h"
#include "stage.h"
#include "state_machine.h"
#include "navigation.h"
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


Encoder enc_inner(INNERENC1, INNERENC2);
Encoder enc_outer(OUTERENC1, OUTERENC2);
Encoder enc_input(INPUTENC1, INPUTENC2);

Buttons buttons;
State state(&buttons);
Screen left(&state, 22, 24, 26);
Screen centre(&state, 22, 25, 27);
Screen right(&state, 32, 34, 36);
Revolve inner(4, 5, 6, enc_inner);
Revolve outer(11, 10, 9, enc_outer);
Cuestack cuestack;
Interface interface(&cuestack, &enc_input, &keypad, &ringLeds, &pauseLeds, &keypadLeds, &buttons);
Displays displays(&state, &left, &right, &centre, &buttons);
Stage stage(&state, &inner, &outer, &interface, &ringLeds, &buttons);
Navigation navigation(&state, &buttons);

void setup();
void loop();

void setup() {
	// Timer1.initialize(100000);
	// Timer1.attachInterrupt(updateFlags);

	Serial.begin(38400);
	while (!Serial) {
	}

	// interface.leds.setupLeds();
	// interface.setupSwitches();

	// cuestack.loadCuestack();
	// cuestack.loadExampleCues();
	// displays.begin();
	// displays.setMode(NORMAL);
}

void loop() {
	stage.loop();
	displays.loop();
	navigation.loop();

	delay(1000);
}

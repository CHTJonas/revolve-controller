#pragma once
#include "cuestack.h"
#include "LedInterface.h"
#include <Adafruit_NeoPixel.h>
#include <Bounce2.h>
#include <Encoder.h>
#include <Keypad.h>

class Interface {
public:
	// Constructor
	Interface(Cuestack& cuestack, Encoder& enc_input, Keypad& keypad, Adafruit_NeoPixel& ringLeds, Adafruit_NeoPixel& pauseLeds, Adafruit_NeoPixel& keypadLeds);

	// Navigation and editing
	bool updateMenu(int menuMax);
	bool editVars(int mode);
	void limitMovements(int(&movements)[10]) const;
	void limitEncSettings();
	void limitKpSettings();
	void limitCueParams();
	void loadCurrentCue();
	void loadCue(int number);

	// Input helpers
	int getInputEnc() const;
	void updateKeypad();
	void resetKeypad();
	char getKey();
	void waitSelectRelease();
	void waitBackRelease();

	// Setup functions
	void setupSwitches();

	// Cuestack
	Cuestack& cuestack;

	// Debouced switches
	Bounce select = Bounce();
	Bounce back = Bounce();

	// Input Encoder
	Encoder& enc_input;

	// Keypad
	Keypad& keypad;

	LedInterface leds;

	// Current manual values
	int currentMovements[10];

	// Cue display values
	int cueMovements[10];
	int cueNumber;
	int cueParams[3];

	// Settings parameters
	float encSettings[4];
	int defaultValues[10];
	double kpSettings[6];

	// Screen navigation variables
	int menu_pos;
	int editing;
	char key;
	char currentKey;
	int usingKeypad;
	int keypadValue;

private:
};

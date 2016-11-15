#pragma once
#include "InputInterface.h"
#include "OutputLedInterface.h"
#include "buttons.h"
#include "cuestack.h"
#include <Adafruit_NeoPixel.h>

class InputInterface;

class Interface {
public:
	// Constructor
	Interface(
	    Cuestack& cuestack,
	    Encoder& enc_input,
	    Keypad& keypad,
	    Adafruit_NeoPixel& ringLeds,
	    Adafruit_NeoPixel& pauseLeds,
	    Adafruit_NeoPixel& keypadLeds);

	// Navigation and editing
	bool updateMenu(int menuMax);
	bool editVars(int mode);
	void limitMovements(int (&movements)[10]) const;
	void limitEncSettings();
	void limitKpSettings();
	void limitCueParams();
	void loadCurrentCue();
	void loadCue(int number);

	// Setup functions
	void setupSwitches();

	// Cuestack
	Cuestack& cuestack;

	InputInterface input;
	OutputLedInterface leds;

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

private:
};

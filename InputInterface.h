#pragma once
#include <Bounce2.h>
#include <Encoder.h>
#include <Keypad.h>

class InputInterface
{
public:
	InputInterface(Encoder& encoder, Keypad& keypad);

	int getInputEncoder();

	void updateKeypad();
	char getKey();
	void resetKeypad();

	void waitSelectRelease();
	void waitBackRelease();
	
	Bounce select = Bounce();
	Bounce back = Bounce();
	Encoder& enc_input;
	Keypad& keypad;

	char key;
	char currentKey;
	bool usingKeypad;
	int value = 0;

	bool editing;
};



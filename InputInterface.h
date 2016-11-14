#pragma once
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

	Encoder& enc_input;
	Keypad& keypad;

	char key;
	char currentKey;
	bool usingKeypad;
	int value = 0;

	bool editing;
};



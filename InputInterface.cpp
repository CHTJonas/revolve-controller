#include "InputInterface.h"

InputInterface::InputInterface(Encoder& encoder, Keypad& keypad) : enc_input(encoder), keypad(keypad)
{
}

int InputInterface::getInputEncoder() {
	auto value = enc_input.read() / 4;
	
	if (abs(value) > 0) {
		enc_input.write(0);
	}

	// Skip acceleration if not editing (i.e. navigate menus at sensible speed)
	if (editing) {
		if (abs(value) > 4) {
			value = value * 2;
		}
		if (abs(value) > 6) {
			value = value * 3;
		}
	}

	return -value;
}

void InputInterface::updateKeypad() {
	auto newKey = keypad.getKey();
	if (newKey) {
		key = newKey;  // Holds last pressed key - reset to zero when read
		currentKey = newKey;  // Current key being pressed (if any)
	}

	if (key) {
		usingKeypad = true;
	}

	// Reset currentKey if key released
	if (keypad.getState() == HOLD || keypad.getState() == PRESSED) {
		currentKey = key;
	}
	else {
		currentKey = 0;
	}
}

// Returns value of last pressed key, then resets key
char InputInterface::getKey() {
	auto returnKey = key;
	key = 0;
	return returnKey;
}

void InputInterface::resetKeypad() {
	key = 0;
	currentKey = 0;
	usingKeypad = false;
	value = 0;
}

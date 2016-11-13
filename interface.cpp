#include "interface.h"
#include "utils.h" 
#include <EEPROM.h>

Interface::Interface(
	Cuestack& cuestack,
	Encoder& enc_input,
	Keypad& keypad,
	Adafruit_NeoPixel& ringLeds,
	Adafruit_NeoPixel& pauseLeds,
	Adafruit_NeoPixel& keypadLeds)
	: cuestack(cuestack), enc_input(enc_input), keypad(keypad), leds(OutputLedInterface(ringLeds, pauseLeds, keypadLeds)) {

	// Initialise settings from EEPROM
	EEPROM.get(EELED_SETTINGS, leds.ledSettings);

	EEPROM.get(EEDEFAULT_VALUES, defaultValues);
	EEPROM.get(EEDEFAULT_VALUES, currentMovements);
	cuestack.updateDefaultValues();

	EEPROM.get(EEINNER_ENC_RATIO, encSettings[2]);
	EEPROM.get(EEOUTER_ENC_RATIO, encSettings[3]);

	EEPROM.get(EEKP_SETTINGS, kpSettings);

	encSettings[0] = encSettings[2] > 0 ? 0 : 1;
	encSettings[1] = encSettings[3] > 0 ? 0 : 1;

	// Remove any negatives
	encSettings[2] = abs(encSettings[2]);
	encSettings[3] = abs(encSettings[3]);

	// Initialise navigation variables
	editing = 0;
	menu_pos = 0;
	usingKeypad = 0;
}

bool Interface::editVars(int mode) {
	// Check if keypad not in use and if key has been pressed
	updateKeypad();

	auto delta = false, change = false;

	if (usingKeypad)
	{
		auto pressedKey = getKey();
		delta = false;

		if (pressedKey) {
			change = true;
			if (pressedKey == '#' || pressedKey == '*') {
				value = 0;
			}
			else if (String(value).length() < 4)
			{
				value = value * 10 + atoi(&pressedKey);
			}
		}
	}
	else // using encoder
	{
		delta = true;
		value = getInputEnc();
		if (value)
		{
			change = true;
		}
	}

	if (change)
	{
		switch (mode) {
		case MAN:
			currentMovements[menu_pos] = delta ? currentMovements[menu_pos] + value : value;
			break;
		case BRIGHTNESS:
			leds.ledSettings[menu_pos] = delta ? leds.ledSettings[menu_pos] + value : value;
			break;
		case ENCSETTINGS:
			if (menu_pos < 2) {
				encSettings[menu_pos] = value > 0 ? 1 : 0;
			}
			else {
				encSettings[menu_pos] = delta ? (encSettings[menu_pos] + value) / 100.0f : value / 100.0f;
			}
			break;
		case DEFAULTVALUES:
			defaultValues[menu_pos] = delta ? defaultValues[menu_pos] + value : value;
			break;
		case KPSETTINGS:
			kpSettings[menu_pos] = delta ? (kpSettings[menu_pos] + value) / 1000.0f : value / 1000.0f;
			break;
		case PROGRAM_MOVEMENTS:
			if (cueParams[1] == 0) {  // If inner disabled
				cueMovements[menu_pos + 5] = delta ? kpSettings[menu_pos] + value : value;
			}
			else {
				cueMovements[menu_pos] = delta ? kpSettings[menu_pos] + value : value;
			}
			break;
		case PROGRAM_PARAMS:
			cueNumber = delta ? cueNumber + value : value;
			break;
		default:
			break;
		}
		return true;
	}

	return false;
}

void Interface::limitMovements(int(&movements)[10]) const {
	movements[0] = clamp(movements[0], 0, 359);
	movements[1] = clamp(movements[1], MINSPEED, 100);
	movements[2] = clamp(movements[2], 1, MAXACCEL);
	movements[3] = clamp(movements[3], 0, 1);
	movements[4] = clamp(movements[4], 0, 50);
	movements[5] = clamp(movements[5], 0, 359);
	movements[6] = clamp(movements[6], MINSPEED, 100);
	movements[7] = clamp(movements[7], 1, MAXACCEL);
	movements[8] = clamp(movements[8], 0, 1);
	movements[9] = clamp(movements[9], 0, 50);
}

void Interface::limitEncSettings() {
	encSettings[0] = clamp(encSettings[0], 0.0f, 1.0f);
	encSettings[1] = clamp(encSettings[1], 0.0f, 1.0f);
	encSettings[2] = clamp(encSettings[2], 0.01f, 99.99f);
	encSettings[3] = clamp(encSettings[3], 0.01f, 99.99f);
}

void Interface::limitKpSettings() {
	for (auto i = 0; i < 6; i++) {
		kpSettings[i] = clamp(kpSettings[i], 0.000, 9.999);
	}
}

void Interface::limitCueParams() {
	cueNumber = clamp(cueNumber, 0, 999);
	cueParams[0] = clamp(cueParams[0], 0, 1);
	cueParams[1] = clamp(cueParams[1], 0, 1);
	cueParams[2] = clamp(cueParams[2], 0, 1);
}

void Interface::loadCurrentCue() {
	cuestack.getMovements(cueMovements);
	cuestack.getNumber(cueNumber);
	cuestack.getParams(cueParams);
}

void Interface::loadCue(int number) {
	auto currentCue = cuestack.currentCue;
	cuestack.currentCue = number;
	cuestack.getMovements(cueMovements);
	cuestack.getNumber(cueNumber);
	cuestack.getParams(cueParams);
	cuestack.currentCue = currentCue;
}

bool Interface::updateMenu(int menuMax) {
	auto encValue = getInputEnc();
	auto oldMenuPos = menu_pos;

	if (encValue > 0 && menu_pos < menuMax) {
		menu_pos = min(menuMax, menu_pos + encValue);
	}
	else if (encValue < 0 && menu_pos > 0) {
		menu_pos = max(0, menu_pos + encValue);
	}

	return !(menu_pos == oldMenuPos);
}

int Interface::getInputEnc() const {
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

void Interface::updateKeypad() {
	auto newKey = keypad.getKey();
	if (newKey) {
		key = newKey;  // Holds last pressed key - reset to zero when read
		currentKey = newKey;  // Current key being pressed (if any)
	}

	// Enable keypad input
	if (!usingKeypad && key) {
		usingKeypad = 1;
	}

	// Reset currentKey if key released
	if (keypad.getState() == HOLD || keypad.getState() == PRESSED) {
		currentKey = key;
	}
	else {
		currentKey = 0;
	}
}

void Interface::resetKeypad() {
	key = 0;
	currentKey = 0;
	usingKeypad = 0;
	value = 0;
}

// Returns value of last pressed key, then resets key
char Interface::getKey() {
	auto returnKey = key;
	key = 0;
	return returnKey;
}

void Interface::setupSwitches() {
	pinMode(GO, INPUT_PULLUP);
	pinMode(BACK, INPUT);
	pinMode(PAUSE, INPUT_PULLUP);
	pinMode(SELECT, INPUT_PULLUP);  // Connects to +5v when pressed

	pinMode(INNERHOME, INPUT_PULLUP);
	pinMode(OUTERHOME, INPUT_PULLUP);

	pinMode(ESTOPNC1, INPUT_PULLUP);
	pinMode(ESTOPNC2, INPUT_PULLUP);
	pinMode(ESTOPNC3, INPUT_PULLUP);
	pinMode(ESTOPNO, INPUT_PULLUP);

	// Setup debouncers
	select.attach(SELECT);
	select.interval(10);

	back.attach(BACK);
	back.interval(10);
}

void Interface::waitSelectRelease() {
	while (select.read() == LOW) {
		select.update();
	}
}

void Interface::waitBackRelease() {
	while (back.read()) {
		back.update();
	}
}

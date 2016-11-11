#include "interface.h"

template <class T> constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
	return max(min(v, hi), lo);
}

Interface::Interface(Cuestack& cuestack, Encoder& enc_input, Keypad& keypad, Adafruit_NeoPixel& ringLeds, Adafruit_NeoPixel& pauseLeds, Adafruit_NeoPixel& keypadLeds)
	: cuestack(cuestack), enc_input(enc_input), keypad(keypad), ringLeds(ringLeds), pauseLeds(pauseLeds), keypadLeds(keypadLeds) {

	// Initialise settings from EEPROM
	EEPROM.get(EELED_SETTINGS, ledSettings);

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

	// Keypad input if enabled
	if (usingKeypad) {
		char pressedKey = getKey();

		if (pressedKey) {
			// Reset to zero if # or * pressed
			if (pressedKey == '#' || pressedKey == '*')
				keypadValue = 0;

			// Otherwise concatenate onto keypadvalue if less than four digits
			else if (String(keypadValue).length() < 4)
				keypadValue = (String(keypadValue) + pressedKey).toInt();

			// Update current selected parameter
			switch (mode) {
			case MAN:
				currentMovements[menu_pos] = keypadValue;
				break;
			case BRIGHTNESS:
				ledSettings[menu_pos] = keypadValue;
				break;
			case ENCSETTINGS:
				if (menu_pos < 2) {
					if (keypadValue > 0)
						encSettings[menu_pos] = 1;
					else
						encSettings[menu_pos] = 0;
				}
				else {
					encSettings[menu_pos] = (static_cast<float>(keypadValue) / 100);
				}
				break;
			case DEFAULTVALUES:
				defaultValues[menu_pos] = keypadValue;
				break;
			case KPSETTINGS:
				kpSettings[menu_pos] = (static_cast<float>(keypadValue) / 1000);
				break;
			case PROGRAM_MOVEMENTS:
				if (cueParams[1] == 0) {  // If inner disabled
					cueMovements[menu_pos + 5] = keypadValue;
				}
				else {
					cueMovements[menu_pos] = keypadValue;
				}
				break;
			case PROGRAM_PARAMS:
				cueNumber = keypadValue;
				break;
			}
			return true;
		}
		else {
			return false;  // Do not update displays
		}
	}

	// Otherwise encoder input
	else {
		// Add change to currently selected parameter
		int change = getInputEnc();
		if (change) {
			// Choose which settings to update based on mode
			switch (mode) {
			case MAN:
				currentMovements[menu_pos] += change;
				break;
			case BRIGHTNESS:
				ledSettings[menu_pos] += change;
				break;
			case ENCSETTINGS:
				if (menu_pos < 2) {
					if (change > 0)
						encSettings[menu_pos] = 1;
					else
						encSettings[menu_pos] = 0;
				}
				else {
					encSettings[menu_pos] += (static_cast<float>(change) / 100);
				}
				break;
			case DEFAULTVALUES:
				defaultValues[menu_pos] += change;
				break;
			case KPSETTINGS:
				kpSettings[menu_pos] += (static_cast<float>(change) / 1000);
				break;
			case PROGRAM_MOVEMENTS:
				if (cueParams[1] == 0) {  // If inner disabled
					cueMovements[menu_pos + 5] += change;
				}
				else {
					cueMovements[menu_pos] += change;
				}
				break;
			case PROGRAM_PARAMS:
				cueNumber += static_cast<float>(change) / 10;
				break;
			}
			return true;  // Update displays (has been a change)
		}
		else {
			return false;  // No change
		}
	}
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

void Interface::limitLedSettings() {
	for (auto i = 0; i < sizeof(ledSettings) / sizeof(*ledSettings); i++) {
		ledSettings[i] = clamp(ledSettings[i], 0, 255);
	}
}

void Interface::limitEncSettings() {
	encSettings[0] = clamp(encSettings[0], 0.0f, 1.0f);
	encSettings[1] = clamp(encSettings[1], 0.0f, 1.0f);
	encSettings[2] = clamp(encSettings[2], 0.01f, 99.99f);
	encSettings[3] = clamp(encSettings[3], 0.01f, 99.99f);
}

void Interface::limitKpSettings() {
	for (auto i = 0; i < 6; i++)
	{
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

void Interface::flashLed(int led, int interval) {
	auto currentState = digitalRead(led);

	if (flashCounter == 0) {
		flashCounter = millis();
	}

	if (millis() > (flashCounter + interval)) {
		digitalWrite(led, !currentState);
		flashCounter = 0;
	}
}

void Interface::updatePauseLeds() const {
	if (digitalRead(PAUSE) == LOW) {
		pauseLedsColor(0, 255, 0);
	}
	else {
		pauseLedsColor(255, 0, 0);
	}
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
	keypadValue = 0;
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

void Interface::setupLeds() {

	pinMode(ENCR, OUTPUT);
	pinMode(ENCG, OUTPUT);
	pinMode(ENCB, OUTPUT);
	pinMode(GOLED, OUTPUT);
	pinMode(SELECTLED, OUTPUT);

	// Encoder LEDS are common anode
	digitalWrite(ENCR, HIGH);
	digitalWrite(ENCG, LOW);
	digitalWrite(ENCB, HIGH);

	ringLeds.begin();
	pauseLeds.begin();
	keypadLeds.begin();

	ringLeds.setBrightness(ledSettings[0]);
	ringLeds.show();

	pauseLeds.setBrightness(ledSettings[0]);
	pauseLedsColor(0, 0, 0);

	keypadLeds.setBrightness(ledSettings[0]);
	keypadLedsColor(ledSettings[1], ledSettings[2], ledSettings[3]);

	// Set initial LED values
	digitalWrite(ENCG, LOW);
	digitalWrite(SELECTLED, HIGH);
}

void Interface::encRed() {
	digitalWrite(ENCR, LOW);
	digitalWrite(ENCG, HIGH);
	digitalWrite(ENCB, HIGH);
}

void Interface::encGreen() {
	digitalWrite(ENCR, HIGH);
	digitalWrite(ENCG, LOW);
	digitalWrite(ENCB, HIGH);
}

void Interface::encBlue() {
	digitalWrite(ENCR, HIGH);
	digitalWrite(ENCG, HIGH);
	digitalWrite(ENCB, LOW);
}

void Interface::encOff() {
	digitalWrite(ENCR, HIGH);
	digitalWrite(ENCG, HIGH);
	digitalWrite(ENCB, HIGH);
}

void Interface::ringLedsColor(int r, int g, int b) const {
	for (auto i = 0; i < 24; i++)
		ringLeds.setPixelColor(i, r, g, b);
	ringLeds.show();
}

void Interface::pauseLedsColor(int r, int g, int b) const {
	pauseLeds.setPixelColor(0, r, g, b);
	pauseLeds.setPixelColor(1, r, g, b);
	pauseLeds.show();
}

void Interface::keypadLedsColor(int r, int g, int b) const {
	for (auto i = 0; i < 4; i++)
		keypadLeds.setPixelColor(i, r, g, b);
	keypadLeds.show();
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

void Interface::allLedsOn() const {
	digitalWrite(GOLED, HIGH);
	digitalWrite(SELECTLED, HIGH);
	digitalWrite(ENCR, LOW);
	digitalWrite(ENCG, LOW);
	digitalWrite(ENCB, LOW);
	pauseLedsColor(255, 255, 255);
	keypadLedsColor(255, 255, 255);
}

void Interface::allLedsOff() const {
	digitalWrite(GOLED, LOW);
	digitalWrite(SELECTLED, LOW);
	digitalWrite(ENCR, HIGH);
	digitalWrite(ENCG, HIGH);
	digitalWrite(ENCB, HIGH);
	pauseLedsColor(0, 0, 0);
	keypadLedsColor(0, 0, 0);
}

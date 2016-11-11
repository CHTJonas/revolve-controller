#include "LedInterface.h"

template <class T> constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
	return max(min(v, hi), lo);
}

void LedInterface::flashLed(int led, int interval) {
	auto currentState = digitalRead(led);

	if (flashCounter == 0) {
		flashCounter = millis();
	}

	if (millis() > (flashCounter + interval)) {
		digitalWrite(led, !currentState);
		flashCounter = 0;
	}
}

void LedInterface::updatePauseLeds() const {
	if (digitalRead(PAUSE) == LOW) {
		pauseLedsColor(0, 255, 0);
	}
	else {
		pauseLedsColor(255, 0, 0);
	}
}

void LedInterface::encRed() {
	digitalWrite(ENCR, LOW);
	digitalWrite(ENCG, HIGH);
	digitalWrite(ENCB, HIGH);
}

void LedInterface::encGreen() {
	digitalWrite(ENCR, HIGH);
	digitalWrite(ENCG, LOW);
	digitalWrite(ENCB, HIGH);
}

void LedInterface::encBlue() {
	digitalWrite(ENCR, HIGH);
	digitalWrite(ENCG, HIGH);
	digitalWrite(ENCB, LOW);
}

void LedInterface::encOff() {
	digitalWrite(ENCR, HIGH);
	digitalWrite(ENCG, HIGH);
	digitalWrite(ENCB, HIGH);
}

void LedInterface::ringLedsColor(int r, int g, int b) const {
	for (auto i = 0; i < 24; i++)
		ringLeds.setPixelColor(i, r, g, b);
	ringLeds.show();
}

void LedInterface::pauseLedsColor(int r, int g, int b) const {
	pauseLeds.setPixelColor(0, r, g, b);
	pauseLeds.setPixelColor(1, r, g, b);
	pauseLeds.show();
}

void LedInterface::keypadLedsColor(int r, int g, int b) const {
	for (auto i = 0; i < 4; i++)
		keypadLeds.setPixelColor(i, r, g, b);
	keypadLeds.show();
}


void LedInterface::allLedsOn() const {
	digitalWrite(GOLED, HIGH);
	digitalWrite(SELECTLED, HIGH);
	digitalWrite(ENCR, LOW);
	digitalWrite(ENCG, LOW);
	digitalWrite(ENCB, LOW);
	pauseLedsColor(255, 255, 255);
	keypadLedsColor(255, 255, 255);
}

void LedInterface::allLedsOff() const {
	digitalWrite(GOLED, LOW);
	digitalWrite(SELECTLED, LOW);
	digitalWrite(ENCR, HIGH);
	digitalWrite(ENCG, HIGH);
	digitalWrite(ENCB, HIGH);
	pauseLedsColor(0, 0, 0);
	keypadLedsColor(0, 0, 0);
}

void LedInterface::limitLedSettings() {
	for (auto i = 0; i < sizeof(ledSettings) / sizeof(*ledSettings); i++) {
		ledSettings[i] = clamp(ledSettings[i], 0, 255);
	}
}

LedInterface::LedInterface(Adafruit_NeoPixel& ringLeds, Adafruit_NeoPixel& pauseLeds, Adafruit_NeoPixel& keypadLeds)
	: ringLeds(ringLeds), pauseLeds(pauseLeds), keypadLeds(keypadLeds)
{
}

void LedInterface::setupLeds() {
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

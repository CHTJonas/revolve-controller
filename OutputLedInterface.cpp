#include "OutputLedInterface.h"

OutputLedInterface::OutputLedInterface(Adafruit_NeoPixel* ringLeds, Adafruit_NeoPixel* pauseLeds, Adafruit_NeoPixel* keypadLeds)
      : ringLeds(ringLeds), pauseLeds(pauseLeds), keypadLeds(keypadLeds) {
}

void OutputLedInterface::setupLeds() {
	pinMode(ENCR, OUTPUT);
	pinMode(ENCG, OUTPUT);
	pinMode(ENCB, OUTPUT);
	pinMode(GOLED, OUTPUT);
	pinMode(SELECTLED, OUTPUT);

	// Encoder LEDS are common anode
	encoderLedColor(false, true, false);

	ringLeds->begin();
	pauseLeds->begin();
	keypadLeds->begin();

	ringLeds->setBrightness(ledSettings[0]);
	ringLeds->show();

	pauseLeds->setBrightness(ledSettings[0]);
	pauseLedsColor(0, 0, 0);

	keypadLeds->setBrightness(ledSettings[0]);
	keypadLedsColor(ledSettings[1], ledSettings[2], ledSettings[3]);

	// Set initial LED values
	digitalWrite(ENCG, LOW);
	digitalWrite(SELECTLED, HIGH);
}

void OutputLedInterface::flashLed(int led, int interval) {
	auto currentState = digitalRead(led);

	if (flashCounter == 0) {
		flashCounter = millis();
	}

	if (millis() > (flashCounter + interval)) {
		digitalWrite(led, !currentState);
		flashCounter = 0;
	}
}

void OutputLedInterface::encoderLedColor(bool r, bool g, bool b) const {
	digitalWrite(ENCR, r ? LOW : HIGH);
	digitalWrite(ENCG, g ? LOW : HIGH);
	digitalWrite(ENCB, b ? LOW : HIGH);
}

void OutputLedInterface::ringLedsColor(int r, int g, int b) const {
	for (auto i = 0; i < 24; i++) {
		ringLeds->setPixelColor(i, r, g, b);
	}
	ringLeds->show();
}

void OutputLedInterface::pauseLedsColor(int r, int g, int b) const {
	pauseLeds->setPixelColor(0, r, g, b);
	pauseLeds->setPixelColor(1, r, g, b);
	pauseLeds->show();
}

void OutputLedInterface::keypadLedsColor(int r, int g, int b) const {
	for (auto i = 0; i < 4; i++)
		keypadLeds->setPixelColor(i, r, g, b);
	keypadLeds->show();
}

void OutputLedInterface::updatePauseLeds() const {
	if (digitalRead(DMH) == LOW) {
		pauseLedsColor(0, 255, 0);
	} else {
		pauseLedsColor(255, 0, 0);
	}
}

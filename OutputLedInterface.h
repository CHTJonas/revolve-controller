#pragma once
#include "cuestack.h"
#include <Adafruit_NeoPixel.h>
#include <Bounce2.h>

class OutputLedInterface {
public:
	OutputLedInterface(Adafruit_NeoPixel& ringLeds, Adafruit_NeoPixel& pauseLeds, Adafruit_NeoPixel& keypadLeds);
	void setup();

	void flashLed(int led, int interval);
	void updatePauseLeds() const;
	void ringLedsColor(int r, int g, int b) const;
	void pauseLedsColor(int r, int g, int b) const;
	void keypadLedsColor(int r, int g, int b) const;
	void encoderLedColor(bool r, bool g, bool b) const;

	Adafruit_NeoPixel& ringLeds;
	Adafruit_NeoPixel& pauseLeds;
	Adafruit_NeoPixel& keypadLeds;

	uint8_t ledSettings[4];

private:
	unsigned long flashCounter = 0;
};

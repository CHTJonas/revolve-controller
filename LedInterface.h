#pragma once
#include "cuestack.h"
#include <Adafruit_NeoPixel.h>
#include <Bounce2.h>
#include <Encoder.h>
#include <Keypad.h>

class LedInterface
{
public:
	LedInterface(Adafruit_NeoPixel& ringLeds, Adafruit_NeoPixel& pauseLeds, Adafruit_NeoPixel& keypadLeds);

	void setupLeds();

	void flashLed(int led, int interval);
	void updatePauseLeds() const;
	void ringLedsColor(int r, int g, int b) const;
	void pauseLedsColor(int r, int g, int b) const;
	void keypadLedsColor(int r, int g, int b) const;
	static void encRed();
	static void encGreen();
	static void encBlue();
	static void encOff();
	void allLedsOn() const;
	void allLedsOff() const;

	Adafruit_NeoPixel& ringLeds;
	Adafruit_NeoPixel& pauseLeds;
	Adafruit_NeoPixel& keypadLeds;

	uint8_t ledSettings[4];

private:
	long int flashCounter = 0;

};


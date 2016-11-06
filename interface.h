#pragma once
#include <Encoder.h>
#include <Keypad.h>
#include <Adafruit_NeoPixel.h>
#include <Bounce2.h>
#include "cuestack.h"

class Interface {
  public:

  // Constructor
  Interface(Cuestack& cuestack, Encoder& enc_input, Keypad& keypad, Adafruit_NeoPixel& ringLeds, Adafruit_NeoPixel& pauseLeds, Adafruit_NeoPixel& keypadLeds);

  // Navigation and editing
  bool updateMenu(int menuMax);
  bool editVars(int mode);
	static void limitVariable(int& variable, int varMin, int varMax);
	static void limitVariable(float& variable, float varMin, float varMax);
	static void limitVariable(double& variable, double varMin, double varMax);
  void limitMovements(int (&movements)[10]) const;
  void limitLedSettings();
  void limitEncSettings();
  void limitKpSettings();
  void limitCueParams();
  void loadCurrentCue();
  void loadCue(int number);
  
  // LEDs
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

  // Input helpers
  int getInputEnc() const;
  void updateKeypad();
  void resetKeypad();
  char getKey();
  void waitSelectRelease();
  void waitBackRelease();

  // Setup functions
  void setupSwitches();
  void setupLeds();

  // Cuestack
  Cuestack& _cuestack;
  
  // Debouced switches
  Bounce select = Bounce();
  Bounce back = Bounce();

  // Input Encoder
  Encoder& _enc_input;
  
  // Keypad
  Keypad& _keypad;

  Adafruit_NeoPixel& _ringLeds;
  Adafruit_NeoPixel& _pauseLeds;
  Adafruit_NeoPixel& _keypadLeds;

  // Current manual values
  int currentMovements[10];

  // Cue display values
  int cueMovements[10];
  float cueNumber;
  int cueParams[3];

  // Settings parameters
  int ledSettings[4];
  float encSettings[4];
  int defaultValues[10];
  double kpSettings[6];

  // Screen navigation variables
  int menu_pos;
  int editing;
  char key;
  char currentKey;
  int usingKeypad;
  int keypadValue;

  private:
    long int flashCounter=0;
};

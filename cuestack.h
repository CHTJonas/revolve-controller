#pragma once
#include "constants.h"
#include <Arduino.h>

typedef struct {
	int pos_i;
	byte speed_i;
	byte acc_i;
	byte dir_i;
	byte revs_i;

	int pos_o;
	byte speed_o;
	byte acc_o;
	byte dir_o;
	byte revs_o;

	int num = 0;
	int byteWaster =
	    0;  // num used to be a float = 4 bytes, so this is a placeholder until we can rearchitect the struct
	byte auto_follow = 0;
	byte en_i = 1;
	byte en_o = 1;
	byte active = 0;
} Cue;

class Cuestack {
public:
	// Constructor
	Cuestack();
	void setup();

	// Cue initialisation
	void updateDefaultValues();
	void initialiseCue(int number);
	void resetCue(int number);
	void resetCuestack();

	// EEPROM saving and retrieving
	void loadCuestack();
	void saveCuestack() const;

	// Getters for current cue data
	void getMovements(int (&outputValues)[10]);
	void getNumber(int& outputNumber) const;
	void getParams(int (&outputParams)[3]);
	void setMovements(int inputValues[10]);
	void setNumber(int inputNumber);
	void setParams(int inputParams[3]);
	int getCueIndex(int number) const;

	// Cue number validation and sorting
	bool validCueNumber(float number) const;
	int activeCues() const;
	void sortCues();

	// Testing function for example cue data
	static void loadExampleCues();

	int currentCue = 0;
	int totalCues = 0;

	// Default cue values
	Cue defaultValues;

	// Master array to hold up to 100 written cues
	Cue stack[MAX_CUES];
};

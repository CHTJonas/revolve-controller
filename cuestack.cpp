#include "cuestack.h"

Cuestack::Cuestack() {
}

void Cuestack::updateDefaultValues() {
	int newDefaultValues[10];
	EEPROM.get(EEDEFAULT_VALUES, newDefaultValues);

	defaultValues.pos_i = newDefaultValues[0];
	defaultValues.speed_i = newDefaultValues[1];
	defaultValues.acc_i = newDefaultValues[2];
	defaultValues.dir_i = newDefaultValues[3];
	defaultValues.revs_i = newDefaultValues[4];
	defaultValues.pos_o = newDefaultValues[5];
	defaultValues.speed_o = newDefaultValues[6];
	defaultValues.acc_o = newDefaultValues[7];
	defaultValues.dir_o = newDefaultValues[8];
	defaultValues.revs_o = newDefaultValues[9];
}

void Cuestack::initialiseCue(int number) {
	stack[number].pos_i = defaultValues.pos_i;
	stack[number].speed_i = defaultValues.speed_i;
	stack[number].acc_i = defaultValues.acc_i;
	stack[number].dir_i = defaultValues.dir_i;
	stack[number].revs_i = defaultValues.revs_i;
	stack[number].pos_o = defaultValues.pos_o;
	stack[number].speed_o = defaultValues.speed_o;
	stack[number].acc_o = defaultValues.acc_o;
	stack[number].dir_o = defaultValues.dir_o;
	stack[number].revs_o = defaultValues.revs_o;
	stack[number].auto_follow = 0;
	stack[number].en_i = 1;
	stack[number].en_o = 1;
}

void Cuestack::resetCue(int number) {
	stack[number].num = 0;
	initialiseCue(number);
	stack[number].active = 0;
}

void Cuestack::resetCuestack() {
	for (auto i = 0; i < 100; i++) {
		stack[i].num = 0;
		initialiseCue(i);
		stack[i].active = 0;
	}
	stack[0].active = 1;
	stack[0].num = 10;
	currentCue = 0;
	totalCues = 1;

	// Write to EEPROM
	saveCuestack();
}
void Cuestack::loadCuestack() {
	for (auto i = 0; i < 100; i++) {
		EEPROM.get(EECUESTACK_START + i * 20, stack[i]);
	}
	currentCue = 0;
	totalCues = activeCues();
}

void Cuestack::saveCuestack() const {
	for (auto i = 0; i < 100; i++) {
		EEPROM.put(EECUESTACK_START + i * 20, stack[i]);
	}
}

void Cuestack::getMovements(int (&outputValues)[10]) {
	auto cue = stack[currentCue];
	outputValues[0] = cue.pos_i;
	outputValues[1] = cue.speed_i;
	outputValues[2] = cue.acc_i;
	outputValues[3] = cue.dir_i;
	outputValues[4] = cue.revs_i;
	outputValues[5] = cue.pos_o;
	outputValues[6] = cue.speed_o;
	outputValues[7] = cue.acc_o;
	outputValues[8] = cue.dir_o;
	outputValues[9] = cue.revs_o;
}

void Cuestack::getNumber(int& outputNumber) const {
	outputNumber = stack[currentCue].num;
}

void Cuestack::getParams(int (&outputParams)[3]) {
	auto cue = stack[currentCue];
	outputParams[0] = cue.auto_follow;
	outputParams[1] = cue.en_i;
	outputParams[2] = cue.en_o;
}

void Cuestack::setMovements(int inputValues[10]) {
	stack[currentCue].pos_i = inputValues[0];
	stack[currentCue].speed_i = inputValues[1];
	stack[currentCue].acc_i = inputValues[2];
	stack[currentCue].dir_i = inputValues[3];
	stack[currentCue].revs_i = inputValues[4];
	stack[currentCue].pos_o = inputValues[5];
	stack[currentCue].speed_o = inputValues[6];
	stack[currentCue].acc_o = inputValues[7];
	stack[currentCue].dir_o = inputValues[8];
	stack[currentCue].revs_o = inputValues[9];
}

void Cuestack::setNumber(int inputNumber) {
	stack[currentCue].num = inputNumber;
}

void Cuestack::setParams(int inputParams[3]) {
	stack[currentCue].auto_follow = inputParams[0];
	stack[currentCue].en_i = inputParams[1];
	stack[currentCue].en_o = inputParams[2];
}

int Cuestack::getCueIndex(int number) const {
	for (auto i = 0; i < 100; i++) {
		if (number == stack[i].num) {
			return i;
		}
	}
}

// Checks to see if number already exists in stack
bool Cuestack::validCueNumber(float number) const {
	for (auto i = 0; i < 100; i++) {
		if (number == stack[i].num) {
			return true;
		}
	}

	return false;
}

// Counts total active cues
int Cuestack::activeCues() const {
	auto active = 0;
	for (auto i = 0; i < 100; i++) {
		if (stack[i].active)
			active++;
	}
	return active;
}

// Sorts cuestack (e.g. after changing number, deleting cues etc.
// Simple bubblesort unofortunately, but only performed quite rarely
void Cuestack::sortCues() {
	auto sorted = 0;
	Cue scratch1;
	Cue scratch2;

	while (!sorted) {
		// Assume sorted, will get changed if anything swapped
		sorted = 1;

		// Try every cue
		// Don't run over end of array - each pass checks i and i+1!! (Causes issues....)
		for (auto i = 0; i < 99; i++) {

			// Move non active cues up (only if one above isn't active also, otherwise waste of time!)
			if (!stack[i].active && stack[i + 1].active) {
				stack[i] = stack[i + 1];  // Move cue up
				resetCue(i + 1);  // Reset now duplicate cue
				sorted = 0;  // Not sorted - need to go through again
			}

			// If num higher than num of next cue, swap
			// Don't bother testing if either isn't active
			if (stack[i].active && stack[i + 1].active && stack[i].num > stack[i + 1].num) {
				scratch1 = stack[i];
				scratch2 = stack[i + 1];

				// Re-enter into array
				stack[i + 1] = scratch1;
				stack[i] = scratch2;

				// Not sorted!
				sorted = 0;
			}
		}
	}
}

void Cuestack::loadExampleCues() {
}

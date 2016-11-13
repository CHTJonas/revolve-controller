#include "InputButtonsInterface.h"
#include <Arduino.h>
#include "constants.h"


bool InputButtonsInterface::dmhEngaged() {
	return !digitalRead(DMH);
}

bool InputButtonsInterface::goEngaged() {
	return !digitalRead(GO);
}

bool InputButtonsInterface::eStopsEngaged() {
	// Commented out line for non-conencted external esstop testing
	// if !(digitalRead(ESTOPNC1)==LOW && digitalRead(ESTOPNC2)==LOW && digitalRead(ESTOPNC3)==LOW &&
	// digitalRead(ESTOPNO)==HIGH){
	return !(digitalRead(ESTOPNC1) == LOW && digitalRead(ESTOPNO) == HIGH);
}


void InputButtonsInterface::waitSelectRelease() {
	while (back.read() == LOW) {
		back.update();
	}
}

void InputButtonsInterface::waitBackRelease() {
	while (inputEncoder.read()) {
		inputEncoder.update();
	}
}

#include "InputButtonsInterface.h"
#include "constants.h"
#include <Arduino.h>

Bounce InputButtonsInterface::back = Bounce();
Bounce InputButtonsInterface::inputEncoder = Bounce();

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

bool InputButtonsInterface::inputEncoderPressed() {
	inputEncoder.update();
	if (inputEncoder.read() == LOW) {
		while (inputEncoder.read() == LOW) {
			inputEncoder.update();
		}
		return true;
	}
	return false;
}

bool InputButtonsInterface::backPressed() {
	back.update();
	if (back.read()) {
		while (back.read()) {
			back.update();
		}
		return true;
	}

	return false;
}

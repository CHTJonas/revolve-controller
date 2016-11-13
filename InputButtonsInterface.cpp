#include "InputButtonsInterface.h"
#include <Arduino.h>

void InputButtonsInterface::waitSelectRelease() {
	while (select.read() == LOW) {
		select.update();
	}
}

void InputButtonsInterface::waitBackRelease() {
	while (back.read()) {
		back.update();
	}
}

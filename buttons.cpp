#include "buttons.h"
#include "constants.h"
#include <Arduino.h>

Button::Button(int pin, bool toggle) : pin(pin), toggle(toggle), old_state(BUTTON_STATE_UNKNOWN) {
	pinMode(pin, toggle ? INPUT_PULLUP : INPUT);
}

bool Button::engaged() {
	return (digitalRead(pin) == HIGH) ^ toggle;
}

void Button::step() {
	old_state = BUTTON_STATE_UNKNOWN;
}

bool Button::had_rising_edge() {
	if (engaged()) {
		if (old_state == BUTTON_STATE_LOW) {
			old_state = BUTTON_STATE_HIGH;
			return true;
		} else {
			old_state = BUTTON_STATE_HIGH;
			return false;
		}
	} else {
		old_state = BUTTON_STATE_LOW;
		return false;
	}
}

EStopButton::EStopButton(int nc_pin, int no_pin) : nc_pin(nc_pin), no_pin(no_pin) {
	pinMode(nc_pin, INPUT_PULLUP);
	pinMode(no_pin, INPUT_PULLUP);
}

bool EStopButton::engaged() {
	// Commented out line for non-conencted external esstop testing
	// return !(digitalRead(nc_pin1)==LOW && digitalRead(nc_pin2)==LOW && digitalRead(nc_pin3)==LOW &&
	// digitalRead(no_pin)==HIGH)
	return !digitalRead(nc_pin) == LOW || !digitalRead(no_pin) == HIGH;
}

void Buttons::step() {
	dmh.step();
	go.step();
	back.step();
	inputEncoder.step();
	inner_home.step();
	outer_home.step();
}

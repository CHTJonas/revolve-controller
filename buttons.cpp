#include "buttons.h"
#include "constants.h"
#include <Arduino.h>

Button::Button(int pin, bool toggle) : pin(pin), toggle(toggle), old_state(BUTTON_STATE_UNKNOWN) {
	pinMode(pin, toggle ? INPUT_PULLUP : INPUT);
}

bool Button::engaged() {
	return (digitalRead(pin) == HIGH) ^ toggle;
}

void Button::state_changed() {
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

EStopButton::EStopButton(int no_pin, int nc_pin1, int nc_pin2, int nc_pin3)
      : no_pin(no_pin), nc_pin1(nc_pin1), nc_pin2(nc_pin2), nc_pin3(nc_pin3) {
	pinMode(no_pin, INPUT_PULLUP);
	pinMode(nc_pin1, INPUT_PULLUP);
	pinMode(nc_pin2, INPUT_PULLUP);
	pinMode(nc_pin3, INPUT_PULLUP);
}

bool EStopButton::engaged() {
	return !(
	    digitalRead(nc_pin1) == LOW && digitalRead(nc_pin2) == LOW && digitalRead(nc_pin3) == LOW &&
	    digitalRead(no_pin) == HIGH);
}

void Buttons::state_changed() {
	dmh.state_changed();
	go.state_changed();
	back.state_changed();
	input_encoder_button.state_changed();
	inner_home.state_changed();
	outer_home.state_changed();
	input_encoder.write(0);
}

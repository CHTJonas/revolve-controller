#include "buttons.h"
#include "constants.h"
#include "pins.h"
#include <Arduino.h>

Button Buttons::dmh = Button(DMH, true);
Button Buttons::go = Button(GO, true);
EStopButton Buttons::e_stop = EStopButton(ESTOPNC1, ESTOPNC2, ESTOPNC3, ESTOPNO);
Button Buttons::back = Button(SELECT, true);
Button Buttons::select = Button(BACK, false);

Button::Button(int pin, bool toggle) : pin(pin), toggle(toggle), old_state(BUTTON_STATE_UNKNOWN), last_high_time(0) {
}

void Button::state_changed() {
	old_state = BUTTON_STATE_UNKNOWN;
}

bool Button::engaged() {
	return digitalRead(pin) ^ toggle;
}

bool Button::risen_since_state_change() {
	bool engaged = this->engaged();
	const bool result = (old_state == BUTTON_STATE_LOW) && engaged && (last_high_time > millis() + 10);
	if (engaged) {
		last_high_time = millis();
	}
	old_state = engaged ? BUTTON_STATE_HIGH : BUTTON_STATE_LOW;
	return result;
}

EStopButton::EStopButton(int nc1, int nc2, int nc3, int no)
      : nc1_b(nc1, false), nc2_b(nc2, false), nc3_b(nc3, false), no_b(no, true), nc1(nc1), nc2(nc2), nc3(nc3), no(no) {
}

bool EStopButton::engaged() {
	return digitalRead(nc1) || digitalRead(nc2) || digitalRead(nc3) || !digitalRead(no);
}

void Buttons::state_changed() {
	dmh.state_changed();
	go.state_changed();
	back.state_changed();
	select.state_changed();
}

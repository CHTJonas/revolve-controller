#include "buttons.h"
#include "constants.h"
#include <Arduino.h>

Button Buttons::dmh = Button(DMH, true);
Button Buttons::go = Button(GO, true);
EStopButton Buttons::e_stop = EStopButton(ESTOPNC1, ESTOPNC2, ESTOPNC3, ESTOPNO);
Button Buttons::back = Button(SELECT, true);
Button Buttons::select = Button(BACK, false);

Button::Button(int pin, bool toggle) : pin(pin), toggle(toggle) {
}

bool Button::engaged() {
	return digitalRead(pin) ^ toggle;
}

EStopButton::EStopButton(int nc1, int nc2, int nc3, int no)
      : nc1(nc1), nc2(nc2), nc3(nc3), no(no), nc1_b(nc1, false), nc2_b(nc2, false), nc3_b(nc3, false), no_b(no, true) {
}

bool EStopButton::engaged() {
	return digitalRead(nc1) || digitalRead(nc2) || digitalRead(nc3) || !digitalRead(no);
}

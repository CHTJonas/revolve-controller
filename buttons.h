#pragma once
#include "constants.h"
#include <Encoder.h>

typedef enum ButtonState { BUTTON_STATE_LOW, BUTTON_STATE_HIGH, BUTTON_STATE_UNKNOWN } ButtonState;

class Button {
public:
	Button(int pin, bool toggle);

	bool engaged();
	bool had_rising_edge();

	void state_changed();

private:
	int pin;
	bool toggle;
	ButtonState old_state;
};

class EStopButton {
public:
	EStopButton(int no_pin, int nc_pin1, int nc_pin2, int nc_pin3);

	bool engaged();

private:
	int no_pin;
	int nc_pin1;
	int nc_pin2;
	int nc_pin3;
};

class Buttons {
public:
	void state_changed();

	Button dmh = Button(DMH, true);
	Button go = Button(GO, true);
	EStopButton e_stop = EStopButton(ESTOPNO, ESTOPNC1, ESTOPNC2, ESTOPNC3);
	Button back = Button(SELECT, true);
	Button input_encoder_button = Button(BACK, false);
	Button inner_home = Button(INNERHOME, true);
	Button outer_home = Button(OUTERHOME, true);
	Encoder input_encoder = Encoder(INPUTENC1, INPUTENC2);
};

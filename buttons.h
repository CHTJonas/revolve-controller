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
	EStopButton(int nc_pin, int no_pin);

	bool engaged();

private:
	int nc_pin;
	int no_pin;
};

class Buttons {
public:
	void state_changed();

	Button dmh = Button(DMH, true);
	Button go = Button(GO, true);
	EStopButton e_stop = EStopButton(ESTOPNC1, ESTOPNO);
	Button back = Button(SELECT, true);
	Button input_encoder_button = Button(BACK, false);
	Button inner_home = Button(INNERHOME, true);
	Button outer_home = Button(OUTERHOME, true);
	Encoder input_encoder = Encoder(INPUTENC1, INPUTENC2);
};

#pragma once
#include <Bounce2.h>
#include "constants.h"

typedef enum ButtonState {
	BUTTON_STATE_LOW,
	BUTTON_STATE_HIGH,
	BUTTON_STATE_UNKNOWN
} ButtonState;

class Button {
public:
	Button(int pin, bool toggle);

	bool engaged();
	bool had_rising_edge();

	void step();

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

class Buttons
{
public:
	void step();

	Button dmh = Button(DMH, true);
	Button go = Button(GO, true);
	EStopButton e_stop = EStopButton(ESTOPNC1, ESTOPNO);
	Button back = Button(SELECT, true);
	Button inputEncoder = Button(BACK, false);
	Button inner_home = Button(INNERHOME, true);
	Button outer_home = Button(OUTERHOME, true);
};

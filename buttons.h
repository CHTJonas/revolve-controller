#pragma once

class Button {
public:
	Button(int pin, bool toggle);
	void state_changed();

	bool engaged();
	bool risen_since_state_change();

private:
	int pin;
	bool toggle;
	enum {
		BUTTON_STATE_UNKNOWN,
		BUTTON_STATE_LOW,
		BUTTON_STATE_HIGH
	} old_state;
};

class EStopButton {
public:
	EStopButton(int nc1, int nc2, int nc3, int no);

	bool engaged();
	Button nc1_b;
	Button nc2_b;
	Button nc3_b;
	Button no_b;

private:
	int nc1;
	int nc2;
	int nc3;
	int no;
};

class Buttons {
public:
	static void state_changed();

	static Button dmh;
	static Button go;
	static EStopButton e_stop;
	static Button back;
	static Button select;
};

#pragma once

class Button {
public:
	Button(int pin, bool toggle);

	bool engaged();

private:
	int pin;
	bool toggle;
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
	static bool dmhEngaged();
	static bool goEngaged();
	static bool eStopsEngaged();
	static bool inputEncoderPressed();
	static bool backPressed();

	static Button dmh;
	static Button go;
	static EStopButton e_stop;
	static Button back;
	static Button select;
};

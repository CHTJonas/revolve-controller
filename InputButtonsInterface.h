#pragma once
#include <Bounce2.h>

class InputButtonsInterface {

public:
	static bool dmhEngaged();
	static bool goEngaged();
	static bool eStopsEngaged();
	static bool inputEncoderPressed();
	static bool backPressed();

	static Bounce back;
	static Bounce inputEncoder;
};

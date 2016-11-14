#pragma once
#include <Bounce2.h>

class InputButtonsInterface
{

public:

	static bool dmhEngaged();
	static bool goEngaged();
	static bool eStopsEngaged();

	bool inputEncoderPressed();
	bool backPressed();

	Bounce back = Bounce();
	Bounce inputEncoder = Bounce();

	void waitSelectRelease();
	void waitBackRelease();
};


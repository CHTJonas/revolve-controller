#pragma once
#include <Bounce2.h>

class InputButtonsInterface
{

public:

	bool dmhEngaged();
	bool goEngaged();
	bool eStopsEngaged();
	bool inputEncoderEngaged();
	bool backEngaged();

	Bounce back = Bounce();
	Bounce inputEncoder = Bounce();

	void waitSelectRelease();
	void waitBackRelease();
};


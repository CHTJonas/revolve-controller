#pragma once
#include <Bounce2.h>

class InputButtonsInterface
{

public:
	Bounce select = Bounce();
	Bounce back = Bounce();

	void waitSelectRelease();
	void waitBackRelease();
};


#pragma once
#include "screen.h"

class Displays {
public:
	Displays(Screen *left, Screen* centre, Screen* right);

	void step();

private:
	Screen *left;
	Screen *centre;
	Screen *right;
};

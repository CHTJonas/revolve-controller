#pragma once

#include "cuestack.h"
#include "displays.h"
#include "interface.h"
#include "stage.h"
#include "state.h"

class Navigation {
public:
	Navigation(State* state, Cuestack* cuestack, Interface* interface, Displays* displays, Stage* stage);
	void setup();
	void loop();

private:
	State* state;
	Cuestack* cuestack;
	Interface* interface;
	Displays* displays;
	Stage* stage;
};

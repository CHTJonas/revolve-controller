#include "state_machine.h"
#include "buttons.h"

class Navigation {
public:
	Navigation(State *state, Buttons *buttons);

	void loop();

private:
	State *state;
	Buttons *buttons;
};

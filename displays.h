#pragma once
#include "buttons.h"
#include "screen.h"
#include "state_machine.h"

typedef struct MenuItem {
	const char* text;
	StateEnum new_state;
} MenuItem;

class Displays {
public:
	Displays(State* state, Screen* left, Screen* centre, Screen* right, Buttons* buttons);

	void step();

private:
	State* state;
	Screen* left;
	Screen* centre;
	Screen* right;
	Buttons* buttons;

	void draw_mainmenu();
	void draw_run_ready();
	void draw_run_drive();
	void draw_run_brake();
	void draw_run_estop();
	void draw_about();

	void write_menu(const MenuItem items[], const int count, const int index);
	void update_output_screen();
};

#pragma once
#include "screen.h"
#include "state_machine.h"

typedef struct MenuItem {
    const char *text;
    StateEnum new_state;
} MenuItem;

class Displays {
public:
	Displays(State *state, Screen *left, Screen* centre, Screen* right);

	void step();

private:
	State *state;
	Screen *left;
	Screen *centre;
	Screen *right;

	void draw_mainmenu();
	void draw_run_ready();
	void draw_run_drive();
	void draw_run_brake();
	void draw_estopped();
	void draw_about();

	void write_menu(const MenuItem items[], const int count, const int index);
	void update_output_screen();
};


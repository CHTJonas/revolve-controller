#pragma once

#include "state_machine.h"

typedef struct MenuItem {
	const char* text;
	StateEnum state;
} MenuItem;

const MenuItem main_menu_items[] = { { "run", STATE_RUN_READY }, { "about", STATE_ABOUT }, { "debug", STATE_DEBUG } };
const int main_menu_count = sizeof(main_menu_items) / sizeof(*main_menu_items);

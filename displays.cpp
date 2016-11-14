#include "displays.h"
#include "buttons.h"
#include "logo.h"
#include "screen.h"
#include "state_machine.h"
#include "utils.h"

Displays::Displays(State* state, Screen* left, Screen* centre, Screen* right, Buttons* buttons)
      : state(state), left(left), centre(centre), right(right), buttons(buttons) {
}

void Displays::step() {
	update_output_screen();

	switch (state->state) {
	case STATE_MAINMENU:
		draw_mainmenu();
		break;

	case STATE_RUN_READY:
		draw_run_ready();
		break;

	case STATE_RUN_DRIVE:
		draw_run_drive();
		break;

	case STATE_RUN_BRAKE:
		draw_run_brake();
		break;

	case STATE_RUN_ESTOP:
		draw_run_estop();
		break;

	case STATE_ABOUT:
		draw_about();
		break;

	case STATE_DEBUG:
		draw_debug();
		break;
	}
}

const MenuItem main_menu_items[] = { { "run", STATE_RUN_READY }, { "about", STATE_ABOUT }, { "debug", STATE_DEBUG } };

const int main_menu_count = sizeof(main_menu_items) / sizeof(*main_menu_items);

void Displays::update_output_screen() {
	char buffer[16];
	snprintf(buffer, 16, "Target: v_i: %i", 12);
	right->write_text(0, 0, buffer);
	snprintf(buffer, 16, "Output: v_i: %i", 13);
	right->write_text(0, 1, buffer);
	snprintf(buffer, 16, "DBG: s%ie%i", state->state, buttons->e_stop.engaged());
	right->write_text(0, 1, buffer);
}

void Displays::write_menu(const MenuItem items[], const int count, const int index) {
	if (index != 0) {
		centre->write_text(0, 0, items[index - 1].text);
	}
	centre->write_text(0, 1, items[index].text);
	if (index != count - 1) {
		centre->write_text(0, 2, items[index + 1].text);
	}
}

void Displays::draw_mainmenu() {
	left->draw_image(screen_logo);
	write_menu(main_menu_items, main_menu_count, state->data.mainmenu.selected_item_index);
}
void Displays::draw_run_ready() {
}
void Displays::draw_run_drive() {
}
void Displays::draw_run_brake() {
}
void Displays::draw_run_estop() {
	left->write_text(0, 1, "     ESTOPPED    ");
	centre->write_text(0, 1, "Emergency Stopped");
}
void Displays::draw_about() {
	left->draw_image(screen_logo);
	centre->write_text(0, 0, "Panto revolve controller 2016");
	centre->write_text(0, 1, "By many people");
}

void Displays::draw_debug() {
	char buffer[16];

	snprintf(
	    buffer,
	    16,
	    "dmh%i,go%i,e_stop%i,back%i,"
	    "inputEncoder%i,inner_home%i,outer_home%i",
	    buttons->dmh.engaged(),
	    buttons->go.engaged(),
	    buttons->e_stop.engaged(),
	    buttons->back.engaged(),
	    buttons->inputEncoder.engaged(),
	    buttons->inner_home.engaged(),
	    buttons->outer_home.engaged());
	left->write_text(0, 0, buffer);

#ifndef GIT_VERSION
#define GIT_VERSION "UNKNOWN"
#define GIT_VERSION_UNKNOWN
#endif

	snprintf(
			buffer,
			16,
			"version: %s",
			GIT_VERSION);
	centre->write_text(0, 0, buffer);

#ifdef GIT_VERSION_UNKNOWN
#undef GIT_VERSION
#undef GIT_VERSION_UNKNOWN
#endif

}

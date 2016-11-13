#include "displays.h"
#include "screen.h"
#include "state_machine.h"
#include "utils.h"
#include "logo.h"

Displays::Displays(State* state, Screen* left, Screen* centre, Screen* right) :
	state(state), left(left), centre(centre), right(right) {
}

void Displays::step() {
	if (eStopsEngaged()) {
		draw_estopped();
		return;
	}

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

		case STATE_ABOUT:
			draw_about();
			break;
	}

	update_output_screen();
}

const MenuItem main_menu_items[2] = {
	{"run", STATE_RUN_READY},
	{"about", STATE_ABOUT}
};

const int main_menu_count = sizeof(main_menu_items)/sizeof(*main_menu_items);

void Displays::update_output_screen() {
	char buffer[16];
	snprintf(buffer, 16, "Inner speed: %i", 12);
	right->write_text(0, 0, buffer);
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
void Displays::draw_run_ready() { }
void Displays::draw_run_drive() { }
void Displays::draw_run_brake() { }
void Displays::draw_about() {
	left->draw_image(screen_logo);
	centre->write_text(0, 0, "Panto revolve controller 2016");
	centre->write_text(0, 1, "By many people");
}
void Displays::draw_estopped() { }

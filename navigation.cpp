#include "state_machine.h"
#include "buttons.h"
#include "navigation.h"
#include "menus.h"
#include "utils.h"

Navigation::Navigation(State *state, Buttons *buttons) : state(state), buttons(buttons) {
}

void Navigation::loop() {
	switch (state->get_state()) {
	case STATE_MAINMENU:
		if (buttons->input_encoder_button.had_rising_edge()) {
			state->set_DANGEROUS_MUST_SET_DATA(main_menu_items[state->data.mainmenu.selected_item_index].state);
		}
		state->data.mainmenu.selected_item_index = clamp<long int>(
				state->data.mainmenu.selected_item_index - buttons->input_encoder.read() / 3,
				0,
				main_menu_count - 1);
		buttons->input_encoder.write(
				buttons->input_encoder.read() % 3);
		break;
	default:
		break;
	}
}

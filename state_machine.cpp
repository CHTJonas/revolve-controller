#include "state_machine.h"
#include "buttons.h"

State::State(Buttons *buttons): buttons(buttons) {
}

void State::set_mainmenu() {
	buttons->state_changed();
	state = STATE_MAINMENU;
	data.mainmenu = {};
}
void State::set_run_ready() {
	buttons->state_changed();
	state = STATE_RUN_READY;
	data.run_ready = {};
}
void State::set_run_drive() {
	buttons->state_changed();
	state = STATE_RUN_DRIVE;
	data.run_drive = {};
}
void State::set_run_brake(
    unsigned long start_time,
    long inner_start_speed,
    long outer_start_speed,
    bool inner_at_speed,
    bool outer_at_speed) {
	buttons->state_changed();
	state = STATE_RUN_BRAKE;
	data.run_brake = {.start_time = start_time,
		          .inner_start_speed = inner_start_speed,
		          .outer_start_speed = outer_start_speed,
		          .inner_at_speed = inner_at_speed,
		          .outer_at_speed = outer_at_speed };
}
void State::set_run_estop() {
	buttons->state_changed();
	state = STATE_RUN_ESTOP;
	data.run_estop = {};
}
void State::set_about() {
	buttons->state_changed();
	state = STATE_ABOUT;
	data.about = {};
}
void State::set_debug() {
	buttons->state_changed();
	state = STATE_DEBUG;
	data.debug = {};
}

void State::set_DANGEROUS_MUST_SET_DATA(StateEnum _state) {
	buttons->state_changed();
	state = _state;
}

StateEnum State::get_state() {
	return state;
}

#include "state_machine.h"

State::State() {
}

void State::set_mainmenu() {
	state = STATE_MAINMENU;
	data.mainmenu = {};
}
void State::set_run_ready() {
	state = STATE_RUN_READY;
	data.run_ready = {};
}
void State::set_run_drive() {
	state = STATE_RUN_DRIVE;
	data.run_drive = {};
}
void State::set_run_brake(
    unsigned long start_time,
    long inner_start_speed,
    long outer_start_speed,
    bool inner_at_speed,
    bool outer_at_speed) {
	state = STATE_RUN_BRAKE;
	data.run_brake = {
		.start_time = start_time,
		.inner_start_speed = inner_start_speed,
		.outer_start_speed = outer_start_speed,
		.inner_at_speed = inner_at_speed,
		.outer_at_speed = outer_at_speed
	};
}
void State::set_run_estop() {
	state = STATE_RUN_ESTOP;
	data.run_estop = {};
}
void State::set_about() {
	state = STATE_ABOUT;
	data.about = {};
}
void State::set_debug() {
	state = STATE_DEBUG;
	data.debug = {};
}

void State::set_DANGEROUS_MUST_SET_DATA(StateEnum _state) {
	state = _state;
}

StateEnum State::get_state() {
	return state;
}

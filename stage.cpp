#include "stage.h"
#include "utils.h"
#include <EEPROM.h>

Stage::Stage(
    State* state, Revolve* inner, Revolve* outer, Interface* interface, Adafruit_NeoPixel* ringLeds, Buttons* buttons)
      : state(state), inner(inner), outer(outer), interface(interface), ringLeds(ringLeds), buttons(buttons) {
	updateEncRatios();
	updateKpSettings();
}

void Stage::loop() {

	switch (state->get_state()) {
	case STATE_RUN_READY:
		checkEstops();
		ready();
		break;

	case STATE_RUN_DRIVE:
		checkEstops();
		drive();
		break;

	case STATE_RUN_BRAKE:
		checkEstops();
		brake();
		break;  // but not break the brake. Because that'd be bad. Probably...

	case STATE_RUN_ESTOP:
		if (!buttons->e_stop.engaged()) {
			state->set_run_ready();
		}
		break;

	default:
		break;
	}
}

void Stage::setupDrive(
    int position, int speed, int acceleration, int direction, int revolutions, DriveData* data, Revolve* wheel) {
	double kp, currentPosition, setPosition, currentSpeed = 0.;
	currentPosition = wheel->getPos();
	wheel->setDir(direction);

	auto directionBoolean = wheel->getDir() == FORWARDS;
	revolutions += (directionBoolean == (position < inner->displayPos()));
	setPosition =
	    currentPosition + position - inner->displayPos() + (directionBoolean ? 1 : -1) * 360 * revolutions;

	speed = clamp(abs(speed), MINSPEED + 1, 100);
	acceleration = clamp(abs(acceleration), 1, MAXACCEL);
	auto tenths_accel = (acceleration) / 10.0;

	kp = wheel->kp_0 + ((100 - speed) * wheel->kp_smin) / 100 + ((acceleration)*wheel->kp_amax) / (MAXACCEL);

	*data = DriveData{ currentPosition, currentSpeed, setPosition, directionBoolean, tenths_accel, nullptr };

	setupPid(speed, kp, data, wheel);
}

/***** Stage states *******/

void Stage::ready() {
	if (buttons->dmh.engaged() && buttons->go.had_rising_edge()) {
		state->set_run_drive();
		auto cue = interface->cuestack->stack[interface->cuestack->currentCue];
		setupDrive(
		    cue.pos_i, cue.speed_i, cue.acc_i, cue.dir_i, cue.revs_i, &state->data.run_drive.innerData, inner);
		setupDrive(
		    cue.pos_o, cue.speed_o, cue.acc_o, cue.dir_o, cue.revs_o, &state->data.run_drive.outerData, outer);
		return;
	}
}

void Stage::drive() {
	if (!buttons->dmh.engaged()) {
		state->set_run_brake(millis(), inner->getSpeed(), outer->getSpeed(), false, false);
		return;
	}

	auto innerDriveData = state->data.run_drive.innerData;
	auto outerDriveData = state->data.run_drive.outerData;

	auto inner_done =
	    innerDriveData.directionBoolean != (innerDriveData.currentPosition < innerDriveData.setPosition);
	auto outer_done =
	    outerDriveData.directionBoolean != (outerDriveData.currentPosition < outerDriveData.setPosition);

	if (inner_done && outer_done) {
		state->set_run_ready();
		return;
	}

	if (!inner_done) {
		spin_revolve(
		    &innerDriveData.currentPosition,
		    &innerDriveData.currentSpeed,
		    innerDriveData.tenths_accel,
		    innerDriveData.pid,
		    inner);
	} else {
		inner->setSpeed(0);
	}

	if (!outer_done) {
		spin_revolve(
		    &outerDriveData.currentPosition,
		    &outerDriveData.currentSpeed,
		    outerDriveData.tenths_accel,
		    outerDriveData.pid,
		    outer);
	} else {
		outer->setSpeed(0);
	}
}

void Stage::brake() {
	if (buttons->dmh.engaged() && buttons->go.had_rising_edge()) {
		state->set_run_ready();
		return;
	}

	const unsigned long inner_speed = state->data.run_brake.inner_at_speed
	    ? 0UL
	    : state->data.run_brake.inner_start_speed - (millis() - state->data.run_brake.start_time) * acceleration;

	const unsigned long outer_speed = state->data.run_brake.outer_at_speed
	    ? 0UL
	    : state->data.run_brake.outer_start_speed - (millis() - state->data.run_brake.start_time) * acceleration;

	state->data.run_brake.inner_at_speed = (inner_speed == 0);
	state->data.run_brake.outer_at_speed = (outer_speed == 0);

	if (state->data.run_brake.inner_at_speed && state->data.run_brake.outer_at_speed) {
		state->set_run_ready();
	}

	inner->setSpeed(inner_speed);
	outer->setSpeed(outer_speed);
}

/***** Emergency Stop *****/

void Stage::checkEstops() {
	if (buttons->e_stop.engaged()) {
		inner->setSpeed(0);
		outer->setSpeed(0);
		state->set_run_estop();
	}
}

/**** Drive functions *****/

void Stage::spin_revolve(double* currentPosition, double* currentSpeed, double tenths_accel, PID* pid, Revolve* wheel) {
	// Update position and compute PID
	*currentPosition = inner->getPos();
	pid->Compute();

	// Limit acceleration
	if (wheel->tenths >= 1) {
		const auto allowedSpeed =
		    clamp(*currentSpeed, wheel->cur_speed - tenths_accel, wheel->cur_speed + tenths_accel);
		wheel->setSpeed(allowedSpeed);
		wheel->tenths = 0;
	}
}

void Stage::setupPid(int maxSpeed, double kp, DriveData* data, Revolve* wheel) {
	auto mode = data->setPosition < data->currentPosition ? REVERSE : DIRECT;
	auto pid = PID(&data->currentPosition, &data->currentSpeed, &data->setPosition, kp, wheel->ki, wheel->kd, mode);
	pid.SetOutputLimits(MINSPEED, maxSpeed);
	pid.SetSampleTime(75);
	pid.SetMode(AUTOMATIC);

	data->pid = &pid;
}

/***** Cues *****/

void Stage::runCurrentCue() {
	// Turn off switch leds
	digitalWrite(SELECTLED, LOW);
	digitalWrite(GOLED, LOW);

	// Flag to recursively call function if required
	int auto_follow = interface->cueParams[0];

	// Move - both enabled
	if (interface->cueParams[1] && interface->cueParams[2]) {
		// gotoPos();
	}
	// Move - inner disabled
	else if (interface->cueParams[1] == 0 && interface->cueParams[2]) {
		// gotoPos(); // TODO pull parameters from git history
	}
	// Move - outer disabled
	else if (interface->cueParams[1] && interface->cueParams[2] == 0) {
		// gotoPos();
	}

	// Increment cue to currently selected cue with menu_pos, and increase menu_pos to automatically select next one
	interface->cuestack->currentCue = interface->menu_pos;
	if (interface->menu_pos < (interface->cuestack->totalCues - 1))
		interface->menu_pos++;

	// Load next cue data
	interface->loadCue(interface->menu_pos);

	// Recursively call runCurrentCue whilst previous cue had autofollow enabled, unless we are at last cue (where
	// menu_pos = currentCue as it won't have been incremented)
	if (auto_follow && interface->cuestack->currentCue != interface->menu_pos)
		runCurrentCue();

	// Turn on switch leds
	digitalWrite(SELECTLED, HIGH);
	digitalWrite(GOLED, HIGH);
}

/***** Wheel homing *****/

void Stage::gotoHome() {
	home_wheel(inner, INNERHOME);
	// Set inner ring green
	for (int i = 12; i < 24; i++) {
		ringLeds->setPixelColor(i, 0, 255, 0);
	}
	ringLeds->show();

	home_wheel(outer, OUTERHOME);

	// Move back to calibrated home (will have overshot)
	// gotoPos(); // TODO get parameters from git history
}

void Stage::home_wheel(Revolve* wheel, int wheelPin) {
	// Spin until home switch pressed
	wheel->setDir(FORWARDS);
	wheel->setSpeed(HOMESPEED);

	// Wait for inner home switch
	while (digitalRead(wheelPin)) {

		// Check for emergency stop
		if (!buttons->dmh.engaged() || buttons->e_stop.engaged()) {
			emergencyStop();

			// Restart
			digitalWrite(GOLED, LOW);
			wheel->setSpeed(HOMESPEED);

			// TODO check this
			// below code was origianlly only in E-Stops
			if (digitalRead(DMH) == LOW && digitalRead(GO) == LOW) {
				wheel->setSpeed(HOMESPEED);
			}
		}
	}
	// Reset at home pin
	wheel->resetPos();

	// Stop
	wheel->setSpeed(0);
}

/***** Import EEPROM data *****/

void Stage::updateEncRatios() const {
	EEPROM.get(EEINNER_ENC_RATIO, inner->enc_ratio);
	EEPROM.get(EEOUTER_ENC_RATIO, outer->enc_ratio);
}

void Stage::updateKpSettings() const {
	double kpSettings[6];
	EEPROM.get(EEKP_SETTINGS, kpSettings);

	inner->kp_0 = kpSettings[0];
	inner->kp_smin = kpSettings[1];
	inner->kp_amax = kpSettings[2];
	outer->kp_0 = kpSettings[3];
	outer->kp_smin = kpSettings[4];
	outer->kp_amax = kpSettings[5];
}

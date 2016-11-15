#include "stage.h"
#include "utils.h"
#include "pins.h"
#include <EEPROM.h>
#include "revolve_controller.h"

Stage::Stage(
    State* state, Revolve* inner, Revolve* outer, Displays* displays, Interface* interface, Adafruit_NeoPixel* ringLeds)
      : state(state), inner(inner), outer(outer), displays(displays), interface(interface), ringLeds(ringLeds) {
	updateEncRatios();
	updateKpSettings();
}

void Stage::loop() {

	switch (state->state) {

	case STATE_RUN_READY:
		checkEstops();
		run_ready();
		break;

	case STATE_RUN_DRIVE:
		checkEstops();
		run_drive();
		break;

	case STATE_RUN_BRAKE:
		checkEstops();
		run_brake();
		break;  // but not break the brake. Because that'd be bad. Probably...

	case STATE_MANUAL_READY:
		checkEstops();
		manual_ready();
		break;

	case STATE_MANUAL_DRIVE:
		checkEstops();
		manual_drive();
		break;

	case STATE_MANUAL_BRAKE:
		checkEstops();
		manual_brake();
		break;

	case STATE_PROGRAM_MAIN:
		checkEstops();
		goToCurrentCue(PROGRAM);
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

/***** Set stage states **********/
void Stage::setStateRunReady() {
	state->state = STATE_RUN_READY;
	state->data.run_ready = {};
}

void Stage::setStateRunDrive() {
	auto cue = interface->cuestack.stack[interface->cuestack.currentCue];
	state->state = STATE_RUN_DRIVE;
	state->data.run_drive = {.innerData = { 0, 0, 0, 0, 0, nullptr }, .outerData = { 0, 0, 0, 0, 0, nullptr } };
	setupDrive(cue.pos_i, cue.speed_i, cue.acc_i, cue.dir_i, cue.revs_i, &state->data.run_drive.innerData, inner);
	setupDrive(cue.pos_o, cue.speed_o, cue.acc_o, cue.dir_o, cue.revs_o, &state->data.run_drive.outerData, outer);
}

void Stage::setStateRunBrake() {
	state->state = STATE_RUN_BRAKE;
	state->data.run_brake = { millis(), inner->getSpeed(), outer->getSpeed(), false, false };
}

void Stage::setStateManualReady() {
	state->state = STATE_MANUAL_READY;
	state->data.manual_ready = {};
}

void Stage::setStateManualDrive() {
	state->state = STATE_MANUAL_DRIVE;
	state->data.manual_drive = {}; // FIXME
}

void Stage::setStateManualBrake() {
	state->state = STATE_MANUAL_BRAKE;
	state->data.manual_brake = {}; // FIXME
}

/***** Stage states *******/

void Stage::run_ready() {
	if (Buttons::dmh.engaged() && Buttons::go.engaged()) {
		setStateRunDrive();
		return;
	}
}

void Stage::run_drive() {
	if (!Buttons::dmh.engaged()) {
		setStateRunBrake();
		return;
	}

	auto innerDriveData = state->data.run_drive.innerData;
	auto outerDriveData = state->data.run_drive.outerData;

	auto inner_done =
	    innerDriveData.directionBoolean != (innerDriveData.currentPosition < innerDriveData.setPosition);
	auto outer_done =
	    outerDriveData.directionBoolean != (outerDriveData.currentPosition < outerDriveData.setPosition);

	if (inner_done && outer_done) {
		setStateRunReady();
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

void Stage::run_brake() {
	if (Buttons::dmh.engaged() && Buttons::go.engaged()) {
		setStateRunDrive();
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
		setStateRunReady();
	}

	inner->setSpeed(inner_speed);
	outer->setSpeed(outer_speed);
}

void Stage::manual_ready() {
	if (Buttons::dmh.engaged() && Buttons::go.engaged()) {
		setStateManualDrive();
	}
}

void Stage::manual_drive() {
	if (!Buttons::dmh.engaged()) {
		setStateManualBrake();
	}

	// FIXME: move
}

void Stage::manual_brake() {
	if (Buttons::dmh.engaged() && Buttons::go.engaged()) {
		setStateManualDrive();
	}

	// FIXME: stop
}

bool Stage::checkEstops() {
	if (Buttons::e_stop.engaged()) {
		emergencyStop();
		return true;
	} else {
		return false;
	}
}

void Stage::emergencyStop() {
	inner->setSpeed(0);
	outer->setSpeed(0);

	while (Buttons::e_stop.engaged()) {
        // loop endlessly and do nothing until emergency
        // stop buttons are all disengaged
	}

	state->state = STATE_RUN_READY;
	state->data = {};
}

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
	interface->cuestack.currentCue = interface->menu_pos;
	if (interface->menu_pos < (interface->cuestack.totalCues - 1))
		interface->menu_pos++;

	// Load next cue data
	interface->loadCue(interface->menu_pos);

	// Update displays
	displays->forceUpdateDisplays(1, 1, 1, 1);

	// Recursively call runCurrentCue whilst previous cue had autofollow enabled, unless we are at last cue (where
	// menu_pos = currentCue as it won't have been incremented)
	if (auto_follow && interface->cuestack.currentCue != interface->menu_pos)
		runCurrentCue();

	// Turn on switch leds
	digitalWrite(SELECTLED, HIGH);
	digitalWrite(GOLED, HIGH);
}

/***** Wheel homing *****/

void Stage::gotoHome() {
	state->state = STATE_HOMING_INPROGRESS;
	state->data.homing_inprogress = {};
	displays->setMode();

	home_wheel(inner, INNERHOME);
	// Set inner ring green
	for (int i = 12; i < 24; i++) {
		ringLeds->setPixelColor(i, 0, 255, 0);
	}
	ringLeds->show();

	home_wheel(outer, OUTERHOME);

	state->state = STATE_HOMING_COMPLETE;
	state->data.homing_complete = {};
	displays->setMode();
	// Move back to calibrated home (will have overshot)
	// gotoPos(); // TODO get parameters from git history

	state->state = STATE_MAINMENU;
	state->data.mainmenu = {};
	displays->setMode();
}

void Stage::home_wheel(Revolve* wheel, int wheelPin) {
	// Spin until home switch pressed
	wheel->setDir(FORWARDS);
	wheel->setSpeed(HOMESPEED);

	// Wait for inner home switch
	while (digitalRead(wheelPin)) {

		// Check for emergency stop
		if (!Buttons::dmh.engaged() || Buttons::e_stop.engaged()) {
			emergencyStop();

			if (Buttons::dmh.engaged() && Buttons::go.engaged()) {
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

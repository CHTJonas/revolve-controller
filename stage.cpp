#include "stage.h"

Stage::Stage(Revolve& inner, Revolve& outer, Displays& displays, Interface& interface, Adafruit_NeoPixel& ringLeds) : _inner(inner), _outer(outer), _displays(displays), _interface(interface), _ringLeds(ringLeds) {
	updateEncRatios();
	updateKpSettings();
}

// Updates revolve enc_ratios from EEPROM
void Stage::updateEncRatios() const
{
	EEPROM.get(EEINNER_ENC_RATIO, _inner._enc_ratio);
	EEPROM.get(EEOUTER_ENC_RATIO, _outer._enc_ratio);
}

// Updates kp values from EEPROM
void Stage::updateKpSettings() const
{
	double kpSettings[6];
	EEPROM.get(EEKP_SETTINGS, kpSettings);

	_inner._kp_0 = kpSettings[0];
	_inner._kp_smin = kpSettings[1];
	_inner._kp_amax = kpSettings[2];
	_outer._kp_0 = kpSettings[3];
	_outer._kp_smin = kpSettings[4];
	_outer._kp_amax = kpSettings[5];
}

void Stage::home_wheel(Revolve& wheel, int wheelPin)
{
	// Spin until home switch pressed
	wheel.setDir(FORWARDS);
	wheel.setSpeed(HOMESPEED);

	// Wait for inner home switch
	while (digitalRead(wheelPin)) {

		// Check for emergency stop
		if (digitalRead(PAUSE) || eStopsEngaged()) {
			emergencyStop();

			// Restart
			_interface.pauseLedsColor(0, 255, 0);
			digitalWrite(GOLED, LOW);
			wheel.setSpeed(HOMESPEED);

			// TODO check this
			// below code was origianlly only in E-Stops
			if (digitalRead(PAUSE) == LOW && digitalRead(GO) == LOW) {
				wheel.setSpeed(HOMESPEED);
			}
		}
	}
	// Reset at home pin
	wheel.resetPos();

	// Stop
	wheel.setSpeed(0);
}

// Initial homing sequence
void Stage::gotoHome()
{
	_displays.setMode(HOMING);

	home_wheel(_inner, INNERHOME);
	// Set inner ring green
	for (int i = 12; i < 24; i++) {
		_ringLeds.setPixelColor(i, 0, 255, 0);
	}
	_ringLeds.show();

	home_wheel(_outer, OUTERHOME);
	// Set outer ring green
	_interface.ringLedsColor(0, 255, 0);

	_displays.setMode(HOMED);
	// Move back to calibrated home (will have overshot)
	gotoPos(0, 0, MINSPEED, MINSPEED, 1, 1, BACKWARDS, BACKWARDS, 0, 0);

	_displays.setMode(NORMAL);
}

void Stage::emergencyStop()
{
	_inner.setSpeed(0);
	_outer.setSpeed(0);

	_state.state = REVOLVE_ESTOP;
	_state.data.estop = {};
	_displays.setMode(ESTOP); // TODO

	// hold until we're ready to go again
	while (eStopsEngaged()) {
	}

	_state.state = REVOLVE_READY;
}

void Stage::setStateReady()
{
	_state.state = REVOLVE_READY;
	_state.data.ready = {};
}

void Stage::setStateDrive()
{
	_state.state = REVOLVE_DRIVE;
	_state.data.drive = {};
}

void Stage::setStateBrake(unsigned long start_time, long inner_start_position, long outer_start_position)
{
	_state.state = REVOLVE_BRAKE;
	_state.data.brake = { start_time, inner_start_position, outer_start_position };
}

void Stage::runStage()
{
	_state.state = REVOLVE_READY;
	_state.data.ready = {};

	while (true) {
		checkEstops();

		switch (_state.state) {

		case REVOLVE_READY:
			if (dmhEngaged() && goEngaged()) {
				setStateDrive();
			}
			break;

		case REVOLVE_DRIVE:
			if (!dmhEngaged()) {
				setStateBrake(millis(), _inner.getPos(), _outer.getPos());
			}
			break;

		case REVOLVE_BRAKE:
			if (dmhEngaged() && goEngaged()) {
				setStateDrive();
			}
			brake();
			break;

		default:
			break;
		}
	}
}

void Stage::brake()
{
	int inner_speed = (_inner.getPos() - _state.data.brake.inner_start_position) / (millis() - _state.data.brake.start_time);
	int outer_speed = (_outer.getPos() - _state.data.brake.outer_start_position) / (millis() - _state.data.brake.start_time);

	if (checkEstops())
	{
		return;
	}
	_inner.setSpeed(max(0, inner_speed - _pause_max_speed));
	_outer.setSpeed(max(0, outer_speed - _pause_max_speed));
}

bool Stage::dmhEngaged()
{
	return !digitalRead(PAUSE);
}

bool Stage::goEngaged()
{
	return !digitalRead(GO);
}

bool Stage::checkEstops()
{
	if (eStopsEngaged())
	{
		emergencyStop();
		return true;
	}
	else {
		return false;
	}
}


bool Stage::eStopsEngaged()
{
	// Commented out line for non-conencted external esstop testing
	//if !(digitalRead(ESTOPNC1)==LOW && digitalRead(ESTOPNC2)==LOW && digitalRead(ESTOPNC3)==LOW && digitalRead(ESTOPNO)==HIGH){
	return !(digitalRead(ESTOPNC1) == LOW && digitalRead(ESTOPNO) == HIGH);
}

void Stage::resumeDrive(int restartSpeed) const
{

}

void Stage::deadMansRestart(int restartSpeed) const
{
	auto restart = 0;

	// Update LEDs
	_interface.updatePauseLeds();
	digitalWrite(GOLED, HIGH);

	// Update Displays
	_displays.forceUpdateDisplays(1, 1, 1, 1);
	_interface.select.update();

	// Wait for repress
	while (_interface.select.read() && !restart) {
		_interface.updatePauseLeds();
		_interface.select.update();

		// Restart if pause and go pressed
		if (digitalRead(PAUSE) == LOW && digitalRead(GO) == LOW) {
			_inner.setSpeed(restartSpeed);
			_outer.setSpeed(restartSpeed);

			// Update LEDs
			_interface.updatePauseLeds();
			digitalWrite(GOLED, LOW);

			// Re enter main loop
			restart = 1;
		}
	}
}

void Stage::gotoPos(int pos_inner, int pos_outer, int maxSpeed_inner, int maxSpeed_outer, int accel_inner, int accel_outer, int dir_inner, int dir_outer, int revs_inner, int revs_outer)
{
	// PID setup variables
	double kp_inner, curPos_inner, setPos_inner, curSpeed_inner;
	double kp_outer, curPos_outer, setPos_outer, curSpeed_outer;

	// Update current absolute position
	curPos_inner = _inner.getPos();
	curPos_outer = _outer.getPos();

	// Set direction
	_inner.setDir(dir_inner);
	_outer.setDir(dir_outer);

	// Set final absolute position using revs input

	// Inner Wheel
	auto revolutionDegrees = 360 * revs_inner;
	auto inner_sign = _inner.getDir() == FORWARDS ? 1 : -1;
	revolutionDegrees += (inner_sign ^ (pos_inner < _inner.displayPos())) ? 0 : 360;
	setPos_inner = curPos_inner + pos_inner - _inner.displayPos() + inner_sign * revolutionDegrees;

	// Outer Wheel
	revolutionDegrees = 360 * revs_outer;
	auto outer_sign = _outer.getDir() == FORWARDS ? 1 : -1;
	revolutionDegrees += (outer_sign ^ (pos_outer < _outer.displayPos())) ? 0 : 360;
	setPos_outer = curPos_outer + pos_outer - _outer.displayPos() + outer_sign * revolutionDegrees;

	// Sanitise speed input
	maxSpeed_inner = abs(maxSpeed_inner);
	maxSpeed_inner = min(100, maxSpeed_inner);
	maxSpeed_inner = max(MINSPEED + 1, maxSpeed_inner);

	maxSpeed_outer = abs(maxSpeed_outer);
	maxSpeed_outer = min(100, maxSpeed_outer);
	maxSpeed_outer = max(MINSPEED + 1, maxSpeed_outer);

	// Sanitise max acceleration input
	accel_inner = abs(accel_inner);
	accel_inner = min(MAXACCEL, accel_inner);
	accel_inner = max(1, accel_inner);

	accel_outer = abs(accel_outer);
	accel_outer = min(MAXACCEL, accel_outer);
	accel_outer = max(1, accel_outer);

	// Convert accel from increase/second to increase/(1/10) second
	float tenths_accel_inner = static_cast<float>(accel_inner) / 10.0;
	float tenths_accel_outer = static_cast<float>(accel_outer) / 10.0;

	// Calculate kp for this move using constants from inital constructor and required speed, acceleration
	kp_inner = _inner._kp_0 + ((100 - maxSpeed_inner) * _inner._kp_smin) / 100 + ((accel_inner)* _inner._kp_amax) / (MAXACCEL);
	kp_outer = _outer._kp_0 + ((100 - maxSpeed_outer) * _outer._kp_smin) / 100 + ((accel_outer)* _outer._kp_amax) / (MAXACCEL);

	// Setup PID object (flip to reverse mode if negative)
	PID pid_inner = setPos_inner < curPos_inner ? pid_inner = PID(&curPos_inner, &curSpeed_inner, &setPos_inner, kp_inner, _inner._ki, _inner._kd, REVERSE) : pid_inner = PID(&curPos_inner, &curSpeed_inner, &setPos_inner, kp_inner, _inner._ki, _inner._kd, DIRECT);
	pid_inner.SetOutputLimits(MINSPEED, maxSpeed_inner);
	pid_inner.SetSampleTime(75);
	pid_inner.SetMode(AUTOMATIC);

	PID pid_outer = setPos_outer < curPos_outer ? pid_outer = PID(&curPos_outer, &curSpeed_outer, &setPos_outer, kp_outer, _outer._ki, _outer._kd, REVERSE) : pid_outer = PID(&curPos_outer, &curSpeed_outer, &setPos_outer, kp_outer, _outer._ki, _outer._kd, DIRECT);
	pid_outer.SetOutputLimits(MINSPEED, maxSpeed_outer);
	pid_outer.SetSampleTime(75);
	pid_outer.SetMode(AUTOMATIC);

	auto inner_done = 0;
	auto outer_done = 0;

	_interface.select.update();

	while ((inner_done == 0 || outer_done == 0) && _interface.select.read() == HIGH) { // Stop cue if select pressed mid way
		// Update Select debounce and displays
		_interface.select.update();
		_displays.updateDisplays(0, 0, 1, 1);

		// Inner revolve
		if ((inner_sign && curPos_inner < setPos_inner) || (!inner_sign && curPos_inner > setPos_inner)) {
			// Update position and compute PID
			curPos_inner = _inner.getPos();
			pid_inner.Compute();

			// Limit acceleration
			if (_inner._tenths >= 1) {
				auto allowedSpeed = min(curSpeed_inner, _inner._cur_speed + tenths_accel_inner);
				allowedSpeed = max(allowedSpeed, _inner._cur_speed - tenths_accel_inner);
				_inner.setSpeed(allowedSpeed);
				_inner._tenths = 0;
			}
		}
		// Otherwise done, set done flag
		else if (inner_done == 0) {
			inner_done = 1;
			// Stop once position reached
			_inner.setSpeed(0);
		}

		// Outer
		if ((outer_sign && curPos_outer < setPos_outer) || (!outer_sign && curPos_outer > setPos_outer)) {
			// Update position and compute PID
			curPos_outer = _outer.getPos();
			pid_outer.Compute();

			// Limit acceleration
			if (_outer._tenths >= 1) {
				auto allowedSpeed = min(curSpeed_outer, _outer._cur_speed + tenths_accel_inner);
				allowedSpeed = max(allowedSpeed, _outer._cur_speed - tenths_accel_inner);
				_outer.setSpeed(allowedSpeed);
				_outer._tenths = 0;
			}
		}
		// Otherwise done, set done flag
		else if (outer_done == 0) {
			outer_done = 1;
			// Stop once position reached
			_outer.setSpeed(0);
		}

	}


	// Stop revolves if Select has been pressed
	if (outer_done == 0)
		_outer.setSpeed(0);
	if (inner_done == 0)
		_inner.setSpeed(0);
	_displays.forceUpdateDisplays(0, 0, 1, 1);
	_interface.waitSelectRelease();
}

void Stage::runCurrentCue()
{
	// Turn off switch leds
	_interface.encOff();
	digitalWrite(SELECTLED, LOW);
	digitalWrite(GOLED, LOW);

	// Flag to recursively call function if required
	int auto_follow = _interface.cueParams[0];

	// Move - both enabled
	if (_interface.cueParams[1] && _interface.cueParams[2]) {
		gotoPos(_interface.cueMovements[0], _interface.cueMovements[5], _interface.cueMovements[1], _interface.cueMovements[6], _interface.cueMovements[2], _interface.cueMovements[7], _interface.cueMovements[3], _interface.cueMovements[8], _interface.cueMovements[4], _interface.cueMovements[9]);
	}
	// Move - inner disabled
	else if (_interface.cueParams[1] == 0 && _interface.cueParams[2]) {
		gotoPos(_inner.displayPos(), _interface.cueMovements[5], MINSPEED, _interface.cueMovements[6], 1, _interface.cueMovements[7], 0, _interface.cueMovements[8], 0, _interface.cueMovements[9]);
	}
	// Move - outer disabled
	else if (_interface.cueParams[1] && _interface.cueParams[2] == 0) {
		gotoPos(_interface.cueMovements[0], _outer.displayPos(), _interface.cueMovements[1], MINSPEED, _interface.cueMovements[2], 1, _interface.cueMovements[3], 0, _interface.cueMovements[4], 0);
	}


	// Increment cue to currently selected cue with menu_pos, and increase menu_pos to automatically select next one
	_interface._cuestack.currentCue = _interface.menu_pos;
	if (_interface.menu_pos < (_interface._cuestack.totalCues - 1))
		_interface.menu_pos++;

	// Load next cue data
	_interface.loadCue(_interface.menu_pos);

	// Update _displays
	_displays.forceUpdateDisplays(1, 1, 1, 1);

	// Recursively call runCurrentCue whilst previous cue had autofollow enabled, unless we are at last cue (where menu_pos = currentCue as it won't have been incremented)
	if (auto_follow && _interface._cuestack.currentCue != _interface.menu_pos)
		runCurrentCue();

	// Turn on switch leds
	digitalWrite(SELECTLED, HIGH);
	digitalWrite(GOLED, HIGH);
}

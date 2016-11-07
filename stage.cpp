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

void Stage::setStateDrive(long unsigned inner_target_speed, long unsigned outer_target_speed, long unsigned inner_target_position, long unsigned outer_target_position)
{
	_state.state = REVOLVE_DRIVE;
	_state.data.drive = { millis(), _inner.getSpeed(), _outer.getSpeed(), inner_target_speed, outer_target_speed, inner_target_position, outer_target_position, false, false };
}

void Stage::setStateBrake()
{
	_state.state = REVOLVE_BRAKE;
	_state.data.brake = { millis(), _inner.getSpeed(), _outer.getSpeed(), false, false };
}

void Stage::runStage()
{
	_state.state = REVOLVE_READY;
	_state.data.ready = {};

	while (true) {
		checkEstops();

		switch (_state.state) {

		case REVOLVE_READY:
			ready();
			break;

		case REVOLVE_DRIVE:
			drive();
			break;

		case REVOLVE_BRAKE:
			brake();
			break;

		default:
			break;
		}
	}
}

void Stage::ready() {
	if (dmhEngaged() && goEngaged()) {
		setStateDrive(0, 0, 0, 0);
	}
}

void Stage::brake()
{
	if (dmhEngaged() && goEngaged()) {
		setStateDrive(0, 0, 0, 0);
	}

	const unsigned long inner_speed = [&] () {
		if (_state.data.brake.inner_at_speed)
			return 0UL;
		else 
			return _state.data.brake.inner_start_speed - (millis() - _state.data.brake.start_time) * _acceleration;
	}();
	const unsigned long outer_speed = [&] () {
		if (_state.data.brake.outer_at_speed)
			return 0UL;
		else 
			return _state.data.brake.outer_start_speed - (millis() - _state.data.brake.start_time) * _acceleration;
	}();

	if (inner_speed == 0) {
		_state.data.brake.inner_at_speed = true;
	}

	if (outer_speed == 0) {
		_state.data.brake.outer_at_speed = true;
	}

	if (_state.data.brake.inner_at_speed && _state.data.brake.outer_at_speed) {
		setStateReady();
	}

	_inner.setSpeed(inner_speed);
	_outer.setSpeed(outer_speed);
}

void Stage::drive()
{
	if (!dmhEngaged()) {
		setStateBrake();
	}

	_inner.setSpeed(0);
	_outer.setSpeed(0);
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

template <class T>
constexpr const T& clamp( const T& v, const T& lo, const T& hi ) {
	return max(min(v, hi), lo);
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
	maxSpeed_inner = clamp(abs(maxSpeed_inner), MINSPEED + 1, 100);
	maxSpeed_outer = clamp(abs(maxSpeed_outer), MINSPEED + 1, 100);

	// Sanitise max acceleration input
	accel_inner = clamp(abs(accel_inner), 1, MAXACCEL);
	accel_outer = clamp(abs(accel_outer), 1, MAXACCEL);

	// Convert accel from increase/second to increase/(1/10) second
	double tenths_accel_inner = static_cast<double>(accel_inner) / 10.0;
	double tenths_accel_outer = static_cast<double>(accel_outer) / 10.0;

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

	auto inner_done = false;
	auto outer_done = false;

	while (!inner_done || !outer_done) { 
		inner_done = !(inner_sign && curPos_inner < setPos_inner) || (!inner_sign && curPos_inner > setPos_inner);
		outer_done = !(outer_sign && curPos_outer < setPos_outer) || (!outer_sign && curPos_outer > setPos_outer);

		// Inner revolve
		if (!inner_done) {
			// Update position and compute PID
			curPos_inner = _inner.getPos();
			pid_inner.Compute();

			// Limit acceleration
			if (_inner._tenths >= 1) {
				const auto allowedSpeed = clamp(curSpeed_inner, _inner._cur_speed - tenths_accel_inner, _inner._cur_speed + tenths_accel_inner);
				_inner.setSpeed(allowedSpeed);
				_inner._tenths = 0;
			}
		}
		else {
			_inner.setSpeed(0);
		}

		// Outer
		if (!outer_done) {
			// Update position and compute PID
			curPos_outer = _outer.getPos();
			pid_outer.Compute();

			// Limit acceleration
			if (_outer._tenths >= 1) {
				const auto allowedSpeed = clamp(curSpeed_outer, _outer._cur_speed - tenths_accel_outer, _outer._cur_speed + tenths_accel_outer);
				_outer.setSpeed(allowedSpeed);
				_outer._tenths = 0;
			}
		}
		else {
			_outer.setSpeed(0);
		}
	}
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

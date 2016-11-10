#include "stage.h"

template <class T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
	return max(min(v, hi), lo);
}

Stage::Stage(Revolve* inner, Revolve* outer, Displays* displays, Interface *interface, Adafruit_NeoPixel *ringLeds) : _inner(inner), _outer(outer), _displays(displays), _interface(interface), _ringLeds(ringLeds) {
	updateEncRatios();
	updateKpSettings();
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
			break; // but not break the brake. cause that'd be bad. probably.

		default:
			break;
		}
	}
}

/***** Set stage states **********/
void Stage::setStateReady()
{
	_state.state = REVOLVE_READY;
	_state.data.ready = {};
}

void Stage::setStateDrive()
{
	_state.state = REVOLVE_DRIVE;
}

void Stage::setStateBrake()
{
	_state.state = REVOLVE_BRAKE;
	_state.data.brake = { millis(), _inner->getSpeed(), _outer->getSpeed(), false, false };
}

/***** Stage states *******/

void Stage::ready() {
	if (dmhEngaged() && goEngaged()) {
		setStateDrive();
		return;
	}
}

void Stage::drive()
{
	if (!dmhEngaged())
	{
		setStateBrake();
		return;
	}

	auto innerDriveData = _state.data.drive.innerData;
	auto outerDriveData = _state.data.drive.outerData;

	auto inner_done = innerDriveData.directionBoolean != (innerDriveData.currentPosition < innerDriveData.setPosition);
	auto outer_done = outerDriveData.directionBoolean != (outerDriveData.currentPosition < outerDriveData.setPosition);

	if (inner_done && outer_done)
	{
		setStateReady();
		return;
	}

	if (!inner_done) {
		spin_revolve(innerDriveData.currentPosition, innerDriveData.currentSpeed, innerDriveData.tenths_accel, innerDriveData.pid, _inner);
	}
	else {
		_inner->setSpeed(0);
	}

	if (!outer_done) {
		spin_revolve(outerDriveData.currentPosition, outerDriveData.currentSpeed, outerDriveData.tenths_accel, outerDriveData.pid, _outer);
	}
	else {
		_outer->setSpeed(0);
	}
}

void Stage::brake()
{
	if (dmhEngaged() && goEngaged()) {
		setStateDrive();
		return;
	}

	const unsigned long inner_speed = [&]() {
		if (_state.data.brake.inner_at_speed)
			return 0UL;
		else
			return _state.data.brake.inner_start_speed - (millis() - _state.data.brake.start_time) * _acceleration;
	}();
	const unsigned long outer_speed = [&]() {
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

	_inner->setSpeed(inner_speed);
	_outer->setSpeed(outer_speed);
}

/***** Key switches *****/

bool Stage::dmhEngaged()
{
	return !digitalRead(PAUSE);
}

bool Stage::goEngaged()
{
	return !digitalRead(GO);
}

bool Stage::eStopsEngaged()
{
	// Commented out line for non-conencted external esstop testing
	//if !(digitalRead(ESTOPNC1)==LOW && digitalRead(ESTOPNC2)==LOW && digitalRead(ESTOPNC3)==LOW && digitalRead(ESTOPNO)==HIGH){
	return !(digitalRead(ESTOPNC1) == LOW && digitalRead(ESTOPNO) == HIGH);
}

/***** Emergency Stop *****/

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

void Stage::emergencyStop()
{
	_inner->setSpeed(0);
	_outer->setSpeed(0);

	_state.state = REVOLVE_ESTOP;
	_state.data.estop = {};
	_displays->setMode(ESTOP); // TODO

							   // hold until we're ready to go again
	while (eStopsEngaged()) {
	}

	_state.state = REVOLVE_READY;
}

/**** Drive functions *****/

void Stage::spin_revolve(double* currentPosition, double* currentSpeed, double tenths_accel, PID* pid, Revolve* wheel)
{
	// Update position and compute PID
	*currentPosition = _inner->getPos();
	pid->Compute();

	// Limit acceleration
	if (wheel->_tenths >= 1) {
		const auto allowedSpeed = clamp(*currentSpeed, wheel->_cur_speed - tenths_accel, wheel->_cur_speed + tenths_accel);
		wheel->setSpeed(allowedSpeed);
		wheel->_tenths = 0;
	}
}

DriveData* Stage::setupDrive(int position, int speed, int acceleration, int direction, int revolutions, Revolve* wheel)
{
	double kp, currentPosition, setPosition, currentSpeed;
	currentPosition = wheel->getPos();
	wheel->setDir(direction);

	auto directionBoolean = wheel->getDir() == FORWARDS;
	revolutions += (directionBoolean == (position < _inner->displayPos()));
	setPosition = currentPosition + position - _inner->displayPos() + (directionBoolean ? 1 : -1) * 360 * revolutions;

	speed = clamp(abs(speed), MINSPEED + 1, 100);
	acceleration = clamp(abs(acceleration), 1, MAXACCEL);
	auto tenths_accel = (acceleration) / 10.0;

	kp = wheel->_kp_0 + ((100 - speed) * wheel->_kp_smin) / 100 + ((acceleration)* wheel->_kp_amax) / (MAXACCEL);

	DriveData data = { &currentPosition, &currentSpeed, &setPosition, directionBoolean, tenths_accel, nullptr };
	setupPid(speed, kp, &data, wheel);

	if (wheel == _inner)
	{
		_state.data.drive.innerData = data;
	}
	else if (wheel == _outer)
	{
		_state.data.drive.outerData = data;
	}

	return &data;
}

void Stage::setupPid(int maxSpeed, double kp, DriveData* data, Revolve* wheel)
{
	auto mode = data->setPosition < data->currentPosition ? REVERSE : DIRECT;
	auto pid = PID(data->currentPosition, data->currentSpeed, data->setPosition, kp, wheel->_ki, wheel->_kd, mode);
	pid.SetOutputLimits(MINSPEED, maxSpeed);
	pid.SetSampleTime(75);
	pid.SetMode(AUTOMATIC);

	data->pid = &pid;
}

/***** Cues *****/

void Stage::runCurrentCue()
{
	// Turn off switch leds
	_interface->encOff();
	digitalWrite(SELECTLED, LOW);
	digitalWrite(GOLED, LOW);

	// Flag to recursively call function if required
	int auto_follow = _interface->cueParams[0];

	// Move - both enabled
	if (_interface->cueParams[1] && _interface->cueParams[2]) {
		//gotoPos();
	}
	// Move - inner disabled
	else if (_interface->cueParams[1] == 0 && _interface->cueParams[2]) {
		// gotoPos(); // TODO pull parameters from git history
	}
	// Move - outer disabled
	else if (_interface->cueParams[1] && _interface->cueParams[2] == 0) {
		//gotoPos();
	}


	// Increment cue to currently selected cue with menu_pos, and increase menu_pos to automatically select next one
	_interface->_cuestack.currentCue = _interface->menu_pos;
	if (_interface->menu_pos < (_interface->_cuestack.totalCues - 1))
		_interface->menu_pos++;

	// Load next cue data
	_interface->loadCue(_interface->menu_pos);

	// Update _displays
	_displays->forceUpdateDisplays(1, 1, 1, 1);

	// Recursively call runCurrentCue whilst previous cue had autofollow enabled, unless we are at last cue (where menu_pos = currentCue as it won't have been incremented)
	if (auto_follow && _interface->_cuestack.currentCue != _interface->menu_pos)
		runCurrentCue();

	// Turn on switch leds
	digitalWrite(SELECTLED, HIGH);
	digitalWrite(GOLED, HIGH);
}

/***** Wheel homing *****/

void Stage::gotoHome()
{
	_displays->setMode(HOMING);

	home_wheel(_inner, INNERHOME);
	// Set inner ring green
	for (int i = 12; i < 24; i++) {
		_ringLeds->setPixelColor(i, 0, 255, 0);
	}
	_ringLeds->show();

	home_wheel(_outer, OUTERHOME);
	// Set outer ring green
	_interface->ringLedsColor(0, 255, 0);

	_displays->setMode(HOMED);
	// Move back to calibrated home (will have overshot)
	// gotoPos(); // TODO get parameters from git history

	_displays->setMode(NORMAL);
}

void Stage::home_wheel(Revolve* wheel, int wheelPin)
{
	setDriveGoal(0, HOMESPEED, 1, FORWARDS, 0, wheel);


	// Spin until home switch pressed
	wheel->setDir(FORWARDS);
	wheel->setSpeed(HOMESPEED);

	// Wait for inner home switch
	while (digitalRead(wheelPin)) {

		// Check for emergency stop
		if (digitalRead(PAUSE) || eStopsEngaged()) {
			emergencyStop();

			// Restart
			_interface->pauseLedsColor(0, 255, 0);
			digitalWrite(GOLED, LOW);
			wheel->setSpeed(HOMESPEED);

			// TODO check this
			// below code was origianlly only in E-Stops
			if (digitalRead(PAUSE) == LOW && digitalRead(GO) == LOW) {
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

void Stage::updateEncRatios() const
{
	EEPROM.get(EEINNER_ENC_RATIO, _inner->_enc_ratio);
	EEPROM.get(EEOUTER_ENC_RATIO, _outer->_enc_ratio);
}

void Stage::updateKpSettings() const
{
	double kpSettings[6];
	EEPROM.get(EEKP_SETTINGS, kpSettings);

	_inner->_kp_0 = kpSettings[0];
	_inner->_kp_smin = kpSettings[1];
	_inner->_kp_amax = kpSettings[2];
	_outer->_kp_0 = kpSettings[3];
	_outer->_kp_smin = kpSettings[4];
	_outer->_kp_amax = kpSettings[5];
}
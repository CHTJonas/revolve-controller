#include "stage.h"

Stage::Stage(Revolve& inner, Revolve& outer, Displays& displays, Interface& interface, Adafruit_NeoPixel& ringLeds): _inner(inner), _outer(outer), _displays(displays), _interface(interface), _ringLeds(ringLeds) {
  updateEncRatios();
  updateKpSettings();
}

// Updates revolve enc_ratios from EEPROM
void Stage::updateEncRatios() {
  EEPROM.get(EEINNER_ENC_RATIO, _inner._enc_ratio);
  EEPROM.get(EEOUTER_ENC_RATIO, _outer._enc_ratio);
}

// Updates kp values from EEPROM
void Stage::updateKpSettings() {
  double kpSettings[6];
  EEPROM.get(EEKP_SETTINGS, kpSettings);

  _inner._kp_0 = kpSettings[0];
  _inner._kp_smin = kpSettings[1];
  _inner._kp_amax = kpSettings[2];
  _outer._kp_0 = kpSettings[3];
  _outer._kp_smin = kpSettings[4];
  _outer._kp_amax = kpSettings[5];
}

// Initial homing sequence
void Stage::gotoHome() {
  _displays.setMode(HOMING);

  // Spin until home switch pressed
  _inner.setDir(FORWARDS);
  _inner.setSpeed(HOMESPEED);

  // Wait for inner home switch
  while (digitalRead(INNERHOME)) {

    // Check Dead Mans Switch
    if (digitalRead(PAUSE)) {

      // Update LEDs
      _interface.pauseLedsColor(255, 0, 0);
      digitalWrite(GOLED, HIGH);

      // EM stop
      _inner.setSpeed(0);

      // Wait for repress
      while (digitalRead(PAUSE) || digitalRead(GO)) {
      }

      // Green button
      _interface.pauseLedsColor(0, 255, 0);
      digitalWrite(GOLED, LOW);

      // Restart
      _inner.setSpeed(HOMESPEED);
    }

    // Check Estops
    if (checkEstops()) {

      // Update LEDs
      _interface.pauseLedsColor(0, 0, 0);
      digitalWrite(GOLED, LOW);

      // Go into ESTOP mode
      _displays.setMode(ESTOP);

      // EM stop
      _inner.setSpeed(0);

      // Wait for reset
      while (checkEstops()) {
      }

      // Reset Mode
      _displays.setMode(HOMING);

      // Update LEDs
      _interface.pauseLedsColor(255, 0, 0);
      digitalWrite(GOLED, HIGH);

      // Restart
      if (digitalRead(PAUSE) == LOW && digitalRead(GO) == LOW) {
        _inner.setSpeed(HOMESPEED);
      }
    }
  }
  // Reset at home pin
  _inner.resetPos();

  // Stop
  _inner.setSpeed(0);

  // Set inner ring green
  for (int i = 12; i < 24; i++) {
    _ringLeds.setPixelColor(i, 0, 255, 0);
  }
  _ringLeds.show();

  // Spin until home switch pressed
  _outer.setDir(FORWARDS);
  _outer.setSpeed(HOMESPEED);

  // Wait for outer home switch
  while (digitalRead(OUTERHOME)) {
    if (digitalRead(PAUSE)) {

      // Update LEDs
      _interface.pauseLedsColor(255, 0, 0);
      digitalWrite(GOLED, HIGH);

      // EM stop
      _outer.setSpeed(0);

      // Wait for repress
      while (digitalRead(PAUSE) || digitalRead(GO)) {
      }

      // Green button
      _interface.pauseLedsColor(0, 255, 0);
      digitalWrite(GOLED, LOW);

      // Restart
      _outer.setSpeed(HOMESPEED);
    }

    // Check Estops
    if (checkEstops()) {
      // Update LEDs
      _interface.pauseLedsColor(0, 0, 0);
      digitalWrite(GOLED, LOW);

      // EM stop
      _inner.setSpeed(0);

      // Wait for reset
      while (checkEstops()) {
      }

      // Update LEDs
      _interface.pauseLedsColor(255, 0, 0);
      digitalWrite(GOLED, HIGH);

      // Restart
      if (digitalRead(PAUSE) == LOW && digitalRead(GO) == LOW) {
        _inner.setSpeed(HOMESPEED);
      }
    }
  }
  // Reset at home pin
  _outer.resetPos();

  // Stop
  _outer.setSpeed(0);

  // Set outer ring green
  _interface.ringLedsColor(0, 255, 0);
  _displays.setMode(HOMED);

  // Move back to calibrated home (will have overshot)
  gotoPos(0, 0, MINSPEED, MINSPEED, 1, 1, BACKWARDS, BACKWARDS, 0, 0);

  _displays.setMode(NORMAL);
}

bool Stage::checkEstops() {
  // Commented out line for non-conencted external esstop testing
  //if(digitalRead(ESTOPNC1)==LOW && digitalRead(ESTOPNC2)==LOW && digitalRead(ESTOPNC3)==LOW && digitalRead(ESTOPNO)==HIGH){
  if (digitalRead(ESTOPNC1) == LOW && digitalRead(ESTOPNO) == HIGH)
    return false;
  else
    return true;
}

void Stage::deadMansRestart(int restartSpeed) {
  int restart = 0;

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

void Stage::gotoPos(int pos_inner, int pos_outer, int maxSpeed_inner, int maxSpeed_outer, int accel_inner, int accel_outer, int dir_inner, int dir_outer, int revs_inner, int revs_outer) {
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
  // Backwards
  if (_inner.getDir() == BACKWARDS) {
    if (pos_inner > _inner.displayPos()) {
      setPos_inner = curPos_inner - ((_inner.displayPos() - pos_inner) + (360 * (revs_inner + 1))); // Subtract required position + 360 * complete revolutions plus 1 extra from current position
    }
    else {
      setPos_inner = curPos_inner - ((_inner.displayPos() - pos_inner) + (360 * revs_inner)); // Subtract required position + 360 * complete revolutions from current position
    }
  }
  // Forwards
  else {
    if (pos_inner < _inner.displayPos()) {
      setPos_inner = curPos_inner + (pos_inner - _inner.displayPos()) + (360 * (revs_inner + 1)); // Add required position + 360 * complete revolutions plus 1 extra from current position
    }
    else {
      setPos_inner = curPos_inner + (pos_inner - _inner.displayPos()) + (360 * revs_inner); // Add required position + 360 * complete revolutions from current position
    }
  }

  // Backwards
  if (_outer.getDir() == BACKWARDS) {
    if (pos_outer > _outer.displayPos()) {
      setPos_outer = curPos_outer - ((_outer.displayPos() - pos_outer) + (360 * (revs_outer + 1))); // Subtract required position + 360 * complete revolutions plus 1 extra from current position
    }
    else {
      setPos_outer = curPos_outer - ((_outer.displayPos() - pos_outer) + (360 * revs_outer)); // Subtract required position + 360 * complete revolutions from current position
    }
  }
  // Forwards
  else {
    if (pos_outer < _outer.displayPos()) {
      setPos_outer = curPos_outer + (pos_outer - _outer.displayPos()) + (360 * (revs_outer + 1)); // Add required position + 360 * complete revolutions plus 1 extra from current position
    }
    else {
      setPos_outer = curPos_outer + (pos_outer - _outer.displayPos()) + (360 * revs_outer); // Add required position + 360 * complete revolutions from current position
    }
  }

  // Sanitise speed input
  maxSpeed_inner = abs(maxSpeed_inner);
  if (maxSpeed_inner > 100) {
    maxSpeed_inner = 100;
  }
  if (maxSpeed_inner < MINSPEED) {
    maxSpeed_inner = MINSPEED;
  }
  if (maxSpeed_inner == MINSPEED) {
    maxSpeed_inner++;
  }

  maxSpeed_outer = abs(maxSpeed_outer);
  if (maxSpeed_outer > 100) {
    maxSpeed_outer = 100;
  }
  if (maxSpeed_outer < MINSPEED) {
    maxSpeed_outer = MINSPEED;
  }
  if (maxSpeed_outer == MINSPEED) {
    maxSpeed_outer++;
  }

  // Sanitise max acceleration input
  accel_inner = abs(accel_inner);
  if (accel_inner > MAXACCEL) {
    accel_inner = MAXACCEL;
  }
  // Cannot be 0
  if (accel_inner < 1) {
    accel_inner = 1;
  }
  accel_outer = abs(accel_outer);
  if (accel_outer > MAXACCEL) {
    accel_outer = MAXACCEL;
  }
  // Cannot be 0
  if (accel_outer < 1) {
    accel_outer = 1;
  }

  // Convert accel from increase/second to increase/(1/10) second
  float tenths_accel_inner = (float)accel_inner / 10.0;
  float tenths_accel_outer = (float)accel_outer / 10.0;

  // Calculate kp for this move using constants from inital constructor and required speed, acceleration
  kp_inner = _inner._kp_0 + ((100 - maxSpeed_inner) * _inner._kp_smin) / 100 + ((accel_inner) * _inner._kp_amax) / (MAXACCEL);
  kp_outer = _outer._kp_0 + ((100 - maxSpeed_outer) * _outer._kp_smin) / 100 + ((accel_outer) * _outer._kp_amax) / (MAXACCEL);

  // Setup PID object (flip to reverse mode if negative)
  PID pid_inner = setPos_inner < curPos_inner ? pid_inner = PID(&curPos_inner, &curSpeed_inner, &setPos_inner, kp_inner, _inner._ki, _inner._kd, REVERSE) : pid_inner = PID(&curPos_inner, &curSpeed_inner, &setPos_inner, kp_inner, _inner._ki, _inner._kd, DIRECT);
  pid_inner.SetOutputLimits(MINSPEED, maxSpeed_inner);
  pid_inner.SetSampleTime(75);
  pid_inner.SetMode(AUTOMATIC);

  PID pid_outer = setPos_outer < curPos_outer ? pid_outer = PID(&curPos_outer, &curSpeed_outer, &setPos_outer, kp_outer, _outer._ki, _outer._kd, REVERSE) : pid_outer = PID(&curPos_outer, &curSpeed_outer, &setPos_outer, kp_outer, _outer._ki, _outer._kd, DIRECT);
  pid_outer.SetOutputLimits(MINSPEED, maxSpeed_outer);
  pid_outer.SetSampleTime(75);
  pid_outer.SetMode(AUTOMATIC);

  int inner_done = 0;
  int outer_done = 0;

  _interface.select.update();

  while ((inner_done == 0 || outer_done == 0) && _interface.select.read() == HIGH) { // Stop cue if select pressed mid way
    // Update Select debounce
    _interface.select.update();
    // Check Estops
    if (checkEstops()) {

      // Store curent mode - could be man or show (or program_gotocue)!
      int curMode = _displays.mode;

      // Turn off GO and Pause LEDs
      _interface.pauseLedsColor(0, 0, 0);
      digitalWrite(GOLED, LOW);

      // Go into ESTOP mode
      _displays.mode = ESTOP;
      _displays.forceUpdateDisplays(1, 1, 1, 1);

      // EM stop
      _inner.setSpeed(0);
      _outer.setSpeed(0);

      // Wait for reset
      while (checkEstops()) {
        _displays.updateDisplays(1, 1, 1, 1);
      }

      // Reset to last mode
      _displays.mode = curMode;
      _displays.forceUpdateDisplays(1, 1, 1, 1);

      // Restart
      deadMansRestart(MINSPEED);
    }

    // Check Dead Mans Switch
    if (digitalRead(PAUSE)) {

      // EM stop
      _inner.setSpeed(0);
      _outer.setSpeed(0);

      // Restart
      deadMansRestart(MINSPEED);
    }

    // Update displays
    _displays.updateDisplays(0, 0, 1, 1);

    // Inner revolve
    // Backwards
    if (dir_inner == BACKWARDS) {
      if (curPos_inner > setPos_inner) { // Stop as soon as we reach position - minimise overshoot, no oscillation allowed

        // Update position and comput PID
        curPos_inner = _inner.getPos();
        pid_inner.Compute();

        // Limit acceleration
        if (curSpeed_inner > (_inner._cur_speed + tenths_accel_inner) && _inner._tenths >= 1) {
          _inner.setSpeed(_inner._cur_speed + tenths_accel_inner); // Set to max allowed if requested speed increase too large
          _inner._tenths = 0;
        }

        // Limit decceleration
        else if (curSpeed_inner < (_inner._cur_speed - tenths_accel_inner) && _inner._tenths >= 1) {
          _inner.setSpeed(_inner._cur_speed - tenths_accel_inner); // Set to min allowed if requested speed decrease too large
          _inner._tenths = 0;
        }

        // If requested change within limits, set to requested speed
        else if (_inner._tenths >= 1 && curSpeed_inner <= (_inner._cur_speed + tenths_accel_inner) && curSpeed_inner >= (_inner._cur_speed - tenths_accel_inner)) {
          _inner.setSpeed(curSpeed_inner);
          _inner._tenths = 0;
        }
      }
      // Otherwise done, set done flag
      else if (inner_done == 0) {
        inner_done = 1;
        // Stop once position reached
        _inner.setSpeed(0);
      }
    }

    // Forwards
    else {
      if (curPos_inner < setPos_inner) { // Stop as soon as we reach position - minimise overshoot, no oscillation allowed

        // Update position and comput PID
        curPos_inner = _inner.getPos();
        pid_inner.Compute();

        // Limit acceleration
        if (curSpeed_inner > (_inner._cur_speed + tenths_accel_inner) && _inner._tenths >= 1) {
          _inner.setSpeed(_inner._cur_speed + tenths_accel_inner); // Set to max allowed if requested speed increase too large
          _inner._tenths = 0;
        }

        // Limit deceleration
        else if (curSpeed_inner < (_inner._cur_speed - tenths_accel_inner) && _inner._tenths >= 1) {
          _inner.setSpeed(_inner._cur_speed - tenths_accel_inner); // Set to min allowed if requested speed decrease too large
          _inner._tenths = 0;
        }

        // If requested change within limits, set to requested speed
        else if (_inner._tenths >= 1 && curSpeed_inner <= (_inner._cur_speed + tenths_accel_inner) && curSpeed_inner >= (_inner._cur_speed - tenths_accel_inner)) {
          _inner.setSpeed(curSpeed_inner);
          _inner._tenths = 0;
        }
      }
      // Otherwise done, set done flag
      else if (inner_done == 0) {
        inner_done = 1;
        // Stop once position reached
        _inner.setSpeed(0);
      }
    }

    // Outer
    // Backwards
    if (dir_outer == BACKWARDS) {
      if (curPos_outer > setPos_outer) { // Stop as soon as we reach position - minimise overshoot, no oscillation allowed

        // Update position and comput PID
        curPos_outer = _outer.getPos();
        pid_outer.Compute();

        // Limit acceleration
        if (curSpeed_outer > (_outer._cur_speed + tenths_accel_outer) && _outer._tenths >= 1) {
          _outer.setSpeed(_outer._cur_speed + tenths_accel_outer); // Set to max allowed if requested speed increase too large
          _outer._tenths = 0;
        }

        // Limit decceleration
        else if (curSpeed_outer < (_outer._cur_speed - tenths_accel_outer) && _outer._tenths >= 1) {
          _outer.setSpeed(_outer._cur_speed - tenths_accel_outer); // Set to min allowed if requested speed decrease too large
          _outer._tenths = 0;
        }

        // If requested change within limits, set to requested speed
        else if (_outer._tenths >= 1 && curSpeed_outer <= (_outer._cur_speed + tenths_accel_outer) && curSpeed_outer >= (_outer._cur_speed - tenths_accel_outer)) {
          _outer.setSpeed(curSpeed_outer);
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

    // Forwards
    else {
      if (curPos_outer < setPos_outer) { // Stop as soon as we reach position - minimise overshoot, no osciallation allowed

        // Update position and comput PID
        curPos_outer = _outer.getPos();
        pid_outer.Compute();

        // Limit acceleration
        if (curSpeed_outer > (_outer._cur_speed + tenths_accel_outer) && _outer._tenths >= 1) {
          _outer.setSpeed(_outer._cur_speed + tenths_accel_outer); // Set to max allowed if requested speed increase too large
          _outer._tenths = 0;
        }

        // Limit deceleration
        else if (curSpeed_outer < (_outer._cur_speed - tenths_accel_outer) && _outer._tenths >= 1) {
          _outer.setSpeed(_outer._cur_speed - tenths_accel_outer); // Set to min allowed if requested speed decrease too large
          _outer._tenths = 0;
        }

        // If requested change within limits, set to requested speed
        else if (_outer._tenths >= 1 && curSpeed_outer <= (_outer._cur_speed + tenths_accel_outer) && curSpeed_outer >= (_outer._cur_speed - tenths_accel_outer)) {
          _outer.setSpeed(curSpeed_outer);
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
  }


  // Stop revolves if Select has been pressed
  if (outer_done == 0)
    _outer.setSpeed(0);
  if (inner_done == 0)
    _inner.setSpeed(0);
  _displays.forceUpdateDisplays(0, 0, 1, 1);
  _interface.waitSelectRelease();
}

void Stage::runCurrentCue() {
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

#pragma once
#include <PID_v1.h>
#include <Encoder.h>
#include <Arduino.h>
#include "constants.h"

#define FORWARDS 0
#define BACKWARDS 1

class Revolve {
public:

	// Constructor
	Revolve(int start_pin, int dir_pin, int speed_pin, Encoder& enc);

	// Internal helper functions
	void start() const;
	void stop() const;
	void waitForStop() const;

	// Getters and setters
	int getSpeed() const;
	int getDir() const;
	long getPos();
	void setSpeed(float speed);
	void setDir(int dir);
	void resetPos() const;
	int displayPos();
	void setDebug(int debug);

	// Public flags
	volatile int _tenths;
	int _debug;

	// Speed and acceleration limits
	float _max_speed;
	int _em_stop_time;
	float _enc_ratio;

	// Current position/speed data
	float _cur_speed;
	int _dir;
	long _cur_pos;

	// PID globals
	double _kp_0;
	double _kp_smin;
	double _kp_amax;
	double _ki;
	double _kd;

private:

	// External object references
	Encoder& _enc;

	// Pins
	int _start_pin;
	int _dir_pin;
	int _speed_pin;

};

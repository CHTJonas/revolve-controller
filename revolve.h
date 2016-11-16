#pragma once
#include "constants.h"
#include <Arduino.h>
#include <Encoder.h>
#include <PID_v1.h>

#define FORWARDS 0
#define BACKWARDS 1

class Revolve {
public:
	// Constructor
	Revolve(int start_pin, int dir_pin, int speed_pin, Encoder* enc);

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
	volatile int tenths;
	int debug;

	// Speed and acceleration limits
	float max_speed;
	int em_stop_time;
	float enc_ratio;

	// Current position/speed data
	float cur_speed;
	int dir;
	long cur_pos;

	// PID globals
	double kp_0;
	double kp_smin;
	double kp_amax;
	double ki;
	double kd;

private:
	// External object references
	Encoder* enc;

	// Pins
	int start_pin;
	int dir_pin;
	int speed_pin;
};

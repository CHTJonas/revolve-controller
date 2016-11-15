#include "revolve.h"
#include "buttons.h"

// Constructor
Revolve::Revolve(int start_pin, int dir_pin, int speed_pin, Encoder& enc) : enc(enc) {

	// Set physical pins
	start_pin = start_pin;
	dir_pin = dir_pin;
	speed_pin = speed_pin;

	// Speed and acceleration parameters
	cur_speed = 0;  // Updated with current speed whenever speed is changed
	em_stop_time = 500;  // EM stop time as setup on motor controller
	enc_ratio = 1;  // Encoder degrees per revolve degrees (updated from from eeprom by stage)
	debug = 0;  // Debug flag to serial print debug information
	tenths = 0;  // Incremented every 1/10 second by main loop to limit acceleration

	// PID control variables
	kp_0 = 0;  // Initial kp for 100 speed and 0 acceleration
	kp_smin = 0;  // Difference between min and max kp between speed 0 and 100 for fixed acceleration
	kp_amax = 0;  // Difference between min and max kp between 0 acceleration and MAXACCEL for fixed speed
	ki = 0;  // Integral control coefficient (usually not needed)
	kd = 0;  // Differential control coefficient (usually not needed)

	// Setup output pins
	pinMode(start_pin, OUTPUT);
	pinMode(dir_pin, OUTPUT);
	pinMode(speed_pin, OUTPUT);
}

// INTERNAL HELPER FUNCTIONS

// Enables motor control
void Revolve::start() const {
	digitalWrite(start_pin, HIGH);
}

// Stops motor - usually to stop energisation at 0Hz. Will cause EM stop if activated at speed.
void Revolve::stop() const {
	digitalWrite(start_pin, LOW);
	waitForStop();  // wait for full stop if stopping from high speed (uses em_stop_time)
}

// Waits for motor to fully stop using em_stop_time
void Revolve::waitForStop() const {
	auto startTime = millis();  // Don't use delay so encoders catch final position
	while (millis() < (startTime + em_stop_time)) {
		// Break out if pause button released (revolve won't come to full stop if button briefly released)
		if (Buttons::dmh.engaged()) {
			break;
		}
	}
}

// GETTERS AND SETTERS

// Getter for cur_speed
int Revolve::getSpeed() const {
	return cur_speed;
}

// Getter for dir
int Revolve::getDir() const {
	return dir;
}

// Getter for current absolute position from encoder - updates cur_pos
long Revolve::getPos() {
	auto pos = (enc.read() / 4L);  // Encoder has 4 steps per degree
	pos = pos / enc_ratio;  // Divide by encoder degrees per revolve degree
	cur_pos = pos;
	return pos;
}

// Set speed - will accelerate according to limits set on controller (i.e. max acceleration)
void Revolve::setSpeed(float speed) {

	// Nothing below MINSPEED
	if (speed < MINSPEED && speed != 0) {
		speed = MINSPEED;
	}

	// Convert from % to duty cycle
	auto req_duty_cycle = static_cast<int>(speed * 255) / 100;

	// Limit max
	if (req_duty_cycle > 255)
		req_duty_cycle = 255;

	// No negatives
	if (req_duty_cycle < 0)
		req_duty_cycle = 0;

	// Start if at standstill
	if (cur_speed == 0) {
		start();
	}

	// Set speed, update cur_speed
	analogWrite(speed_pin, req_duty_cycle);
	cur_speed = speed;

	// Stop if speed has been set to 0
	if (cur_speed == 0) {
		stop();
	}
}

// Sets direction pin (will have no effect at speed until speed taken to 0)
void Revolve::setDir(int dir) {
	if (dir == BACKWARDS) {
		digitalWrite(dir_pin, HIGH);
		dir = BACKWARDS;  // Update direction variable
	} else {
		digitalWrite(dir_pin, LOW);
		dir = FORWARDS;  // Update direction variable
	}
}

// Returns relative position between 0 and 360 degrees
int Revolve::displayPos() {
	auto pos = getPos();
	int realPos = pos % 360;

	// No negatives
	if (realPos < 0) {
		realPos += 360;
	}

	return realPos;
}

// Reset position to 0
void Revolve::resetPos() const {
	enc.write(0);
}

// Set debug flag to allow serial printing by functions
void Revolve::setDebug(int debug) {
	if (debug == 1) {
		debug = 1;
	} else {
		debug = 0;
	}
}

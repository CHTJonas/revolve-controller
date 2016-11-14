#pragma once
#include "displays.h"
#include "state_machine.h"
#include "revolve.h"
#include "interface.h"
#include "buttons.h"

#define INNER 0
#define OUTER 1

class Stage {
public:
	// Constructor
	Stage(
	    State* state,
	    Revolve* inner,
	    Revolve* outer,
	    Interface* interface,
	    Adafruit_NeoPixel* ringLeds,
		Buttons* buttons);
	void step();

	// Settings updating
	void updateEncRatios() const;
	void updateKpSettings() const;

	// Control
	void home_wheel(Revolve* wheel, int wheelPin);
	void gotoHome();
	void emergencyStop();
	void ready();
	void brake();
	void drive();
	void checkEstops();

	void setupPid(int maxSpeed, double kp, DriveData* data, Revolve* wheel);
	void setupDrive(
		int position, int speed, int acceleration, int direction, int revolutions, DriveData* data, Revolve* wheel);
	void spin_revolve(double* currentPosition, double* currentSpeed, double tenths_accel, PID* pid, Revolve* wheel);
	void runCurrentCue();

	State* state;
	Revolve* inner;
	Revolve* outer;
	int acceleration = 10;  // TODO check for sanity

private:
	Interface* interface;
	Adafruit_NeoPixel* ringLeds;
	Buttons* buttons;

	void setDriveGoal(int position, int speed, int acceleration, int direction, int revolutions, Revolve* wheel);

	void setStateReady();
	void setStateDrive();
	void setStateBrake();
};

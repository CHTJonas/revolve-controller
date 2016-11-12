#pragma once
#include "displays.h"
#include "state_machine.h"

#define INNER 0
#define OUTER 1

class Stage {
public:
	// Constructor
	Stage(
	    State* state,
	    Revolve* inner,
	    Revolve* outer,
	    Displays* displays,
	    Interface* interface,
	    Adafruit_NeoPixel* ringLeds);
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
	static bool dmhEngaged();
	static bool goEngaged();
	bool checkEstops();
	static bool eStopsEngaged();

	void setupPid(int maxSpeed, double kp, DriveData* data, Revolve* wheel);
	void setupDrive(
	    int position, int speed, int acceleration, int direction, int revolutions, Revolve* wheel, DriveData* data);
	void spin_revolve(double* currentPosition, double* currentSpeed, double tenths_accel, PID* pid, Revolve* wheel);
	void runCurrentCue();

	State* state;
	Revolve* inner;
	Revolve* outer;
	int acceleration = 10;  // TODO check for sanity

private:
	Displays* displays;
	Interface* interface;
	Adafruit_NeoPixel* ringLeds;

	void setDriveGoal(int position, int speed, int acceleration, int direction, int revolutions, Revolve* wheel);

	void setStateReady();
	void setStateDrive(
	    int inner_position,
	    int inner_speed,
	    int inner_acceleration,
	    int inner_direction,
	    int inner_revolutions,
	    int outer_position,
	    int outer_speed,
	    int outer_acceleration,
	    int outer_direction,
	    int outer_revolutions);
	void setStateBrake();
};

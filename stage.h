#pragma once
#include "displays.h"
#include "state.h"

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
	void loop();

	// Settings updating
	void updateEncRatios() const;
	void updateKpSettings() const;

	// Control
	void home_wheel(Revolve* wheel, int wheelPin);
	void gotoHome();
	void emergencyStop();
	void run_ready();
	void run_brake();
	void run_drive();
	void manual_ready();
	void manual_drive();
	void manual_brake();

	bool checkEstops();

	void setupPid(int maxSpeed, double kp, DriveData* data, Revolve* wheel);
	void setupDrive(
	    int position, int speed, int acceleration, int direction, int revolutions, DriveData* data, Revolve* wheel);
	void spin_revolve(double* currentPosition, double* currentSpeed, double tenths_accel, PID* pid, Revolve* wheel);
	void runCurrentCue();

	int acceleration = 10;  // TODO(waiwaing, #19) check for sanity

private:
	State* state;
	Revolve* inner;
	Revolve* outer;
	Displays* displays;
	Interface* interface;
	Adafruit_NeoPixel* ringLeds;

	void setDriveGoal(int position, int speed, int acceleration, int direction, int revolutions, Revolve* wheel);

	void setStateRunReady();
	void setStateRunDrive();
	void setStateRunBrake();
	void setStateManualReady();
	void setStateManualDrive();
	void setStateManualBrake();
};

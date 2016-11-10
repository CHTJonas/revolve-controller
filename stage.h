#pragma once
#include "displays.h"

#define INNER 0
#define OUTER 1

typedef struct DriveData {
	double* currentPosition;
	double* currentSpeed;
	double* setPosition;

	bool directionBoolean;
	double tenths_accel;

	PID* pid;
} DriveData;

enum RevolveStateEnum { REVOLVE_READY, REVOLVE_DRIVE, REVOLVE_BRAKE, REVOLVE_ESTOP };

typedef struct RevolveState {
	RevolveStateEnum state;
	union {
		struct {
		} ready;
		struct {
			DriveData innerData;
			DriveData outerData;
		} drive;
		struct {
			unsigned long start_time;
			long inner_start_speed;
			long outer_start_speed;
			bool inner_at_speed;
			bool outer_at_speed;
		} brake;
		struct {
		} estop;
	} data;
} RevolveState;

class Stage {
      public:
	// Constructor
	Stage(Revolve* inner, Revolve* outer, Displays* displays, Interface* interface, Adafruit_NeoPixel* ringLeds);
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
	DriveData*
	setupDrive(int position, int speed, int acceleration, int direction, int revolutions, Revolve* wheel);
	void spin_revolve(double* currentPosition, double* currentSpeed, double tenths_accel, PID* pid, Revolve* wheel);
	void runCurrentCue();

	Revolve* outer;
	Revolve* inner;
	RevolveState state;
	int acceleration = 10;  // TODO check for sanity

      private:
	Displays* displays;
	Interface* interface;
	Adafruit_NeoPixel* ringLeds;

	void setStateReady();
	void setDriveGoal(int position, int speed, int acceleration, int direction, int revolutions, Revolve* wheel);
	void setStateDrive();
	void setStateBrake();
};

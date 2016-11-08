#pragma once
#include "displays.h"

#define INNER 0
#define OUTER 1

enum RevolveStateEnum
{
	REVOLVE_READY,
	REVOLVE_DRIVE,
	REVOLVE_BRAKE,
	REVOLVE_ESTOP
};

typedef struct RevolveState {
	RevolveStateEnum state;
	union {
		struct {} ready;
		struct {
			unsigned long start_time;
			long inner_start_speed;
			long outer_start_speed;
			long inner_target_speed;
			long outer_target_speed;
			long inner_target_position;
			long outer_target_position;
			bool inner_at_speed;
			bool outer_at_speed;
		} drive;
		struct
		{
			unsigned long start_time;
			long inner_start_speed;
			long outer_start_speed;
			bool inner_at_speed;
			bool outer_at_speed;
		} brake;
		struct {} estop;
	} data;
} RevolveState;

class Stage {
public:

	// Constructor
	Stage(Revolve* inner, Revolve* outer, Displays* displays, Interface* interface, Adafruit_NeoPixel* ringLeds);

	// Settings updating
	void updateEncRatios() const;
	void updateKpSettings() const;

	// Control
	void home_wheel(Revolve* wheel, int wheelPin);
	void gotoHome();
	void emergencyStop();
	void runStage();
	void ready();
	void brake();
	void drive();
	static bool dmhEngaged();
	static bool goEngaged();
	bool checkEstops();
	static bool eStopsEngaged();

	void resumeDrive(int restartSpeed) const;
	PID setupPid(int maxSpeed, double kp, double* currentPosition, double* setPosition, double* currentSpeed, Revolve* wheel);
	void setupDrive(int position, int speed, int acceleration, int direction, int revolutions, Revolve* wheel);
	void gotoPos(int pos_inner, int pos_outer, int maxSpeed_inner, int maxSpeed_outer, int accel_inner, int accel_outer, int dir_inner, int dir_outer, int revs_inner, int revs_outer);
	void spin_revolve(double* currentPosition, double* currentSpeed, double tenths_accel, PID* pid, Revolve* wheel);
	void runCurrentCue();

	Revolve* _outer;
	Revolve* _inner;
	RevolveState _state;
	int _acceleration = 10; // TODO check for sanity

private:

	Displays* _displays;
	Interface* _interface;
	Adafruit_NeoPixel* _ringLeds;

	void setStateReady();
	void setStateDrive(unsigned long inner_target_speed, unsigned long outer_target_speed, unsigned long inner_target_position, unsigned long outer_target_position);
	void setStateBrake();
};

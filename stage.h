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
		struct {} drive;
		struct
		{
			unsigned long start_time;
			long inner_start_position;
			long outer_start_position;
		} brake;
		struct {} estop;
	} data;
} RevolveState;

class Stage {
public:

	// Constructor
	Stage(Revolve& inner, Revolve& outer, Displays& displays, Interface& interface, Adafruit_NeoPixel& ringLeds);

	// Settings updating
	void updateEncRatios() const;
	void updateKpSettings() const;

	// Control
	void home_wheel(Revolve& wheel, int wheelPin);
	void gotoHome();
	void emergencyStop();
	void runStage();
	void brake();
	static bool dmhEngaged();
	static bool goEngaged();
	bool checkEstops();
	static bool eStopsEngaged();
	void resumeDrive(int restartSpeed) const;
	void deadMansRestart(int restartSpeed) const;
	void gotoPos(int pos_inner, int pos_outer, int maxSpeed_inner, int maxSpeed_outer, int accel_inner, int accel_outer, int dir_inner, int dir_outer, int revs_inner, int revs_outer);
	void runCurrentCue();

	Revolve& _inner;
	Revolve& _outer;
	RevolveState _state;
	int _pause_max_speed = 10; // TODO check for sanity

private:

	Displays& _displays;
	Interface& _interface;
	Adafruit_NeoPixel& _ringLeds;

	void setStateReady();
	void setStateDrive();
	void setStateBrake(unsigned long start_time, long inner_start_position, long outer_start_position);

};

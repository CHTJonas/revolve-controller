#pragma once

#include <PID_v1.h>
#include <stdbool.h>

typedef struct DriveData {
	double currentPosition;
	double currentSpeed;
	double setPosition;

	bool directionBoolean;
	double tenths_accel;

	PID* pid;
} DriveData;

typedef enum StateEnum {
	STATE_MAINMENU,
	STATE_RUN_READY,
	STATE_RUN_DRIVE,
	STATE_RUN_BRAKE,
	STATE_RUN_ESTOP,
	STATE_ABOUT,
	STATE_DEBUG,
} StateEnum;

class State {
private:
	StateEnum state;

public:
	union {
		struct {
			int selected_item_index;
		} mainmenu;
		struct {
		} run_ready;
		struct {
			DriveData innerData;
			DriveData outerData;
		} run_drive;
		struct {
			unsigned long start_time;
			long inner_start_speed;
			long outer_start_speed;
			bool inner_at_speed;
			bool outer_at_speed;
		} run_brake;
		struct {
		} run_estop;
		struct {
		} about;
		struct {
		} debug;
	} data;

public:
	State();

	void set_mainmenu();
	void set_run_ready();
	void set_run_drive();
	void set_run_brake(
	    unsigned long start_time,
	    long inner_start_speed,
	    long outer_start_speed,
	    bool inner_at_speed,
	    bool outer_at_speed);
	void set_run_estop();
	void set_about();
	void set_debug();

	void set_DANGEROUS_MUST_SET_DATA(StateEnum state);

	StateEnum get_state();
};

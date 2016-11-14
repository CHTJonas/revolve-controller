#pragma once

#include <stdbool.h>
#include <PID_v1.h>

typedef struct DriveData {
	double currentPosition;
	double currentSpeed;
	double setPosition;

	bool directionBoolean;
	double tenths_accel;

	PID* pid;
} DriveData;

enum StateEnum {
	STATE_MAINMENU,
	STATE_RUN_READY,
	STATE_RUN_DRIVE,
	STATE_RUN_BRAKE,
	STATE_RUN_ESTOP,
	STATE_ABOUT,
};

typedef struct State {
	enum StateEnum state;

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
	} data;
} State;

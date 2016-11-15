#pragma once

#include <stdbool.h>

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
	STATE_HOMING_INPROGRESS,
	STATE_HOMING_COMPLETE,
	STATE_MANUAL,
	STATE_SHOW,
	STATE_PROGRAM_MAIN,
	STATE_PROGRAM_MOVEMENTS,
	STATE_PROGRAM_DELETE,
	STATE_PROGRAM_CUELIST,
	STATE_PROGRAM_GOTOCUE,
	STATE_PROGRAM_SAVED,
	STATE_PROGRAM_PARAMS,
	STATE_SETTINGS,
	STATE_HARDWARETEST,
	STATE_BRIGHTNESS,
	STATE_ENCSETTINGS,
	STATE_ESTOP,
	STATE_DEFAULTVALUES,
	STATE_KPSETTINGS,
	STATE_RESET_CUESTACK,
	STATE_CUESTACK_BACKUP,
	STATE_RUN_READY,
	STATE_RUN_DRIVE,
	STATE_RUN_BRAKE,
	STATE_DEBUG,
};

typedef struct State {
	enum StateEnum state;

	union {
		struct {
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
		} startup;
		struct {
		} homing_inprogress;
		struct {
		} homing_complete;
		struct {
		} manual;
		struct {
		} show;
		struct {
		} program_main;
		struct {
		} program_movements;
		struct {
		} program_delete;
		struct {
		} program_cuelist;
		struct {
		} program_gotocue;
		struct {
		} program_saved;
		struct {
		} program_params;
		struct {
		} settings;
		struct {
		} hardwaretest;
		struct {
		} brightness;
		struct {
		} encsettings;
		struct {
		} estop;
		struct {
		} defaultvalues;
		struct {
		} kpsettings;
		struct {
		} reset_cuestack;
		struct {
		} cuestack_backup;
		struct {
		} debug;

	} data;
} State;

#pragma once

enum StateEnum {
	STATE_MAINMENU,
	STATE_RUN,
};

enum RunStateEnum { STATE_RUN_READY, STATE_RUN_DRIVE, STATE_RUN_BRAKE, STATE_RUN_ESTOP };

typedef struct State {
	enum StateEnum state;

	union {
		struct mainmenu {};
		struct {
			enum RunStateEnum state;

			union {
				struct ready {};
				struct drive {};
				struct brake {};
				struct estop {};
			} data;
		} run;
	} data;
} State;

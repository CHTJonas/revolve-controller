// Pins
#pragma once
#define GO 16
#define BACK 53
#define PAUSE 13
#define SELECT 14

#define ENCR 51
#define ENCG 49
#define ENCB 47

#define GOLED 17
#define SELECTLED 15
#define RINGLEDS 29
#define PAUSELEDS 31
#define KEYPADLEDS 33

#define INNERHOME 39
#define OUTERHOME 41

#define ESTOPNC1 60
#define ESTOPNC2 57
#define ESTOPNC3 54
#define ESTOPNO 55

#define INPUTENC1 3
#define INPUTENC2 2
#define INNERENC1 20
#define INNERENC2 21
#define OUTERENC1 19
#define OUTERENC2 18

#define KEY1 38
#define KEY2 40
#define KEY3 42
#define KEY4 44
#define KEY5 46
#define KEY6 48
#define KEY7 50

// Revolve parameters
#define MINSPEED 10
#define HOMESPEED 20
#define MAXACCEL 100

// Display modes
#define STARTUP 0
#define HOMING 1
#define NORMAL 2
#define MAN 3
#define PROGRAM 4
#define SHOW 5
#define SETTINGS 6
#define HARDWARETEST 7
#define HOMED 8
#define BRIGHTNESS 9
#define ESTOP 10
#define ENCSETTINGS 11
#define DEFAULTVALUES 12
#define KPSETTINGS 13
#define PROGRAM_MOVEMENTS 14
#define PROGRAM_PARAMS 15
#define PROGRAM_CUELIST 16
#define PROGRAM_DELETE 17
#define PROGRAM_GOTOCUE 18
#define PROGRAM_SAVED 19
#define RESET_CUESTACK 20
#define CUESTACK_BACKUP 21
#define CUESTACK_LOAD 22

// EEPROM addresses
#define EELED_SETTINGS 0
#define EEINNER_ENC_RATIO 8  // 4 byte float
#define EEOUTER_ENC_RATIO 12  // 4 byte float
#define EEDEFAULT_VALUES 16  // 10 ints (20 bytes)
#define EEKP_SETTINGS 36  // 6 4 byte floats (24 bytes)
#define EECUESTACK_START 200  // Start of cuestack, 100x 20byte structs (2000 bytes!)

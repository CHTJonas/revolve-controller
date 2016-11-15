#pragma once

// The maximum number of cues in the Cuestack
#define MAX_CUES 100

// The width and height of the displays
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// EEPROM addresses
#define EELED_SETTINGS 0
#define EEINNER_ENC_RATIO 8  // 4 byte float
#define EEOUTER_ENC_RATIO 12  // 4 byte float
#define EEDEFAULT_VALUES 16  // 10 ints (20 bytes)
#define EEKP_SETTINGS 36  // 6 4 byte floats (24 bytes)
#define EECUESTACK_START 200  // Start of cuestack, 100x 20byte structs (2000 bytes!)

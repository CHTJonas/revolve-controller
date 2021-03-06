#include "navigation.h"
#include "constants.h"
#include "pins.h"
#include "state.h"
#include <EEPROM.h>

Navigation::Navigation(State* state, Cuestack* cuestack, Interface* interface, Displays* displays, Stage* stage)
      : state(state), cuestack(cuestack), interface(interface), displays(displays), stage(stage) {
}

void Navigation::setup() {
}

void Navigation::loop() {
	// Main switch statement for navigating screens
	switch (state->state) {
	case STATE_HOMING_INPROGRESS:
		if (Buttons::back.risen_since_state_change()) {
			state->state = STATE_SETTINGS;
			state->data.settings = {};
			state->changed();
		}
		break;

	// Main Menu
	case STATE_MAINMENU:
		interface->updateMenu(3);

		// If select is pressed, move into appropriate mode
		if (Buttons::select.risen_since_state_change()) {
			int menu_pos = interface->menu_pos;
			interface->menu_pos = 0;

			switch (menu_pos) {
			case 0:
				state->state = STATE_MANUAL_READY;
				state->data.manual_ready = {};
				state->changed();
				break;
			case 1:
				interface->loadCurrentCue();
				interface->menu_pos = 2;
				state->state = STATE_PROGRAM_MAIN;
				state->data.program_main = {};
				state->changed();
				break;
			case 2:
				interface->loadCurrentCue();
				interface->menu_pos = cuestack->currentCue;
				state->state = STATE_SHOW;
				state->data.show = {};
				state->changed();
				break;
			case 3:
				state->state = STATE_SETTINGS;
				state->data.settings = {};
				state->changed();
				break;
			}
		}
		break;

	// Manual control
	case STATE_MANUAL_READY:
		interface->updateMenu(9);

		if (Buttons::back.risen_since_state_change()) {
			// Reset menu_pos and change mode
			interface->menu_pos = 0;
			state->state = STATE_MAINMENU;
			state->data.mainmenu = {};
			state->changed();
		}

		updateSetting(manualLimiter, MANUAL);
		break;

	case STATE_PROGRAM_SAVED:
		if (millis() > state->data.program_saved.time + 1000) {
			state->state = STATE_MAINMENU;
			state->data.mainmenu = {};
			state->changed();
		}
		break;

	// Mode to edit cue stack
	case STATE_PROGRAM_MAIN:
		interface->updateMenu(2);

		if (Buttons::back.risen_since_state_change()) {
			// Save Cuestack
			cuestack->saveCuestack();

			interface->menu_pos = 0;

			state->state = STATE_PROGRAM_SAVED;
			state->data.program_saved = { millis() };
			state->changed();
		}

		// If select pressed move into corresponding setting
		if (Buttons::select.risen_since_state_change()) {
			// Choose based on menu_pos
			switch (interface->menu_pos) {
			case 0:
				interface->menu_pos = 0;
				state->state = STATE_PROGRAM_MOVEMENTS;
				state->data.program_movements = {};
				state->changed();
				break;
			case 1:
				interface->menu_pos = 0;
				state->state = STATE_PROGRAM_PARAMS;
				state->data.program_params = {};
				state->changed();
				break;
			case 2:
				interface->menu_pos = cuestack->currentCue;
				state->state = STATE_PROGRAM_CUELIST;
				state->data.program_cuelist = {};
				state->changed();
				break;
			}
		}

		break;

	case STATE_PROGRAM_MOVEMENTS:
		if (interface->cueParams[1] == 0 ||
		    interface->cueParams[2] == 0) {  // If either half disabled for this cue
			interface->updateMenu(4);
		} else {
			interface->updateMenu(9);
		}

		updateSetting(movementLimiter, PROGRAM_MOVEMENTS);

		// Back one level only
		if (Buttons::back.risen_since_state_change()) {
			interface->menu_pos = 0;
			state->state = STATE_PROGRAM_MAIN;
			state->data.program_main = {};
			state->changed();
		}

		break;

	case STATE_PROGRAM_DELETE:
		// If select pressed, delete cue
		if (Buttons::select.risen_since_state_change()) {
			// Reset current cue
			cuestack->resetCue(cuestack->currentCue);
			// Decrement currentCue so we don't fall off end of active part of array
			// if we delete last one!
			if (cuestack->currentCue > 0) {
				cuestack->currentCue--;
			}
			// One less cue now
			cuestack->totalCues--;
			interface->loadCurrentCue();
			// Move to cue screen to select next cue to edit
			interface->menu_pos = cuestack->currentCue;
			// Sort cues to remove blank line
			cuestack->sortCues();
			state->state = STATE_PROGRAM_CUELIST;
			state->data.program_cuelist = {};
			state->changed();
		}

		// If back pressed, don't
		if (Buttons::back.risen_since_state_change()) {
			interface->menu_pos = 5;
			state->state = STATE_PROGRAM_PARAMS;
			state->data.program_params = {};
			state->changed();
		}
		break;

	case STATE_PROGRAM_PARAMS:
		interface->updateMenu(5);

		if (Buttons::select.risen_since_state_change()) {
			if (interface->menu_pos < 4) {
				// Go into editing mode, reset keypad
				interface->editing = 1;
				interface->input.resetKeypad();

				// TODO(devel@dtwood.uk, #16): Refactor
				while (interface->editing) {
					// Just flip Yes/No variables on select
					if (interface->menu_pos > 0) {
						interface->cueParams[interface->menu_pos - 1] =
						    !interface->cueParams[interface->menu_pos - 1];
						interface->editing = 0;
						interface->limitCueParams();
						displays->forceUpdateDisplays(0, 1, 0, 0);
					} else {
						// Turn of GO and Pause LEDS
						digitalWrite(GOLED, LOW);

						if (interface->editVars(PROGRAM_PARAMS)) {
							// Only limit variables after every change if using encoder
							if (!interface->input.usingKeypad) {
								interface->limitCueParams();
							}
							displays->forceUpdateDisplays(0, 1, 0, 0);
						}
					}

					// If select pressed to confirm value, exit editing mode
					if (Buttons::select.risen_since_state_change()) {
						// Limit variables from keypad
						interface->limitCueParams();

						// Need to check cue number for validity if it has changed, increase
						// until valid one is found
						// Cannot get stuck in infinite loop as 1000 possible cue numbers but
						// only 100 cue slots
						if (interface->cueNumber != cuestack->stack[cuestack->currentCue].num) {
							while (!cuestack->validCueNumber(interface->cueNumber)) {
								if (interface->cueNumber < 99.9) {
									interface->cueNumber += 0.1;
								} else {
									interface->cueNumber = 0;
								}
							}
						}
						// Exit editing
						interface->editing = 0;
					}
				}

				// Update display, sort, update cue
				cuestack->setNumber(interface->cueNumber);
				cuestack->setParams(interface->cueParams);
				cuestack->sortCues();
				cuestack->currentCue =
				    cuestack->getCueIndex(interface->cueNumber);  // If sorting has occured, cue
				// highlighter in list is in wrong place
				// now
				displays->forceUpdateDisplays(1, 1, 1, 0);
			}

			// Add cue
			if (interface->menu_pos == 4) {
				// Find largest current cue number (stack is sorted by number)
				int activeCues = cuestack->activeCues();
				int cueNum = cuestack->stack[activeCues - 1].num;

				// Initialise next cue in stack with cueNum and default values
				cuestack->stack[activeCues].active = 1;
				cuestack->stack[activeCues].num = (cueNum + 1) / 10;
				cuestack->initialiseCue(activeCues);

				// Set as current cue, load into display arrays
				cuestack->currentCue = activeCues;
				cuestack->totalCues++;
				interface->loadCurrentCue();
				interface->menu_pos = 0;
				state->state = STATE_PROGRAM_MOVEMENTS;
				state->data.program_movements = {};
				state->changed();
			}

			// Delete cue
			if (interface->menu_pos == 5) {
				// Bring up warning dialog
				state->state = STATE_PROGRAM_DELETE;
				state->data.program_delete = {};
				state->changed();
			}
		}

		// Back one level only

		if (Buttons::back.risen_since_state_change()) {
			interface->menu_pos = 1;
			state->state = STATE_PROGRAM_MAIN;
			state->data.program_main = {};
			state->changed();
		}

		goToCurrentCue(PROGRAM_PARAMS);
		break;

	case STATE_PROGRAM_CUELIST:
		interface->updateMenu(cuestack->totalCues - 1);

		// If select pressed, load appropriate cue
		if (Buttons::select.risen_since_state_change()) {
			// Load cue data
			cuestack->currentCue = interface->menu_pos;
			interface->loadCurrentCue();

			// Back to PROGRAM
			interface->menu_pos = 0;
			state->state = STATE_PROGRAM_MAIN;
			state->data.program_main = {};
			state->changed();
		}

		// Back one level only
		if (Buttons::back.risen_since_state_change()) {
			interface->menu_pos = 2;
			state->state = STATE_PROGRAM_MAIN;
			state->data.program_main = {};
			state->changed();
		}
		break;

	// Run mode for during show - cannot edit cues
	case STATE_SHOW:

		// Only allow cue position jogging if select pressed
		if (Buttons::select.risen_since_state_change()) {
			// TODO(devel@dtwood.uk, #17): Refactor
			// Reset input encoder before start of edit
			interface->input.getInputEncoder();
			while (true) {
				// If select presed again, exit jogging mode
				if (Buttons::select.risen_since_state_change()) {
					break;
				}

				// Update selected cue
				interface->updateMenu(cuestack->totalCues - 1);
			}

			// Load cue data for highlighted cue
			interface->loadCue(interface->menu_pos);
			displays->forceUpdateDisplays(1, 1, 0, 0);
		}

		// Back one level only
		if (Buttons::back.risen_since_state_change()) {
			interface->menu_pos = 2;
			state->state = STATE_MAINMENU;
			state->data.mainmenu = {};
			state->changed();
		}
		break;

	// Change system settings
	case STATE_SETTINGS:
		interface->updateMenu(9);

		// Back to main menu if back pressed
		if (Buttons::back.risen_since_state_change()) {
			interface->menu_pos = 0;
			state->state = STATE_MAINMENU;
			state->data.mainmenu = {};
			state->changed();
		}

		// If select pressed move into corresponding setting
		if (Buttons::select.risen_since_state_change()) {
			// Choose based on menu_pos
			switch (interface->menu_pos) {
			case 0:

				// Homing Mode
				state->state = STATE_HOMING_INPROGRESS;
				state->data.homing_inprogress = {};
				state->changed();
				break;

			// Edit PID Constants
			case 1:
				interface->menu_pos = 0;
				state->state = STATE_KPSETTINGS;
				state->data.kpsettings = {};
				state->changed();
				break;

			// Edit default cue values
			case 2:
				interface->menu_pos = 0;
				state->state = STATE_DEFAULTVALUES;
				state->data.defaultvalues = {};
				state->changed();
				break;

			case 3:
				interface->menu_pos = 0;
				state->state = STATE_CUESTACK_BACKUP;
				state->data.cuestack_backup = {};
				state->changed();
				break;

			// Reset cuestack
			case 4:
				interface->menu_pos = 0;
				state->state = STATE_MAINMENU;
				state->data.mainmenu = {};
				state->changed();
				break;

			// Edit encoder settings
			case 5:
				interface->menu_pos = 0;
				state->state = STATE_ENCSETTINGS;
				state->data.encsettings = {};
				state->changed();
				break;

			// Edit LED settings
			case 6:
				interface->menu_pos = 0;
				state->state = STATE_BRIGHTNESS;
				state->data.brightness = {};
				state->changed();
				break;

			// Hardware test mode
			case 7:
				// All encoder lights on prevents switch from reading properly due to voltage
				interface->leds.encoderLedColor(false, false, true);
				state->state = STATE_HARDWARETEST;
				state->data.hardwaretest = {};
				state->changed();
				break;

			case 8:
				state->state = STATE_DEBUG;
				state->data.debug = {};
				state->changed();
				break;

			case 9:
				state->state = STATE_ABOUT;
				state->data.about = {};
				state->changed();
				break;

			default:
				interface->menu_pos = 0;
				state->state = STATE_MAINMENU;
				state->data.mainmenu = {};
				state->changed();
				break;
			}
		}
		break;

	// Mode to test all switches and LEDs
	case STATE_HARDWARETEST:

		// Read keypad
		interface->input.updateKeypad();

		// Exit if Go and Pause pressed
		if (Buttons::dmh.engaged() && Buttons::go.engaged()) {
			// Reset LEDs to settings state
			digitalWrite(SELECTLED, HIGH);

			// Back to settings
			state->state = STATE_SETTINGS;
			state->data.settings = {};
			state->changed();
		}
		break;

	case STATE_BRIGHTNESS:

		// Update menu_pos
		interface->updateMenu(3);

		// Exit back to settings if back pressed
		if (Buttons::back.risen_since_state_change()) {
			// Write new values, update current movements
			EEPROM.put(EELED_SETTINGS, interface->leds.ledSettings);

			// Back to settings
			interface->menu_pos = 5;
			state->state = STATE_SETTINGS;
			state->data.settings = {};
			state->changed();
		}

		updateSetting(brightnessLimiter, BRIGHTNESS);
		break;

	case STATE_ENCSETTINGS:

		// Update menu_pos
		interface->updateMenu(3);

		// Exit back to settings if back pressed
		if (Buttons::back.risen_since_state_change()) {
			// encSettings[0/1] store if direction is reversed
			EEPROM.put(EEINNER_ENC_RATIO, (interface->encSettings[0] ? -1 : 1) * interface->encSettings[2]);
			EEPROM.put(EEOUTER_ENC_RATIO, (interface->encSettings[1] ? -1 : 1) * interface->encSettings[3]);

			// Update actual values in object
			stage->updateEncRatios();

			// Back to settings
			interface->menu_pos = 4;
			state->state = STATE_SETTINGS;
			state->data.settings = {};
			state->changed();
		}

		updateSetting(encoderLimiter, ENCSETTINGS);
		break;

	// Set default cue values
	case STATE_DEFAULTVALUES:

		// Update selection screen
		interface->updateMenu(9);

		// Exit to main menu if back pressed
		if (Buttons::back.risen_since_state_change()) {
			// Store values back into EEPROM
			EEPROM.put(EEDEFAULT_VALUES, interface->defaultValues);
			EEPROM.get(EEDEFAULT_VALUES, interface->currentMovements);
			cuestack->updateDefaultValues();

			// Reset menu_pos and change mode
			interface->menu_pos = 2;
			state->state = STATE_SETTINGS;
			state->data.settings = {};
			state->changed();
		}

		updateSetting(eepromLimiter, DEFAULTVALUES);
		break;

	case STATE_KPSETTINGS:

		// Update menu_pos
		interface->updateMenu(5);

		// Exit back to settings if back pressed
		if (Buttons::back.risen_since_state_change()) {
			// Write new values on exit
			EEPROM.put(EEKP_SETTINGS, interface->kpSettings);

			// Update actual values in object
			stage->updateKpSettings();

			// Back to settings
			interface->menu_pos = 1;
			state->state = STATE_SETTINGS;
			state->data.settings = {};
			state->changed();
		}

		updateSetting(kpLimiter, KPSETTINGS);
		break;

	case STATE_CUESTACK_BACKUP:
		for (int i = 0; i < 100; i++) {
			char* encoding = encodeCue(cuestack->stack[i]);
			Serial.write(encoding);
			Serial.flush();
			delay(100);
			delete[] encoding;
		}
		state->state = STATE_MAINMENU;
		state->data.mainmenu = {};
		state->changed();

		break;
	}
}

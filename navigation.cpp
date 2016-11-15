#include "navigation.h"
#include "revolve_controller.h"
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
	// During homing everything handled by stage and display classes
	case STATE_HOMING_INPROGRESS:
		break;

	// Main Menu
	case STATE_MAINMENU:
		// Update menu with encoder
		interface->updateMenu(3);

		// If select is pressed, move into appropriate mode
		if (Buttons::select.engaged()) {
			// Reset menu position
			int menu_pos = interface->menu_pos;
			interface->menu_pos = 0;

			switch (menu_pos) {
			case 0:
				state->state = STATE_MANUAL_READY;
				state->data.manual_ready = {};
				displays->setMode();
				break;
			case 1:
				interface->loadCurrentCue();
				interface->menu_pos = 2;
				state->state = STATE_PROGRAM_MAIN;
				state->data.program_main = {};
				displays->setMode();
				break;
			case 2:
				interface->loadCurrentCue();
				interface->menu_pos = cuestack->currentCue;
				state->state = STATE_SHOW;
				state->data.show = {};
				displays->setMode();
				break;
			case 3:
				state->state = STATE_SETTINGS;
				state->data.settings = {};
				displays->setMode();
				break;
			}
		}
		break;

	// Manual control
	case STATE_MANUAL_READY:
		// Update selection screen
		interface->updateMenu(9);

		if (Buttons::back.engaged()) {
			// Reset menu_pos and change mode
			interface->menu_pos = 0;
			state->state = STATE_MAINMENU;
			state->data.mainmenu = {};
			displays->setMode();
		}

		updateSetting(manualLimiter, MANUAL);
		break;

	// Mode to edit cue stack
	case STATE_PROGRAM_MAIN:
		// Update menu and displays
		if (interface->updateMenu(2)) {
			displays->forceUpdateDisplays(1, 1, 1, 0);
		}

		if (Buttons::back.engaged()) {
			// Save Cuestack
			cuestack->saveCuestack();

			// Show saved message (without using delay)
			state->state = STATE_PROGRAM_SAVED;
			state->data.program_saved = {};
			displays->setMode();
			auto startTime = millis();
			while (millis() < startTime + 1000) {
			}

			interface->menu_pos = 0;
			state->state = STATE_MAINMENU;
			state->data.mainmenu = {};
			displays->setMode();
		}

		// If select pressed move into corresponding setting
		if (Buttons::select.engaged()) {
			// Choose based on menu_pos
			switch (interface->menu_pos) {
			case 0:
				interface->menu_pos = 0;
				state->state = STATE_PROGRAM_MOVEMENTS;
				state->data.program_movements = {};
				displays->setMode();
				break;
			case 1:
				interface->menu_pos = 0;
				state->state = STATE_PROGRAM_PARAMS;
				state->data.program_params = {};
				displays->setMode();
				break;
			case 2:
				interface->menu_pos = cuestack->currentCue;
				state->state = STATE_PROGRAM_CUELIST;
				state->data.program_cuelist = {};
				displays->setMode();
				break;
			}
		}

		goToCurrentCue(PROGRAM);
		break;

	case STATE_PROGRAM_MOVEMENTS:
		if (interface->cueParams[1] == 0 ||
		    interface->cueParams[2] == 0) {  // If either half disabled for this cue
			if (interface->updateMenu(4)) {
				displays->forceUpdateDisplays(1, 0, 0, 0);
			}
		} else {
			if (interface->updateMenu(9)) {
				displays->forceUpdateDisplays(1, 0, 0, 0);
			}
		}

		updateSetting(movementLimiter, PROGRAM_MOVEMENTS);

		// Back one level only
		if (Buttons::back.engaged()) {
			interface->menu_pos = 0;
			state->state = STATE_PROGRAM_MAIN;
			state->data.program_main = {};
			displays->setMode();
		}

		goToCurrentCue(PROGRAM_MOVEMENTS);
		break;

	case STATE_PROGRAM_PARAMS:
		if (interface->updateMenu(5)) {
			displays->forceUpdateDisplays(0, 1, 0, 0);
		}

		if (Buttons::select.engaged()) {
			if (interface->menu_pos < 4) {

				// Go into editing mode, reset keypad
				interface->editing = 1;
				interface->input.resetKeypad();

				while (interface->editing) {
					// Just flip Yes/No variables on select
					if (interface->menu_pos > 0) {
						interface->cueParams[interface->menu_pos - 1] =
						    !interface->cueParams[interface->menu_pos - 1];
						interface->editing = 0;
						interface->limitCueParams();
						displays->forceUpdateDisplays(0, 1, 0, 0);
					}

					// Else edit number
					else {
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
					if (Buttons::select.engaged()) {
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
								} else
									interface->cueNumber = 0;
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
				displays->setMode();
			}

			// Delete cue
			if (interface->menu_pos == 5) {
				// Bring up warning dialog
				state->state = STATE_PROGRAM_DELETE;
				state->data.program_delete = {};
				displays->setMode();
				auto decision = false;

				while (!decision) {
					// If select pressed, delete cue
					if (Buttons::select.engaged()) {
						// Reset current cue
						cuestack->resetCue(cuestack->currentCue);
						// Decrement currentCue so we don't fall off end of active part of array
						// if we delete last one!
						if (cuestack->currentCue > 0)
							cuestack->currentCue--;
						// One less cue now
						cuestack->totalCues--;
						interface->loadCurrentCue();
						// Move to cue screen to select next cue to edit
						interface->menu_pos = cuestack->currentCue;
						// Sort cues to remove blank line
						cuestack->sortCues();
						state->state = STATE_PROGRAM_CUELIST;
						state->data.program_cuelist = {};
						displays->setMode();
						decision = true;
					}

					// If back pressed, don't
					if (Buttons::back.engaged()) {
						interface->menu_pos = 5;
						state->state = STATE_PROGRAM_PARAMS;
						state->data.program_params = {};
						displays->setMode();
						decision = true;
					}
				}
			}
		}

		// Back one level only

		if (Buttons::back.engaged()) {
			interface->menu_pos = 1;
			state->state = STATE_PROGRAM_MAIN;
			state->data.program_main = {};
			displays->setMode();
		}

		goToCurrentCue(PROGRAM_PARAMS);
		break;

	case STATE_PROGRAM_CUELIST:
		if (interface->updateMenu(cuestack->totalCues - 1)) {
			displays->forceUpdateDisplays(0, 0, 1, 0);
		}

		// If select pressed, load appropriate cue
		if (Buttons::select.engaged()) {
			// Load cue data
			cuestack->currentCue = interface->menu_pos;
			interface->loadCurrentCue();

			// Back to PROGRAM
			interface->menu_pos = 0;
			state->state = STATE_PROGRAM_MAIN;
			state->data.program_main = {};
			displays->setMode();
		}

		// Back one level only
		if (Buttons::back.engaged()) {
			interface->menu_pos = 2;
			state->state = STATE_PROGRAM_MAIN;
			state->data.program_main = {};
			displays->setMode();
		}
		break;

	// Run mode for during show - cannot edit cues
	case STATE_SHOW:

		// Only allow cue position jogging if select pressed
		if (Buttons::select.engaged()) {
			// Reset input encoder before start of edit
			interface->input.getInputEncoder();
			while (true) {
				// If select presed again, exit jogging mode
				if (Buttons::select.engaged()) {
					break;
				}

				// Update selected cue
				if (interface->updateMenu(cuestack->totalCues - 1)) {
					displays->forceUpdateDisplays(0, 1, 0, 0);
				}
			}

			// Load cue data for highlighted cue
			interface->loadCue(interface->menu_pos);
			displays->forceUpdateDisplays(1, 1, 0, 0);
		}

		// Goto current cue if Go and Pause pressed
		if (Buttons::dmh.engaged() && Buttons::go.engaged()) {
			stage->runCurrentCue();
		}

		// Back one level only
		if (Buttons::back.engaged()) {
			interface->menu_pos = 2;
			state->state = STATE_MAINMENU;
			state->data.mainmenu = {};
			displays->setMode();
		}
		break;

	// Change system settings
	case STATE_SETTINGS:
		// Update menu position
		if (interface->updateMenu(8)) {
			displays->forceUpdateDisplays(0, 1, 0, 0);
		}

		// Back to main menu if back pressed
		if (Buttons::back.engaged()) {
			interface->menu_pos = 0;
			state->state = STATE_MAINMENU;
			state->data.mainmenu = {};
			displays->setMode();
		}

		// If select pressed move into corresponding setting
		if (Buttons::select.engaged()) {
			// Choose based on menu_pos
			switch (interface->menu_pos) {
			case 0:

				// Homing Mode
				state->state = STATE_HOMING_INPROGRESS;
				state->data.homing_inprogress = {};
				displays->setMode();

				while (true) {
					// Exit (1 level only, back to settings) if back pressed, reset LEDs
					if (Buttons::back.engaged()) {
						digitalWrite(GOLED, LOW);
						break;
					}

					// Run homing sequence if Pause and Go pressed
					if (Buttons::dmh.engaged() && Buttons::go.engaged()) {
						stage->gotoHome();
						break;
					}
				}

				// Back to settings mode
				state->state = STATE_SETTINGS;
				state->data.settings = {};
				displays->setMode();
				break;

			// Edit PID Constants
			case 1:
				interface->menu_pos = 0;
				state->state = STATE_KPSETTINGS;
				state->data.kpsettings = {};
				displays->setMode();
				break;

			// Edit default cue values
			case 2:
				interface->menu_pos = 0;
				state->state = STATE_DEFAULTVALUES;
				state->data.defaultvalues = {};
				displays->setMode();
				break;

			case 3:
				interface->menu_pos = 0;
				state->state = STATE_CUESTACK_BACKUP;
				state->data.cuestack_backup = {};
				displays->setMode();
				break;

			// Reset cuestack
			case 4:
				interface->menu_pos = 0;
				state->state = STATE_MAINMENU;
				state->data.mainmenu = {};
				displays->setMode();
				break;

			// Edit encoder settings
			case 5:
				interface->menu_pos = 0;
				state->state = STATE_ENCSETTINGS;
				state->data.encsettings = {};
				displays->setMode();
				break;

			// Edit LED settings
			case 6:
				interface->menu_pos = 0;
				state->state = STATE_BRIGHTNESS;
				state->data.brightness = {};
				displays->setMode();
				break;

			// Hardware test mode
			case 7:
				// All encoder lights on prevents switch from reading properly due to voltage
				interface->leds.encoderLedColor(false, false, true);
				state->state = STATE_HARDWARETEST;
				state->data.hardwaretest = {};
				displays->setMode();
				break;

			case 8:
				state->state = STATE_DEBUG;
				state->data.debug = {};
				displays->setMode();
				break;

			default:
				interface->menu_pos = 0;
				state->state = STATE_MAINMENU;
				state->data.mainmenu = {};
				displays->setMode();
				break;
			}
		}
		break;

	// Mode to test all switches and LEDs
	case STATE_HARDWARETEST:

		// Read keypad
		interface->input.updateKeypad();

		// Update end display to show which button being pressed
		displays->updateDisplays(0, 0, 1, 0);

		// Exit if Go and Pause pressed
		if (Buttons::dmh.engaged() && Buttons::go.engaged()) {
			// Reset LEDs to settings state
			digitalWrite(SELECTLED, HIGH);

			// Back to settings
			state->state = STATE_SETTINGS;
			state->data.settings = {};
			displays->setMode();
		}
		break;

	case STATE_BRIGHTNESS:

		// Update menu_pos
		if (interface->updateMenu(3)) {
			displays->forceUpdateDisplays(0, 1, 0, 0);
		}

		// Exit back to settings if back pressed
		if (Buttons::back.engaged()) {
			// Write new values, update current movements
			EEPROM.put(EELED_SETTINGS, interface->leds.ledSettings);

			// Back to settings
			interface->menu_pos = 5;
			state->state = STATE_SETTINGS;
			state->data.settings = {};
			displays->setMode();
		}

		updateSetting(brightnessLimiter, BRIGHTNESS);
		break;

	case STATE_ENCSETTINGS:

		// Update menu_pos
		if (interface->updateMenu(3)) {
			displays->forceUpdateDisplays(0, 1, 0, 0);
		}

		// Exit back to settings if back pressed
		if (Buttons::back.engaged()) {

			// encSettings[0/1] store if direction is reversed
			EEPROM.put(EEINNER_ENC_RATIO, (interface->encSettings[0] ? -1 : 1) * interface->encSettings[2]);
			EEPROM.put(EEOUTER_ENC_RATIO, (interface->encSettings[1] ? -1 : 1) * interface->encSettings[3]);

			// Update actual values in object
			stage->updateEncRatios();

			// Back to settings
			interface->menu_pos = 4;
			state->state = STATE_SETTINGS;
			state->data.settings = {};
			displays->setMode();
		}

		updateSetting(encoderLimiter, ENCSETTINGS);
		break;

	// Set default cue values
	case STATE_DEFAULTVALUES:

		// Update selection screen
		if (interface->updateMenu(9)) {
			displays->forceUpdateDisplays(0, 1, 0, 0);
		}

		// Exit to main menu if back pressed
		if (Buttons::back.engaged()) {
			// Store values back into EEPROM
			EEPROM.put(EEDEFAULT_VALUES, interface->defaultValues);
			EEPROM.get(EEDEFAULT_VALUES, interface->currentMovements);
			cuestack->updateDefaultValues();

			// Reset menu_pos and change mode
			interface->menu_pos = 2;
			state->state = STATE_SETTINGS;
			state->data.settings = {};
			displays->setMode();
		}

		updateSetting(eepromLimiter, DEFAULTVALUES);
		break;

	case STATE_KPSETTINGS:

		// Update menu_pos
		if (interface->updateMenu(5)) {
			displays->forceUpdateDisplays(0, 1, 0, 0);
		}

		// Exit back to settings if back pressed

		if (Buttons::back.engaged()) {
			// Write new values on exit
			EEPROM.put(EEKP_SETTINGS, interface->kpSettings);

			// Update actual values in object
			stage->updateKpSettings();

			// Back to settings
			interface->menu_pos = 1;
			state->state = STATE_SETTINGS;
			state->data.settings = {};
			displays->setMode();
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
		displays->setMode();

		break;
	}
}

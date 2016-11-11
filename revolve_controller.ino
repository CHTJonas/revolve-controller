#include "stage.h"
#include "state_machine.h"
#include <TimerOne.h>

char keys[4][3] = { {'1', '2', '3'}, {'4', '5', '6'}, {'7', '8', '9'}, {'*', '0', '#'} };
byte ROWS[4] = { KEY1, KEY2, KEY3, KEY4 };
byte COLS[3] = { KEY5, KEY6, KEY7 };
Keypad keypad = Keypad(makeKeymap(keys), ROWS, COLS, 4, 3);

Adafruit_NeoPixel pauseLeds = Adafruit_NeoPixel(2, PAUSELEDS, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ringLeds = Adafruit_NeoPixel(24, RINGLEDS, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel keypadLeds = Adafruit_NeoPixel(4, KEYPADLEDS, NEO_GRB + NEO_KHZ800);

Encoder enc_inner(INNERENC1, INNERENC2);
Encoder enc_outer(OUTERENC1, OUTERENC2);
Encoder enc_input(INPUTENC1, INPUTENC2);

U8GLIB_ST7920_128X64 cue1(22, 24, 26, U8G_PIN_NONE);
U8GLIB_ST7920_128X64 menu(23, 25, 27, U8G_PIN_NONE);
U8GLIB_ST7920_128X64 info(32, 34, 36, U8G_PIN_NONE);

// Con: Revolve(start_pin, dir_pin, speed_pin, enc);
State state; //= State { STATE_MAINMENU };
Revolve inner(4, 5, 6, enc_inner);
Revolve outer(11, 10, 9, enc_outer);
Cuestack cuestack;
Interface interface(cuestack, enc_input, keypad, ringLeds, pauseLeds, keypadLeds);
Displays displays(cue1, menu, info, ringLeds, inner, outer, keypad, interface, cuestack);
Stage stage(&state, &inner, &outer, &displays, &interface, &ringLeds);
char* encodeCue(Cue cue);
void updateFlags();
void setup();
void loop();
void goToCurrentCue(int target_mode);
void updateSetting(void(*settingLimiter)(void), int mode);
void brightnessLimiter();
void encoderLimiter();
void eepromLimiter();
void kpLimiter();
void manualLimiter();
void movementLimiter();

void setup() {
	Timer1.initialize(100000);
	Timer1.attachInterrupt(updateFlags);

	Serial.begin(1200);

	interface.setupLeds();
	interface.setupSwitches();

	// cuestack.loadCuestack();
	cuestack.loadExampleCues();
	displays.begin();
	displays.setMode(NORMAL);
}

void goToCurrentCue(int target_mode)
{
	// Goto current cue if Go and Pause pressed
	if (digitalRead(GO) == LOW && digitalRead(PAUSE) == LOW) {
		// Update displays to show realtime position
		displays.setMode(PROGRAM_GOTOCUE);

		// Turn off switch leds
		interface.encOff();
		digitalWrite(SELECTLED, LOW);
		digitalWrite(GOLED, LOW);

		// Move - both enabled
		if (interface.cueParams[1] && interface.cueParams[2]) {
			// stage.gotoPos();
		}
		// Move - inner disabled
		else if (interface.cueParams[1] == 0 && interface.cueParams[2]) {
			// stage.gotoPos();
		}
		// Move - outer disabled
		else if (interface.cueParams[1] && interface.cueParams[2] == 0) {
			// stage.gotoPos();
		}

		// Turn on switch leds
		interface.encGreen();
		digitalWrite(SELECTLED, HIGH);
		digitalWrite(GOLED, HIGH);

		// Reset mode
		displays.setMode(target_mode);
	}
}

void updateSetting(void(*settingLimiter)(void), int mode)
{
	interface.select.update();
	if (interface.select.read() == LOW) {
		interface.waitSelectRelease();

		interface.encRed();

		interface.editing = 1;
		interface.resetKeypad();

		while (interface.editing) {
			if (interface.editVars(mode))
			{
				if (!interface.usingKeypad) {
					(*settingLimiter)();
				}
				displays.forceUpdateDisplays(0, 1, 0, 1); // the parameters originally varied but since we're redoing everything...
			}

			// If select pressed to confirm value, exit editing mode
			interface.select.update();
			if (interface.select.read() == LOW) {
				interface.waitSelectRelease();

				(*settingLimiter)();
				interface.editing = 0;
			}
		}

		interface.encGreen();
	}
}

void brightnessLimiter()
{
	interface.limitLedSettings();
	keypadLeds.setBrightness(interface.ledSettings[0]);
	interface.keypadLedsColor(
	interface.ledSettings[1],
	interface.ledSettings[2],
	interface.ledSettings[3]);
	ringLeds.setBrightness(interface.ledSettings[0]);
	ringLeds.show();
	displays.forceUpdateDisplays(0, 1, 0, 1);
}

void encoderLimiter()
{
	interface.limitEncSettings();
	displays.forceUpdateDisplays(0, 1, 0, 0);
}

void eepromLimiter()
{
	interface.limitMovements(interface.defaultValues);
	displays.forceUpdateDisplays(0, 1, 0, 0);
}

void kpLimiter()
{
	interface.limitKpSettings();
	displays.forceUpdateDisplays(0, 1, 0, 0);
}

void manualLimiter()
{
	interface.limitMovements(interface.currentMovements);
	displays.forceUpdateDisplays(0, 1, 0, 0);
}

void movementLimiter()
{
	interface.limitMovements(interface.cueMovements);
	cuestack.setMovements(interface.cueMovements);
	displays.forceUpdateDisplays(1, 0, 1, 0);
}

void loop() {
	//stage.step();
	//displays.step();
	//return;
	// Main switch statement for navigating screens
	switch (displays.mode) {
		// Waits for Pause and GO to be pressed then initiates homing
	case STARTUP:

		// Wait for both switches
		while (digitalRead(GO) || digitalRead(PAUSE)) {
			interface.flashLed(GOLED, 500);
			interface.updatePauseLeds();
		}

		// Start homing
		digitalWrite(GOLED, LOW);
		stage.gotoHome();

		// Reset LEDs, go to NORMAL mode
		interface.pauseLedsColor(0, 0, 0);
		digitalWrite(GOLED, LOW);
		displays.setMode(NORMAL);
		break;

		// During homing everything handled by stage and display classes
	case HOMING:
		break;

		// Main Menu
	case NORMAL:
		// Update menu with encoder
		if (interface.updateMenu(3)) {
			displays.forceUpdateDisplays(0, 1, 0, 0);
		}

		// If select is pressed, move into appropriate mode
		interface.select.update();
		if (interface.select.read() == LOW) {
			interface.waitSelectRelease();

			// Reset menu position
			int menu_pos = interface.menu_pos;
			interface.menu_pos = 0;

			// Load current cue if required, set cursor to cuelist screen
			if (menu_pos == 1) {
				interface.loadCurrentCue();
				interface.menu_pos = 2;
			}

			// Set correct menu_pos for showmode, make sure cue is loaded
			if (menu_pos == 2) {
				interface.loadCurrentCue();
				interface.menu_pos = cuestack.currentCue;
			}

			// Go to required Mode
			displays.setMode(menu_pos + 3);
		}
		break;

		// Manual control
	case MAN:
		// Turn on GO and Pause LEDS
		digitalWrite(GOLED, HIGH);
		interface.updatePauseLeds();

		// Update selection screen
		if (interface.updateMenu(9)) {
			displays.forceUpdateDisplays(0, 1, 0, 0);
		}

		// Exit to main menu if back pressed
		if (digitalRead(BACK)) {

			// Turn off GO and Pause
			digitalWrite(GOLED, LOW);
			interface.pauseLedsColor(0, 0, 0);

			// Reset menu_pos and change mode
			interface.menu_pos = 0;
			displays.setMode(NORMAL);
		}

		updateSetting(manualLimiter, MANUAL);

		// Move to required position if Go and Pause pressed
		if (digitalRead(GO) == LOW && digitalRead(PAUSE) == LOW) {

			// Turn off switch leds
			interface.encOff();
			digitalWrite(SELECTLED, LOW);
			digitalWrite(GOLED, LOW);

			// Move
			// stage.gotoPos();

			// Turn on switch leds
			interface.encGreen();
			digitalWrite(SELECTLED, HIGH);
			digitalWrite(GOLED, HIGH);
		}
		break;

		// Mode to edit cue stack
	case PROGRAM:
		// Turn on GO and Pause LEDS
		digitalWrite(GOLED, HIGH);
		interface.updatePauseLeds();

		// Update menu and displays
		if (interface.updateMenu(2)) {
			displays.forceUpdateDisplays(1, 1, 1, 0);
		}

		// Back to main menu if back pressed
		if (digitalRead(BACK)) {
			// Save Cuestack
			cuestack.saveCuestack();

			// Show saved message (without using delay)
			displays.setMode(PROGRAM_SAVED);
			long startTime = millis();
			while (millis() < startTime + 1000) {
			}

			// Turn off GO and Pause LEDS
			digitalWrite(GOLED, LOW);
			interface.pauseLedsColor(0, 0, 0);
			interface.menu_pos = 0;
			displays.setMode(NORMAL);
		}

		// If select pressed move into corresponding setting
		interface.select.update();
		if (interface.select.read() == LOW) {
			interface.waitSelectRelease();

			// Choose based on menu_pos
			switch (interface.menu_pos) {
			case 0:
				interface.menu_pos = 0;
				displays.setMode(PROGRAM_MOVEMENTS);
				break;
			case 1:
				interface.menu_pos = 0;
				displays.setMode(PROGRAM_PARAMS);
				break;
			case 2:
				interface.menu_pos = cuestack.currentCue;
				displays.setMode(PROGRAM_CUELIST);
				break;
			}
		}

		goToCurrentCue(PROGRAM);
		break;

	case PROGRAM_MOVEMENTS:
		// Turn on GO and Pause LEDS
		digitalWrite(GOLED, HIGH);
		interface.updatePauseLeds();

		if (interface.cueParams[1] == 0 ||
		interface.cueParams[2] == 0) {  // If either half disabled for this cue
			if (interface.updateMenu(4)) {
				displays.forceUpdateDisplays(1, 0, 0, 0);
			}
		}
		else {
			if (interface.updateMenu(9)) {
				displays.forceUpdateDisplays(1, 0, 0, 0);
			}
		}

		updateSetting(movementLimiter, PROGRAM_MOVEMENTS);

		// Back one level only
		interface.back.update();
		if (interface.back.read()) {
			interface.waitBackRelease();
			interface.menu_pos = 0;
			displays.setMode(PROGRAM);
		}

		goToCurrentCue(PROGRAM_MOVEMENTS);
		break;

	case PROGRAM_PARAMS:
		// Turn on GO and Pause LEDS
		digitalWrite(GOLED, HIGH);
		interface.updatePauseLeds();

		if (interface.updateMenu(5)) {
			displays.forceUpdateDisplays(0, 1, 0, 0);
		}

		// If select pressed, edit appropriate parameter
		interface.select.update();
		if (interface.select.read() == LOW) {
			interface.waitSelectRelease();

			if (interface.menu_pos < 4) {
				// Change LED colors
				interface.encRed();

				// Go into editing mode, reset keypad
				interface.editing = 1;
				interface.resetKeypad();

				while (interface.editing) {
					// Just flip Yes/No variables on select
					if (interface.menu_pos > 0) {
						interface.cueParams[interface.menu_pos - 1] =
							!interface.cueParams[interface.menu_pos - 1];
						interface.editing = 0;
						interface.limitCueParams();
						displays.forceUpdateDisplays(0, 1, 0, 0);
					}

					// Else edit number
					else {
						// Turn of GO and Pause LEDS
						digitalWrite(GOLED, LOW);
						interface.pauseLedsColor(0, 0, 0);  // Moved into here so they don't
															// flash when just flipping yes/no
															// parameter

						if (interface.editVars(PROGRAM_PARAMS)) {
							// Only limit variables after every change if using encoder
							if (!interface.usingKeypad) {
								interface.limitCueParams();
							}
							displays.forceUpdateDisplays(0, 1, 0, 0);
						}
					}

					// If select pressed to confirm value, exit editing mode
					interface.select.update();
					if (interface.select.read() == LOW) {
						interface.waitSelectRelease();

						// Limit variables from keypad
						interface.limitCueParams();

						// Need to check cue number for validity if it has changed, increase
						// until valid one is found
						// Cannot get stuck in infinite loop as 1000 possible cue numbers but
						// only 100 cue slots
						if (interface.cueNumber != cuestack.stack[cuestack.currentCue].num) {
							while (!cuestack.validCueNumber(interface.cueNumber)) {
								if (interface.cueNumber < 99.9) {
									interface.cueNumber += 0.1;
								}
								else
								interface.cueNumber = 0;
							}
						}
						// Exit editing
						interface.editing = 0;
					}
				}
				// Change back LED colors
				interface.encGreen();

				// Update display, sort, update cue
				cuestack.setNumber(interface.cueNumber);
				cuestack.setParams(interface.cueParams);
				cuestack.sortCues();
				cuestack.currentCue =
					cuestack.getCueIndex(interface.cueNumber);  // If sorting has occured, cue
																// highlighter in list is in wrong place
																// now
				displays.forceUpdateDisplays(1, 1, 1, 0);
			}

			// Add cue
			if (interface.menu_pos == 4) {
				// Find largest current cue number (stack is sorted by number)
				int activeCues = cuestack.activeCues();
				int cueNum = cuestack.stack[activeCues - 1].num;

				// Initialise next cue in stack with cueNum and default values
				cuestack.stack[activeCues].active = 1;
				cuestack.stack[activeCues].num = (cueNum + 1) / 10;
				cuestack.initialiseCue(activeCues);

				// Set as current cue, load into display arrays
				cuestack.currentCue = activeCues;
				cuestack.totalCues++;
				interface.loadCurrentCue();
				interface.menu_pos = 0;
				displays.setMode(PROGRAM_MOVEMENTS);
			}

			// Delete cue
			if (interface.menu_pos == 5) {
				// Turn of GO and Pause LEDS
				digitalWrite(GOLED, LOW);
				interface.pauseLedsColor(0, 0, 0);  // Moved into here so they don't flash when just flipping yes/no parameter

				// Bring up warning dialog
				displays.setMode(PROGRAM_DELETE);
				int decision = 0;

				while (!decision) {

					// If select pressed, delete cue
					interface.select.update();
					if (interface.select.read() == LOW) {
						interface.waitSelectRelease();
						// Reset current cue
						cuestack.resetCue(cuestack.currentCue);
						// Decrement currentCue so we don't fall off end of active part of array
						// if we delete last one!
						if (cuestack.currentCue > 0)
							cuestack.currentCue--;
						// One less cue now
						cuestack.totalCues--;
						interface.loadCurrentCue();
						// Move to cue screen to select next cue to edit
						interface.menu_pos = cuestack.currentCue;
						// Sort cues to remove blank line
						cuestack.sortCues();
						displays.setMode(PROGRAM_CUELIST);
						decision = 1;
					}

					// If back pressed, don't!
					interface.back.update();
					if (interface.back.read() == HIGH) {
						interface.waitBackRelease();
						// Go back into PROGRAM_PARAMS
						interface.menu_pos = 5;
						displays.setMode(PROGRAM_PARAMS);
						decision = 1;
					}
				}
			}
		}

		// Back one level only
		interface.back.update();
		if (interface.back.read()) {
			interface.waitBackRelease();
			interface.menu_pos = 1;
			displays.setMode(PROGRAM);
		}

		goToCurrentCue(PROGRAM_PARAMS);
		break;

	case PROGRAM_CUELIST:
		// Turn of GO and Pause LEDS
		digitalWrite(GOLED, LOW);
		interface.pauseLedsColor(0, 0, 0);

		if (interface.updateMenu(cuestack.totalCues - 1)) {
			displays.forceUpdateDisplays(0, 0, 1, 0);
		}

		// If select pressed, load appropriate cue
		interface.select.update();
		if (interface.select.read() == LOW) {
			interface.waitSelectRelease();

			// Load cue data
			cuestack.currentCue = interface.menu_pos;
			interface.loadCurrentCue();

			// Back to PROGRAM
			interface.menu_pos = 0;
			displays.setMode(PROGRAM);
		}

		// Back one level only
		interface.back.update();
		if (interface.back.read()) {
			interface.waitBackRelease();
			interface.menu_pos = 2;
			displays.setMode(PROGRAM);
		}
		break;

		// Run mode for during show - cannot edit cues
	case SHOW:
		// Turn on GO and Pause LEDS
		digitalWrite(GOLED, HIGH);
		interface.updatePauseLeds();
		interface.encOff();
		digitalWrite(SELECTLED, HIGH);

		// Only allow cue position jogging if select pressed
		interface.select.update();
		if (interface.select.read() == LOW) {
			// Update LEDs
			digitalWrite(GOLED, LOW);
			interface.pauseLedsColor(0, 0, 0);
			interface.encGreen();

			// Reset input encoder before start of edit
			interface.getInputEnc();

			interface.waitSelectRelease();
			while (true) {
				interface.select.update();

				// If select presed again, exit jogging mode
				if (interface.select.read() == LOW) {
					interface.waitSelectRelease();
					break;
				}

				// Update selected cue
				if (interface.updateMenu(cuestack.totalCues - 1)) {
					displays.forceUpdateDisplays(0, 1, 0, 0);
				}
			}
			// Update LEDs
			digitalWrite(GOLED, HIGH);
			interface.updatePauseLeds();
			interface.encOff();
			// Load cue data for highlighted cue
			interface.loadCue(interface.menu_pos);
			displays.forceUpdateDisplays(1, 1, 0, 0);
		}

		// Goto current cue if Go and Pause pressed
		if (digitalRead(GO) == LOW && digitalRead(PAUSE) == LOW) {
			stage.runCurrentCue();
		}

		// Back one level only
		interface.back.update();
		if (interface.back.read()) {
			interface.waitBackRelease();
			interface.menu_pos = 2;
			displays.setMode(NORMAL);
		}
		break;

		// Change system settings
	case SETTINGS:

		// Update menu position
		if (interface.updateMenu(7)) {
			displays.forceUpdateDisplays(0, 1, 0, 0);
		}

		// Back to main menu if back pressed
		if (digitalRead(BACK)) {
			interface.menu_pos = 0;
			displays.setMode(NORMAL);
		}

		// If select pressed move into corresponding setting
		interface.select.update();
		if (interface.select.read() == LOW) {
			interface.waitSelectRelease();

			// Choose based on menu_pos
			switch (interface.menu_pos) {
			case 0:

				// Homing Mode
				displays.setMode(STARTUP);

				while (true) {

					// Flash GO led, update pause led
					interface.flashLed(GOLED, 500);
					interface.updatePauseLeds();

					// Exit (1 level only, back to settings) if back pressed, reset LEDs
					interface.back.update();
					if (interface.back.read()) {
						interface.waitBackRelease();
						digitalWrite(GOLED, LOW);
						interface.pauseLedsColor(0, 0, 0);
						break;
					}

					// Run homing sequence if Pause and Go pressed
					if (digitalRead(GO) == LOW && digitalRead(PAUSE) == LOW) {

						// Update LEDS
						digitalWrite(GOLED, HIGH);
						interface.pauseLedsColor(0, 255, 0);

						stage.gotoHome();

						// Reset LEDS
						interface.pauseLedsColor(0, 0, 0);
						digitalWrite(GOLED, LOW);
						break;
					}
				}

				// Back to settings mode
				displays.setMode(SETTINGS);
				break;

				// Edit PID Constants
			case 1:
				interface.menu_pos = 0;
				displays.setMode(KPSETTINGS);
				break;

				// Edit default cue values
			case 2:
				interface.menu_pos = 0;
				displays.setMode(DEFAULTVALUES);
				break;

			case 3:
				interface.menu_pos = 0;
				displays.setMode(CUESTACK_BACKUP);
				break;

				// Reset cuestack
			case 4:
				interface.menu_pos = 0;
				displays.setMode(NORMAL);
				break;

				// Edit encoder settings
			case 5:
				interface.menu_pos = 0;
				displays.setMode(ENCSETTINGS);
				break;

				// Edit LED settings
			case 6:
				interface.menu_pos = 0;
				displays.setMode(BRIGHTNESS);
				break;

				// Hardware test mode
			case 7:
				interface.allLedsOn();
				interface.encBlue();  // All encoder lights on prevents switch from reading properly due
									  // to voltage drop
				displays.setMode(HARDWARETEST);
				break;

			default:
				interface.menu_pos = 0;
				displays.setMode(NORMAL);
				break;
			}
		}
		break;

		// Mode to test all switches and LEDs
	case HARDWARETEST:

		// Read keypad
		interface.updateKeypad();

		// Update end display to show which button being pressed
		displays.updateDisplays(0, 0, 1, 0);

		// Exit if Go and Pause pressed
		if (digitalRead(GO) == LOW && digitalRead(PAUSE) == LOW) {

			// Reset LEDs to settings state
			interface.allLedsOff();
			interface.keypadLedsColor(
			interface.ledSettings[1], interface.ledSettings[2], interface.ledSettings[3]);
			interface.encGreen();
			digitalWrite(SELECTLED, HIGH);

			// Back to settings
			displays.setMode(SETTINGS);
		}
		break;

	case BRIGHTNESS:

		// Update menu_pos
		if (interface.updateMenu(3)) {
			displays.forceUpdateDisplays(0, 1, 0, 0);
		}

		// Exit back to settings if back pressed
		interface.back.update();
		if (interface.back.read()) {
			interface.waitBackRelease();

			// Write new values, update current movements
			EEPROM.put(EELED_SETTINGS, interface.ledSettings);

			// Back to settings
			interface.menu_pos = 5;
			displays.setMode(SETTINGS);
		}

		updateSetting(brightnessLimiter, BRIGHTNESS);
		break;

	case ENCSETTINGS:

		// Update menu_pos
		if (interface.updateMenu(3)) {
			displays.forceUpdateDisplays(0, 1, 0, 0);
		}

		// Exit back to settings if back pressed
		interface.back.update();
		if (interface.back.read()) {
			interface.waitBackRelease();

			// encSettings[0/1] store if direction is reversed
			EEPROM.put(EEINNER_ENC_RATIO, (interface.encSettings[0] ? -1 : 1) * interface.encSettings[2]);
			EEPROM.put(EEOUTER_ENC_RATIO, (interface.encSettings[1] ? -1 : 1) * interface.encSettings[3]);

			// Update actual values in object
			stage.updateEncRatios();

			// Back to settings
			interface.menu_pos = 4;
			displays.setMode(SETTINGS);
		}

		updateSetting(encoderLimiter, ENCSETTINGS);
		break;

		// Set default cue values
	case DEFAULTVALUES:

		// Update selection screen
		if (interface.updateMenu(9)) {
			displays.forceUpdateDisplays(0, 1, 0, 0);
		}

		// Exit to main menu if back pressed
		if (digitalRead(BACK)) {
			// Store values back into EEPROM
			EEPROM.put(EEDEFAULT_VALUES, interface.defaultValues);
			EEPROM.get(EEDEFAULT_VALUES, interface.currentMovements);
			cuestack.updateDefaultValues();

			// Reset menu_pos and change mode
			interface.menu_pos = 2;
			displays.setMode(SETTINGS);
		}

		updateSetting(eepromLimiter, DEFAULTVALUES);
		break;

	case KPSETTINGS:

		// Update menu_pos
		if (interface.updateMenu(5)) {
			displays.forceUpdateDisplays(0, 1, 0, 0);
		}

		// Exit back to settings if back pressed
		interface.back.update();
		if (interface.back.read()) {
			interface.waitBackRelease();

			// Write new values on exit
			EEPROM.put(EEKP_SETTINGS, interface.kpSettings);

			// Update actual values in object
			stage.updateKpSettings();

			// Back to settings
			interface.menu_pos = 1;
			displays.setMode(SETTINGS);
		}

		updateSetting(kpLimiter, KPSETTINGS);
		break;

	case CUESTACK_BACKUP:
		for (int i = 0; i < 100; i++) {
			char* encoding = encodeCue(cuestack.stack[i]);
			int x = Serial.write(encoding);
			Serial.flush();
			delay(100);
			delete[] encoding;
		}
		displays.setMode(NORMAL);

		break;
	}
}

void updateFlags() {
	inner.tenths = 1;
	outer.tenths = 1;
	displays.update = 1;
}

char* encodeCue(Cue cue) {
	auto cue_bytes = reinterpret_cast<char*>(&cue);
	auto encoded = new char[(sizeof cue) * 2 + 1];
	for (auto i = 0; i < sizeof cue; i++) {
		encoded[2 * i] = cue_bytes[i] | 0x0f;
		encoded[2 * i + 1] = cue_bytes[i] | 0xf0;
	}

	encoded[(sizeof cue) * 2] = '\0';
	return encoded;
}

void setup();
void loop();

char* encodeCue(Cue cue);
void updateFlags();
void goToCurrentCue(int target_mode);
void updateSetting(void (*settingLimiter)(void), int mode);
void brightnessLimiter();
void encoderLimiter();
void eepromLimiter();
void kpLimiter();
void manualLimiter();
void movementLimiter();

#include "constants.h"
#include <Arduino.h>

bool eStopsEngaged() {
	// Commented out line for non-conencted external esstop testing
	// if !(digitalRead(ESTOPNC1)==LOW && digitalRead(ESTOPNC2)==LOW && digitalRead(ESTOPNC3)==LOW &&
	// digitalRead(ESTOPNO)==HIGH){
	return !(digitalRead(ESTOPNC1) == LOW && digitalRead(ESTOPNO) == HIGH);
}

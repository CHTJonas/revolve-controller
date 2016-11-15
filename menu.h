#pragma once
#include "constants.h"

class Menu {
public:
	// Constructor
	Menu(int size);

	int menu_position;
	char* elements[];

private:
};

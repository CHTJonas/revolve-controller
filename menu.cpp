#include "menu.h"

Menu::Menu(const char* menuOptionStrings[], int numberOfOptions) {
	m_menuOptionStrings = new const char*[numberOfOptions];

	for (int i = 0; i < numberOfOptions; i++) {
		m_menuOptionStrings[i] = menuOptionStrings[i];
	}
}

Menu::~Menu() {
	delete[] m_menuOptionStrings;
}

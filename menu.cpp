#include "menu.h"
#include "constants.h"

<<<<<<< HEAD
#include "interface.h"
#include "revolve.h"
#include "state.h"
#include "strings.h"
#include "menu.h"
#include <U8glib.h>
#include "displays.h"
#include "logo.h"
#include "state.h"
#include "pins.h"
#include "strings.h"

Menu::Menu(const char* menuOptionStrings[], int numberOfOptions, U8GLIB_ST7920_128X64& screen) {
    m_screen = screen;
    m_numberOfOptions = numberOfOptions;
    m_menuOptionStrings = new const char*[numberOfOptions];
    for (int i = 0; i < numberOfOptions; i++) {
      m_menuOptionStrings[i] = menuOptionStrings[i];
   }
}

Menu::~Menu() {
	delete[] m_menuOptionStrings;
}

void Menu::draw() {
    m_screen.setFont(large_font);
    for (int i = 0; i < m_numberOfOptions; i++) {
        drawStrCentre((i * 16) + 12, m_menuOptionStrings[i]);
        m_screen.setDefaultForegroundColor();
    }
}

void Menu::drawStrCentre(int y, const char* text) {
	int width = m_screen.getStrWidth(text);
	int x = (SCREEN_WIDTH - width) / 2;
	m_screen.setPrintPos(x, y);
	m_screen.print(text);
}

void Menu::drawStrCentre(int y, char text) {
	const char string[] = { text, '\0' };
	drawStrCentre(y, string);
}

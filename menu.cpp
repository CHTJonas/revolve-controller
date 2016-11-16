#include "menu.h"
#include "constants.h"

Menu::Menu(const char* menuOptionStrings[], int numberOfOptions,
    U8GLIB_ST7920_128X64& screen, Interface& interface)
    : m_screen(screen), m_interface(interface){
    m_numberOfOptions = numberOfOptions;
    m_menuOptionStrings = new const char*[numberOfOptions];
    for (int i = 0; i < numberOfOptions; i++) {
      m_menuOptionStrings[i] = menuOptionStrings[i];
   }
}

Menu::~Menu() {
	delete[] m_menuOptionStrings;
}

void Menu::drawStrCentre(int y, const char* text) const {
	int width = m_screen.getStrWidth(text);
	int x = (SCREEN_WIDTH - width) / 2;
	m_screen.setPrintPos(x, y);
	m_screen.print(text);
}

void Menu::drawStrCentre(int y, char text) {
	const char string[] = { text, '\0' };
	drawStrCentre(y, string);
}

void Menu::draw() const {
    m_screen.setFont(large_font);
    // we can display max 4 options on the screen at a time
    int max = m_numberOfOptions < 4 ? m_numberOfOptions : 4;
    for (int i = 0; i < max; i++) {
        if ((i + m_numberOfOptions / 4) == m_interface.menu_pos) {
            // Position highlight box from top left corner
            m_screen.drawBox(0, (m_interface.menu_pos * 16), SCREEN_WIDTH, 16);
            m_screen.setDefaultBackgroundColor();
            drawStrCentre((i * 16) + 12, m_menuOptionStrings[i + m_numberOfOptions / 4]);
        }
        drawStrCentre((i * 16) + 12, m_menuOptionStrings[i + m_numberOfOptions / 4]);
        m_screen.setDefaultForegroundColor();
    }
}

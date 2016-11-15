class Menu {
public:
	Menu(const char* menuOptionStrings[], int numberOfOptions);
	~Menu();
	// BaseMenu *getNextMenu(int iChoice, bool& iIsQuitOptionSelected);
	// void draw();

private:
	char** m_menuOptionStrings;
	int m_numberOfOptions;
};

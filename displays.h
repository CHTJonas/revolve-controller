#include <U8glib.h>
#include "revolve.h"
#include "interface.h"

class Displays {
  public:

  // Constructor
  Displays(U8GLIB_ST7920_128X64& cue, U8GLIB_ST7920_128X64& menu, U8GLIB_ST7920_128X64& info, Adafruit_NeoPixel& ringLeds, Revolve& inner, Revolve& outer, Keypad& keypad, Interface& interface, Cuestack& cuestack);

  // Initialisation
  void begin();
  void setMode(int newMode);

  // Screen Drawing
  void drawStrCenter(U8GLIB_ST7920_128X64& lcd, int y, int text);
  void drawStrCenter(U8GLIB_ST7920_128X64& lcd, int y, const char* text);
  void drawStrCenter(U8GLIB_ST7920_128X64& lcd, int y, char text);
  void drawCueLayout(U8GLIB_ST7920_128X64& lcd, int (&values)[10], int cursorEnable);
  void drawParamsLayout(U8GLIB_ST7920_128X64& lcd, int cursorEnable);
  void drawCuelistLayout(U8GLIB_ST7920_128X64& lcd, int index, int cursorEnable);
  void drawCue();
  void drawMenu();
  void drawInfo();
  void updateRingLeds();
  void updateDisplays(int cue1, int menu, int info, int ringLeds);
  void forceUpdateDisplays(int cue1, int menu, int info, int ringLeds);

  int _update;
  int mode;
  int ledOuter;
  int ledInner;

  private:

  char *menu_strings[4] = {"Manual Operation", "Program Cues", "Show Mode", "Settings"};
  char *settings_strings[8] = {"Home Revolve", "PID Constants", "Default Cue Values", "Backup Cuestack","Reset Cuestack","Encoder Settings","LED Settings","Hardware Test"};
  char *enc_settings_strings[4] = {"Inner Dir:","Outer Dir:","Inner RO:","Outer RO:"};
  char *led_settings_strings[4] = {"Brightness:","LEDs Red:","LEDs Green:","LEDs Blue"};
  char *kp_settings_strings[6]= {"Inner kp_0:","Inner kp_smin:","Inner kp_amax:","Outer kp_0:","Outer kp_smin:","Outer kp_amax:"};
  char *program_strings[4]={"Next Cue","Previous Cue","Add Cue","Delete Cue"};
  char *param_strings[3]={"Auto Follow:","Inner EN:","Outer EN:"};

  const u8g_fntpgm_uint8_t *font=u8g_font_profont11; // 8px high row
  const u8g_fntpgm_uint8_t *xlarge_font=u8g_font_profont22; // 16px high row
  const u8g_fntpgm_uint8_t *large_font=u8g_font_profont15; // 10px high row
  const u8g_fntpgm_uint8_t *small_font=u8g_font_5x7; // 7px high row
  
  U8GLIB_ST7920_128X64& _cue;
  U8GLIB_ST7920_128X64& _menu;
  U8GLIB_ST7920_128X64& _info;

  Adafruit_NeoPixel& _ringLeds;

  Revolve& _inner;
  Revolve& _outer;

  Keypad& _keypad;

  Interface& _interface;

  Cuestack& _cuestack;

};

static unsigned char screen_logo[] U8G_PROGMEM = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
   0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xc0, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x1f, 0x04, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc,
   0x3f, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xfe, 0x7f, 0x1f, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x9f, 0x07, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x18, 0x00, 0xff,
   0xff, 0xdf, 0x07, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0,
   0x01, 0x38, 0x80, 0xff, 0xff, 0xff, 0x03, 0x00, 0x1e, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xf8, 0x01, 0x7c, 0xc0, 0xff, 0xff, 0xff, 0x83, 0x07,
   0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x03, 0xfe, 0xe0, 0xff,
   0xff, 0xff, 0xc3, 0x0f, 0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe,
   0x07, 0xff, 0xf3, 0x7f, 0x80, 0xff, 0xe3, 0xff, 0xff, 0x07, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xff, 0x1f, 0xff, 0xff, 0x7f, 0x80, 0xff, 0xf1, 0xff,
   0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff, 0x9f, 0xff, 0xff, 0xff,
   0x83, 0xff, 0xf9, 0xff, 0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xff,
   0xff, 0xff, 0xff, 0xff, 0x87, 0xff, 0xfd, 0xff, 0xff, 0x07, 0x00, 0x00,
   0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0x87, 0xff, 0xff, 0xff,
   0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff,
   0x87, 0xff, 0xff, 0xff, 0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xff,
   0x1f, 0xfc, 0xff, 0xff, 0x87, 0xff, 0x1f, 0xf8, 0xff, 0x07, 0x00, 0x00,
   0x00, 0x00, 0xf8, 0xff, 0x07, 0xf0, 0xff, 0x03, 0x86, 0xff, 0x07, 0xf0,
   0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xff, 0xc3, 0xc0, 0xff, 0x01,
   0x84, 0xff, 0x03, 0xe0, 0xff, 0x3f, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff,
   0xf1, 0xc3, 0xff, 0xc0, 0x80, 0xff, 0xc1, 0xc7, 0xff, 0xff, 0x00, 0x00,
   0x00, 0x00, 0xfe, 0xff, 0xc1, 0xc7, 0xff, 0xf0, 0x81, 0xff, 0xe0, 0xc3,
   0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xc1, 0x87, 0x7f, 0xf0,
   0x83, 0xff, 0xf0, 0xc1, 0xff, 0x7f, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff,
   0xe3, 0x83, 0x7f, 0xf8, 0x87, 0x7f, 0xf0, 0xc1, 0xff, 0x3f, 0x00, 0x00,
   0x00, 0x00, 0xfc, 0xff, 0xff, 0x80, 0x7f, 0xf8, 0x87, 0x7f, 0xf0, 0xc3,
   0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0x1f, 0x80, 0x7f, 0xf8,
   0x87, 0x7f, 0xf0, 0xff, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff,
   0x03, 0x86, 0x7f, 0xf8, 0x87, 0x7f, 0xf0, 0xff, 0xff, 0x0f, 0x00, 0x00,
   0x00, 0x00, 0xc0, 0xff, 0x81, 0x87, 0x7f, 0xf8, 0x87, 0x7f, 0xf0, 0xc7,
   0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xff, 0xc1, 0x83, 0x78, 0xf0,
   0x83, 0x7f, 0xf0, 0xc7, 0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xff,
   0xe0, 0x83, 0x78, 0xf0, 0x83, 0xff, 0xf0, 0xc7, 0xff, 0x01, 0x00, 0x00,
   0x00, 0x00, 0xc0, 0xff, 0xe0, 0x83, 0xf8, 0xe0, 0x81, 0xff, 0xe0, 0xc3,
   0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0xc0, 0x80, 0xfc, 0x00,
   0x00, 0xfc, 0x01, 0xe0, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff,
   0x01, 0x04, 0xfc, 0x01, 0x04, 0xfc, 0x03, 0xf0, 0x7f, 0x00, 0x00, 0x00,
   0x00, 0x00, 0xf0, 0xff, 0x03, 0x0e, 0xfe, 0x03, 0x06, 0xfc, 0x07, 0xf8,
   0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f, 0x00, 0x00, 0x00,
   0x00, 0x00, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xff, 0xff, 0x3f, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x7f,
   0xff, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00,
   0x00, 0x00, 0xfe, 0x3f, 0xff, 0x1f, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff,
   0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x1f, 0xff, 0x0f, 0xfc, 0xf7,
   0xff, 0xff, 0xff, 0xfc, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x1f,
   0xfc, 0x07, 0xe0, 0xc3, 0xff, 0xf9, 0x7f, 0xfc, 0x07, 0x00, 0x00, 0x00,
   0x00, 0x00, 0xc0, 0x0f, 0xe8, 0x07, 0xc0, 0x83, 0xff, 0xe1, 0x7f, 0x00,
   0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x07, 0xe0, 0x03, 0x80, 0x80,
   0xfe, 0x80, 0x3f, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x04,
   0xc0, 0x03, 0x00, 0x00, 0xfc, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x7c, 0x00, 0x1c, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0,
   0x3f, 0x83, 0xf1, 0x07, 0x86, 0xff, 0xf9, 0xc1, 0x1f, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x03, 0x83, 0x31, 0x00, 0x0f, 0x18, 0x18, 0xc3,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x83, 0x31, 0x00,
   0x09, 0x18, 0x18, 0xc6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x03, 0x83, 0x31, 0x80, 0x19, 0x18, 0x18, 0xc6, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x03, 0x83, 0x31, 0x80, 0x19, 0x18, 0x18, 0xc6,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xf1, 0x83,
   0x10, 0x18, 0x18, 0xc3, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x03, 0x83, 0x31, 0xc0, 0x30, 0x18, 0xf8, 0xc1, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x03, 0x83, 0x31, 0xc0, 0x3f, 0x18, 0x18, 0xc1,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x83, 0x31, 0xc0,
   0x30, 0x18, 0x18, 0xc3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x03, 0x83, 0x31, 0xc0, 0x30, 0x18, 0x18, 0xc6, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x03, 0x83, 0x31, 0xc0, 0x30, 0x18, 0x18, 0xcc,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x83, 0xf1, 0xc7,
   0x30, 0x18, 0x18, 0xc8, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00 };

#include "displays.h"

#define INNER 0
#define OUTER 1

class Stage {
  public:

  // Constructor
  Stage(Revolve& inner, Revolve& outer, Displays& displays, Interface& interface, Adafruit_NeoPixel& ringLeds);

  // Settings updating
  void updateEncRatios();
  void updateKpSettings();

  // Control
  void gotoHome();
  bool checkEstops();
  void deadMansRestart(int restartSpeed);
  void gotoPos(int pos_inner, int pos_outer, int maxSpeed_inner, int maxSpeed_outer, int accel_inner, int accel_outer, int dir_inner, int dir_outer, int revs_inner, int revs_outer);
  void runCurrentCue();
  
  Revolve& _inner;
  Revolve& _outer;
  
  private:

  Displays& _displays;
  Interface& _interface;
  Adafruit_NeoPixel& _ringLeds;
  
};

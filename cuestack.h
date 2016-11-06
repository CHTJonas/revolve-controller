#include <Arduino.h>
#include "constants.h"
#include <EEPROM.h>


typedef struct {
  int pos_i;
  byte speed_i;
  byte acc_i;
  byte dir_i;
  byte revs_i;

  int pos_o;
  byte speed_o;
  byte acc_o;
  byte dir_o;
  byte revs_o;

  float num=0;
  byte auto_follow=0;
  byte en_i=1;
  byte en_o=1;
  byte active=0;
} Cue;

class Cuestack {
  public:

  // Constructor
  Cuestack();

  // Cue initialisation
  void updateDefaultValues();
  void initialiseCue(int number);
  void resetCue(int number);
  void resetCuestack();

  // EEPROM saving and retrieving
  void loadCuestack();
  void saveCuestack();

  // Getters for current cue data
  void getMovements(int (&outputValues)[10]);
  void getNumber(float& outputNumber);
  void getParams(int (&outputParams)[3]);
  void setMovements(int inputValues[10]);
  void setNumber(float inputNumber);
  void setParams(int inputParams[3]);
  int getCueIndex(float number);

  // Cue number validation and sorting
  bool validCueNumber(float number);
  int activeCues();
  void sortCues();
  boolean isEqual(float f1, float f2);

  // Testing function for example cue data
  void loadExampleCues();

  int currentCue=0;
  int totalCues=0;

  // Default cue values
  Cue defaultValues;
  
  // Master array to hold up to 100 written cues
  Cue stack[100];

  private:
};


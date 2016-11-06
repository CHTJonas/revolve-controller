#include "interface.h"

Interface::Interface(Cuestack& cuestack, Encoder& enc_input, Keypad& keypad, Adafruit_NeoPixel& ringLeds, Adafruit_NeoPixel& pauseLeds, Adafruit_NeoPixel& keypadLeds): _cuestack(cuestack), _enc_input(enc_input), _keypad(keypad), _ringLeds(ringLeds), _pauseLeds(pauseLeds), _keypadLeds(keypadLeds){
  
  // Initialise settings from EEPROM
  EEPROM.get(EELED_SETTINGS,ledSettings);

  EEPROM.get(EEDEFAULT_VALUES,defaultValues);
  EEPROM.get(EEDEFAULT_VALUES,currentMovements);
  _cuestack.updateDefaultValues();

  EEPROM.get(EEINNER_ENC_RATIO,encSettings[2]);
  EEPROM.get(EEOUTER_ENC_RATIO,encSettings[3]);

  EEPROM.get(EEKP_SETTINGS,kpSettings);
  
  if(encSettings[2]>0) // -ve ratio for reverse direction
    encSettings[0]=0;
  else 
    encSettings[0]=1;
  if(encSettings[3]>0) // -ve ratio for reverse direction
    encSettings[1]=0;
  else 
    encSettings[1]=1;

  // Remove any negatives
  encSettings[2]=abs(encSettings[2]);
  encSettings[3]=abs(encSettings[3]);

  // Initialise navigation variables
  editing=0;
  menu_pos=0;
  usingKeypad=0;
}

bool Interface::editVars(int mode){
  // Check if keypad not in use and if key has been pressed
  updateKeypad();

  // Keypad input if enabled
  if(usingKeypad){
    char pressedKey = getKey();
    
    if(pressedKey){
      // Reset to zero if # or * pressed
      if(pressedKey=='#' || pressedKey=='*')
        keypadValue=0;
        
      // Otherwise concatenate onto keypadvalue if less than four digits
      else if(String(keypadValue).length()<4)
        keypadValue=(String(keypadValue)+pressedKey).toInt();

      // Update current selected parameter
      switch(mode){
        case MAN:
          currentMovements[menu_pos]=keypadValue;
          break;
        case BRIGHTNESS:
          ledSettings[menu_pos]=keypadValue;
          break;
        case ENCSETTINGS:
          if(menu_pos<2){
            if(keypadValue>0)
              encSettings[menu_pos]=1;
            else
              encSettings[menu_pos]=0;
          }
          else{
            encSettings[menu_pos]=(static_cast<float>(keypadValue)/100);
          }
          break;
        case DEFAULTVALUES:
          defaultValues[menu_pos]=keypadValue;
          break;
        case KPSETTINGS:
          kpSettings[menu_pos]=(static_cast<float>(keypadValue)/1000);
          break;
        case PROGRAM_MOVEMENTS:
          if(cueParams[1]==0){// If inner disabled
            cueMovements[menu_pos+5]=keypadValue;
          }
          else{
            cueMovements[menu_pos]=keypadValue;
          }
          break;
        case PROGRAM_PARAMS:
          cueNumber=static_cast<float>(keypadValue)/10;
          break;
      }
      return true;
    }
    else{
      return false; // Do not update displays
    }
  }

  // Otherwise encoder input
  else {
    // Add change to currently selected parameter
    int change = getInputEnc();
    if(change){
      // Choose which settings to update based on mode
      switch(mode){
        case MAN:
          currentMovements[menu_pos]+=change;
          break;
        case BRIGHTNESS:
          ledSettings[menu_pos]+=change;
          break;
        case ENCSETTINGS:
          if(menu_pos<2){
            if(change>0)
              encSettings[menu_pos]=1;
            else
              encSettings[menu_pos]=0;
          }
          else{
            encSettings[menu_pos]+=(static_cast<float>(change)/100);
          }
          break;
        case DEFAULTVALUES:
          defaultValues[menu_pos]+=change;
          break;
        case KPSETTINGS:
          kpSettings[menu_pos]+=(static_cast<float>(change)/1000);
          break;
        case PROGRAM_MOVEMENTS:
          if(cueParams[1]==0){// If inner disabled
            cueMovements[menu_pos+5]+=change;
          }
          else{
            cueMovements[menu_pos]+=change;
          }
          break;
        case PROGRAM_PARAMS:
          cueNumber+=static_cast<float>(change)/10;
          break;
      }
      return true; // Update displays (has been a change)
    }
    else {
      return false; // No change
    }
  }
}

void Interface::limitVariable(int& variable, int varMin, int varMax){
  if(variable>varMax){
    variable=varMax;
  }
  if(variable<varMin){
    variable=varMin;
  }
}

void Interface::limitVariable(float& variable, float varMin, float varMax){
  if(variable>varMax){
    variable=varMax;
  }
  if(variable<varMin){
    variable=varMin;
  }
}

void Interface::limitVariable(double& variable, double varMin, double varMax){
  if(variable>varMax){
    variable=varMax;
  }
  if(variable<varMin){
    variable=varMin;
  }
}

void Interface::limitMovements(int (&movements)[10]) const
{
  limitVariable(movements[0],0,359);
  limitVariable(movements[1],MINSPEED,100);
  limitVariable(movements[2],1,MAXACCEL);
  limitVariable(movements[3],0,1);
  limitVariable(movements[4],0,50);
  limitVariable(movements[5],0,359);
  limitVariable(movements[6],MINSPEED,100);
  limitVariable(movements[7],1,MAXACCEL);
  limitVariable(movements[8],0,1);
  limitVariable(movements[9],0,50);
}

void Interface::limitLedSettings(){
  for(auto i=0;i<sizeof(ledSettings);i++)
    limitVariable(ledSettings[i],0,255);
}

void Interface::limitEncSettings(){
  limitVariable(encSettings[0],0,1);
  limitVariable(encSettings[1],0,1);
  limitVariable(encSettings[2],0.01,99.99);
  limitVariable(encSettings[3],0.01,99.99);
}

void Interface::limitKpSettings(){
  for(auto i=0;i<6;i++)
    limitVariable(kpSettings[i],0.000,9.999);
}

void Interface::limitCueParams(){
  limitVariable(cueNumber,0.0,99.9);
  limitVariable(cueParams[0],0,1);
  limitVariable(cueParams[1],0,1);
  limitVariable(cueParams[2],0,1);
}

void Interface::loadCurrentCue(){
  _cuestack.getMovements(cueMovements);
  _cuestack.getNumber(cueNumber);
  _cuestack.getParams(cueParams);
}

void Interface::loadCue(int number){
	auto currentCue=_cuestack.currentCue;
  _cuestack.currentCue=number;
  _cuestack.getMovements(cueMovements);
  _cuestack.getNumber(cueNumber);
  _cuestack.getParams(cueParams);
  _cuestack.currentCue=currentCue;
}

bool Interface::updateMenu(int menuMax){
	auto encValue=getInputEnc();
	auto oldMenuPos=menu_pos;
  
  if(encValue>0 && menu_pos<menuMax){
    if((menu_pos+encValue)>menuMax){
      menu_pos=menuMax;
    }
    else{
      menu_pos+=encValue;
    }
  }
  else if(encValue<0 && menu_pos>0){
    if((menu_pos+encValue)<0){
      menu_pos=0;
    }
    else{
      menu_pos+=encValue;
    }
  }
  if(menu_pos!=oldMenuPos){
    return true;
  }
  else{
    return false;
  }
}

void Interface::flashLed(int led, int interval){
  
  int currentState=digitalRead(led);

  if(flashCounter==0)
    flashCounter=millis();

  if(millis()>(flashCounter+interval)){
    digitalWrite(led,!currentState);
    flashCounter=0;
  }
}

void Interface::updatePauseLeds() const
{
  if(digitalRead(PAUSE)==LOW){
    pauseLedsColor(0,255,0);
  }
  else{
    pauseLedsColor(255,0,0);
  }
}

int Interface::getInputEnc() const
{
	auto value = _enc_input.read()/4;
  if(abs(value)>0){
    _enc_input.write(0);
  }
  // Skip acceleration if not editing (i.e. navigate menus at sensible speed)
  if(editing){
    if(abs(value)>4){
      value=value*2;
    }
    if(abs(value)>6){
      value=value*3;
    }
  }
  return -value;
}

void Interface::updateKeypad(){
	auto newKey=_keypad.getKey();
  if(newKey){
    key=newKey; // Holds last pressed key - reset to zero when read
    currentKey=newKey; // Current key being pressed (if any)
  }

  // Enable keypad input
  if(!usingKeypad && key){
    usingKeypad=1;
  }

  // Reset currentKey if key released
  if(_keypad.getState()==HOLD || _keypad.getState()==PRESSED){
    currentKey=key;
  }
  else{
    currentKey=0;
  }
}

void Interface::resetKeypad(){
  key=0;
  currentKey=0;
  usingKeypad=0;
  keypadValue=0;
}

// Returns value of last pressed key, then resets key
char Interface::getKey(){
	auto returnKey = key;
  key=0;
  return returnKey;
}

void Interface::setupSwitches(){
  pinMode(GO,INPUT_PULLUP);
  pinMode(BACK,INPUT);
  pinMode(PAUSE,INPUT_PULLUP);
  pinMode(SELECT,INPUT_PULLUP); // Connects to +5v when pressed

  pinMode(INNERHOME,INPUT_PULLUP);
  pinMode(OUTERHOME,INPUT_PULLUP);

  pinMode(ESTOPNC1,INPUT_PULLUP);
  pinMode(ESTOPNC2,INPUT_PULLUP);
  pinMode(ESTOPNC3,INPUT_PULLUP);
  pinMode(ESTOPNO,INPUT_PULLUP);
  
  // Setup debouncers
  select.attach(SELECT);
  select.interval(10);

  back.attach(BACK);
  back.interval(10);
}


void Interface::setupLeds(){
  
  pinMode(ENCR,OUTPUT);
  pinMode(ENCG,OUTPUT);
  pinMode(ENCB,OUTPUT);
  pinMode(GOLED,OUTPUT);
  pinMode(SELECTLED,OUTPUT);

  // Encoder LEDS are common anode
  digitalWrite(ENCR,HIGH);
  digitalWrite(ENCG,LOW);
  digitalWrite(ENCB,HIGH);

  _ringLeds.begin();
  _pauseLeds.begin();
  _keypadLeds.begin();

  _ringLeds.setBrightness(ledSettings[0]);
  _ringLeds.show();

  _pauseLeds.setBrightness(ledSettings[0]);
  pauseLedsColor(0,0,0);

  _keypadLeds.setBrightness(ledSettings[0]);
  keypadLedsColor(ledSettings[1],ledSettings[2],ledSettings[3]);
  

  // Set initial LED values
  digitalWrite(ENCG,LOW);
  digitalWrite(SELECTLED,HIGH);
}

void Interface::encRed(){
  digitalWrite(ENCR,LOW);
  digitalWrite(ENCG,HIGH);
  digitalWrite(ENCB,HIGH);
}

void Interface::encGreen(){
  digitalWrite(ENCR,HIGH);
  digitalWrite(ENCG,LOW);
  digitalWrite(ENCB,HIGH);
}

void Interface::encBlue(){
  digitalWrite(ENCR,HIGH);
  digitalWrite(ENCG,HIGH);
  digitalWrite(ENCB,LOW);
}

void Interface::encOff(){
  digitalWrite(ENCR,HIGH);
  digitalWrite(ENCG,HIGH);
  digitalWrite(ENCB,HIGH);
}

void Interface::ringLedsColor(int r, int g, int b) const
{
  for(auto i=0;i<24;i++)
    _ringLeds.setPixelColor(i,r,g,b);
  _ringLeds.show();
}

void Interface::pauseLedsColor(int r, int g, int b) const
{
  _pauseLeds.setPixelColor(0,r,g,b);
  _pauseLeds.setPixelColor(1,r,g,b);
  _pauseLeds.show();
}

void Interface::keypadLedsColor(int r, int g, int b) const
{
  for(auto i=0;i<4;i++)
    _keypadLeds.setPixelColor(i,r,g,b);
  _keypadLeds.show();
}

void Interface::waitSelectRelease(){
  while(select.read()==LOW){
    select.update();
  }
}

void Interface::waitBackRelease(){
  while(back.read()){
    back.update();
  }
}

void Interface::allLedsOn() const
{
  digitalWrite(GOLED,HIGH);
  digitalWrite(SELECTLED,HIGH);
  digitalWrite(ENCR,LOW);
  digitalWrite(ENCG,LOW);
  digitalWrite(ENCB,LOW);
  pauseLedsColor(255,255,255);
  keypadLedsColor(255,255,255);
}

void Interface::allLedsOff() const
{
  digitalWrite(GOLED,LOW);
  digitalWrite(SELECTLED,LOW);
  digitalWrite(ENCR,HIGH);
  digitalWrite(ENCG,HIGH);
  digitalWrite(ENCB,HIGH);
  pauseLedsColor(0,0,0);
  keypadLedsColor(0,0,0);
}

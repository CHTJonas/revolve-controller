#include "displays.h"

// Constructor
Displays::Displays(U8GLIB_ST7920_128X64& cue, U8GLIB_ST7920_128X64& menu, U8GLIB_ST7920_128X64& info, Adafruit_NeoPixel& ringLeds, Revolve& inner, Revolve& outer, Keypad& keypad, Interface& interface, Cuestack& cuestack): _cue(cue), _menu(menu), _info(info), _ringLeds(ringLeds), _inner(inner), _outer(outer), _keypad(keypad), _interface(interface), _cuestack(cuestack){
  mode=STARTUP;
}

void Displays::begin(){
  _cue.begin(); 
  _cue.setColorIndex(1);

  _menu.begin(); 
  _menu.setColorIndex(1);
  
  _info.begin(); 
  _info.setColorIndex(1);

  mode=STARTUP;
  _interface.menu_pos=0;
  updateDisplays(1,1,1,1);
}

void Displays::setMode(int newMode){
  // Reset input encoder in case value has accrued
  _interface.getInputEnc();
  mode=newMode;
  forceUpdateDisplays(1,1,1,1);
}

void Displays::drawStrCenter(U8GLIB_ST7920_128X64& lcd, int y, int text){
  const char* string = (const char*)text;
  int width = lcd.getStrWidth(string);
  int x = (128-width)/2;
  lcd.setPrintPos(x,y);
  lcd.print(text);
} 

void Displays::drawStrCenter(U8GLIB_ST7920_128X64& lcd, int y, const char* text){
  int width = lcd.getStrWidth(text);
  int x = (128-width)/2;
  lcd.setPrintPos(x,y);
  lcd.print(text);
} 

void Displays::drawStrCenter(U8GLIB_ST7920_128X64& lcd, int y, char text){
  const char* string = "a";
  int width = lcd.getStrWidth(string);
  int x = (128-width)/2;
  lcd.setPrintPos(x,y);
  lcd.print(text);
} 

void Displays::drawCueLayout(U8GLIB_ST7920_128X64& lcd, int (&values)[10],int cursorEnable){
  lcd.setFont(font);
  
  int menu_pos_shift=_interface.menu_pos;
  if(mode!=MAN &&_interface.cueParams[1]==0){ // Shift menu_pos by 5 if inner disabled
    menu_pos_shift+=5;
  }

  // Only draw if enabled
  if(mode==MAN || (mode!=MAN && _interface.cueParams[1]==1)){
    lcd.drawHLine(0,0,128);
    // Inner label
    lcd.drawBox(0,1,lcd.getStrWidth("INNER")+4,10);
    lcd.setDefaultBackgroundColor();
    lcd.drawStr(2,9,"INNER");
    lcd.setDefaultForegroundColor();
  
    // Position
    lcd.drawStr(lcd.getStrWidth("INNER")+6,9,"Position: ");
    lcd.setPrintPos(lcd.getStrWidth("INNERPosition: ")+4,9);
    
    if(cursorEnable && _interface.menu_pos==0){
      lcd.drawBox(lcd.getStrWidth("INNERPosition: ")+3,1,19,9);
      lcd.setDefaultBackgroundColor();
    } 
      lcd.print(values[0]);
      lcd.setDefaultForegroundColor();
  
    // Speed
    lcd.drawStr(2,19,"Speed: ");
    lcd.setPrintPos(lcd.getStrWidth("Speed: "),19);
    
    if(cursorEnable && _interface.menu_pos==1){
      lcd.drawBox(lcd.getStrWidth("Speed: ")-1,11,19,9);
      lcd.setDefaultBackgroundColor();
    }
    lcd.print(values[1]);
    lcd.setDefaultForegroundColor();
  
    // Acceleration
    lcd.drawStr(70,19,"Acc:  ");
    lcd.setPrintPos(lcd.getStrWidth("Acc:  ")+70,19);
  
    if(cursorEnable && _interface.menu_pos==2){
      lcd.drawBox(lcd.getStrWidth("Acc:  ")+69,11,19,9);
      lcd.setDefaultBackgroundColor();
    }
    lcd.print(values[2]);
    lcd.setDefaultForegroundColor();
  
    // Direction
    lcd.drawStr(2,29,"Dir: ");
    lcd.setPrintPos(lcd.getStrWidth("Speed: "),29);
    
    if(cursorEnable && _interface.menu_pos==3){
      lcd.drawBox(lcd.getStrWidth("Speed: ")-1,21,19,9);
      lcd.setDefaultBackgroundColor();
    }
    if(values[3]){
      lcd.print("ACW");
    }
    else {
      lcd.print("CW");
    }
    lcd.setDefaultForegroundColor();
  
    // Extra Revolutions
    lcd.drawStr(70,29,"Revs: ");
    lcd.setPrintPos(lcd.getStrWidth("Acc:  ")+70,29);
  
    if(cursorEnable && _interface.menu_pos==4){
      lcd.drawBox(lcd.getStrWidth("Acc:  ")+69,21,13,9);
      lcd.setDefaultBackgroundColor();
    }
    lcd.print(values[4]);
    lcd.setDefaultForegroundColor();  
  }

  // Only draw if enabled
  if(mode==MAN || (mode!=MAN && _interface.cueParams[2]==1)){
    // Outer
    lcd.drawHLine(0,32,128);
  
    // Outer label
    lcd.drawBox(0,33,lcd.getStrWidth("OUTER")+4,10);
    lcd.setDefaultBackgroundColor();
    lcd.drawStr(2,41,"OUTER");
    lcd.setDefaultForegroundColor();
  
    // Position
    lcd.drawStr(lcd.getStrWidth("OUTER")+6,41,"Position: ");
    lcd.setPrintPos(lcd.getStrWidth("INNERPosition: ")+4,41);
    
    if(cursorEnable && menu_pos_shift==5){
      lcd.drawBox(lcd.getStrWidth("INNERPosition: ")+3,33,19,9);
      lcd.setDefaultBackgroundColor();
    }
    lcd.print(values[5]);
    lcd.setDefaultForegroundColor();
  
    // Speed
    lcd.drawStr(2,51,"Speed: ");
    lcd.setPrintPos(lcd.getStrWidth("Speed: "),51);
    
    if(cursorEnable && menu_pos_shift==6){
      lcd.drawBox(lcd.getStrWidth("Speed: ")-1,43,19,9);
      lcd.setDefaultBackgroundColor();
    }
    lcd.print(values[6]);
    lcd.setDefaultForegroundColor();
  
    // Acceleration
    lcd.drawStr(70,51,"Acc:  ");
    lcd.setPrintPos(lcd.getStrWidth("Acc:  ")+70,51);
  
    if(cursorEnable && menu_pos_shift==7){
      lcd.drawBox(lcd.getStrWidth("Acc:  ")+69,43,19,9);
      lcd.setDefaultBackgroundColor();
    }
    lcd.print(values[7]);
    lcd.setDefaultForegroundColor();
  
    // Direction
    lcd.drawStr(2,61,"Dir: ");
    lcd.setPrintPos(lcd.getStrWidth("Speed: "),61);
    
    if(cursorEnable && menu_pos_shift==8){
      lcd.drawBox(lcd.getStrWidth("Speed: ")-1,53,19,9);
      lcd.setDefaultBackgroundColor();
    }
    if(values[8]){
      lcd.print("ACW");
    }
    else {
      lcd.print("CW");
    }
    lcd.setDefaultForegroundColor();
  
    // Extra Revolutions
    lcd.drawStr(70,61,"Revs: ");
    lcd.setPrintPos(lcd.getStrWidth("Acc:  ")+70,61);
  
    if(cursorEnable && menu_pos_shift==9){
      lcd.drawBox(lcd.getStrWidth("Acc:  ")+69,53,13,9);
      lcd.setDefaultBackgroundColor();
    }
    lcd.print(values[9]);
    lcd.setDefaultForegroundColor();
  }
}

void Displays::drawParamsLayout(U8GLIB_ST7920_128X64& lcd, int cursorEnable){
  _menu.setFont(font);
  _menu.drawStr(4,10,"Cue Number:");
  _menu.setPrintPos(90,10);
  if(_interface.menu_pos==0 && cursorEnable){
    _menu.drawBox(88,1,27,11);
    _menu.setDefaultBackgroundColor();
  }

  if(_interface.cueNumber-floor(_interface.cueNumber)==0) // Don't display something like 1.0 (but do display 2.4)
    _menu.print(_interface.cueNumber,0);
  else
    _menu.print(_interface.cueNumber,1);
  _menu.setDefaultForegroundColor();

  for(int i=0;i<3;i++){
    _menu.drawStr(4,((i+2)*10),param_strings[i]);
    _menu.setPrintPos(90,((i+2)*10));
    
    if((_interface.menu_pos-1)==i && cursorEnable){
      _menu.drawBox(88,1+(i+1)*10,21,11);
      _menu.setDefaultBackgroundColor();
    }
    
    if(_interface.cueParams[i]==0)
      _menu.print("NO");
    else
      _menu.print("YES");
    _menu.setDefaultForegroundColor();
  }
  
  if(_interface.menu_pos==4 && cursorEnable){
    _menu.drawBox(0,41,128,11);
    _menu.setDefaultBackgroundColor();
  }
  drawStrCenter(_menu,50,"Add Cue");
  _menu.setDefaultForegroundColor();

  if(_interface.menu_pos==5 && cursorEnable){
    _menu.drawBox(0,51,128,11);
    _menu.setDefaultBackgroundColor();
  }
  drawStrCenter(_menu,60,"Delete Cue");
  _menu.setDefaultForegroundColor();
}

void Displays::drawCuelistLayout(U8GLIB_ST7920_128X64& lcd, int index, int cursorEnable){
  // Set font and row height
  lcd.setFont(small_font);

  // Work out pagination
  int cueListPage=index/7;

  // Header highlight
  lcd.drawBox(0,0,128,8);
  lcd.setDefaultBackgroundColor();
  
  // Draw header row
  lcd.drawStr(5,7,"#");
  lcd.drawStr(30,7,"Inn.");
  lcd.drawStr(55,7,"R");
  lcd.drawStr(75,7,"Out.");
  lcd.drawStr(100,7,"R");
  lcd.drawStr(115,7,"AF");
  lcd.setDefaultForegroundColor();

  // Draw correct page of cues
  for(int i=(cueListPage*7);i<(cueListPage+1)*7;i++){
    if(_cuestack.stack[i].active){
      int iPos=(i-(cueListPage*7)); // Offset page number from i used to position elements on page
      
      // Draw currentCue box
      if(_cuestack.currentCue==i){
        lcd.setDefaultForegroundColor();
        lcd.drawFrame(0,(iPos+1)*8,128,8);
      }
      
      // Draw highlight box if cue selected
      if(index==i && cursorEnable){
        lcd.drawBox(0,(iPos+1)*8,128,8);
        lcd.setDefaultBackgroundColor();
      }
      
      // Number - remove all .0
      lcd.setPrintPos(2,(8*(iPos+2))-1);
      if(_cuestack.stack[i].num-floor(_cuestack.stack[i].num)==0)
        lcd.print(_cuestack.stack[i].num,0);
      else
        lcd.print(_cuestack.stack[i].num,1);

      if(_cuestack.stack[i].en_i){
        lcd.setPrintPos(30,(8*(iPos+2))-1);
        lcd.print(_cuestack.stack[i].pos_i);
        if(_cuestack.stack[i].revs_i){
          lcd.setPrintPos(55,(8*(iPos+2))-1);
          lcd.print(_cuestack.stack[i].revs_i);
        }
      }

      if(_cuestack.stack[i].en_o){
        lcd.setPrintPos(75,(8*(iPos+2))-1);
        lcd.print(_cuestack.stack[i].pos_o);
        if(_cuestack.stack[i].revs_o){
          lcd.setPrintPos(100,(8*(iPos+2))-1);
          lcd.print(_cuestack.stack[i].revs_o);
        }
      }
  
  
      // Auto follow arrow
      if(_cuestack.stack[i].auto_follow){
        lcd.setFont(u8g_font_m2icon_7);
        lcd.drawStr180(120,(8*(iPos+1)),"\x62");
        lcd.setFont(small_font);
      }
  
      lcd.setDefaultForegroundColor();
    }
  }
  lcd.setFont(font);
}

void Displays::drawCue(){
  _cue.setFont(font);

  switch (mode){
    case STARTUP:
    case HOMING:
      _cue.setFont(large_font);
      drawStrCenter(_cue,15,"Panto Revolve");
      drawStrCenter(_cue,30,"Controller");
      _cue.setFont(font);
      drawStrCenter(_cue,45,"Designed by Jack");
      _cue.setFont(small_font);
      drawStrCenter(_cue,55,"August 2016");
      break;

    case SHOW:
      drawCueLayout(_cue,_interface.cueMovements,0);
      break;

    case PROGRAM:
    case PROGRAM_PARAMS:
    case PROGRAM_CUELIST:
    case PROGRAM_DELETE:
    case PROGRAM_GOTOCUE:
    case PROGRAM_SAVED:
      // Add box if screen selected in PROGRAM
      if(_interface.menu_pos==0 && mode==PROGRAM){
        _cue.setDefaultForegroundColor();
        _cue.drawFrame(0,0,128,64);
        _cue.drawFrame(1,1,126,62);
        _cue.setDefaultForegroundColor();
      }
      drawCueLayout(_cue,_interface.cueMovements,0);
      break;
      
    case PROGRAM_MOVEMENTS:
      drawCueLayout(_cue,_interface.cueMovements,1);
      break;
      
    case ESTOP:
      _cue.setFont(xlarge_font);
      drawStrCenter(_cue,16,"ESTOP");
      _cue.setFont(large_font);
      drawStrCenter(_cue,32,"Reset all Estops");
      drawStrCenter(_cue,48,"to continue");
      break;
      
    default:
      _menu.drawXBMP(0,0,128,64,screen_logo);
      break;
  }
}

void Displays::drawMenu(){
  _menu.setFont(font);

  switch (mode){
    case STARTUP:
    case HOMING:
      _menu.drawXBMP(0,0,128,64,screen_logo);
      break;
    case NORMAL:
      _menu.setFont(large_font);

      for(int i=0;i<4;i++){
        if(i==_interface.menu_pos){
          // Position highlight box from top left corner
          _menu.drawBox(0,(_interface.menu_pos*16),128,16);
          _menu.setDefaultBackgroundColor();
          drawStrCenter(_menu,(i*16)+12,menu_strings[i]);
        }
        drawStrCenter(_menu,(i*16)+12,menu_strings[i]);
        _menu.setDefaultForegroundColor();
      }
      break;
      
    case MAN:
      drawCueLayout(_menu,_interface.currentMovements,1);
      break;

    case SHOW:
      drawCuelistLayout(_menu,_interface.menu_pos,1);
      break;

    case PROGRAM:
    case PROGRAM_MOVEMENTS:
    case PROGRAM_CUELIST:
    case PROGRAM_GOTOCUE:
      // Add box if screen selected in PROGRAM
      if(_interface.menu_pos==1 && mode==PROGRAM){
        _menu.setDefaultForegroundColor();
        _menu.drawFrame(0,0,128,64);
        _menu.drawFrame(1,1,126,62);
        _menu.setDefaultForegroundColor();
      }
      drawParamsLayout(_menu,0);
      break;
      
    case PROGRAM_DELETE:
      _menu.setFont(large_font);
      drawStrCenter(_menu,35,"Delete Cue?");
      _menu.setFont(font);
      break;

    case PROGRAM_SAVED:
      _menu.setFont(large_font);
      drawStrCenter(_menu,35,"Cuestack Saved");
      _menu.setFont(font);
      break;
      
    case PROGRAM_PARAMS:
      drawParamsLayout(_menu,1);
      break;
    
    case SETTINGS:
      _menu.setFont(large_font);
      if(_interface.menu_pos<4){
        for(int i=0;i<4;i++){
          if(i==_interface.menu_pos){
            // Position highlight box from top left corner
            _menu.drawBox(0,(_interface.menu_pos*16),128,16);
            _menu.setDefaultBackgroundColor();
            drawStrCenter(_menu,(i*16)+12,settings_strings[i]);
          }
          drawStrCenter(_menu,(i*16)+12,settings_strings[i]);
          _menu.setDefaultForegroundColor();
        }
      }
      else if(_interface.menu_pos<8){
        for(int i=0;i<4;i++){
          if((i+4)==_interface.menu_pos){
            // Position highlight box from top left corner
            _menu.drawBox(0,(i*16),128,16);
            _menu.setDefaultBackgroundColor();
            drawStrCenter(_menu,(i*16)+12,settings_strings[i+4]);
          }        
          drawStrCenter(_menu,(i*16)+12,settings_strings[i+4]);
          _menu.setDefaultForegroundColor();
        }
      }
      
      break;

    case HARDWARETEST:
      _menu.setFont(large_font);
      drawStrCenter(_menu,16,"Hardware Test");
      _menu.setFont(font);
      drawStrCenter(_menu,40,"Hold Go and");
      drawStrCenter(_menu,48,"Pause to exit");
      break;

    case BRIGHTNESS:
      _menu.setFont(large_font);

      for(int i=0;i<4;i++){
        _menu.drawStr(0,13+(i*16),led_settings_strings[i]);
        _menu.setPrintPos(90,13+(i*16));
        
        if(_interface.menu_pos==i){
          _menu.drawBox(88,i*16,24,16);
          _menu.setDefaultBackgroundColor();
        }

        _menu.print(_interface.ledSettings[i]);
        _menu.setDefaultForegroundColor();
      }
      _menu.setFont(font);
      break;

    case ENCSETTINGS:
      _menu.setFont(large_font);

      for(int i=0;i<4;i++){
        _menu.drawStr(0,13+(i*16),enc_settings_strings[i]);
        _menu.setPrintPos(90,13+(i*16));
        
        if(_interface.menu_pos==i){
          _menu.drawBox(88,i*16,40,16);
          _menu.setDefaultBackgroundColor(); 
        }
        
        if(_interface.encSettings[i]==0 && (i==0 || i==1))
          _menu.print("FWD");
        else if(_interface.encSettings[i]==1 && (i==0 || i==1))
          _menu.print("REV");
        else
          _menu.print(_interface.encSettings[i]);
        _menu.setDefaultForegroundColor();
      }
      _menu.setFont(font);
      break;

    case ESTOP:
      _menu.setFont(xlarge_font);
      drawStrCenter(_menu,16,"ESTOP");
      _menu.setFont(large_font);
      drawStrCenter(_menu,32,"Reset all Estops");
      drawStrCenter(_menu,48,"to continue");
      break;

    case DEFAULTVALUES:
      drawCueLayout(_menu,_interface.defaultValues,0);
      break;

    case KPSETTINGS:
      _menu.setFont(font);

      for(int i=0;i<6;i++){
        _menu.drawStr(0,10+(i*10),kp_settings_strings[i]);
        _menu.setPrintPos(90,10+(i*10));
        
        if(_interface.menu_pos==i){
          _menu.drawBox(88,(i*10)+2,34,10);
          _menu.setDefaultBackgroundColor();
        }
          
        _menu.print(_interface.kpSettings[i],3);
        _menu.setDefaultForegroundColor();
      }
      _menu.setFont(font);
      break;

    case RESET_CUESTACK:
      _menu.setFont(large_font);
      drawStrCenter(_menu,19,"Reset");
      drawStrCenter(_menu,35,"ENTIRE CUESTACK?");
      _menu.setFont(font);
      drawStrCenter(_menu,50,"Press Go, Pause and");
      drawStrCenter(_menu,60,"Select to Continue");
      break;
    default:
      break;
  }
}

void Displays::drawInfo(){
  _info.setFont(font);

  switch (mode){
    case STARTUP:
      _info.setFont(large_font);
      drawStrCenter(_info,20,"READY TO HOME");
      _info.setFont(font);
      drawStrCenter(_info,40,"ENSURE REVOLVE CLEAR");
      drawStrCenter(_info,50,"Press GO to home");
      break;
    case HOMING:
      _info.setFont(xlarge_font);
      drawStrCenter(_info,20,"HOMING");
      _info.setFont(font);
      drawStrCenter(_info,40,"Please wait");
      break;
    case HARDWARETEST:
      _info.setFont(xlarge_font);
      
      if(digitalRead(GO)==LOW){
        drawStrCenter(_info,40,"GO");
      }
      else if(digitalRead(PAUSE)==LOW){
        drawStrCenter(_info,40,"PAUSE");
      }
      else if(digitalRead(BACK)==HIGH){
        drawStrCenter(_info,40,"BACK");
      }
      else if(digitalRead(SELECT)==LOW){
        drawStrCenter(_info,40,"SELECT");
      }
      else if(_interface.currentKey){
        drawStrCenter(_info,40,_interface.currentKey);
      }
      _info.setFont(font);
      break;

    case ESTOP:
      _info.setFont(xlarge_font);
      drawStrCenter(_info,16,"ESTOP");
      _info.setFont(large_font);
      drawStrCenter(_info,32,"Reset all Estops");
      drawStrCenter(_info,48,"to continue");
      break;
      
    case MAN:
    case SHOW:
    case PROGRAM_GOTOCUE:
      _info.setFont(large_font);
      _info.drawStr(0,10,"Inner");
      _info.setFont(font);
      _info.drawStr(0,20,"Position: ");
      _info.setPrintPos(_info.getStrWidth("Position: "),20);
      _info.print(_inner.displayPos());
      _info.drawStr(0,28,"Speed: ");
      _info.setPrintPos(_info.getStrWidth("Speed: "),28);
      _info.print(_inner.getSpeed());
      
      _info.setFont(large_font);
      _info.drawStr(0,42,"Outer");
      _info.setFont(font);
      _info.drawStr(0,52,"Position: ");
      _info.setPrintPos(_info.getStrWidth("Position: "),52);
      _info.print(_outer.displayPos());
      _info.drawStr(0,60,"Speed: ");
      _info.setPrintPos(_info.getStrWidth("Speed: "),60);
      _info.print(_outer.getSpeed());
      break;

    case PROGRAM:
    case PROGRAM_MOVEMENTS:
    case PROGRAM_PARAMS:
    case PROGRAM_DELETE:
    case PROGRAM_SAVED:
      // Add box if screen selected in PROGRAM
      if(_interface.menu_pos==2 && mode==PROGRAM){
        _info.setDefaultForegroundColor();
        _info.drawFrame(0,0,128,64);
        _info.drawFrame(1,1,126,62);
        _info.setDefaultForegroundColor();
      }
      drawCuelistLayout(_info,_cuestack.currentCue,1);
      break;
      
    case PROGRAM_CUELIST:
      drawCuelistLayout(_info,_interface.menu_pos,1);
      break;
      
    default:
      _info.drawXBMP(0,0,128,64,screen_logo);
      break;
  }
}

void Displays::updateRingLeds(){

  switch (mode){
    case STARTUP:
    case HOMING:
    case RESET_CUESTACK:
      for(int i=0;i<24;i++){
        _ringLeds.setPixelColor(i,255,0,0);
      }
      _ringLeds.show();
      break;
      
    case HARDWARETEST:
      for(int i=0;i<24;i++){
        _ringLeds.setPixelColor(i,255,255,255);
      }
      _ringLeds.show();
      break;
    case HOMED:
    case PROGRAM_SAVED:
      for(int i=0;i<24;i++){
        _ringLeds.setPixelColor(i,0,255,0);
      }
      _ringLeds.show();
      break;
      
    case ESTOP:
      for(int i=0;i<24;i++){
        _ringLeds.setPixelColor(i,255,0,0);
      }
      _ringLeds.show();
      break;
      
    case MAN:
    case SHOW:
    case PROGRAM_GOTOCUE:
      ledOuter=_outer.displayPos();
      ledInner=_inner.displayPos();
    
      // Get position in terms of 12ths of a circle
      ledOuter=(ledOuter+15)/30;
      ledInner=(ledInner+15)/30;
    
      // Flip as LEDS in reverse order
      ledOuter=12-ledOuter;
      ledInner=12-ledInner;
    
      // Add 9 then % 12 as LEDS offset by -3
      ledOuter=(ledOuter+9)%12;
      ledInner=(ledInner+9)%12;
    
      // Inner is 12 pixels further on
      ledInner+=12;
    
      // Blank first
      for(int i=0;i<24;i++){
        _ringLeds.setPixelColor(i,50,0,0);
      }
    
      // Set green LEDS
      _ringLeds.setPixelColor(ledOuter,0,255,0);
      _ringLeds.setPixelColor(ledInner,0,255,0);

      if(ledOuter==0){
        _ringLeds.setPixelColor(11,100,75,0);
        _ringLeds.setPixelColor(1,100,75,0);
      }
      else if(ledOuter==11){
        _ringLeds.setPixelColor(0,100,75,0);
        _ringLeds.setPixelColor(10,100,75,0);
      }
      else{
        _ringLeds.setPixelColor(ledOuter-1,100,75,0);
        _ringLeds.setPixelColor(ledOuter+1,100,75,0);
      }

      if(ledInner==12){
        _ringLeds.setPixelColor(13,100,75,0);
        _ringLeds.setPixelColor(23,100,75,0);
      }
      else if(ledInner==23){
        _ringLeds.setPixelColor(12,100,75,0);
        _ringLeds.setPixelColor(22,100,75,0);
      }
      else{
        _ringLeds.setPixelColor(ledInner-1,100,75,0);
        _ringLeds.setPixelColor(ledInner+1,100,75,0);
      }
      
      _ringLeds.show();
      break;

    default:
      _interface.ringLedsColor(200,0,255);
      break;
  }
}

void Displays::updateDisplays(int cue, int menu, int info, int ringLeds){
  if(_update){
    if(cue){
      _cue.firstPage();
      do {
      drawCue();
      } while(_cue.nextPage());
    }

    if(menu){
      _menu.firstPage();
      do {
      drawMenu();
      } while(_menu.nextPage());
    }

    if(info){
      _info.firstPage();
      do {
      drawInfo();
      } while(_info.nextPage());
    }

    if(ringLeds){
      updateRingLeds();
    }
    
    _update=0;
  }
}

void Displays::forceUpdateDisplays(int cue1, int menu, int info, int ringLeds){
  _update++;
  updateDisplays(cue1,menu,info,ringLeds);
}



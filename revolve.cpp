#include "revolve.h"

// Constructor
  Revolve::Revolve(int start_pin, int dir_pin, int speed_pin, Encoder& enc): _enc(enc){

  // Set physical pins
	_start_pin=start_pin;
	_dir_pin=dir_pin;
	_speed_pin=speed_pin;

  // Speed and acceleration parameters
  _cur_speed=0;         // Updated with current speed whenever speed is changed
  _em_stop_time=500;    // EM stop time as setup on motor controller
  _enc_ratio=1;         // Encoder degrees per revolve degrees (updated from from eeprom by stage)
  _debug=0;             // Debug flag to serial print debug information
  _tenths=0;            // Incremented every 1/10 second by main loop to limit acceleration

  // PID control variables
  _kp_0=0;        // Initial kp for 100 speed and 0 acceleration
  _kp_smin=0;     // Difference between min and max kp between speed 0 and 100 for fixed acceleration
  _kp_amax=0;     // Difference between min and max kp between 0 acceleration and MAXACCEL for fixed speed
  _ki=0;          // Integral control coefficient (usually not needed)
  _kd=0;          // Differential control coefficient (usually not needed)
  
  // Setup output pins
	pinMode(_start_pin,OUTPUT);
	pinMode(_dir_pin,OUTPUT);
  pinMode(_speed_pin,OUTPUT);
}

// INTERNAL HELPER FUNCTIONS

// Enables motor control
void Revolve::start() const
{
	digitalWrite(_start_pin,HIGH);
}

// Stops motor - usually to stop energisation at 0Hz. Will cause EM stop if activated at speed.
void Revolve::stop() const
{
	digitalWrite(_start_pin,LOW);
  waitForStop(); // wait for full stop if stopping from high speed (uses _em_stop_time)
}

// Waits for motor to fully stop using _em_stop_time
void Revolve::waitForStop() const
{
	auto startTime=millis(); // Don't use delay so encoders catch final position
  while(millis()<(startTime+_em_stop_time)){
    // Break out if pause button released (revolve won't come to full stop if button briefly released)
    if(digitalRead(PAUSE)==LOW){
      break;
    }
  }
}

// GETTERS AND SETTERS

// Getter for _cur_speed
int Revolve::getSpeed() const
{
  return _cur_speed;
}

// Getter for _dir
int Revolve::getDir() const
{
  return _dir;
}

// Getter for current absolute position from encoder - updates _cur_pos
long Revolve::getPos(){
  long pos = (_enc.read()/4L); // Encoder has 4 steps per degree
  pos = pos/_enc_ratio;             // Divide by encoder degrees per revolve degree
  _cur_pos=pos;
  return pos;
}

// Set speed - will accelerate according to limits set on controller (i.e. max acceleration)
void Revolve::setSpeed(float speed){

  // Nothing below MINSPEED
  if(speed<MINSPEED && speed!=0){
    speed=MINSPEED;
  }
  
  // Convert from % to duty cycle
	auto req_duty_cycle = static_cast<int>(speed * 255)/100;

  // Limit max
  if(req_duty_cycle>255)
    req_duty_cycle=255;

  // No negatives
  if(req_duty_cycle<0)
    req_duty_cycle=0;

  // Start if at standstill
  if(_cur_speed==0){
    start();
  }

  // Set speed, update _cur_speed
  analogWrite(_speed_pin,req_duty_cycle);
  _cur_speed=speed;

  // Stop if speed has been set to 0
  if(_cur_speed==0){
    stop();
  }
}

// Sets direction pin (will have no effect at speed until speed taken to 0)
void Revolve::setDir(int dir){
  if(dir==BACKWARDS){
    digitalWrite(_dir_pin,HIGH);
    _dir=BACKWARDS; // Update direction variable
  }
  else {
    digitalWrite(_dir_pin,LOW);
   _dir=FORWARDS; // Update direction variable
  }
}

// Returns relative position between 0 and 360 degrees
int Revolve::displayPos(){
	auto pos=getPos();
  int realPos=pos%360;

  // No negatives
  if(realPos<0){
    realPos+=360;
  }

  return realPos;
}

// Reset position to 0
void Revolve::resetPos() const
{
  _enc.write(0);
}

// Set debug flag to allow serial printing by functions
void Revolve::setDebug(int debug){
  if(debug==1){
    _debug=1;
  }
  else{
    debug=0;
  }
}

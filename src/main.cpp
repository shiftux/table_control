/*********************************
 * Electric table height controller
 * Author: shiftux
 * Jan 2021
 ********************************/

#include <Arduino.h>

bool debug = true;

// pin names
// pinout https://components101.com/microcontrollers/arduino-nano
#define UP_HALL_SENSOR 14
#define LOW_HALL_SENSOR 15
#define END_SWITCH 16
#define SWITCH_UP 5
#define SWITCH_DOWN 6
#define BUTTON_UP 7
#define BUTTON_DOWN 8
#define RELAIS_4 9
#define RELAIS_3 10
#define RELAIS_2 11
#define RELAIS_1 12
#define OFF 0
#define ON 1

// states
enum states {
  STOP,
  MOVING_UP,
  MOVING_DOWN,
  SHIFT_UP,
  SHIFT_DOWN
};

enum switchPositions {
  SWITCH_POS_HIGH,
  SWITCH_POS_LOW
};

// global vars
states state = STOP;
switchPositions switchPosition  = SWITCH_POS_HIGH;
int switchState = 3;
long movmentStartTime;
int upMovementTimeout = 16000; // ms
int downMovementTimeout = 13500; // ms

// switch & button state getters
int getSwitchPos(){
  if(digitalRead(SWITCH_DOWN) == 1){
    switchPosition = SWITCH_POS_LOW;
  }
  if(digitalRead(SWITCH_UP) == 1){
    switchPosition = SWITCH_POS_HIGH;
  }
  if(debug) { Serial.print("Got switch state: ");Serial.println(switchPosition); }
  return switchPosition;
};
bool switchToggleUp(){
  if(getSwitchPos() == SWITCH_POS_HIGH && switchState != SWITCH_POS_HIGH){
    switchState = SWITCH_POS_HIGH;
    return true;
  } else {
    return false;
  }
};
bool switchToggleDown(){
  if(getSwitchPos() == SWITCH_POS_LOW && switchState != SWITCH_POS_LOW){
    switchState = SWITCH_POS_LOW;
    return true;
  } else {
    return false;
  }
};
bool buttonDownActive(){
  return !digitalRead(BUTTON_DOWN);
}
bool buttonUpActive(){
  return !digitalRead(BUTTON_UP);
}

// motor movements
void motorStop(){
  digitalWrite(RELAIS_1, OFF);
  digitalWrite(RELAIS_2, OFF);
  digitalWrite(RELAIS_3, OFF);
  digitalWrite(RELAIS_4, OFF);
};
void motorMoveUp(){
  motorStop();
  digitalWrite(RELAIS_1, ON);
  digitalWrite(RELAIS_3, ON);
};
void motorMoveDown(){
  motorStop();
  digitalWrite(RELAIS_2, ON);
  digitalWrite(RELAIS_4, ON);
};
void stop(){
  motorStop();
  if(debug) { Serial.println("Setting state: STOP"); }
  state = STOP;
};

// limit checks
bool hitLowerLimit(){
  if (digitalRead(LOW_HALL_SENSOR) == 0){ // hall sensor is 0 when magnet is present
    if(debug) { Serial.println("Hit lower hall sensor"); }
    return true;
  }
  else if (digitalRead(END_SWITCH) == 1){
    if(debug) { Serial.println("Hit lower lower end switch"); }
    return true;
  }
  else {
    return false;
  }
};
bool hitUpperLimit(){
  if (digitalRead(UP_HALL_SENSOR) == 0){
    if(debug) { Serial.println("Hit upper hall sensor"); }
    return true;
  }
  else {
    return false;
  }
};
bool downTimeoutHit(){
  if (millis() - movmentStartTime > downMovementTimeout){
    if(debug) { Serial.println("Hit down time limit timeout"); }
    return true;
  } else {
    return false;
  }
};
bool upTimeoutHit(){
  if (millis() - movmentStartTime > upMovementTimeout){
    if(debug) { Serial.println("Hit up time limit timeout"); }
    return true;
  } else {
    return false;
  }
};

// state actions
void stopActions(){
  motorStop();
  if (switchToggleDown()){
    if(debug) { Serial.println("Setting state: SHIFT_DOWN"); }
    state = SHIFT_DOWN;
  }
  else if (switchToggleUp()){
    if(debug) { Serial.println("Setting state: SHIFT_UP"); }
    state = SHIFT_UP;
  }
  else if (buttonDownActive()){
    if(debug) { Serial.println("Setting state: MOVING_DOWN"); }
    state = MOVING_DOWN;
  }
  else if (buttonUpActive()){
    if(debug) { Serial.println("Setting state: MOVING_UP"); }
    state = MOVING_UP;
  }
  else {
    if(debug) { Serial.println("Setting state: STOP"); }
    state = STOP;
  }
  movmentStartTime = millis();
};

void movingUpActions(){
  if (hitUpperLimit()){
    if(debug) { Serial.println("not moving up: found upper limit"); }
    stop();
  }
  else if (!buttonUpActive()){
    if(debug) { Serial.println("not moving up: button released"); }
    stop();
  }
  else {
    if(debug) { Serial.println("moving up"); }
    motorMoveUp();
  }
};

void movingDownActions(){
  if (hitLowerLimit()){
    if(debug) { Serial.println("not moving down: found lower limit"); }
    stop();
  }
  else if (!buttonDownActive()){
    if(debug) { Serial.println("not moving down: button released"); }
    stop();
  }
  else {
    if(debug) { Serial.println("moving down"); }
    motorMoveDown();
  }
};

void shiftUpActions(){
  if (hitUpperLimit()){
    stop();
  }
  else if (upTimeoutHit()){
    stop();
  }
  else if (switchToggleDown()){
    motorStop();
    state = SHIFT_DOWN;
  }
  else if (buttonDownActive()){
    motorStop();
    state = MOVING_DOWN;
  }
  else{
    motorMoveUp();
  }
};

void shiftDownActions(){
  if (hitLowerLimit()){
    stop();
  }
  else if (downTimeoutHit()){
    stop();
  }
  else if (switchToggleUp()){
    motorStop();
    state = SHIFT_UP;
  }
  else if (buttonUpActive()){
    motorStop();
    state = MOVING_UP;
  }
  else{
    motorMoveDown();
  }
};

void my_init(){
  pinMode(UP_HALL_SENSOR, INPUT);
  pinMode(LOW_HALL_SENSOR, INPUT);
  pinMode(END_SWITCH, INPUT);
  pinMode(SWITCH_UP, INPUT);
  pinMode(SWITCH_DOWN, INPUT);
  pinMode(BUTTON_UP, INPUT);
  pinMode(BUTTON_DOWN, INPUT);
  pinMode(RELAIS_1, OUTPUT);
  pinMode(RELAIS_2, OUTPUT);
  pinMode(RELAIS_3, OUTPUT);
  pinMode(RELAIS_4, OUTPUT);
}

void setup() {
  my_init();
  if(debug) { Serial.begin(9600); }// open the serial port at 9600 bps
  switchState = getSwitchPos();
  movmentStartTime = millis();
  stop();
}

// main loop
void loop() {
  if (state == STOP) {
    stopActions();
  }
  else if (state == MOVING_UP) {
    movingUpActions();
  }
  else if (state == MOVING_DOWN) {
    movingDownActions();
  }
  else if (state == SHIFT_UP) {
    shiftUpActions();
  }
  else if (state == SHIFT_DOWN) {
    shiftDownActions();
  }
  else{
    stop();
  }
}

/*********************************
 * Electric table height controller
 * Author: shiftux
 * Jan 2021
 ********************************/

#include <Arduino.h>

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
  SWITCH_POS_LOW,
  SWITCH_POS_NEUTRAL
};

// global vars
states state = STOP;
switchPositions switchPosition  = SWITCH_POS_HIGH;
int switchState = 3;
long movmentStartTime;
int upMovementTimeout = 16000; // ms
int downMovementTimeout = 12000; // ms

// switch & button state getters
int getSwitchPos(){
  if(digitalRead(SWITCH_DOWN) == 1){
    switchPosition = SWITCH_POS_LOW;
  }
  else if(digitalRead(SWITCH_UP) == 1){
    switchPosition = SWITCH_POS_HIGH;
  }
  else{
    switchPosition = SWITCH_POS_NEUTRAL;
  }
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
  state = STOP;
};

// limit checks
bool hitLowerLimit(){
  if (digitalRead(LOW_HALL_SENSOR) == 0){ // hall sensor is 0 when magnet is present
    return true;
  }
  else if (digitalRead(END_SWITCH) == 1){
    return true;
  }
  else {
    return false;
  }
};
bool hitUpperLimit(){
  if (digitalRead(UP_HALL_SENSOR) == 0){
    return true;
  }
  else {
    return false;
  }
};
bool downTimeoutHit(){
  if (millis() - movmentStartTime > downMovementTimeout){
    return true;
  } else {
    return false;
  }
};
bool upTimeoutHit(){
  if (millis() - movmentStartTime > upMovementTimeout){
    return true;
  } else {
    return false;
  }
};

// state actions
void stopActions(){
  motorStop();
  if (switchToggleDown()){
    state = SHIFT_DOWN;
  }
  else if (switchToggleUp()){
    state = SHIFT_UP;
  }
  else if (buttonDownActive()){
    state = MOVING_DOWN;
  }
  else if (buttonUpActive()){
    state = MOVING_UP;
  }
  else {
    state = STOP;
  }
  movmentStartTime = millis();
};

void movingUpActions(){
  if (hitUpperLimit()){
    stop();
  }
  else if (!buttonUpActive()){
    stop();
  }
  else {
    motorMoveUp();
  }
};

void movingDownActions(){
  if (hitLowerLimit()){
    stop();
  }
  else if (!buttonDownActive()){
    stop();
  }
  else {
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
    stop();
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
    stop();
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
  // Serial.begin(9600); // open the serial port at 9600 bps
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

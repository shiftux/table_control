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
#define SWITCH_UP 10
#define SWITCH_DOWN 9
#define BUTTON_UP 8
#define BUTTON_DOWN 7
#define RELAIS_DOWN 11
#define RELAIS_UP 12
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
int movementTimeout = 3000; // ms

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
  return digitalRead(BUTTON_DOWN);
}
bool buttonUpActive(){
  return digitalRead(BUTTON_UP);
}

// motor movements
void motorMoveUp(){
  digitalWrite(RELAIS_UP, HIGH);
};
void motorMoveDown(){
  digitalWrite(RELAIS_DOWN, HIGH);
};
void motorStop(){
  digitalWrite(RELAIS_DOWN, LOW);
  digitalWrite(RELAIS_UP, LOW);
};
void stop(){
  motorStop();
  state = STOP;
};

// limit checks
bool hitLowerLimit(){
  if (digitalRead(LOW_HALL_SENSOR) == 1){
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
  if (digitalRead(UP_HALL_SENSOR) == 1){
    return true;
  }
  else {
    return false;
  }
};
bool timeoutHit(){
  if (millis() - movmentStartTime > movementTimeout){
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
  else if (timeoutHit()){
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
  else if (timeoutHit()){
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
  pinMode(RELAIS_DOWN, OUTPUT);
  pinMode(RELAIS_UP, OUTPUT);
}

void setup() {
  my_init();
  Serial.begin(9600); // open the serial port at 9600 bps
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
    digitalWrite(RELAIS_DOWN, OFF);
    digitalWrite(RELAIS_UP, OFF);
  }
  // Serial.print("State: ");
  // Serial.println(state);
  // Serial.println(digitalRead(END_SWITCH));
  // Serial.println(digitalRead(SWITCH_DOWN));
  // Serial.print("pos : ");
  // Serial.println(getSwitchPos());
  // Serial.print("state : ");
  // Serial.println(switchState);
  // if (switchToggleUp()){Serial.println("TOGGLED UP");};
  // if (switchToggleDown()){Serial.println("TOGGLED DOWN");};
  // delay(1000);
}

#include <Arduino.h>

#define GPI_START_PIN 2
#define GPI_STOP_PIN 3
#define GPO_LED_ON_PIN 4


enum State {
  IDLE,
  START_DEBOUNCE,
  START,
  STARTED,
  STOP_DEBOUNCE,
  STOPPED
};

enum Event {
  NULL_EVENT,
  START_EVENT,
  DEFAULT_EVENT
};

State currentState;
Event currentEvent;

unsigned long startTime;
unsigned long stopTime;
unsigned long elapsedTime;

void startTimer() {
  if(currentState == STARTED) {
    return;
  }
  startTime = millis();
  currentState = START_DEBOUNCE;
}

void stopTimer() {
  if(currentState == IDLE) {
    return;
  }
  stopTime = millis();
  currentState = STOP_DEBOUNCE;
}

void setup() {
  Serial.begin(115200);

  attachInterrupt(digitalPinToInterrupt(GPI_START_PIN), startTimer, RISING);
  attachInterrupt(digitalPinToInterrupt(GPI_STOP_PIN), stopTimer, RISING);

  pinMode(GPI_START_PIN, INPUT);
  pinMode(GPI_STOP_PIN, INPUT);
  pinMode(GPO_LED_ON_PIN, OUTPUT);

  currentState = IDLE;
  currentEvent = NULL_EVENT;

  startTime = 0;
  stopTime = 0;
}

void loop() {
  // Choose the next state
  switch (currentState)
  {
  case IDLE:

    break;

  case START_DEBOUNCE:
    if(millis() - startTime > 100) {
      currentState = START;
    }
    break;

  case START:
    if(currentEvent == DEFAULT_EVENT){
      currentState = STARTED;
    }
    break;
    
  case STARTED:
    break;

  case STOP_DEBOUNCE:
    if(millis() - stopTime > 100) {
      currentState = STOPPED;
    }
    break;

  case STOPPED:
    if(currentEvent == DEFAULT_EVENT)
    {
      currentState = IDLE;
    }
    break;

  default:
    break;
  }

  // Execute the current state
  switch (currentState)
  {
  case IDLE:
      currentEvent = NULL_EVENT;
    break;

  case START_DEBOUNCE:
      currentEvent = NULL_EVENT;
    break;

  case START:
    digitalWrite(GPO_LED_ON_PIN, HIGH);
    currentEvent = DEFAULT_EVENT;
    break;

  case STARTED:
    currentEvent = NULL_EVENT;
    break;

  case STOPPED:
    digitalWrite(GPO_LED_ON_PIN, LOW);
    elapsedTime = stopTime - startTime;
    currentEvent = DEFAULT_EVENT;
    break;

  default:
    break;
  }

}


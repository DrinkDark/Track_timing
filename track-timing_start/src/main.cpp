#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Declaration of the display object
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

#define OLED_RESET 4
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const uint8_t stickman_bitmap[] PROGMEM = {
	0x00, 0x00, 0x00, 0xe0, 0x00, 0x40, 0x01, 0xf0, 
  0x38, 0x08, 0x00, 0x04, 0xf1, 0x02, 0x00, 0xa2, 
	0x7c, 0x42, 0x00, 0x02, 0x30, 0x02, 0x04, 0x04, 
  0x02, 0x08, 0x01, 0xf0, 0x00, 0x00, 0x00, 0x00
};

#define GPI_START_PIN 2
#define GPI_STOP_PIN 3
#define GPO_LED_ON_PIN 4
#define GPO_BUZZER_PIN 5


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
unsigned long intervalTime;

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

void printTime(unsigned long elapsedTime) {
  display.clearDisplay();

  if(elapsedTime > 99999) {
    display.setTextSize(2);
    display.setCursor(20, 10);
  } else {
    display.setTextSize(3);
    display.setCursor(12, 5);
  }

  display.print((float)elapsedTime / 1000);
  display.print("s");
  display.display();
}

void setup() {
  Serial.begin(115200);

  attachInterrupt(digitalPinToInterrupt(GPI_START_PIN), startTimer, RISING);
  attachInterrupt(digitalPinToInterrupt(GPI_STOP_PIN), stopTimer, RISING);

  pinMode(GPI_START_PIN, INPUT);
  pinMode(GPI_STOP_PIN, INPUT);
  pinMode(GPO_LED_ON_PIN, OUTPUT);
  pinMode(GPO_BUZZER_PIN, OUTPUT);

  currentState = IDLE;
  currentEvent = NULL_EVENT;

  startTime = 0;
  stopTime = 0;
  elapsedTime = 0;
  intervalTime = 0;

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(40, 13);

  display.drawFastHLine(0, 2, 128, SSD1306_WHITE);
  display.print("Track-timing");
  display.drawFastHLine(0, 30, 128, SSD1306_WHITE);

  display.drawBitmap(20, 9, stickman_bitmap, 16, 16, SSD1306_WHITE);
  display.display();

  delay(2000);
}

void loop() {
  // Choose the next state
  switch (currentState)
  {
  case IDLE:
    break;

  case START_DEBOUNCE:
    if(millis() - startTime > 200) {
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
    if(millis() - stopTime > 200) {
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

      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(35, 12);
      display.print("Press start");
      display.display();
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

    if(millis() - intervalTime > 100) {
      elapsedTime = millis() - startTime;
      printTime(elapsedTime);
      intervalTime = millis();
    }

    break;

  case STOPPED:
    digitalWrite(GPO_LED_ON_PIN, LOW);
    elapsedTime = stopTime - startTime;
    printTime(elapsedTime);
    
    currentEvent = DEFAULT_EVENT;
    break;

  default:
    break;
  }

}


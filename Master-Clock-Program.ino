/*********************************************************************
  master clock program v1.0
  with clock out, reset out, and start/stop

  --Original code by Potatopasty https://tg-music.neocities.org
  potatopasty@pm.me

  --Code modified by Craig Barnes to display on a 128x32 i2c OLED
  Includes a reset switch to reset the counter and send a MIDI song position

  --Heavily modified by Barcode to add reset trigger when the clock is stopped.
  This is very helpful for sequencers that do not automatically reset to
  the first step when the clock source is stopped. 

  --ToDo: Add DinSync clock and start/stop

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see https://www.gnu.org/licenses/.

*********************************************************************/

#include "Arduino.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <SimpleTimer.h>  // https://github.com/marcelloromani/Arduino-SimpleTimer
#include <NewEncoder.h>   // https://github.com/gfvalvo/NewEncoder
#include <Button.h>       // https://github.com/virgildisgr4ce/Button/

// Set the timer
SimpleTimer timer;
int count = 0;
int run;
int rawValue;
int oldValue;
byte potPercentage;
byte oldPercentage;

// Encoder settings:
// aPin, bPin, minValue, maxValue, initalValue, type (FULL_PULSE for quadrature pulse per detent)
NewEncoder encoder(2, 3, 60, 300, 120, FULL_PULSE);
int16_t prevEncoderValue;

Button buttonStart = Button(4, BUTTON_PULLUP_INTERNAL, true, 50);
Button buttonReset = Button(10, BUTTON_PULLUP_INTERNAL, true, 50);

float BPM;
bool started = false;

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 32  // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1  // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(9600);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  run = false;

  NewEncoder::EncoderState state;
  encoder.begin();

  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW);

  display.clearDisplay();

  // // Intro screen
  display.setCursor(15, 8);
  display.setTextColor(WHITE, BLACK);
  display.setTextSize(2);
  display.print(F("MCP v1.0"));
  display.display();
  delay(1500);
  display.clearDisplay();
  display.display();
}

void loop() {

  if (buttonStart.uniquePress()) {
    if (run == false) {
      run = true;
    } else {
      run = false;

      digitalWrite(5, LOW);
      digitalWrite(6, LOW);
      digitalWrite(7, LOW);
      digitalWrite(8, LOW);
      digitalWrite(9, HIGH);
      delay(50);
      digitalWrite(9, LOW);
    }
  }

  if (buttonReset.uniquePress()) {
    if (run == false) {
      digitalWrite(5, LOW);
      digitalWrite(6, LOW);
      digitalWrite(7, LOW);
      digitalWrite(8, LOW);
      digitalWrite(9, HIGH);
      delay(50);
      digitalWrite(9, LOW);
      count = 0;
    }
  }

  if (run > false) {
    timer.run();
    if (!started) {
      cycle_on();
      started = true;
    }
  }

  control();    // Rotary encoder function
  disp();       // Display function
}

/*********************************************************************/

void cycle_off() {

  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);
  digitalWrite(8, LOW);

  count++;
  if (count == 8) {
    count = 0;
  }
}

/*********************************************************************/

void cycle_on() {

  float duration_percentage = map(analogRead(A1), 0, 1023, 1, 90);
  int cycletime = (60000/BPM);
  long cycle_start = cycletime;
  long cycle_stop = (cycletime * (duration_percentage / 100));

  timer.setTimeout(cycle_start, cycle_on);
  timer.setTimeout(cycle_stop, cycle_off);

  switch (count) {
    case 0:
      digitalWrite(5, HIGH);  // 8 beat
      digitalWrite(6, HIGH);  // 8 beat
      digitalWrite(7, HIGH);  // 4 beat
      digitalWrite(8, HIGH);  // 2 beat
      break;

    case 1:
      digitalWrite(5, HIGH);
      digitalWrite(6, HIGH);
      break;

    case 2:
      digitalWrite(5, HIGH);
      digitalWrite(6, HIGH);
      digitalWrite(7, HIGH);
      break;

    case 3:
      digitalWrite(5, HIGH);
      digitalWrite(6, HIGH);
      break;

    case 4:
      digitalWrite(5, HIGH);
      digitalWrite(6, HIGH);
      digitalWrite(7, HIGH);
      digitalWrite(8, HIGH);
      break;

    case 5:
      digitalWrite(5, HIGH);
      digitalWrite(6, HIGH);
      break;

    case 6:
      digitalWrite(5, HIGH);
      digitalWrite(6, HIGH);
      digitalWrite(7, HIGH);
      break;

    case 7:
      digitalWrite(5, HIGH);
      digitalWrite(6, HIGH);
      break;
  }
}

/*********************************************************************/

void control() {

  int16_t currentValue;
  NewEncoder::EncoderState currentEncoderState;

  encoder.getState(currentEncoderState);
  currentValue = currentEncoderState.currentValue;

  if (currentValue != prevEncoderValue) {
  }
  BPM = (currentValue);
  prevEncoderValue = currentValue;
}

/*********************************************************************/

void disp() {

  float duration_percentage;
  rawValue = analogRead(A1);
  rawValue = analogRead(A1); // double read

  rawValue = constrain(rawValue, 8, 1015);
    if (rawValue < (oldValue - 8) || rawValue > (oldValue + 8)) {
    oldValue = rawValue;
    // convert to percentage
    duration_percentage = map(oldValue, 8, 1008, 1, 90);
  }

  //float duration_percentage = map(analogRead(A1), 0, 1023, 1, 90);
  display.setCursor(0, 0);
  display.setTextColor(WHITE, BLACK);
  display.setTextSize(2);
  display.print("BPM:  ");
  display.print(BPM, 0);

  if (BPM < 100) {
    display.setCursor(94, 0);
    display.print(" ");
  }
  display.setCursor(0, 16);
  display.setTextColor(WHITE, BLACK);
  display.setTextSize(2);
  display.print("Duty: ");
  display.print(duration_percentage, 0);

  if (duration_percentage < 10) {
    display.setCursor(82, 16);
    display.print(" ");
  }
  display.display();
}


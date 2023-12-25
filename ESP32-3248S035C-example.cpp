/*******************************************************************
    Hello World for the ESP32 Cheap Yellow Display.
    Written by Drew Franz

    Copyright 2023 Drew Franz. All Rights Reserved.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
===========================================================================
 *******************************************************************/
// ----------------------------
// Standard Libraries
// ----------------------------

#include <Arduino.h>
#include <SPI.h>
#include <FS.h>

// Un-comment if board has a touchscreen attached
#define HAS_SCREEN

// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------
#ifdef HAS_SCREEN
// A library for interfacing with LCD displays
#include <TFT_eSPI.h>
#include <TFT_eWidget.h>
// Extra font options
#include "Free_Fonts.h"

// Initialize the screen control object
TFT_eSPI tft = TFT_eSPI();

// ----------------------------
// Bare Bones Capacitive Touch Library
// ----------------------------
#include <bb_captouch.h>
// GT911 Capacitive Touch Driver Pinout
#define TOUCH_SDA 33
#define TOUCH_SCL 32
#define TOUCH_INT 21
#define TOUCH_RST 25
#define TOUCH_WIDTH 320
#define TOUCH_HEIGHT 480
BBCapTouch bbct;
const char *szNames[] = {"Unknown", "FT6x36", "GT911", "CST820"};

// ----------------------------
// Set up button objects
// ----------------------------
ButtonWidget btnL = ButtonWidget(&tft);
ButtonWidget btnR = ButtonWidget(&tft);

#define BUTTON_W 100
#define BUTTON_H 50

// Hold the button object references in a struct,
// so that we can iterate over them more easily
ButtonWidget* btn[] = {&btnL , &btnR};;
uint8_t buttonCount = sizeof(btn) / sizeof(btn[0]);

// ----------------------------
// Button action handlers
// ----------------------------
void btnL_pressAction(void) {
  if (btnL.justPressed()) {
    Serial.println("Left button just pressed.");
    tft.fillScreen(TFT_WHITE);
    btnL.drawSmoothButton(true);
    btnR.drawSmoothButton(false);
  }
}

void btnL_releaseAction(void) {
  // static uint32_t waitTime = 1000;
  if (btnL.justReleased()) {
    Serial.println("Left button just released.");
    tft.fillScreen(TFT_BLACK);
    btnL.drawSmoothButton(false);
    btnR.drawSmoothButton(false);
    btnL.setReleaseTime(millis());
    // waitTime = 10000;
  }
  // else {
  //   if (millis() - btnL.getReleaseTime() >= waitTime) {
  //     waitTime = 1000;
  //     btnL.setReleaseTime(millis());
  //     btnL.drawSmoothButton(!btnL.getState());
  //   }
  // }
}

void btnR_pressAction(void) {
  if (btnR.justPressed()) {
    btnR.drawSmoothButton(!btnR.getState(), 3, TFT_BLACK, btnR.getState() ? "OFF" : "ON");
    Serial.print("Right button toggled: ");
    if (btnR.getState()) Serial.println("ON");
    else Serial.println("OFF");
    btnR.setPressTime(millis());
  }

  // If button pressed for more than 1 second.
  if (millis() - btnR.getPressTime() >= 1000) {
    Serial.println("That's a looooong press.");
  } else {
    Serial.println("Right button is being pressed.");
  }
}

void btnR_releaseAction(void) {
  // No action
}

// ----------------------------
// Init the button objects
// ----------------------------
void initButtons() {
  uint16_t x = (tft.width() - BUTTON_W) / 2;
  uint16_t y = tft.height() / 2 - BUTTON_H - 10;
  btnL.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_RED, TFT_BLACK, "Button", 1);
  btnL.setPressAction(btnL_pressAction);
  btnL.setReleaseAction(btnL_releaseAction);
  btnL.drawSmoothButton(false, 3, TFT_BLACK);

  y = tft.height() / 2 + 10;
  btnR.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_BLACK, TFT_GREEN, "OFF", 1);
  btnR.setPressAction(btnR_pressAction);
  btnR.drawSmoothButton(false, 3, TFT_BLACK);
}
#endif // If has touchscreen

// ----------------------------
// Setup
// ----------------------------
void setup() {
  // ------------ Start logging ------------
  Serial.begin(115200);

  #if defined(HAS_SCREEN)
  // -------- Init screen and touch --------
    Serial.println("[INFO] : Starting screen");
    bbct.init(TOUCH_SDA, TOUCH_SCL, TOUCH_RST, TOUCH_INT);
    int iType = bbct.sensorType();
    Serial.printf("[INFO] : Sensor type = %s\n", szNames[iType]);

    tft.begin();
    tft.setRotation(0); //This is the display in landscape
    
    // // Clear the screen before writing to it
    tft.fillScreen(TFT_BLACK);
    tft.setFreeFont(FF18);

    initButtons();
  #endif
}

void loop() {
  delay(50);  // Slow the loop down

  #if defined(HAS_SCREEN)
    int i;

    // Note, the touch info object does support multi-touch,
    // but this example only checks for the first touch instance.
    TOUCHINFO ti;
    int pressed = bbct.getSamples(&ti);

    for (uint8_t b = 0;b < buttonCount; b++) {
      if (ti.count > 0) {
        if (btn[b]->contains(ti.x[0], ti.y[0])) {
          btn[b]->press(true);
          btn[b]->pressAction();
        }
      }
      else {
        btn[b]->press(false);
        btn[b]->releaseAction();
      }
    }
  #endif
}

#include <Arduino.h>
#include <Wire.h>

#include "RAK4631Board.h"

void RAK4631Board::begin() {
  NRF52BoardDCDC::begin();
  pinMode(PIN_VBAT_READ, INPUT);
#ifdef PIN_USER_BTN
  pinMode(PIN_USER_BTN, INPUT_PULLUP);
#endif

#ifdef PIN_USER_BTN_ANA
  pinMode(PIN_USER_BTN_ANA, INPUT_PULLUP);
#endif

#if defined(PIN_BOARD_SDA) && defined(PIN_BOARD_SCL)
  Wire.setPins(PIN_BOARD_SDA, PIN_BOARD_SCL);
#endif

  Wire.begin();

  // Enable power to sensor slots (Slot A and Slot B use WB_IO2 as power enable on some base boards)
  // On RAK19007, this ensures 3V3_S power rail is active for sensor modules
  pinMode(34, OUTPUT);  // WB_IO2 - shared by SLOT_A and SLOT_B
  digitalWrite(34, HIGH);  // Enable power to sensor slots
  delay(10);  // Allow power rail to stabilize

  pinMode(SX126X_POWER_EN, OUTPUT);
  digitalWrite(SX126X_POWER_EN, HIGH);
  delay(10);   // give sx1262 some time to power up
}
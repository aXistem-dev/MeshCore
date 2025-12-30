#include <Arduino.h>
#include "target.h"
#include <helpers/ArduinoHelpers.h>

RAK4631Board board;

#ifndef PIN_USER_BTN
  #define PIN_USER_BTN (-1)
#endif

#ifdef DISPLAY_CLASS
  DISPLAY_CLASS display;
  MomentaryButton user_btn(PIN_USER_BTN, 1000, true, true);

  #if defined(PIN_USER_BTN_ANA)
  MomentaryButton analog_btn(PIN_USER_BTN_ANA, 1000, 20);
  #endif
#endif

RADIO_CLASS radio = new Module(P_LORA_NSS, P_LORA_DIO_1, P_LORA_RESET, P_LORA_BUSY, SPI);

WRAPPER_CLASS radio_driver(radio, board);

VolatileRTCClock fallback_clock;
AutoDiscoverRTCClock rtc_clock(fallback_clock);

// Instantiate WeatherStationSensorManager instead of EnvironmentSensorManager
WeatherStationSensorManager sensors;

bool radio_init() {
  // Standard pattern: initialize RTC clock first (probes I2C for RTC chips)
  // For RAK19007, RTC in slot 2 is on Wire1 (I2C2: pins 24/25), not Wire (I2C1: pins 13/14)
  Serial.println("Initializing RTC clock...");
  Serial.flush();
  
  #ifdef NRF52_PLATFORM
    // Initialize Wire1 for RTC in slot 2
    Wire1.setPins(24, 25);  // WB_I2C2_SDA, WB_I2C2_SCL
    Wire1.begin();
    Wire1.setClock(100000);
    Serial.println("Probing Wire1 (I2C2) for RTC chips...");
    Serial.flush();
    rtc_clock.begin(Wire1);  // Probe Wire1 first for RTC in slot 2
  #else
    Serial.println("Probing Wire (I2C1) for RTC chips...");
    Serial.flush();
    rtc_clock.begin(Wire);
  #endif
  
  // Check if RTC was found by reading the time
  // If RTC is not found, it will use the fallback clock (VolatileRTCClock)
  uint32_t rtc_time = rtc_clock.getCurrentTime();
  Serial.print("RTC time: ");
  Serial.print(rtc_time);
  Serial.print(" (Unix timestamp)");
  
  // Check if time is reasonable (not 0 or very old)
  if (rtc_time == 0 || rtc_time < 1609459200) {  // Before 2021-01-01
    Serial.println(" - WARNING: RTC time appears invalid (may be using fallback clock)");
  } else {
    // Convert to readable format
    uint32_t days = rtc_time / 86400;
    uint32_t seconds = rtc_time % 86400;
    uint32_t hours = seconds / 3600;
    uint32_t minutes = (seconds % 3600) / 60;
    uint32_t secs = seconds % 60;
    Serial.printf(" = %lu days, %02lu:%02lu:%02lu", days, hours, minutes, secs);
    Serial.println(" - RTC appears to be working");
  }
  Serial.flush();
  
  // Then initialize radio
  return radio.std_init(&SPI);
}

uint32_t radio_get_rng_seed() {
  return radio.random(0x7FFFFFFF);
}

void radio_set_params(float freq, float bw, uint8_t sf, uint8_t cr) {
  radio.setFrequency(freq);
  radio.setSpreadingFactor(sf);
  radio.setBandwidth(bw);
  radio.setCodingRate(cr);
}

void radio_set_tx_power(uint8_t dbm) {
  radio.setOutputPower(dbm);
}

mesh::LocalIdentity radio_new_identity() {
  RadioNoiseListener rng(radio);
  return mesh::LocalIdentity(&rng);  // create new random identity
}


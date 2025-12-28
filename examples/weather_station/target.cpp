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
  #ifdef NRF52_PLATFORM
    // Initialize Wire1 for RTC in slot 2
    Wire1.setPins(24, 25);  // WB_I2C2_SDA, WB_I2C2_SCL
    Wire1.begin();
    Wire1.setClock(100000);
    rtc_clock.begin(Wire1);  // Probe Wire1 first for RTC in slot 2
  #else
    rtc_clock.begin(Wire);
  #endif
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


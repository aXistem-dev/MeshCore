#include <Arduino.h>
#include "target.h"
#include <board-config.h>

CubeCellBoard board;

RADIO_CLASS radio = new Module(RADIO_NSS, RADIO_DIO_1, RADIO_RESET, RADIO_BUSY);

WRAPPER_CLASS radio_driver(radio, board);

VolatileRTCClock rtc_clock;
SensorManager sensors;

#ifndef LORA_CR
  #define LORA_CR 5
#endif

bool radio_init() {
  pinMode(RADIO_ANT_SWITCH_POWER, OUTPUT);
  digitalWrite(RADIO_ANT_SWITCH_POWER, HIGH);
  delay(10);

  int status = radio.begin(LORA_FREQ, LORA_BW, LORA_SF, LORA_CR, RADIOLIB_SX126X_SYNC_WORD_PRIVATE, LORA_TX_POWER, 16, 0.0f);
  if (status != RADIOLIB_ERR_NONE) {
    Serial.print("ERROR: radio init failed: ");
    Serial.println(status);
    return false;
  }

#ifdef SX126X_RX_BOOSTED_GAIN
  radio.setRxBoostedGainMode(SX126X_RX_BOOSTED_GAIN);
#endif
  radio.setCRC(1);
  return true;
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

void radio_set_tx_power(int8_t dbm) {
  radio.setOutputPower(dbm);
}

mesh::LocalIdentity radio_new_identity() {
  RadioNoiseListener rng(radio);
  return mesh::LocalIdentity(&rng);
}

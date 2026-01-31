#include <Arduino.h>
#include "target.h"

M5CardputerADVBoard board;

static SPIClass spi;

RADIO_CLASS radio = new Module(P_LORA_NSS, P_LORA_DIO_1, P_LORA_RESET, P_LORA_BUSY, spi);

WRAPPER_CLASS radio_driver(radio, board);

ESP32RTCClock fallback_clock;
AutoDiscoverRTCClock rtc_clock(fallback_clock);

MomentaryButton user_btn(PIN_USER_BTN, 1000, true);
#ifdef DISPLAY_CLASS
DISPLAY_CLASS display;
#endif

#if ENV_INCLUDE_GPS
  #include <helpers/sensors/MicroNMEALocationProvider.h>
  MicroNMEALocationProvider nmea = MicroNMEALocationProvider(Serial1, &rtc_clock);
  EnvironmentSensorManager sensors = EnvironmentSensorManager(nmea);
#else
  EnvironmentSensorManager sensors;
#endif

bool radio_init() {
  Serial.println("=== Starting radio_init ===");
  Serial.printf("LoRa pins: NSS=%d DIO1=%d RST=%d BUSY=%d\n", P_LORA_NSS, P_LORA_DIO_1, P_LORA_RESET, P_LORA_BUSY);
  Serial.printf("SPI pins: SCLK=%d MISO=%d MOSI=%d\n", P_LORA_SCLK, P_LORA_MISO, P_LORA_MOSI);

  // GPIO 46 (LoRa Cap power) is already enabled in board.begin()
  Serial.println("LoRa Cap power already enabled (GPIO 46)");

  Serial.println("Initializing clocks...");
  fallback_clock.begin();
  rtc_clock.begin(Wire);

  // Initialize SPI bus for LoRa module
  Serial.println("Initializing SPI bus...");
  spi.begin(P_LORA_SCLK, P_LORA_MISO, P_LORA_MOSI);

  // Hardware reset sequence for LoRa module (from Stachugit implementation)
  // This ensures proper initialization even if module is connected after power-on
  pinMode(P_LORA_RESET, OUTPUT);
  digitalWrite(P_LORA_RESET, LOW);   // Assert reset
  delay(10);                          // Hold for 10ms
  digitalWrite(P_LORA_RESET, HIGH);  // Release reset
  delay(100);                         // Wait for module to boot (SX1262 needs ~50ms)

  // Initialize radio with SPI
  Serial.println("Calling radio.std_init()...");
  bool result = radio.std_init(&spi);

  if (!result) {
    Serial.println("ERROR: radio init failed");
    Serial.println("Check if LoRa Cap LED is now lit!");
  } else {
    Serial.println("=== Radio initialized successfully! ===");
  }

  return result;
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

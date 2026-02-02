#include <Arduino.h>
#include <Wire.h>

#include "SenseCapSolarBoard.h"
#include "variant.h"

#ifdef NRF52_POWER_MANAGEMENT
static const PowerMgtConfig power_config = {
  .lpcomp_ain_channel = PWRMGT_LPCOMP_AIN,
  .lpcomp_refsel = PWRMGT_LPCOMP_REFSEL,
  .voltage_bootlock = PWRMGT_VOLTAGE_BOOTLOCK
};
#endif

static void blinkBothLeds(int times, int onMs, int offMs) {
#if defined(LED_WHITE) && defined(LED_BLUE)
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_WHITE, HIGH);
    digitalWrite(LED_BLUE, HIGH);
    delay(onMs);
    digitalWrite(LED_WHITE, LOW);
    digitalWrite(LED_BLUE, LOW);
    if (i < times - 1) delay(offMs);
  }
#endif
}

void SenseCapSolarBoard::begin() {
  NRF52Board::begin();

#if defined(PIN_WIRE_SDA) && defined(PIN_WIRE_SCL)
  Wire.setPins(PIN_WIRE_SDA, PIN_WIRE_SCL);
#endif

  Wire.begin();

#ifdef P_LORA_TX_LED
  pinMode(P_LORA_TX_LED, OUTPUT);
  digitalWrite(P_LORA_TX_LED, LOW);
#endif
#if defined(LED_WHITE)
  pinMode(LED_WHITE, OUTPUT);
  digitalWrite(LED_WHITE, LOW);
#endif
#if defined(LED_BLUE)
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, LOW);
#endif

  delay(10);   // give sx1262 some time to power up

#ifdef NRF52_POWER_MANAGEMENT
  checkBootVoltage(&power_config);
#endif

#if defined(SENSECAP_HEADLESS) && !defined(DISPLAY_CLASS)
  // Startup: white + blue on for 5 seconds, then off
  digitalWrite(LED_WHITE, HIGH);
  digitalWrite(LED_BLUE, HIGH);
  delay(5000);
  digitalWrite(LED_WHITE, LOW);
  digitalWrite(LED_BLUE, LOW);
#endif
}

void SenseCapSolarBoard::powerOff() {
#if defined(LED_WHITE) && defined(LED_BLUE)
  // Shutdown: blink white + blue 5 times fast
  blinkBothLeds(5, 150, 150);
#endif
#ifdef NRF52_POWER_MANAGEMENT
  initiateShutdown(SHUTDOWN_REASON_USER);
#else
  (void)0;
#endif
}

#ifdef NRF52_POWER_MANAGEMENT
void SenseCapSolarBoard::initiateShutdown(uint8_t reason) {
  bool enable_lpcomp = (reason == SHUTDOWN_REASON_LOW_VOLTAGE ||
                        reason == SHUTDOWN_REASON_BOOT_PROTECT);
  if (enable_lpcomp) {
    configureVoltageWake(power_config.lpcomp_ain_channel, power_config.lpcomp_refsel);
  }
  enterSystemOff(reason);
}
#endif
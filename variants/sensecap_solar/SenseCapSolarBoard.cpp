#include <Arduino.h>
#include <Wire.h>

#include "SenseCapSolarBoard.h"
#include "variant.h"

#ifdef NRF52_POWER_MANAGEMENT
#include "nrf.h"
extern const uint32_t g_ADigitalPinMap[];
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

#ifdef NRF52_POWER_MANAGEMENT
const PowerMgtConfig power_config = {
  .lpcomp_ain_channel = PWRMGT_LPCOMP_AIN,
  .lpcomp_refsel = PWRMGT_LPCOMP_REFSEL,
  .voltage_bootlock = PWRMGT_VOLTAGE_BOOTLOCK
};

void SenseCapSolarBoard::initiateShutdown(uint8_t reason) {
  bool enable_lpcomp = (reason == SHUTDOWN_REASON_LOW_VOLTAGE ||
                        reason == SHUTDOWN_REASON_BOOT_PROTECT);

  pinMode(VBAT_ENABLE, OUTPUT);
  digitalWrite(VBAT_ENABLE, enable_lpcomp ? LOW : HIGH);

  if (enable_lpcomp) {
    configureVoltageWake(power_config.lpcomp_ain_channel, power_config.lpcomp_refsel);
  }

  enterSystemOff(reason);
}
#endif // NRF52_POWER_MANAGEMENT

void SenseCapSolarBoard::begin() {
  NRF52BoardDCDC::begin();

  pinMode(BATTERY_PIN, INPUT);
  pinMode(VBAT_ENABLE, OUTPUT);
  digitalWrite(VBAT_ENABLE, LOW);
  analogReadResolution(12);
  analogReference(AR_INTERNAL_3_0);
  delay(50);

#ifdef PIN_USER_BTN
  pinMode(PIN_USER_BTN, INPUT_PULLUP);
#elif defined(PIN_BUTTON1)
  pinMode(PIN_BUTTON1, INPUT_PULLUP);
#endif

#if defined(PIN_WIRE_SDA) && defined(PIN_WIRE_SCL)
  Wire.setPins(PIN_WIRE_SDA, PIN_WIRE_SCL);
#endif

  Wire.begin();

#ifdef LED_WHITE
  pinMode(LED_WHITE, OUTPUT);
  digitalWrite(LED_WHITE, HIGH);
#endif
#ifdef LED_BLUE
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, LOW);
#endif

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

#ifdef NRF52_POWER_MANAGEMENT
  checkBootVoltage(&power_config);
#endif

  delay(10);   // give sx1262 some time to power up

#if defined(PIN_USER_BTN) && defined(_SEEED_SENSECAP_SOLAR_H_) && !defined(DISPLAY_CLASS)
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
  // Shutdown: blink white + blue 5 times fast (custom feedback)
  blinkBothLeds(5, 150, 150);
#endif
#ifdef NRF52_POWER_MANAGEMENT
#ifdef PIN_USER_BTN
  while (digitalRead(PIN_USER_BTN) == LOW)
    ;  // Wait for user to release button
  // Keep pull-up enabled in system-off so the wake line doesn't float low.
  nrf_gpio_cfg_sense_input(g_ADigitalPinMap[PIN_USER_BTN], NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
#elif defined(PIN_BUTTON1)
  while (digitalRead(PIN_BUTTON1) == LOW)
    ;
  nrf_gpio_cfg_sense_input(g_ADigitalPinMap[PIN_BUTTON1], NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
#endif
  initiateShutdown(SHUTDOWN_REASON_USER);
#else
  sd_power_system_off();
#endif
}

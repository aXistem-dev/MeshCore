#pragma once

#include <MeshCore.h>
#include <Arduino.h>
#include <helpers/NRF52Board.h>

#if defined(PIN_USER_BTN) && defined(_SEEED_SENSECAP_SOLAR_H_) && !defined(DISPLAY_CLASS)
class EnvironmentSensorManager;
#endif

class SenseCapSolarBoard : public NRF52BoardDCDC {
protected:
#ifdef NRF52_POWER_MANAGEMENT
  void initiateShutdown(uint8_t reason) override;
#endif

#if defined(PIN_USER_BTN) && defined(_SEEED_SENSECAP_SOLAR_H_) && !defined(DISPLAY_CLASS)
  void headlessPollButtons();
  void headlessPollGpsLed();
#endif

public:
  SenseCapSolarBoard() : NRF52Board("SENSECAP_SOLAR_OTA") {}
  void begin();
  void powerOff() override;
  void loop() override;

#if defined(PIN_USER_BTN) && defined(_SEEED_SENSECAP_SOLAR_H_) && !defined(DISPLAY_CLASS)
  void beginHeadless(EnvironmentSensorManager* sensors, void (*onSendAdvert)(void));
#endif

#if defined(P_LORA_TX_LED)
  void onBeforeTransmit() override {
    digitalWrite(P_LORA_TX_LED, HIGH);   // turn TX LED on
  }
  void onAfterTransmit() override {
    digitalWrite(P_LORA_TX_LED, LOW);   // turn TX LED off
  }
#endif

  uint16_t getBattMilliVolts() override {
    digitalWrite(VBAT_ENABLE, LOW);
    int adcvalue = 0;
    analogReadResolution(12);
    analogReference(AR_INTERNAL_3_0);
    delay(10);
    adcvalue = analogRead(BATTERY_PIN);
    return (adcvalue * ADC_MULTIPLIER * AREF_VOLTAGE) / 4.096;
  }

  const char* getManufacturerName() const override {
    return "Seeed SenseCap Solar";
  }

#if defined(PIN_USER_BTN) && defined(_SEEED_SENSECAP_SOLAR_H_) && !defined(DISPLAY_CLASS)
private:
  EnvironmentSensorManager* _headless_sensors = nullptr;
  void (*_headless_on_send_advert)(void) = nullptr;
  static const unsigned long HEADLESS_PRESS_WINDOW_MS = 600;
  uint8_t _headless_usr_press_count = 0;
  unsigned long _headless_usr_window_end = 0;
  bool _headless_usr_was_pressed = false;
  uint8_t _headless_gps_led_state = 0;
  unsigned long _headless_gps_led_ts = 0;
  bool _headless_gps_led_was_valid = false;
  bool _headless_gps_led_was_active = false;
#endif
};

#pragma once

#if defined(PIN_USER_BTN) && defined(_SEEED_SENSECAP_SOLAR_H_) && !defined(DISPLAY_CLASS)

#include <Arduino.h>

namespace mesh { class MainBoard; }
class EnvironmentSensorManager;

class SenseCapHeadless {
public:
  void begin(mesh::MainBoard* board, EnvironmentSensorManager* sensors, void (*onSendAdvert)(void));
  void loop();

private:
  mesh::MainBoard* _board = nullptr;
  EnvironmentSensorManager* _sensors = nullptr;
  void (*_onSendAdvert)(void) = nullptr;
  void pollButtons();
  void pollGpsLed();

  static const unsigned long PRESS_WINDOW_MS = 600;
  uint8_t _usr_press_count = 0;
  unsigned long _usr_window_end = 0;
  bool _usr_was_pressed = false;

  uint8_t _gps_led_state = 0;
  unsigned long _gps_led_ts = 0;
  bool _gps_led_was_valid = false;
  bool _gps_led_was_active = false;  // to detect power-down after hold (state 0 -> state 3)
};

#endif

#if defined(SENSECAP_HEADLESS) && !defined(DISPLAY_CLASS)

#include "SenseCapHeadless.h"
#include "variant.h"
#include <MeshCore.h>
#include <helpers/sensors/EnvironmentSensorManager.h>

#ifndef PIN_BUTTON1
#define PIN_BUTTON1 13
#endif
#ifndef PIN_BUTTON2
#define PIN_BUTTON2 20
#endif

#define BUTTON_PRESSED(pin) (digitalRead(pin) == LOW)

static const unsigned long GPS_LED_SLOW_BLINK_MS = 800;
static const unsigned long GPS_LED_LOCK_CONFIRM_MS = 3000;
static const int GPS_LED_FAST_BLINKS = 3;
static const unsigned long GPS_LED_FAST_ON_MS = 100;
static const unsigned long GPS_LED_FAST_OFF_MS = 100;

void SenseCapHeadless::begin(mesh::MainBoard* board, EnvironmentSensorManager* sensors, void (*onSendAdvert)(void)) {
  _board = board;
  _sensors = sensors;
  _onSendAdvert = onSendAdvert;
  _gps_led_state = 0;
  _gps_led_ts = 0;
  _gps_led_was_valid = false;
  _usr_press_count = 0;
  _usr_window_end = 0;
  _usr_was_pressed = false;
  _pwr_press_start = 0;
  _pwr_was_pressed = false;

  pinMode(PIN_BUTTON1, INPUT_PULLUP);
  pinMode(PIN_BUTTON2, INPUT_PULLUP);
#ifdef LED_WHITE
  pinMode(LED_WHITE, OUTPUT);
  digitalWrite(LED_WHITE, LOW);
#endif
}

void SenseCapHeadless::pollButtons() {
  unsigned long now = millis();

  if (!_board) return;

  bool pwr = BUTTON_PRESSED(PIN_BUTTON1);
  if (pwr) {
    if (!_pwr_was_pressed) {
      _pwr_was_pressed = true;
      _pwr_press_start = now;
    } else if (now - _pwr_press_start >= LONG_PRESS_MS) {
      _board->powerOff();
      _pwr_was_pressed = false;
    }
  } else {
    _pwr_was_pressed = false;
  }

  bool usr = BUTTON_PRESSED(PIN_BUTTON2);
  if (usr && !_usr_was_pressed) {
    _usr_was_pressed = true;
    _usr_press_count++;
    _usr_window_end = now + PRESS_WINDOW_MS;
  }
  if (!usr) {
    _usr_was_pressed = false;
  }

  if (_usr_press_count > 0 && now > _usr_window_end) {
    if (_usr_press_count == 2 && _onSendAdvert) {
      _onSendAdvert();
    } else if (_usr_press_count >= 3 && _sensors) {
      bool active = _sensors->getGpsActive();
      _sensors->setSettingValue("gps", active ? "0" : "1");
      if (active) {
        _gps_led_state = 3;
        _gps_led_ts = now;
      } else {
        _gps_led_state = 1;
        _gps_led_ts = now;
        _gps_led_was_valid = false;
      }
    }
    _usr_press_count = 0;
  }
}

void SenseCapHeadless::pollGpsLed() {
#ifdef LED_WHITE
  if (!_sensors) return;
  unsigned long now = millis();
  bool gps_active = _sensors->getGpsActive();
  bool gps_valid = _sensors->getGpsValid();

  if (_gps_led_state == 0) {
    if (gps_active && !gps_valid) {
      _gps_led_state = 1;
      _gps_led_ts = now;
      _gps_led_was_valid = false;
    }
    return;
  }

  if (_gps_led_state == 1) {
    if (!gps_active) {
      _gps_led_state = 0;
      digitalWrite(LED_WHITE, LOW);
      return;
    }
    if (gps_valid) {
      _gps_led_state = 2;
      _gps_led_ts = now;
      digitalWrite(LED_WHITE, HIGH);
      return;
    }
    unsigned long half = GPS_LED_SLOW_BLINK_MS / 2;
    unsigned long phase = (now - _gps_led_ts) % GPS_LED_SLOW_BLINK_MS;
    digitalWrite(LED_WHITE, (phase < half) ? HIGH : LOW);
    return;
  }

  if (_gps_led_state == 2) {
    if (!gps_active) {
      _gps_led_state = 0;
      digitalWrite(LED_WHITE, LOW);
      return;
    }
    if (now - _gps_led_ts >= GPS_LED_LOCK_CONFIRM_MS) {
      _gps_led_state = 0;
      digitalWrite(LED_WHITE, LOW);
    }
    return;
  }

  if (_gps_led_state == 3) {
    unsigned long cycle = GPS_LED_FAST_ON_MS + GPS_LED_FAST_OFF_MS;
    unsigned long elapsed = now - _gps_led_ts;
    int blink = (int)(elapsed / cycle);
    if (blink >= GPS_LED_FAST_BLINKS) {
      _gps_led_state = 0;
      digitalWrite(LED_WHITE, LOW);
      return;
    }
    unsigned long pos = elapsed % cycle;
    digitalWrite(LED_WHITE, (pos < GPS_LED_FAST_ON_MS) ? HIGH : LOW);
  }
#endif
}

void SenseCapHeadless::loop() {
  pollButtons();
  pollGpsLed();
}

#endif

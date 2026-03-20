#include <Arduino.h>
#include <Wire.h>

#include "SenseCapSolarBoard.h"
#include "variant.h"

#if defined(PIN_USER_BTN) && defined(_SEEED_SENSECAP_SOLAR_H_) && !defined(DISPLAY_CLASS)
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
static const int GPS_LED_POWER_ON_BLINKS = 2;
static const unsigned long GPS_LED_FAST_ON_MS = 100;
static const unsigned long GPS_LED_FAST_OFF_MS = 100;
#endif

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

#if defined(PIN_USER_BTN) && defined(_SEEED_SENSECAP_SOLAR_H_) && !defined(DISPLAY_CLASS)
void SenseCapSolarBoard::beginHeadless(EnvironmentSensorManager* sensors, void (*onSendAdvert)(void)) {
  _headless_sensors = sensors;
  _headless_on_send_advert = onSendAdvert;
  _headless_gps_led_state = 0;
  _headless_gps_led_ts = 0;
  _headless_gps_led_was_valid = false;
  _headless_gps_led_was_active = false;
  _headless_usr_press_count = 0;
  _headless_usr_window_end = 0;
  _headless_usr_was_pressed = false;

  pinMode(PIN_BUTTON2, INPUT_PULLUP);
#ifdef LED_WHITE
  pinMode(LED_WHITE, OUTPUT);
  digitalWrite(LED_WHITE, LOW);
#endif
}

void SenseCapSolarBoard::headlessPollButtons() {
  unsigned long now = millis();
  bool usr = BUTTON_PRESSED(PIN_BUTTON2);
  if (usr && !_headless_usr_was_pressed) {
    _headless_usr_was_pressed = true;
    _headless_usr_press_count++;
    _headless_usr_window_end = now + HEADLESS_PRESS_WINDOW_MS;
  }
  if (!usr) {
    _headless_usr_was_pressed = false;
  }

  if (_headless_usr_press_count > 0 && now > _headless_usr_window_end) {
    if (_headless_usr_press_count == 2 && _headless_on_send_advert) {
      _headless_on_send_advert();
    } else if (_headless_usr_press_count >= 3 && _headless_sensors) {
      bool active = _headless_sensors->getGpsActive();
      _headless_sensors->setSettingValue("gps", active ? "0" : "1");
      if (active) {
        _headless_gps_led_state = 3;
        _headless_gps_led_ts = now;
      } else {
        _headless_gps_led_state = 1;
        _headless_gps_led_ts = now;
        _headless_gps_led_was_valid = false;
      }
    }
    _headless_usr_press_count = 0;
  }
}

void SenseCapSolarBoard::headlessPollGpsLed() {
#ifdef LED_WHITE
  if (!_headless_sensors) return;
  unsigned long now = millis();
  bool gps_active = _headless_sensors->getGpsActive();
  bool gps_valid = _headless_sensors->getGpsValid();

  if (_headless_gps_led_state == 0) {
    if (gps_active && !gps_valid) {
      _headless_gps_led_state = 4;
      _headless_gps_led_ts = now;
      _headless_gps_led_was_valid = false;
      goto update_was_active;
    }
    if (_headless_gps_led_was_active && !gps_active) {
      _headless_gps_led_state = 3;
      _headless_gps_led_ts = now;
      goto update_was_active;
    }
    goto update_was_active;
  }

  if (_headless_gps_led_state == 1) {
    if (!gps_active) {
      _headless_gps_led_state = 3;
      _headless_gps_led_ts = now;
      goto update_was_active;
    }
    if (gps_valid) {
      _headless_gps_led_state = 2;
      _headless_gps_led_ts = now;
      digitalWrite(LED_WHITE, HIGH);
      goto update_was_active;
    }
    unsigned long half = GPS_LED_SLOW_BLINK_MS / 2;
    unsigned long phase = (now - _headless_gps_led_ts) % GPS_LED_SLOW_BLINK_MS;
    digitalWrite(LED_WHITE, (phase < half) ? HIGH : LOW);
    goto update_was_active;
  }

  if (_headless_gps_led_state == 2) {
    if (!gps_active) {
      _headless_gps_led_state = 3;
      _headless_gps_led_ts = now;
      goto update_was_active;
    }
    if (now - _headless_gps_led_ts >= GPS_LED_LOCK_CONFIRM_MS) {
      _headless_gps_led_state = 0;
      digitalWrite(LED_WHITE, LOW);
    }
    goto update_was_active;
  }

  if (_headless_gps_led_state == 3) {
    unsigned long cycle = GPS_LED_FAST_ON_MS + GPS_LED_FAST_OFF_MS;
    unsigned long elapsed = now - _headless_gps_led_ts;
    int blink = (int)(elapsed / cycle);
    if (blink >= GPS_LED_FAST_BLINKS) {
      _headless_gps_led_state = 0;
      digitalWrite(LED_WHITE, LOW);
      goto update_was_active;
    }
    unsigned long pos = elapsed % cycle;
    digitalWrite(LED_WHITE, (pos < GPS_LED_FAST_ON_MS) ? HIGH : LOW);
    goto update_was_active;
  }

  if (_headless_gps_led_state == 4) {
    if (!gps_active) {
      _headless_gps_led_state = 3;
      _headless_gps_led_ts = now;
      goto update_was_active;
    }
    unsigned long cycle = GPS_LED_FAST_ON_MS + GPS_LED_FAST_OFF_MS;
    unsigned long elapsed = now - _headless_gps_led_ts;
    int blink = (int)(elapsed / cycle);
    if (blink >= GPS_LED_POWER_ON_BLINKS) {
      _headless_gps_led_state = 1;
      _headless_gps_led_ts = now;
      goto update_was_active;
    }
    unsigned long pos = elapsed % cycle;
    digitalWrite(LED_WHITE, (pos < GPS_LED_FAST_ON_MS) ? HIGH : LOW);
  }

update_was_active:
  _headless_gps_led_was_active = gps_active;
#endif
}

#endif

void SenseCapSolarBoard::loop() {
#if defined(PIN_USER_BTN) && defined(_SEEED_SENSECAP_SOLAR_H_) && !defined(DISPLAY_CLASS)
  headlessPollButtons();
  headlessPollGpsLed();
#endif
}

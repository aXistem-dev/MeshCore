#pragma once

#include <MeshCore.h>
#include <Arduino.h>

class CubeCellBoard : public mesh::MainBoard {
protected:
  uint8_t startup_reason;

public:
  void begin() {
    startup_reason = BD_STARTUP_NORMAL;
#ifdef P_LORA_TX_LED
    pinMode(P_LORA_TX_LED, OUTPUT);
    digitalWrite(P_LORA_TX_LED, HIGH);
#endif
#ifdef PIN_USER_BTN
    pinMode(PIN_USER_BTN, INPUT_PULLUP);
#endif
  }

  uint8_t getStartupReason() const override { return startup_reason; }

  uint16_t getBattMilliVolts() override {
    uint32_t raw = 0;
    for (int i = 0; i < 8; i++) {
      raw += analogRead(ADC);
    }
    return (uint16_t)((raw * 3300UL) / (8 * 4096));
  }

  const char *getManufacturerName() const override {
    return "Heltec CubeCell GPS-6502";
  }

  void reboot() override {
    NVIC_SystemReset();
  }

  void powerOff() override {
    // CubeCell deep sleep is board-specific; no-op for now.
  }

#if defined(P_LORA_TX_LED)
  void onBeforeTransmit() override {
    digitalWrite(P_LORA_TX_LED, LOW);
  }
  void onAfterTransmit() override {
    digitalWrite(P_LORA_TX_LED, HIGH);
  }
#endif

  bool startOTAUpdate(const char *id, char reply[]) override { return false; }
};

#pragma once

#include "DisplayDriver.h"
#include <Wire.h>
#include <SPI.h>
#include "TFT_eSPI.h"
#include <helpers/RefCountedDigitalPin.h>

// TFT_eSPI's GFXFF font support (Fonts/GFXFF/gfxfont.h) unconditionally bundles every
// GFX font it ships, including some that are also shipped separately by Adafruit GFX
// Library under the same symbol names. Anything that also does e.g.
// `#include <Fonts/FreeMonoBold18pt7b.h>` for the Adafruit copy must skip it here to
// avoid a duplicate-definition error.
#define UI_TFT_ESPI_GFXFF_FONTS_LOADED 1

class ST7735Display : public DisplayDriver {
  bool _isOn;
  RefCountedDigitalPin* _peripher_power;

  bool i2c_probe(TwoWire& wire, uint8_t addr);
public:
#ifdef USE_PIN_TFT
  ST7735Display(RefCountedDigitalPin* peripher_power=NULL) : DisplayDriver(128, 64), 
    //  display(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_SDA, PIN_TFT_SCL, PIN_TFT_RST),
      _peripher_power(peripher_power)
  {
    _isOn = false;
  }
#else
  ST7735Display(RefCountedDigitalPin* peripher_power=NULL) : DisplayDriver(128, 64),
    //  display(&SPI1, PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST),
      _peripher_power(peripher_power)
  {
    _isOn = false;
  }
#endif
  bool begin();

  bool isOn() override { return _isOn; }
  void turnOn() override;
  void turnOff() override;
  void clear() override;
  void startFrame(ColorVal bkg = UIColor::window_bkg) override;
  void setTextSize(int sz) override;
  void setColor(ColorVal c) override;
  void setCursor(int x, int y) override;
  void print(const char* str) override;
  void fillRect(int x, int y, int w, int h) override;
  void drawRect(int x, int y, int w, int h) override;
  void drawXbm(int x, int y, const uint8_t* bits, int w, int h) override;
  uint16_t getTextWidth(const char* str) override;
  void endFrame() override;

protected:
  void _resetAndInit();
};

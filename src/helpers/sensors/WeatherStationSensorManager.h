#pragma once

#include <Mesh.h>
#include <helpers/SensorManager.h>
#include <Wire.h>
#include <Adafruit_BME280.h>

// Weather Station Pin Definitions (WisBlock IO Module)
#define WEATHER_RAIN_PIN     WB_IO1    // Rain gauge -> IO1 header pin
#define WEATHER_WIND_PIN     WB_IO3    // Anemometer -> IO3 header pin
#define WEATHER_VANE_PIN     WB_A0     // Wind vane -> AIN0 header pin
#define WEATHER_REF_VOLTAGE  3.3       // Reference voltage

// Weather Station Constants
#define WEATHER_MM_PER_TIP   0.2794    // Each tip = 0.2794mm of rain
#define WEATHER_WIND_FACTOR  2.4       // km/h per click/second
#define WEATHER_DEBOUNCE_MS  10
#define WEATHER_SEA_LEVEL_PRESSURE_HPA 1028.6  // Calibrated for 50.5m ASL

// Wind vane direction mapping structure
struct VaneReading {
  int minADC;
  int maxADC;
  int direction;
};

class WeatherStationSensorManager : public SensorManager {
protected:
  bool BME280_initialized = false;
  Adafruit_BME280 bme;
  
  // Rain gauge variables
  volatile unsigned long rainTips = 0;
  
  // Rainfall timestamp tracking (circular buffer)
  // Store timestamps in milliseconds since boot (will convert to unix time when querying)
  // Each tip = 0.2794mm
  // Calculations for 24-hour periods:
  //   Light rain (5mm/hr): 120mm/day = ~430 tips
  //   Moderate (10mm/hr): 240mm/day = ~860 tips  
  //   Heavy (50mm/hr): 1200mm/day = ~4298 tips
  //   Very heavy (100mm/hr): 2400mm/day = ~8597 tips
  //   Extreme (200mm/hr): 4800mm/day = ~17,194 tips
  // Using 10000 tips = ~2794mm (2.8m) - covers most extreme 24h scenarios including tropical storms
  static constexpr int MAX_RAIN_TIPS = 10000;  // Enough for extreme 24h rainfall scenarios
  struct RainTip {
    unsigned long timestamp_millis;  // millis() when tip occurred
  };
  RainTip rainTipHistory[MAX_RAIN_TIPS];
  volatile int rainTipWriteIndex = 0;  // Next position to write (circular)
  volatile int rainTipCount = 0;       // Number of tips stored
  unsigned long lastMidnightMillis = 0;  // millis() at last midnight (updated in loop)
  uint32_t lastMidnightUnix = 0;         // Unix timestamp of last midnight
  
  // Anemometer variables
  volatile unsigned long windClicks = 0;
  volatile unsigned long lastWindTime = 0;
  unsigned long lastWindCalc = 0;
  
  // Wind vane calibration map
  static const VaneReading vaneMap[];
  static const int numVaneReadings;
  
  // Timezone offset (in seconds from UTC)
  // Positive = east of UTC, negative = west of UTC
  // Example: UTC+5:30 (India) = 19800, UTC-5 (EST) = -18000
  int32_t timezone_offset_seconds = 0;
  
  // Interrupt service routines (must be static or friend functions)
  static WeatherStationSensorManager* instance;
  static void rainISR();
  static void windISR();
  
  // Helper functions (protected)
  float getWindSpeed();
  int getWindDirection() const;
  
  // Rainfall calculation helpers
  float getRainfallForInterval(uint32_t start_unix, uint32_t end_unix, mesh::RTCClock* rtc) const;
  uint32_t getMidnightUnix(mesh::RTCClock* rtc) const;
  
public:
  // Statistics helpers (for querySeriesData) - public so SensorMesh can call
  void getRainfallStats(uint32_t start_secs_ago, uint32_t end_secs_ago, mesh::RTCClock* rtc, 
                        float& min, float& max, float& avg) const;
  
public:
  WeatherStationSensorManager() {}
  bool begin() override;
  bool querySensors(uint8_t requester_permissions, CayenneLPP& telemetry) override;
  bool querySensors(uint8_t requester_permissions, CayenneLPP& telemetry, mesh::RTCClock* rtc);
  void loop() override;
  
  // Settings interface
  int getNumSettings() const override;
  const char* getSettingName(int i) const override;
  const char* getSettingValue(int i) const override;
  bool setSettingValue(const char* name, const char* value) override;
  
  // Direct access for calibration/debugging
  void resetRainfall() { rainTips = 0; }
  unsigned long getRainTips() const { return rainTips; }
  unsigned long getWindClicks() const { return windClicks; }
  float getRainfall() const { return rainTips * WEATHER_MM_PER_TIP; }
};


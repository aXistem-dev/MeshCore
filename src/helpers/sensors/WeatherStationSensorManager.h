#pragma once

#include <Mesh.h>
#include <helpers/SensorManager.h>
#include <Wire.h>
#include <Adafruit_BME280.h>

// Weather Station Pin Definitions (WisBlock IO Module)
#define WEATHER_RAIN_PIN     WB_IO1    // Rain gauge -> IO1 header pin
#define WEATHER_WIND_PIN     WB_IO3    // Anemometer -> IO3 header pin
#define WEATHER_VANE_PIN     WB_A1     // Wind vane -> AIN1 header pin (moved from WB_A0 to avoid conflict with battery voltage)
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

  // Rain gauge counter (cumulative since boot, receiver derives rate from successive samples)
  volatile unsigned long rainTips = 0;

  // Anemometer variables
  volatile unsigned long windClicks = 0;
  volatile unsigned long lastWindTime = 0;
  unsigned long lastWindCalc = 0;

  // Wind speed history tracking (circular buffer for peak calculation)
  // 5 minutes at 5-second intervals = 60 readings
  static constexpr int MAX_WIND_READINGS = 60;
  struct WindReading {
    float speed_kmh;
    unsigned long timestamp_millis;
  };
  WindReading windHistory[MAX_WIND_READINGS];
  int windWriteIndex = 0;
  int windCount = 0;
  float peakWindSpeed_5min = 0.0f;
  unsigned long lastPeakUpdate = 0;

  // Wind vane calibration map
  static const VaneReading vaneMap[];
  static const int numVaneReadings;

  // Interrupt service routines (must be static or friend functions)
  static WeatherStationSensorManager* instance;
  static void rainISR();
  static void windISR();

  // Helper functions (protected)
  float getWindSpeed();  // Get current momentary wind speed and store in history
  int getWindDirection() const;

  // Wind speed calculation helpers
  void updateWindSpeedHistory(float speed);
  float getPeakWindSpeed5Min() const;

public:
  WeatherStationSensorManager() {}
  bool begin() override;
  bool querySensors(uint8_t requester_permissions, CayenneLPP& telemetry) override;
  void loop() override;

  // Direct access for calibration/debugging
  void resetRainfall() { rainTips = 0; }
  unsigned long getRainTips() const { return rainTips; }
  unsigned long getWindClicks() const { return windClicks; }
  float getRainfall() const { return rainTips * WEATHER_MM_PER_TIP; }
};

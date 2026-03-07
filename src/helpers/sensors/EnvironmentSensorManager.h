#pragma once

#include <Mesh.h>
#include <helpers/SensorManager.h>
#include <helpers/sensors/LocationProvider.h>

class EnvironmentSensorManager : public SensorManager {
protected:
  int next_available_channel = TELEM_CHANNEL_SELF + 1;

  bool AHTX0_initialized = false;
  bool BME280_initialized = false;
  bool BMP280_initialized = false;
  bool INA3221_initialized = false;
  bool INA219_initialized = false;
  bool INA260_initialized = false;
  bool INA226_initialized = false;
  bool SHTC3_initialized = false;
  bool LPS22HB_initialized = false;
  bool MLX90614_initialized = false;
  bool VL53L0X_initialized = false;
  bool SHT4X_initialized = false;
  bool BME680_initialized = false;
  bool BMP085_initialized = false;

  bool gps_detected = false;
  bool gps_active = false;
  uint32_t gps_update_interval_sec = 1;  // Default 1 second

  #if ENV_INCLUDE_GPS && GPS_POWER_SAVE_ACTIVE
  bool gps_setting = false;           // User intent: GPS on/off
  uint8_t gps_saver_mode = 1;         // 0=off, 1=bootonly, 2=periodic
  uint8_t gps_saver_hold = 15;       // 5–240 s
  uint8_t gps_timeout_min = 5;       // 1–15 min
  uint32_t _gps_hold_start_unixtime = 0;
  bool _gps_hold_timer_active = false;
  uint32_t _gps_no_fix_start = 0;    // unix time when GPS powered on
  uint32_t _next_gps_wake_unixtime = 0;  // for periodic mode
  void (*_gps_off_persist_cb)(void*) = nullptr;
  void* _gps_off_persist_user = nullptr;
  #endif

  #if ENV_INCLUDE_GPS
  LocationProvider* _location;
  mesh::RTCClock* _rtc_clock = nullptr;
  void start_gps();
  void stop_gps();
  void initBasicGPS();
  #ifdef RAK_BOARD
  void rakGPSInit();
  bool gpsIsAwake(uint8_t ioPin);
  #endif
  #endif


public:
  #if ENV_INCLUDE_GPS
  EnvironmentSensorManager(LocationProvider &location): _location(&location){};
  LocationProvider* getLocationProvider() { return _location; }
  #else
  EnvironmentSensorManager(){};
  #endif
  bool begin() override;
  bool querySensors(uint8_t requester_permissions, CayenneLPP& telemetry) override;
  #if ENV_INCLUDE_GPS
  void loop() override;
  #endif
  int getNumSettings() const override;
  const char* getSettingName(int i) const override;
  const char* getSettingValue(int i) const override;
  bool setSettingValue(const char* name, const char* value) override;
  void setRTCClock(mesh::RTCClock* rtc) override;
  #if GPS_POWER_SAVE_ACTIVE
  void applyGpsSaverPrefs(uint8_t mode, uint8_t hold, uint8_t timeout_min, uint32_t interval_sec, mesh::RTCClock* rtc);
  void setGpsOffPersistCallback(void (*cb)(void*), void* user) override;
  #endif
};

#pragma once

#include <CayenneLPP.h>
#include "sensors/LocationProvider.h"

/* GPS power-save: active when undefined or non-zero; excluded when GPS_POWER_SAVE=0 (per design) */
#if !defined(GPS_POWER_SAVE) || (GPS_POWER_SAVE != 0)
#define GPS_POWER_SAVE_ACTIVE 1
#else
#define GPS_POWER_SAVE_ACTIVE 0
#endif

#define TELEM_PERM_BASE         0x01   // 'base' permission includes battery
#define TELEM_PERM_LOCATION     0x02
#define TELEM_PERM_ENVIRONMENT  0x04   // permission to access environment sensors

#define TELEM_CHANNEL_SELF   1   // LPP data channel for 'self' device

class SensorManager {
public:
  double node_lat, node_lon;  // modify these, if you want to affect Advert location
  double node_altitude;       // altitude in meters

  SensorManager() { node_lat = 0; node_lon = 0; node_altitude = 0; }
  virtual bool begin() { return false; }
  virtual bool querySensors(uint8_t requester_permissions, CayenneLPP& telemetry) { return false; }
  virtual void loop() { }
  virtual int getNumSettings() const { return 0; }
  virtual const char* getSettingName(int i) const { return NULL; }
  virtual const char* getSettingValue(int i) const { return NULL; }
  virtual bool setSettingValue(const char* name, const char* value) { return false; }
  virtual LocationProvider* getLocationProvider() { return NULL; }
  virtual void setRTCClock(mesh::RTCClock* rtc) { (void)rtc; }

  /// Optional: register callback when GPS is turned off due to no-fix timeout (mode off).
  /// Default no-op; EnvironmentSensorManager overrides when GPS_POWER_SAVE_ACTIVE.
  virtual void setGpsOffPersistCallback(void (*cb)(void*), void* user) { (void)cb; (void)user; }

  // Helper functions to manage setting by keys (useful in many places ...)
  const char* getSettingByKey(const char* key) {
    int num = getNumSettings();
    for (int i = 0; i < num; i++) {
      if (strcmp(getSettingName(i), key) == 0) {
        return getSettingValue(i);
      }
    }
    return NULL;
  }
};

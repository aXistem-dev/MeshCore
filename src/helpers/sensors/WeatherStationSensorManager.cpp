#include "WeatherStationSensorManager.h"
#include <Arduino.h>
#include <helpers/sensors/LPPDataHelpers.h>

// Static instance pointer for ISR access
WeatherStationSensorManager* WeatherStationSensorManager::instance = nullptr;

// Define MAX_WIND_READINGS for linker
constexpr int WeatherStationSensorManager::MAX_WIND_READINGS;

// Wind vane calibration map - calibrated values from actual hardware measurements
const VaneReading WeatherStationSensorManager::vaneMap[] = {
  // S Quadrant (180°-270°) - lowest ADC values
  {50,  70,   180},  // S (180°) - measured: 54-66 ADC
  {115, 145,  225},  // SW (225°) - measured: 122-138 ADC (check before SSW due to overlap)
  {120, 145,  202},  // SSW (202.5°) - measured: 127-140 ADC
  {200, 220,  270},  // W (270°) - measured: 205-213 ADC

  // SE Quadrant (90°-180°)
  {345, 365,  135},  // SE (135°) - measured: 352-357 ADC (check before ESE due to overlap)
  {345, 365,  112},  // ESE (112.5°) - measured: 351-360 ADC
  {655, 675,  90},   // E (90°) - measured: 660-669 ADC

  // NE Quadrant (0°-90°)
  {570, 590,  67},   // ENE (67.5°) - measured: 575-581 ADC
  {760, 790,  45},   // NE (45°) - measured: 769-781 ADC (check before NNE due to overlap)
  {755, 785,  22},   // NNE (22.5°) - measured: 762-779 ADC

  // NW Quadrant (270°-360°) - highest ADC values
  {165, 190,  292},  // WNW (292.5°) - measured: 172-184 ADC
  {500, 550,  315},  // NW (315°) - measured: 503-544 ADC (includes outlier at 544)
  {470, 490,  337},  // NNW (337.5°) - measured: 474-483 ADC
  {825, 850,  0},    // N (0°) - measured: 831-843 ADC (check last to avoid conflicts)
};

const int WeatherStationSensorManager::numVaneReadings = sizeof(WeatherStationSensorManager::vaneMap) / sizeof(VaneReading);

// Interrupt Service Routines
void WeatherStationSensorManager::rainISR() {
  if (instance) {
    static unsigned long lastRainTime = 0;
    unsigned long now = millis();

    if (now - lastRainTime > WEATHER_DEBOUNCE_MS) {
      instance->rainTips++;
      lastRainTime = now;
    }
  }
}

void WeatherStationSensorManager::windISR() {
  if (instance) {
    unsigned long now = millis();

    if (now - instance->lastWindTime > WEATHER_DEBOUNCE_MS) {
      instance->windClicks++;
      instance->lastWindTime = now;
    }
  }
}

bool WeatherStationSensorManager::begin() {
  MESH_DEBUG_PRINTLN("WeatherStationSensorManager::begin() called");

  // Set static instance pointer for ISR access
  instance = this;

  // Initialize wind speed calculation timer
  lastWindCalc = millis();

  // Initialize wind speed history
  windWriteIndex = 0;
  windCount = 0;
  peakWindSpeed_5min = 0.0f;
  lastPeakUpdate = millis();

  // Initialize BME280
  #ifndef WEATHER_BME280_ADDRESS
  #define WEATHER_BME280_ADDRESS 0x76
  #endif

  MESH_DEBUG_PRINTLN("Initializing BME280 on Slot A (I2C1: Wire, pins 13/14)...");

  #ifdef NRF52_PLATFORM
  // Re-initialize Wire for Slot A (SENSOR_SLOT) - I2C1 (Wire, pins 13/14)
  Wire.setPins(13, 14);
  Wire.begin();
  Wire.setClock(100000);
  delay(50);  // Give I2C bus time to stabilize
  MESH_DEBUG_PRINTLN("Wire re-initialized (SDA: 13, SCL: 14, 100kHz)");

  // Scan I2C bus for BME280
  MESH_DEBUG_PRINTLN("Scanning Wire (I2C1) for I2C devices...");
  for (uint8_t addr = 0x76; addr <= 0x77; addr++) {
    Wire.beginTransmission(addr);
    uint8_t error = Wire.endTransmission();
    MESH_DEBUG_PRINTLN("Wire address 0x%02X: %s", addr, error == 0 ? "FOUND" : "not found");
  }

  if (bme.begin(0x76)) {
    MESH_DEBUG_PRINTLN("Found BME280 at 0x76");
    BME280_initialized = true;
  } else if (bme.begin(0x77)) {
    MESH_DEBUG_PRINTLN("Found BME280 at 0x77");
    BME280_initialized = true;
  } else {
    MESH_DEBUG_PRINTLN("BME280 not found at either address");
    BME280_initialized = false;
  }
  #else
  // For non-NRF52, just use Wire
  Wire.begin();
  Wire.setClock(100000);
  delay(50);
  if (bme.begin(0x76)) {
    BME280_initialized = true;
  } else if (bme.begin(0x77)) {
    BME280_initialized = true;
  } else {
    BME280_initialized = false;
  }
  #endif

  if (BME280_initialized) {
    // Configure BME280 for normal mode
    bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                    Adafruit_BME280::SAMPLING_X16,
                    Adafruit_BME280::SAMPLING_X16,
                    Adafruit_BME280::SAMPLING_X16,
                    Adafruit_BME280::FILTER_X16,
                    Adafruit_BME280::STANDBY_MS_0_5);

    MESH_DEBUG_PRINTLN("BME280 sensor ID: 0x%02X", bme.sensorID());
  }

  // Initialize weather station sensors
  pinMode(WEATHER_RAIN_PIN, INPUT_PULLUP);
  pinMode(WEATHER_WIND_PIN, INPUT_PULLUP);
  // Wind vane uses resistor network - needs pullup as voltage reference for voltage divider
  pinMode(WEATHER_VANE_PIN, INPUT_PULLUP);

  // Attach interrupts for rain gauge and anemometer
  attachInterrupt(digitalPinToInterrupt(WEATHER_RAIN_PIN), rainISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(WEATHER_WIND_PIN), windISR, FALLING);

  lastWindCalc = millis();

  MESH_DEBUG_PRINTLN("Weather Station sensors initialized");

  // Return true even if BME280 not found, so mesh can still start
  return true;
}

bool WeatherStationSensorManager::querySensors(uint8_t requester_permissions, CayenneLPP& telemetry) {
  if (!(requester_permissions & TELEM_PERM_ENVIRONMENT)) {
    return false;
  }

  // Channel 1 (TELEM_CHANNEL_SELF): BME280 data
  if (BME280_initialized) {
    float temperature = bme.readTemperature();
    float pressure = bme.readPressure() / 100.0;  // Convert Pa to hPa
    float humidity = bme.readHumidity();
    float altitude = bme.readAltitude(WEATHER_SEA_LEVEL_PRESSURE_HPA);

    telemetry.addTemperature(TELEM_CHANNEL_SELF, temperature);
    telemetry.addBarometricPressure(TELEM_CHANNEL_SELF, pressure);
    telemetry.addRelativeHumidity(TELEM_CHANNEL_SELF, humidity);
    telemetry.addAltitude(TELEM_CHANNEL_SELF, altitude);
  }

  // Channel 2 (TELEM_CHANNEL_SELF+1): Cumulative rain tips + uptime
  telemetry.addGenericSensor(TELEM_CHANNEL_SELF + 1, (float)rainTips);
  telemetry.addUnixTime(TELEM_CHANNEL_SELF + 1, millis() / 1000);

  // Channel 3 (TELEM_CHANNEL_SELF+2): Momentary wind speed + direction
  float windSpeed = getWindSpeed();  // This also stores reading in history
  if (windSpeed < 0.0f) windSpeed = 0.0f;
  if (windSpeed > 327.67f) windSpeed = 327.67f;
  telemetry.addAnalogInput(TELEM_CHANNEL_SELF + 2, windSpeed);

  int windDir = getWindDirection();
  if (windDir < 0) windDir = 0;
  if (windDir > 359) windDir = windDir % 360;
  telemetry.addDirection(TELEM_CHANNEL_SELF + 2, windDir);

  // Channel 4 (TELEM_CHANNEL_SELF+3): Peak wind speed last 5 minutes
  float peakWind = getPeakWindSpeed5Min();
  if (peakWind < 0.0f) peakWind = 0.0f;
  if (peakWind > 327.67f) peakWind = 327.67f;
  telemetry.addAnalogInput(TELEM_CHANNEL_SELF + 3, peakWind);

  return true;
}

void WeatherStationSensorManager::loop() {
  // Update wind speed history periodically (every 5 seconds)
  static unsigned long lastWindUpdate = 0;
  unsigned long now = millis();
  if (now - lastWindUpdate >= 5000) {
    getWindSpeed();  // This calculates and stores current wind speed
    lastWindUpdate = now;
  }
}

float WeatherStationSensorManager::getWindSpeed() {
  unsigned long now = millis();

  // Initialize lastWindCalc on first call
  if (lastWindCalc == 0) {
    lastWindCalc = now;
    return 0.0f;
  }

  float timeDelta = (now - lastWindCalc) / 1000.0;  // Convert to seconds

  // Need at least 0.1 seconds of data for accurate calculation
  if (timeDelta < 0.1) {
    return 0.0f;
  }

  // Read windClicks atomically
  unsigned long clicks = windClicks;

  // Calculate speed
  float clicksPerSecond = (float)clicks / timeDelta;
  float speed = clicksPerSecond * WEATHER_WIND_FACTOR;

  // Clamp to reasonable range (0 to 200 km/h)
  if (speed < 0.0f) speed = 0.0f;
  if (speed > 200.0f) speed = 200.0f;

  // Store reading in history buffer
  updateWindSpeedHistory(speed);

  // Reset for next calculation
  windClicks = 0;
  lastWindCalc = now;

  return speed;
}

void WeatherStationSensorManager::updateWindSpeedHistory(float speed) {
  unsigned long now = millis();

  // Store reading
  int idx = windWriteIndex;
  windHistory[idx].speed_kmh = speed;
  windHistory[idx].timestamp_millis = now;
  windWriteIndex = (idx + 1) % MAX_WIND_READINGS;
  if (windCount < MAX_WIND_READINGS) {
    windCount++;
  }

  // Update peak wind speed for last 5 minutes
  float peak = 0.0f;
  int num_to_check = (windCount < MAX_WIND_READINGS) ? windCount : MAX_WIND_READINGS;
  int start_idx = (windWriteIndex - num_to_check + MAX_WIND_READINGS) % MAX_WIND_READINGS;

  for (int i = 0; i < num_to_check; i++) {
    int check_idx = (start_idx + i) % MAX_WIND_READINGS;
    unsigned long reading_millis = windHistory[check_idx].timestamp_millis;

    // Handle millis() wrap-around
    unsigned long millis_diff;
    if (now >= reading_millis) {
      millis_diff = now - reading_millis;
    } else {
      millis_diff = ((unsigned long)(-1) - reading_millis) + now + 1;
    }

    // Check if reading is within last 5 minutes
    if (millis_diff <= (5 * 60 * 1000)) {
      if (windHistory[check_idx].speed_kmh > peak) {
        peak = windHistory[check_idx].speed_kmh;
      }
    }
  }

  peakWindSpeed_5min = peak;
  lastPeakUpdate = now;
}

float WeatherStationSensorManager::getPeakWindSpeed5Min() const {
  // Return cached peak if recently updated (within last second)
  unsigned long now = millis();
  if (now - lastPeakUpdate < 1000) {
    return peakWindSpeed_5min;
  }

  // Otherwise calculate it
  if (windCount == 0) {
    return 0.0f;
  }

  float peak = 0.0f;
  int num_to_check = (windCount < MAX_WIND_READINGS) ? windCount : MAX_WIND_READINGS;
  int start_idx = (windWriteIndex - num_to_check + MAX_WIND_READINGS) % MAX_WIND_READINGS;

  for (int i = 0; i < num_to_check; i++) {
    int idx = (start_idx + i) % MAX_WIND_READINGS;
    unsigned long reading_millis = windHistory[idx].timestamp_millis;

    // Handle millis() wrap-around
    unsigned long millis_diff;
    if (now >= reading_millis) {
      millis_diff = now - reading_millis;
    } else {
      millis_diff = ((unsigned long)(-1) - reading_millis) + now + 1;
    }

    // Check if reading is within last 5 minutes
    if (millis_diff <= (5 * 60 * 1000)) {
      if (windHistory[idx].speed_kmh > peak) {
        peak = windHistory[idx].speed_kmh;
      }
    }
  }

  return peak;
}

int WeatherStationSensorManager::getWindDirection() const {
  // Set ADC resolution to 10-bit (0-1023) to match calibration table
  analogReadResolution(10);
  int raw = analogRead(WEATHER_VANE_PIN);

  MESH_DEBUG_PRINTLN("getWindDirection - WEATHER_VANE_PIN=%d, raw ADC=%d", WEATHER_VANE_PIN, raw);

  // Find matching direction from lookup table
  for (int i = 0; i < numVaneReadings; i++) {
    if (raw >= vaneMap[i].minADC && raw <= vaneMap[i].maxADC) {
      return vaneMap[i].direction;
    }
  }

  // If no exact match found, find closest direction by ADC value
  int closestIdx = 0;
  int minDiff = 1024;

  for (int i = 0; i < numVaneReadings; i++) {
    int midPoint = (vaneMap[i].minADC + vaneMap[i].maxADC) / 2;
    int diff = abs(raw - midPoint);
    if (diff < minDiff) {
      minDiff = diff;
      closestIdx = i;
    }
  }

  // Return closest direction if within reasonable range (50 ADC units)
  if (minDiff < 50) {
    return vaneMap[closestIdx].direction;
  }

  // Out of range - return 0 (North) as default
  MESH_DEBUG_PRINTLN("getWindDirection - ADC value %d out of range (minDiff=%d), using 0 (North) as default", raw, minDiff);
  return 0;
}

#include "WeatherStationSensorManager.h"
#include <Arduino.h>  // For Serial
#include <helpers/sensors/LPPDataHelpers.h>
#include <limits.h>  // For ULONG_MAX

// Static instance pointer for ISR access
WeatherStationSensorManager* WeatherStationSensorManager::instance = nullptr;

// Define MAX_RAIN_TIPS for linker (even though it's constexpr, some compilers need this)
constexpr int WeatherStationSensorManager::MAX_RAIN_TIPS;

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
      
      // Store timestamp in circular buffer
      int idx = instance->rainTipWriteIndex;
      instance->rainTipHistory[idx].timestamp_millis = now;
      instance->rainTipWriteIndex = (idx + 1) % MAX_RAIN_TIPS;
      if (instance->rainTipCount < MAX_RAIN_TIPS) {
        instance->rainTipCount++;
      }
      
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
  // Use Serial directly for debugging since MESH_DEBUG might not be working
  Serial.println("DEBUG: WeatherStationSensorManager::begin() called");
  Serial.flush();
  
  // Set static instance pointer for ISR access
  instance = this;
  Serial.println("DEBUG: Instance pointer set");
  Serial.flush();
  
  // Initialize rain tip history
  rainTipWriteIndex = 0;
  rainTipCount = 0;
  lastMidnightMillis = 0;
  lastMidnightUnix = 0;
  
  // Initialize wind speed calculation timer
  lastWindCalc = millis();
  
  // Initialize wind speed history
  windWriteIndex = 0;
  windCount = 0;
  peakWindSpeed_5min = 0.0f;
  lastPeakUpdate = millis();
  
  // Initialize BME280 following the pattern from bme280_reader example
  // Re-initialize Wire to ensure it's in the correct state
  #ifndef WEATHER_BME280_ADDRESS
  #define WEATHER_BME280_ADDRESS 0x76
  #endif
  
  // Initialize BME280 - RAK13009 on Slot A (SENSOR_SLOT) uses I2C1 (Wire, pins 13/14)
  // RTC is on Slot B (different slot), so BME280 is definitely on Wire
  Serial.println("DEBUG: Initializing BME280 on Slot A (I2C1: Wire, pins 13/14)...");
  Serial.flush();
  
  #ifdef NRF52_PLATFORM
  // Re-initialize Wire exactly like the original bme280_reader example
  // Slot A (SENSOR_SLOT) uses I2C1 (Wire, pins 13/14)
  Wire.setPins(13, 14);
  Wire.begin();
  Wire.setClock(100000);
  delay(50);  // Give I2C bus time to stabilize after re-initialization
  Serial.println("DEBUG: Wire re-initialized (SDA: 13, SCL: 14, 100kHz)");
  Serial.flush();
  
  // Test I2C bus by scanning before trying bme.begin()
  Serial.println("DEBUG: Scanning Wire (I2C1) for I2C devices...");
  Serial.flush();
  for (uint8_t addr = 0x76; addr <= 0x77; addr++) {
    Wire.beginTransmission(addr);
    uint8_t error = Wire.endTransmission();
    Serial.print("DEBUG: Wire address 0x");
    Serial.print(addr, HEX);
    Serial.print(": ");
    Serial.println(error == 0 ? "FOUND" : "not found");
    Serial.flush();
  }
  
  // Call bme.begin() WITHOUT Wire parameter (like original example line 172)
  // This uses the default Wire instance
  Serial.println("DEBUG: Trying bme.begin(0x76) without Wire parameter...");
  Serial.flush();
  if (bme.begin(0x76)) {
    Serial.println("DEBUG: Found BME280 at 0x76!");
    Serial.flush();
    BME280_initialized = true;
  } else {
    Serial.println("DEBUG: BME280 not at 0x76, trying 0x77...");
    Serial.flush();
    if (bme.begin(0x77)) {
      Serial.println("DEBUG: Found BME280 at 0x77!");
      Serial.flush();
      BME280_initialized = true;
    } else {
      Serial.println("DEBUG: BME280 not found at either address");
      Serial.flush();
      BME280_initialized = false;
    }
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
    
    Serial.print("DEBUG: BME280 sensor ID: 0x");
    Serial.println(bme.sensorID(), HEX);
    Serial.flush();
  }
  
  // Initialize weather station sensors
  pinMode(WEATHER_RAIN_PIN, INPUT_PULLUP);
  pinMode(WEATHER_WIND_PIN, INPUT_PULLUP);
  // Wind vane uses resistor network - needs pullup as voltage reference for voltage divider
  // (Now on WB_A1, no longer conflicts with battery voltage on WB_A0)
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
  // Default implementation without RTC - can't calculate time-based intervals
  // This should not be called, but provide fallback
  return querySensors(requester_permissions, telemetry, nullptr);
}

bool WeatherStationSensorManager::querySensors(uint8_t requester_permissions, CayenneLPP& telemetry, mesh::RTCClock* rtc) {
  if (!(requester_permissions & TELEM_PERM_ENVIRONMENT)) {
    return false;
  }
  
  // BME280 readings (all on TELEM_CHANNEL_SELF - different types can share channel)
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
  
  // Weather station readings
  // Separate channels for each rainfall interval (CayenneLPP only supports one value per type per channel)
  if (rtc) {
    uint32_t now_unix = rtc->getCurrentTime();
    
    // Channel 2: Rainfall last 10 minutes + Average wind speed last 10 minutes
    uint32_t ten_min_ago = now_unix - (10 * 60);
    float rainfall_10min = getRainfallForInterval(ten_min_ago, now_unix, rtc);
    telemetry.addDistance(TELEM_CHANNEL_SELF + 1, rainfall_10min / 1000.0f);  // Convert mm to meters
    float avgWind_10min = getAverageWindSpeedForInterval(ten_min_ago, now_unix, rtc);
    if (avgWind_10min < 0.0f) avgWind_10min = 0.0f;
    if (avgWind_10min > 327.67f) avgWind_10min = 327.67f;
    telemetry.addAnalogInput(TELEM_CHANNEL_SELF + 1, avgWind_10min);
    
    // Channel 3: Rainfall last 1 hour + Average wind speed last 1 hour
    uint32_t one_hour_ago = now_unix - (60 * 60);
    float rainfall_1hr = getRainfallForInterval(one_hour_ago, now_unix, rtc);
    telemetry.addDistance(TELEM_CHANNEL_SELF + 2, rainfall_1hr / 1000.0f);
    float avgWind_1hr = getAverageWindSpeedForInterval(one_hour_ago, now_unix, rtc);
    if (avgWind_1hr < 0.0f) avgWind_1hr = 0.0f;
    if (avgWind_1hr > 327.67f) avgWind_1hr = 327.67f;
    telemetry.addAnalogInput(TELEM_CHANNEL_SELF + 2, avgWind_1hr);
    
    // Channel 4: Rainfall last 24 hours + Average wind speed last 24 hours
    uint32_t twenty_four_hr_ago = now_unix - (24 * 60 * 60);
    float rainfall_24hr = getRainfallForInterval(twenty_four_hr_ago, now_unix, rtc);
    telemetry.addDistance(TELEM_CHANNEL_SELF + 3, rainfall_24hr / 1000.0f);
    float avgWind_24hr = getAverageWindSpeedForInterval(twenty_four_hr_ago, now_unix, rtc);
    if (avgWind_24hr < 0.0f) avgWind_24hr = 0.0f;
    if (avgWind_24hr > 327.67f) avgWind_24hr = 327.67f;
    telemetry.addAnalogInput(TELEM_CHANNEL_SELF + 3, avgWind_24hr);
    
    // Channel 5: Rainfall current day (midnight to now) + Average wind speed current day
    uint32_t midnight_unix = getMidnightUnix(rtc);
    float rainfall_today = getRainfallForInterval(midnight_unix, now_unix, rtc);
    telemetry.addDistance(TELEM_CHANNEL_SELF + 4, rainfall_today / 1000.0f);
    float avgWind_today = getAverageWindSpeedForInterval(midnight_unix, now_unix, rtc);
    if (avgWind_today < 0.0f) avgWind_today = 0.0f;
    if (avgWind_today > 327.67f) avgWind_today = 327.67f;
    telemetry.addAnalogInput(TELEM_CHANNEL_SELF + 4, avgWind_today);
  } else {
    // Fallback: total cumulative rainfall if no RTC
    float rainfall = getRainfall();
    telemetry.addDistance(TELEM_CHANNEL_SELF + 1, rainfall / 1000.0f);
  }
  
  // Channel 6: Current momentary wind speed + Wind direction
  // Wind speed in km/h (using LPP_ANALOG_INPUT type: 2 bytes, 0.01 resolution)
  float windSpeed = getWindSpeed();  // This also stores reading in history
  
  // Clamp to valid range for signed 16-bit: -327.68 to +327.67
  if (windSpeed < 0.0f) windSpeed = 0.0f;
  if (windSpeed > 327.67f) windSpeed = 327.67f;
  
  telemetry.addAnalogInput(TELEM_CHANNEL_SELF + 5, windSpeed);
  
  // Wind direction in degrees (using LPP_DIRECTION type: 2 bytes, 1deg resolution)
  int windDir = getWindDirection();
  if (windDir < 0) windDir = 0;
  if (windDir > 359) windDir = windDir % 360;
  telemetry.addDirection(TELEM_CHANNEL_SELF + 5, windDir);
  
  // Channel 7: Peak wind speed in last 5 minutes
  if (rtc) {
    float peakWind = getPeakWindSpeed5Min(rtc);
    if (peakWind < 0.0f) peakWind = 0.0f;
    if (peakWind > 327.67f) peakWind = 327.67f;
    telemetry.addAnalogInput(TELEM_CHANNEL_SELF + 6, peakWind);
  }
  
  return true;
}

void WeatherStationSensorManager::loop() {
  // Update wind speed history periodically (every 5 seconds)
  // This builds up the history buffer for interval calculations
  static unsigned long lastWindUpdate = 0;
  unsigned long now = millis();
  if (now - lastWindUpdate >= 5000) {  // Every 5 seconds
    getWindSpeed();  // This calculates and stores current wind speed
    lastWindUpdate = now;
  }
}

int WeatherStationSensorManager::getNumSettings() const {
  return 1;  // timezone_offset
}

const char* WeatherStationSensorManager::getSettingName(int i) const {
  if (i == 0) {
    return "timezone_offset";
  }
  return NULL;
}

const char* WeatherStationSensorManager::getSettingValue(int i) const {
  if (i == 0) {
    // Return timezone offset as string (in seconds)
    // We need a static buffer for this - using a workaround with sprintf
    static char buf[16];
    sprintf(buf, "%ld", (long)timezone_offset_seconds);
    return buf;
  }
  return NULL;
}

bool WeatherStationSensorManager::setSettingValue(const char* name, const char* value) {
  if (strcmp(name, "timezone_offset") == 0) {
    // Parse timezone offset (in seconds from UTC)
    // Accepts format: seconds (e.g., "19800" for UTC+5:30, "-18000" for UTC-5)
    // Or hours (e.g., "5.5" for UTC+5:30, "-5" for UTC-5)
    char* endptr;
    float hours = strtof(value, &endptr);
    
    // If value contains a decimal point or is a small number, treat as hours
    if (strchr(value, '.') != NULL || (hours >= -24 && hours <= 24 && *endptr == '\0')) {
      // Convert hours to seconds
      timezone_offset_seconds = (int32_t)(hours * 3600.0f);
    } else {
      // Treat as seconds directly
      timezone_offset_seconds = (int32_t)strtol(value, &endptr, 10);
    }
    
    // Clamp to reasonable range: -12 to +14 hours
    if (timezone_offset_seconds < -43200) timezone_offset_seconds = -43200;  // UTC-12
    if (timezone_offset_seconds > 50400) timezone_offset_seconds = 50400;    // UTC+14
    
    return true;
  }
  return false;
}

float WeatherStationSensorManager::getWindSpeed() {
  unsigned long now = millis();
  
  // Initialize lastWindCalc on first call
  if (lastWindCalc == 0) {
    lastWindCalc = now;
    return 0.0f;  // No data yet
  }
  
  float timeDelta = (now - lastWindCalc) / 1000.0;  // Convert to seconds
  
  // Need at least 0.1 seconds of data for accurate calculation
  if (timeDelta < 0.1) {
    return 0.0f;  // Avoid division by very small numbers
  }
  
  // Read windClicks atomically (it's volatile, so this should be safe)
  unsigned long clicks = windClicks;
  
  // Calculate speed
  float clicksPerSecond = (float)clicks / timeDelta;
  float speed = clicksPerSecond * WEATHER_WIND_FACTOR;
  
  // Clamp to reasonable range (0 to 200 km/h)
  if (speed < 0.0f) speed = 0.0f;
  if (speed > 200.0f) speed = 200.0f;
  
  // Store reading in history buffer (for interval calculations)
  updateWindSpeedHistory(speed);
  
  // Reset for next calculation (atomic operation)
  windClicks = 0;
  lastWindCalc = now;
  
  return speed;
}

// Store wind speed reading in circular buffer
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
  unsigned long five_min_ago = now - (5 * 60 * 1000);
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

// Calculate average wind speed for a time interval
float WeatherStationSensorManager::getAverageWindSpeedForInterval(uint32_t start_unix, uint32_t end_unix, mesh::RTCClock* rtc) const {
  if (!rtc || windCount == 0) {
    return 0.0f;
  }
  
  uint32_t now_unix = rtc->getCurrentTime();
  unsigned long now_millis = millis();
  
  float sum = 0.0f;
  int count = 0;
  int num_to_check = (windCount < MAX_WIND_READINGS) ? windCount : MAX_WIND_READINGS;
  int start_idx = (windWriteIndex - num_to_check + MAX_WIND_READINGS) % MAX_WIND_READINGS;
  
  for (int i = 0; i < num_to_check; i++) {
    int idx = (start_idx + i) % MAX_WIND_READINGS;
    unsigned long reading_millis = windHistory[idx].timestamp_millis;
    
    // Convert millis to approximate unix time
    unsigned long millis_diff;
    if (now_millis >= reading_millis) {
      millis_diff = now_millis - reading_millis;
    } else {
      millis_diff = ((unsigned long)(-1) - reading_millis) + now_millis + 1;
    }
    
    uint32_t reading_unix = now_unix - (millis_diff / 1000);
    
    // Check if reading is within interval
    if (reading_unix >= start_unix && reading_unix <= end_unix) {
      sum += windHistory[idx].speed_kmh;
      count++;
    }
  }
  
  if (count == 0) {
    return 0.0f;
  }
  
  return sum / count;
}

// Get peak wind speed in last 5 minutes
float WeatherStationSensorManager::getPeakWindSpeed5Min(mesh::RTCClock* rtc) const {
  // Return cached peak if recently updated (within last second)
  unsigned long now = millis();
  if (now - lastPeakUpdate < 1000) {
    return peakWindSpeed_5min;
  }
  
  // Otherwise calculate it
  if (windCount == 0) {
    return 0.0f;
  }
  
  unsigned long five_min_ago = now - (5 * 60 * 1000);
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
  // RAK4631Board::getBattMilliVolts() sets it to 12-bit, so we need to reset it
  analogReadResolution(10);
  int raw = analogRead(WEATHER_VANE_PIN);
  
  Serial.printf("DEBUG: getWindDirection - WEATHER_VANE_PIN=%d, raw ADC=%d\n", WEATHER_VANE_PIN, raw);
  
  // Find matching direction from lookup table
  // Table is ordered so more specific ranges are checked first
  for (int i = 0; i < numVaneReadings; i++) {
    if (raw >= vaneMap[i].minADC && raw <= vaneMap[i].maxADC) {
      return vaneMap[i].direction;
    }
  }
  
  // If no exact match found, find closest direction by ADC value
  // This handles values between calibrated ranges
  int closestIdx = 0;
  int minDiff = 1024;  // Max ADC value
  
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
  
  // Out of range - return 0 (North) as default instead of -1
  // This ensures the channel is always present in telemetry
  Serial.printf("DEBUG: getWindDirection - ADC value %d out of range (minDiff=%d), using 0 (North) as default\n", raw, minDiff);
  return 0;  // Default to North if out of range
}

// Calculate rainfall for a specific time interval
float WeatherStationSensorManager::getRainfallForInterval(uint32_t start_unix, uint32_t end_unix, mesh::RTCClock* rtc) const {
  if (!rtc || rainTipCount == 0) {
    return 0.0f;
  }
  
  // Get current unix time and millis for conversion
  uint32_t now_unix = rtc->getCurrentTime();
  unsigned long now_millis = millis();
  
  // Convert unix timestamps to millis offsets
  // We approximate: millis_offset = (unix_time - now_unix) * 1000 + now_millis
  // But we need to be careful about wrap-around and precision
  
  int count = 0;
  int num_to_check = (rainTipCount < MAX_RAIN_TIPS) ? rainTipCount : MAX_RAIN_TIPS;
  
  // Read from circular buffer (oldest to newest)
  // Buffer wraps, so we need to read from (writeIndex - count) to writeIndex
  int start_idx = (rainTipWriteIndex - num_to_check + MAX_RAIN_TIPS) % MAX_RAIN_TIPS;
  
  for (int i = 0; i < num_to_check; i++) {
    int idx = (start_idx + i) % MAX_RAIN_TIPS;
    unsigned long tip_millis = rainTipHistory[idx].timestamp_millis;
    
    // Convert millis to approximate unix time
    // Estimate: tip_unix ≈ now_unix - (now_millis - tip_millis) / 1000
    // Handle millis() wrap-around
    unsigned long millis_diff;
    if (now_millis >= tip_millis) {
      millis_diff = now_millis - tip_millis;
    } else {
      // Handle wrap-around (happens every ~49 days on nRF52)
      // millis() wraps from max value back to 0
      millis_diff = ((unsigned long)(-1) - tip_millis) + now_millis + 1;
    }
    
    uint32_t tip_unix = now_unix - (millis_diff / 1000);
    
    // Check if tip is within interval
    if (tip_unix >= start_unix && tip_unix <= end_unix) {
      count++;
    }
  }
  
  return count * WEATHER_MM_PER_TIP;
}

// Get unix timestamp of midnight (start of current day) in local time
uint32_t WeatherStationSensorManager::getMidnightUnix(mesh::RTCClock* rtc) const {
  if (!rtc) {
    return 0;
  }
  
  uint32_t now_unix = rtc->getCurrentTime();
  
  // Apply timezone offset to get local time
  int64_t now_local = (int64_t)now_unix + timezone_offset_seconds;
  
  // Calculate seconds since midnight in local time
  // Day length: 86400 seconds
  // Use modulo with proper handling for negative values
  int64_t seconds_since_midnight = now_local % 86400;
  if (seconds_since_midnight < 0) {
    seconds_since_midnight += 86400;  // Handle negative modulo
  }
  
  // Calculate local midnight, then convert back to UTC
  int64_t midnight_local = now_local - seconds_since_midnight;
  uint32_t midnight_unix = (uint32_t)(midnight_local - timezone_offset_seconds);
  
  return midnight_unix;
}

// Calculate min/max/avg rainfall statistics for a time interval
void WeatherStationSensorManager::getRainfallStats(uint32_t start_secs_ago, uint32_t end_secs_ago, 
                                                    mesh::RTCClock* rtc, float& min, float& max, float& avg) const {
  if (!rtc || rainTipCount == 0) {
    min = max = avg = 0.0f;
    return;
  }
  
  uint32_t now_unix = rtc->getCurrentTime();
  uint32_t start_unix = now_unix - start_secs_ago;
  uint32_t end_unix = now_unix - end_secs_ago;
  
  // Get all rainfall values in the interval
  unsigned long now_millis = millis();
  int num_to_check = (rainTipCount < MAX_RAIN_TIPS) ? rainTipCount : MAX_RAIN_TIPS;
  int start_idx = (rainTipWriteIndex - num_to_check + MAX_RAIN_TIPS) % MAX_RAIN_TIPS;
  
  // Calculate rainfall for each time window within the interval
  // For simplicity, calculate total rainfall in the interval
  // Min/Max/Avg for rainfall over time would require sub-intervals
  // For now, return the total as avg, and min/max as the same (since it's cumulative)
  float total_rainfall = getRainfallForInterval(start_unix, end_unix, rtc);
  
  // For rainfall, min/max/avg are all the same (it's cumulative)
  // In a more sophisticated implementation, we could calculate rate over sub-intervals
  min = max = avg = total_rainfall;
}


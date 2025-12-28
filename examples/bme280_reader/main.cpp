#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BME280.h>

// ============================================================================
// BME280 CONFIGURATION
// ============================================================================
#define BME280_ADDRESS 0x76
#define SEA_LEVEL_PRESSURE_HPA 1028.6  // Calibrated for 50.5m ASL location

// For RAK4631 Qwic connector, typically uses main I2C bus (Wire) on pins 13/14
// If your Qwic connector uses Wire1, uncomment the following and set the pins:
// #define USE_WIRE1
// #define QWIC_SDA 24
// #define QWIC_SCL 25

Adafruit_BME280 bme;

// ============================================================================
// WEATHER STATION PIN DEFINITIONS (WisBlock IO Module)
// ============================================================================
// Pin Definitions - Using actual IO pins from the headers
#define RAIN_PIN     WB_IO1    // Rain gauge -> IO1 header pin
#define WIND_PIN     WB_IO3    // Anemometer -> IO3 header pin
#define VANE_PIN     WB_A0     // Wind vane -> AIN0 header pin
#define REF_VOLTAGE  3.3       // Reference voltage

// ============================================================================
// RAIN GAUGE VARIABLES
// ============================================================================
volatile unsigned long rainTips = 0;
const float MM_PER_TIP = 0.2794;  // Each tip = 0.2794mm of rain

// ============================================================================
// ANEMOMETER VARIABLES
// ============================================================================
volatile unsigned long windClicks = 0;
volatile unsigned long lastWindTime = 0;
unsigned long lastWindCalc = 0;
const float WIND_FACTOR = 2.4;  // km/h per click/second
const unsigned long DEBOUNCE_MS = 10;

// ============================================================================
// WIND VANE VARIABLES
// ============================================================================
bool calibrationMode = false;

// Wind vane direction mapping structure
struct VaneReading {
  int minADC;
  int maxADC;
  int direction;
};

// Calibrated values based on actual hardware measurements from Calibration.txt
// All values are from manual observations with the wind vane physically pointed
// at each direction. Ranges include margins for noise and variation.
//
// Note: Order matters - more specific ranges should come first to avoid conflicts
// when ranges overlap. The getWindDirection() function will snap to the closest
// observed direction if a value falls between ranges.
VaneReading vaneMap[] = {
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

// ============================================================================
// INTERRUPT SERVICE ROUTINES
// ============================================================================

// Rain gauge ISR - counts bucket tips
void rainISR() {
  static unsigned long lastRainTime = 0;
  unsigned long now = millis();
  
  if (now - lastRainTime > DEBOUNCE_MS) {
    rainTips++;
    lastRainTime = now;
  }
}

// Anemometer ISR - counts wind rotations
void windISR() {
  unsigned long now = millis();
  
  if (now - lastWindTime > DEBOUNCE_MS) {
    windClicks++;
    lastWindTime = now;
  }
}

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================
void handleSerialCommand(char cmd);
void runCalibrationMode();
void runNormalMode();
void printStatistics();
float getRainfall();
float getWindSpeed();
int getWindDirection();
String getCardinalDirection(int degrees);

// ============================================================================
// SETUP
// ============================================================================
void setup() {
  // Initialize serial - wait for it to be ready on nRF52
  Serial.begin(115200);
  while (!Serial && millis() < 5000) {
    delay(10);
  }
  delay(1000);  // Additional delay for serial monitor to connect
  
  Serial.println();
  Serial.println("=====================================");
  Serial.println("  Complete Weather Station");
  Serial.println("  RAK4631 + BME280 + Weather Meter");
  Serial.println("=====================================");
  Serial.flush();
  
  // ========================================================================
  // Initialize BME280
  // ========================================================================
#ifdef USE_WIRE1
  Wire1.setPins(QWIC_SDA, QWIC_SCL);
  Wire1.begin();
  Wire1.setClock(100000);
  Serial.print("Initializing BME280 on Wire1 (SDA: ");
  Serial.print(QWIC_SDA);
  Serial.print(", SCL: ");
  Serial.print(QWIC_SCL);
  Serial.println(")...");
  Serial.flush();
  
  if (!bme.begin(BME280_ADDRESS, &Wire1)) {
    if (!bme.begin(0x77, &Wire1)) {
      Serial.println("ERROR: BME280 not found!");
      while (1) {
        delay(1000);
        Serial.println("Waiting for BME280...");
        Serial.flush();
      }
    }
  }
#else
  Wire.setPins(13, 14);
  Wire.begin();
  Wire.setClock(100000);
  Serial.println("Initializing BME280 on I2C bus (SDA: 13, SCL: 14)...");
  Serial.flush();
  
  if (!bme.begin(BME280_ADDRESS)) {
    if (!bme.begin(0x77)) {
      Serial.println("ERROR: BME280 not found!");
      Serial.println("Continuing without BME280...");
    } else {
      Serial.println("Found BME280 at address 0x77!");
    }
  } else {
    Serial.println("BME280 sensor found!");
    Serial.print("Sensor ID: 0x");
    Serial.println(bme.sensorID(), HEX);
  }
#endif

  // Configure BME280 for normal mode
  bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                  Adafruit_BME280::SAMPLING_X16,
                  Adafruit_BME280::SAMPLING_X16,
                  Adafruit_BME280::SAMPLING_X16,
                  Adafruit_BME280::FILTER_X16,
                  Adafruit_BME280::STANDBY_MS_0_5);
  
  // ========================================================================
  // Initialize Weather Station Sensors
  // ========================================================================
  Serial.println();
  Serial.println("Initializing Weather Station sensors...");
  Serial.print("Rain Gauge: WB_IO1 (pin ");
  Serial.print(RAIN_PIN);
  Serial.println(")");
  Serial.print("Anemometer: WB_IO3 (pin ");
  Serial.print(WIND_PIN);
  Serial.println(")");
  Serial.print("Wind Vane:  WB_A0 (pin ");
  Serial.print(VANE_PIN);
  Serial.println(")");
  
  // Configure GPIO pins with pull-ups for rain and wind
  pinMode(RAIN_PIN, INPUT_PULLUP);
  pinMode(WIND_PIN, INPUT_PULLUP);
  
  // Configure wind vane ADC pin with internal pull-up
  pinMode(VANE_PIN, INPUT_PULLUP);
  
  // Attach interrupts for rain gauge and anemometer
  attachInterrupt(digitalPinToInterrupt(RAIN_PIN), rainISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(WIND_PIN), windISR, FALLING);
  
  lastWindCalc = millis();
  
  Serial.println();
  Serial.println("=====================================");
  Serial.println("Commands:");
  Serial.println("  'c' - Enter calibration mode (wind vane)");
  Serial.println("  'r' - Run normal mode");
  Serial.println("  's' - Show current statistics");
  Serial.println("  'z' - Zero rainfall counter");
  Serial.println("=====================================");
  Serial.println();
  Serial.println("Starting weather station...");
  Serial.println();
  Serial.flush();
}

// ============================================================================
// MAIN LOOP
// ============================================================================
void loop() {
  // Check for serial commands
  if (Serial.available()) {
    char cmd = Serial.read();
    handleSerialCommand(cmd);
  }
  
  if (calibrationMode) {
    runCalibrationMode();
  } else {
    runNormalMode();
  }
}

// ============================================================================
// SERIAL COMMAND HANDLER
// ============================================================================
void handleSerialCommand(char cmd) {
  switch(cmd) {
    case 'c':
    case 'C':
      calibrationMode = true;
      Serial.println("\n=== CALIBRATION MODE ACTIVE ===");
      Serial.println("Rotate wind vane to each direction.");
      Serial.println("Note the ADC value for each position.");
      Serial.println("Send 'r' when done to return to normal mode.");
      Serial.println("===============================\n");
      break;
      
    case 'r':
    case 'R':
      calibrationMode = false;
      Serial.println("\n=== NORMAL MODE ACTIVE ===\n");
      lastWindCalc = millis();
      break;
      
    case 's':
    case 'S':
      printStatistics();
      break;
      
    case 'z':
    case 'Z':
      rainTips = 0;
      Serial.println("Rainfall counter zeroed.");
      break;
      
    default:
      // Ignore other characters
      break;
  }
}

// ============================================================================
// CALIBRATION MODE
// ============================================================================
void runCalibrationMode() {
  int raw = analogRead(VANE_PIN);
  float voltage = (raw / 1023.0) * REF_VOLTAGE;
  
  Serial.print("ADC: ");
  Serial.print(raw);
  Serial.print("\t| Voltage: ");
  Serial.print(voltage, 3);
  Serial.print("V\t| Estimated Dir: ");
  
  int dir = getWindDirection();
  if (dir >= 0) {
    Serial.print(dir);
    Serial.print("° (");
    Serial.print(getCardinalDirection(dir));
    Serial.println(")");
  } else {
    Serial.println("OUT OF RANGE");
  }
  
  delay(500);
}

// ============================================================================
// NORMAL MODE
// ============================================================================
void runNormalMode() {
  delay(5000);  // Update every 5 seconds
  
  // Read BME280 data
  float temperature = bme.readTemperature();
  float pressure = bme.readPressure() / 100.0;  // Convert Pa to hPa
  float humidity = bme.readHumidity();
  float altitude = bme.readAltitude(SEA_LEVEL_PRESSURE_HPA);
  
  // Read weather station data
  float rainfall = getRainfall();
  float windSpeed = getWindSpeed();
  int windDir = getWindDirection();
  
  // Print comprehensive weather data
  Serial.println("========================================");
  Serial.println("=== COMPLETE WEATHER DATA ===");
  Serial.println("========================================");
  
  // BME280 Data
  Serial.println("--- Environmental (BME280) ---");
  Serial.print("Temperature: ");
  Serial.print(temperature, 2);
  Serial.println(" °C");
  
  Serial.print("Pressure:    ");
  Serial.print(pressure, 2);
  Serial.println(" hPa");
  
  Serial.print("Humidity:    ");
  Serial.print(humidity, 2);
  Serial.println(" %");
  
  Serial.print("Altitude:    ");
  Serial.print(altitude, 2);
  Serial.println(" m ASL");
  
  // Weather Station Data
  Serial.println("--- Weather Station ---");
  Serial.print("Rainfall:    ");
  Serial.print(rainfall, 2);
  Serial.println(" mm");
  
  Serial.print("Wind Speed:  ");
  Serial.print(windSpeed, 1);
  Serial.print(" km/h");
  
  Serial.print(" @ ");
  if (windDir >= 0) {
    Serial.print(windDir);
    Serial.print("° (");
    Serial.print(getCardinalDirection(windDir));
    Serial.println(")");
  } else {
    Serial.println("???");
  }
  
  Serial.println("========================================");
  Serial.println();
  Serial.flush();
}

// ============================================================================
// STATISTICS PRINTER
// ============================================================================
void printStatistics() {
  Serial.println("\n=== CURRENT STATISTICS ===");
  
  // BME280 Stats
  float temperature = bme.readTemperature();
  float pressure = bme.readPressure() / 100.0;
  float humidity = bme.readHumidity();
  float altitude = bme.readAltitude(SEA_LEVEL_PRESSURE_HPA);
  
  Serial.println("--- BME280 ---");
  Serial.print("Temperature: ");
  Serial.print(temperature, 2);
  Serial.println(" °C");
  Serial.print("Pressure:    ");
  Serial.print(pressure, 2);
  Serial.println(" hPa");
  Serial.print("Humidity:    ");
  Serial.print(humidity, 2);
  Serial.println(" %");
  Serial.print("Altitude:    ");
  Serial.print(altitude, 2);
  Serial.println(" m ASL");
  
  // Weather Station Stats
  Serial.println("--- Weather Station ---");
  Serial.print("Total Rain Tips: ");
  Serial.println(rainTips);
  Serial.print("Total Rainfall: ");
  Serial.print(getRainfall(), 2);
  Serial.println(" mm");
  Serial.print("Wind Clicks (current period): ");
  Serial.println(windClicks);
  Serial.print("Current Wind Speed: ");
  Serial.print(getWindSpeed(), 1);
  Serial.println(" km/h");
  Serial.print("Current Wind Direction: ");
  int dir = getWindDirection();
  if (dir >= 0) {
    Serial.print(dir);
    Serial.print("° (");
    Serial.print(getCardinalDirection(dir));
    Serial.println(")");
  } else {
    Serial.println("Unknown");
  }
  Serial.println("=========================\n");
}

// ============================================================================
// SENSOR READING FUNCTIONS
// ============================================================================

// Get total rainfall in mm
float getRainfall() {
  return rainTips * MM_PER_TIP;
}

// Calculate wind speed in km/h
float getWindSpeed() {
  unsigned long now = millis();
  float timeDelta = (now - lastWindCalc) / 1000.0;  // Convert to seconds
  
  if (timeDelta < 0.1) {
    return 0;  // Avoid division by very small numbers
  }
  
  float clicksPerSecond = windClicks / timeDelta;
  float speed = clicksPerSecond * WIND_FACTOR;
  
  // Reset for next calculation
  windClicks = 0;
  lastWindCalc = now;
  
  return speed;
}

// Get wind direction in degrees (0-359)
// Snaps to closest direction with observed calibration data
int getWindDirection() {
  int raw = analogRead(VANE_PIN);
  
  // Find matching direction from lookup table
  // Table is ordered so more specific ranges are checked first
  int numReadings = sizeof(vaneMap) / sizeof(VaneReading);
  for (int i = 0; i < numReadings; i++) {
    if (raw >= vaneMap[i].minADC && raw <= vaneMap[i].maxADC) {
      return vaneMap[i].direction;
    }
  }
  
  // If no exact match found, find closest direction by ADC value
  // This handles values between calibrated ranges
  int closestIdx = 0;
  int minDiff = 1024;  // Max ADC value
  
  for (int i = 0; i < numReadings; i++) {
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
  
  // If no match found, return -1 to indicate error
  return -1;
}

// Convert degrees to cardinal direction string
String getCardinalDirection(int degrees) {
  if (degrees < 0) return "???";
  
  const char* directions[] = {
    "N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE",
    "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"
  };
  
  int index = ((degrees + 11) / 22) % 16;
  return String(directions[index]);
}

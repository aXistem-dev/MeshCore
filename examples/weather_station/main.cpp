#include "SensorMesh.h"
#include <helpers/sensors/LPPDataHelpers.h>
#include <helpers/sensors/WeatherStationSensorManager.h>

// Verify we're using the correct target.h
#ifndef WEATHER_STATION_BUILD
#define WEATHER_STATION_BUILD
#endif

#ifdef DISPLAY_CLASS
  #include "UITask.h"
  static UITask ui_task(display);
#endif

class WeatherStationMesh : public SensorMesh {
public:
  WeatherStationMesh(mesh::MainBoard& board, mesh::Radio& radio, mesh::MillisecondClock& ms, mesh::RNG& rng, mesh::RTCClock& rtc, mesh::MeshTables& tables)
     : SensorMesh(board, radio, ms, rng, rtc, tables)
  {
  }

protected:
  /* ========================== custom logic here ========================== */
  void onSensorDataRead() override {
    // Weather station data is automatically read by the sensor manager
    // You can add custom alert logic here if needed
  }

  int querySeriesData(uint32_t start_secs_ago, uint32_t end_secs_ago, MinMaxAvg dest[], int max_num) override {
    // Return rainfall statistics for the requested time interval
    // Channel 2: Rainfall (LPP_DISTANCE type)
    if (max_num >= 1) {
      float min_rain, max_rain, avg_rain;
      sensors.getRainfallStats(start_secs_ago, end_secs_ago, getRTCClock(), min_rain, max_rain, avg_rain);
      
      dest[0]._channel = TELEM_CHANNEL_SELF + 1;  // Channel 2
      dest[0]._lpp_type = LPP_DISTANCE;
      dest[0]._min = min_rain / 1000.0f;  // Convert mm to meters
      dest[0]._max = max_rain / 1000.0f;
      dest[0]._avg = avg_rain / 1000.0f;
      return 1;
    }
    return 0;
  }

  bool handleCustomCommand(uint32_t sender_timestamp, char* command, char* reply) override {
    // Custom commands for weather station
    WeatherStationSensorManager* ws = (WeatherStationSensorManager*)&sensors;
    
    if (strcmp(command, "rain") == 0) {
      float rainfall = ws->getRainTips() * 0.2794f;  // MM_PER_TIP
      sprintf(reply, "Rainfall: %.2f mm (%lu tips)", rainfall, ws->getRainTips());
      return true;
    }
    if (strcmp(command, "wind") == 0) {
      sprintf(reply, "Wind clicks: %lu", ws->getWindClicks());
      return true;
    }
    if (strcmp(command, "reset_rain") == 0) {
      ws->resetRainfall();
      strcpy(reply, "Rainfall counter reset");
      return true;
    }
    return false;  // not handled
  }
  /* ======================================================================= */
};

StdRNG fast_rng;
SimpleMeshTables tables;

WeatherStationMesh the_mesh(board, radio_driver, *new ArduinoMillis(), fast_rng, rtc_clock, tables);

void halt() {
  Serial.println("HALTED!");
  Serial.flush();
  while (1) {
    delay(1000);
  }
}

static char command[160];

void setup() {
  // Initialize Serial - critical for nRF52
  Serial.begin(115200);
  
  // Wait for Serial to be ready on nRF52
  unsigned long start = millis();
  while (!Serial && (millis() - start < 5000)) {
    delay(10);
  }
  
  // Additional delay for serial monitor
  delay(1000);
  
  // Force output with newlines
  Serial.print("\n\n\n");
  Serial.flush();
  
  Serial.println("=== Weather Station Startup ===");
  Serial.flush();
  
  // Verify correct target.h is being used
  #ifdef WEATHER_STATION_TARGET_H
    Serial.println("OK: Using weather_station target.h");
  #else
    Serial.println("ERROR: Using wrong target.h!");
    Serial.flush();
    while(1) delay(1000);  // Halt if wrong target.h
  #endif
  Serial.flush();
  
  Serial.print("Milliseconds since boot: ");
  Serial.println(millis());
  Serial.flush();
  
  // Verify sensors type
  Serial.print("Sensors type check: ");
  #ifdef WEATHER_STATION_TARGET_H
    Serial.println("WeatherStationSensorManager");
  #else
    Serial.println("UNKNOWN - wrong target.h!");
  #endif
  Serial.flush();

  Serial.println("Initializing board...");
  Serial.flush();
  board.begin();
  Serial.println("Board initialized.");
  // Debug battery voltage reading
  uint16_t batt_mv = board.getBattMilliVolts();
  Serial.print("Battery voltage: ");
  Serial.print(batt_mv / 1000.0);
  Serial.print("V (");
  Serial.print(batt_mv);
  Serial.println(" mV)");
  
  // Debug: Read raw ADC value to verify calculation
  analogReadResolution(12);
  uint32_t raw_sum = 0;
  for (int i = 0; i < 8; i++) {
    raw_sum += analogRead(5);  // PIN_VBAT_READ
  }
  uint32_t raw_avg = raw_sum / 8;
  float calculated_mv = (3.0f * 1.73f * 1.187f * 1000.0f * raw_avg) / 4096.0f;
  Serial.print("DEBUG: Raw ADC (12-bit): ");
  Serial.print(raw_avg);
  Serial.print(" / 4095, Calculated: ");
  Serial.print(calculated_mv);
  Serial.println(" mV");
  Serial.flush();
  
  // Allow power rails to stabilize before initializing sensors
  // This ensures 3V3_S power rail for Slot A (BME280) is ready
  delay(100);

#ifdef DISPLAY_CLASS
  if (display.begin()) {
    display.startFrame();
    display.print("Please wait...");
    display.endFrame();
  }
#endif

  Serial.println("Initializing radio...");
  Serial.flush();
  
  // Use standard radio_init() pattern like other examples
  // Note: board.begin() already initialized Wire, so rtc_clock.begin() inside radio_init() should work
  if (!radio_init()) { 
    Serial.println("ERROR: Radio init failed!");
    Serial.flush();
    halt(); 
  }
  Serial.println("Radio initialized.");
  Serial.flush();

  Serial.println("Initializing RNG...");
  Serial.flush();
  fast_rng.begin(radio_get_rng_seed());
  Serial.println("RNG initialized.");
  Serial.flush();

  FILESYSTEM* fs;
#if defined(NRF52_PLATFORM) || defined(STM32_PLATFORM)
  InternalFS.begin();
  fs = &InternalFS;
  IdentityStore store(InternalFS, "");
#elif defined(ESP32)
  SPIFFS.begin(true);
  fs = &SPIFFS;
  IdentityStore store(SPIFFS, "/identity");
#elif defined(RP2040_PLATFORM)
  LittleFS.begin();
  fs = &LittleFS;
  IdentityStore store(LittleFS, "/identity");
  store.begin();
#else
  #error "need to define filesystem"
#endif
  if (!store.load("_main", the_mesh.self_id)) {
    MESH_DEBUG_PRINTLN("Generating new keypair");
    the_mesh.self_id = radio_new_identity();   // create new random identity
    int count = 0;
    while (count < 10 && (the_mesh.self_id.pub_key[0] == 0x00 || the_mesh.self_id.pub_key[0] == 0xFF)) {  // reserved id hashes
      the_mesh.self_id = radio_new_identity(); count++;
    }
    store.save("_main", the_mesh.self_id);
  }

  Serial.print("Weather Station ID: ");
  mesh::Utils::printHex(Serial, the_mesh.self_id.pub_key, PUB_KEY_SIZE); Serial.println();
  Serial.flush();

  command[0] = 0;

  Serial.println("Initializing sensors...");
  Serial.flush();
  Serial.println("  Calling sensors.begin()...");
  Serial.flush();
  bool sensors_ok = sensors.begin();
  Serial.print("  sensors.begin() returned: ");
  Serial.println(sensors_ok ? "true" : "false");
  Serial.println("Sensors initialized.");
  Serial.flush();

  Serial.println("Initializing mesh...");
  Serial.flush();
  the_mesh.begin(fs);
  Serial.println("Mesh initialized.");
  Serial.flush();

#ifdef DISPLAY_CLASS
  ui_task.begin(the_mesh.getNodePrefs(), FIRMWARE_BUILD_DATE, FIRMWARE_VERSION);
#endif

  // send out initial Advertisement to the mesh
  the_mesh.sendSelfAdvertisement(16000);
  
  Serial.println("Weather Station initialized!");
  Serial.println("Custom commands: 'rain', 'wind', 'reset_rain'");
}

void loop() {
  int len = strlen(command);
  while (Serial.available() && len < sizeof(command)-1) {
    char c = Serial.read();
    if (c != '\n') {
      command[len++] = c;
      command[len] = 0;
    }
    Serial.print(c);
  }
  if (len == sizeof(command)-1) {  // command buffer full
    command[sizeof(command)-1] = '\r';
  }

  if (len > 0 && command[len - 1] == '\r') {  // received complete line
    command[len - 1] = 0;  // replace newline with C string null terminator
    char reply[160];
    the_mesh.handleCommand(0, command, reply);  // NOTE: there is no sender_timestamp via serial!
    if (reply[0]) {
      Serial.print("  -> "); Serial.println(reply);
    }

    command[0] = 0;  // reset command buffer
  }

  the_mesh.loop();
  sensors.loop();
#ifdef DISPLAY_CLASS
  ui_task.loop();
#endif
  rtc_clock.tick();
}


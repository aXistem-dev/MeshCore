#include <Arduino.h>   // needed for PlatformIO
#include <Mesh.h>
#include "MyMesh.h"

// Believe it or not, this std C function is busted on some platforms!
static uint32_t _atoi(const char* sp) {
  uint32_t n = 0;
  while (*sp && *sp >= '0' && *sp <= '9') {
    n *= 10;
    n += (*sp++ - '0');
  }
  return n;
}

#if defined(NRF52_PLATFORM) || defined(STM32_PLATFORM)
  #include <InternalFileSystem.h>
  #if defined(QSPIFLASH)
    #include <CustomLFS_QSPIFlash.h>
    DataStore store(InternalFS, QSPIFlash, rtc_clock);
  #else
  #if defined(EXTRAFS)
    #include <CustomLFS.h>
    CustomLFS ExtraFS(0xD4000, 0x19000, 128);
    DataStore store(InternalFS, ExtraFS, rtc_clock);
  #else
    DataStore store(InternalFS, rtc_clock);
  #endif
  #endif
#elif defined(RP2040_PLATFORM)
  #include <LittleFS.h>
  DataStore store(LittleFS, rtc_clock);
#elif defined(ESP32)
  #include <SPIFFS.h>
  DataStore store(SPIFFS, rtc_clock);
#endif

#ifdef ESP32
  #include <WiFi.h>
  #if defined(WIFI_SSID) || defined(WITH_WIFI_INTERFACE)
    bool g_wifi_sta_connected = false;  // shared with UI so it can hide the BLE PIN when WiFi is online
    unsigned long g_wifi_attempt_start = 0;  // Track when WiFi connection attempt started
    bool g_wifi_fallback_to_ble = false;  // Flag to indicate we should fall back to BLE
    #define WIFI_CONNECTION_TIMEOUT_MS 60000  // 60 seconds timeout before falling back to BLE
  #endif
  // Runtime interface selection: WiFi preferred, BLE as fallback
  // Both interfaces are created if both compile-time flags are set
  BaseSerialInterface* serial_interface_ptr = nullptr;
  #if defined(WIFI_SSID) || defined(WITH_WIFI_INTERFACE)
    #include <helpers/esp32/SerialWifiInterface.h>
    SerialWifiInterface wifi_interface;
    #ifndef TCP_PORT
      #define TCP_PORT 5000
    #endif
  #endif
  #if defined(BLE_PIN_CODE)
    #include <helpers/esp32/SerialBLEInterface.h>
    SerialBLEInterface ble_interface;
  #endif
  #if !defined(WIFI_SSID) && !defined(WITH_WIFI_INTERFACE) && !defined(BLE_PIN_CODE)
    #if defined(SERIAL_RX)
      #include <helpers/ArduinoSerialInterface.h>
      ArduinoSerialInterface serial_interface;
      HardwareSerial companion_serial(1);
    #else
      #include <helpers/ArduinoSerialInterface.h>
      ArduinoSerialInterface serial_interface;
    #endif
    #define USE_SERIAL_INTERFACE
  #endif
#elif defined(RP2040_PLATFORM)
  //#ifdef WIFI_SSID
  //  #include <helpers/rp2040/SerialWifiInterface.h>
  //  SerialWifiInterface serial_interface;
  //  #ifndef TCP_PORT
  //    #define TCP_PORT 5000
  //  #endif
  // #elif defined(BLE_PIN_CODE)
  //   #include <helpers/rp2040/SerialBLEInterface.h>
  //   SerialBLEInterface serial_interface;
  #if defined(SERIAL_RX)
    #include <helpers/ArduinoSerialInterface.h>
    ArduinoSerialInterface serial_interface;
    HardwareSerial companion_serial(1);
  #else
    #include <helpers/ArduinoSerialInterface.h>
    ArduinoSerialInterface serial_interface;
  #endif
#elif defined(NRF52_PLATFORM)
  #ifdef BLE_PIN_CODE
    #include <helpers/nrf52/SerialBLEInterface.h>
    SerialBLEInterface serial_interface;
  #else
    #include <helpers/ArduinoSerialInterface.h>
    ArduinoSerialInterface serial_interface;
  #endif
#elif defined(STM32_PLATFORM)
  #include <helpers/ArduinoSerialInterface.h>
  ArduinoSerialInterface serial_interface;
#else
  #error "need to define a serial interface"
#endif

/* GLOBAL OBJECTS */
#ifdef DISPLAY_CLASS
  #include "UITask.h"
  #ifdef ESP32
    #if defined(WIFI_SSID) || defined(WITH_WIFI_INTERFACE) || defined(BLE_PIN_CODE)
      // Initialize UI with BLE interface if available (to ensure pin code is shown)
      // Otherwise use WiFi interface
      #if defined(BLE_PIN_CODE)
        UITask ui_task(&board, &ble_interface);
      #elif defined(WIFI_SSID) || defined(WITH_WIFI_INTERFACE)
        UITask ui_task(&board, &wifi_interface);
      #else
        UITask ui_task(&board, &serial_interface);
      #endif
    #else
      UITask ui_task(&board, &serial_interface);
    #endif
  #else
    UITask ui_task(&board, &serial_interface);
  #endif
#endif

StdRNG fast_rng;
SimpleMeshTables tables;
#ifdef DISPLAY_CLASS
  MyMesh the_mesh(radio_driver, fast_rng, rtc_clock, tables, store, &ui_task);
#else
  MyMesh the_mesh(radio_driver, fast_rng, rtc_clock, tables, store);
#endif

/* END GLOBAL OBJECTS */

void halt() {
  while (1) ;
}

void setup() {
  Serial.begin(115200);

  board.begin();

#ifdef DISPLAY_CLASS
  DisplayDriver* disp = NULL;
  if (display.begin()) {
    disp = &display;
    disp->startFrame();
  #ifdef ST7789
    disp->setTextSize(2);
  #endif
    disp->drawTextCentered(disp->width() / 2, 28, "Loading...");
    disp->endFrame();
  }
#endif

  if (!radio_init()) { halt(); }

  fast_rng.begin(radio_get_rng_seed());

#if defined(NRF52_PLATFORM) || defined(STM32_PLATFORM)
  InternalFS.begin();
  #if defined(QSPIFLASH)
    if (!QSPIFlash.begin()) {
      // debug output might not be available at this point, might be too early. maybe should fall back to InternalFS here?
      MESH_DEBUG_PRINTLN("CustomLFS_QSPIFlash: failed to initialize");
    } else {
      MESH_DEBUG_PRINTLN("CustomLFS_QSPIFlash: initialized successfully");
    }
  #else
  #if defined(EXTRAFS)
      ExtraFS.begin();
  #endif
  #endif
  store.begin();
  the_mesh.begin(
    #ifdef DISPLAY_CLASS
        disp != NULL
    #else
        false
    #endif
  );

#ifdef BLE_PIN_CODE
  char dev_name[32+16];
  sprintf(dev_name, "%s%s", BLE_NAME_PREFIX, the_mesh.getNodeName());
  serial_interface.begin(dev_name, the_mesh.getBLEPin());
#else
  serial_interface.begin(Serial);
#endif
  the_mesh.startInterface(serial_interface);
#elif defined(RP2040_PLATFORM)
  LittleFS.begin();
  store.begin();
  the_mesh.begin(
    #ifdef DISPLAY_CLASS
        disp != NULL
    #else
        false
    #endif
  );

  //#ifdef WIFI_SSID
  //  WiFi.begin(WIFI_SSID, WIFI_PWD);
  //  serial_interface.begin(TCP_PORT);
  // #elif defined(BLE_PIN_CODE)
  //   char dev_name[32+16];
  //   sprintf(dev_name, "%s%s", BLE_NAME_PREFIX, the_mesh.getNodeName());
  //   serial_interface.begin(dev_name, the_mesh.getBLEPin());
  #if defined(SERIAL_RX)
    companion_serial.setPins(SERIAL_RX, SERIAL_TX);
    companion_serial.begin(115200);
    serial_interface.begin(companion_serial);
  #else
    serial_interface.begin(Serial);
  #endif
    the_mesh.startInterface(serial_interface);
#elif defined(ESP32)
  SPIFFS.begin(true);
  store.begin();
  the_mesh.begin(
    #ifdef DISPLAY_CLASS
        disp != NULL
    #else
        false
    #endif
  );

  // Runtime interface selection: Choose WiFi OR BLE, not both
  // If WiFi credentials are valid, try WiFi. Otherwise, use BLE.
  bool wifi_connected = false;
  bool use_wifi = false;
  g_wifi_sta_connected = false;
  
  // Try runtime prefs first (new for repeaters/room servers and companion_radio)
  NodePrefs* prefs = the_mesh.getNodePrefs();
  #if defined(WIFI_SSID) || defined(WITH_WIFI_INTERFACE)
    // Check if we should try WiFi
    if (prefs && prefs->wifi_enabled && strlen(prefs->wifi_ssid) > 0) {
      use_wifi = true;
    }
    // Fall back to compile-time flags (legacy support)
    #ifdef WIFI_SSID
      if (!use_wifi && strlen(WIFI_SSID) > 0) {
        use_wifi = true;
      }
    #endif
    
    // Choose interface: Try WiFi first if credentials exist, otherwise use BLE
    #if defined(BLE_PIN_CODE)
      // Prepare BLE initialization (like upstream always does)
      char dev_name[32+16];
      sprintf(dev_name, "%s%s", BLE_NAME_PREFIX, the_mesh.getNodeName());
      uint32_t pin = the_mesh.getBLEPin();
      
      // If we have WiFi credentials, try WiFi first
      if (use_wifi) {
        // Try WiFi connection
        WiFi.mode(WIFI_STA);
        if (prefs && prefs->wifi_enabled && strlen(prefs->wifi_ssid) > 0) {
          WiFi.begin(prefs->wifi_ssid, prefs->wifi_password);
        }
        #ifdef WIFI_SSID
          else if (strlen(WIFI_SSID) > 0) {
            WiFi.begin(WIFI_SSID, WIFI_PWD);
          }
        #endif
        
        // WiFi.begin() is non-blocking - connection happens in background
        // Wait longer for WiFi to connect (up to 10 seconds) if WiFi is explicitly enabled
        // This gives WiFi a fair chance to connect before falling back to BLE
        unsigned long wifi_start = millis();
        // Use longer timeout if WiFi is enabled via prefs OR if compile-time WIFI_SSID is set
        bool wifi_explicitly_enabled = (prefs && prefs->wifi_enabled) || 
                                       #ifdef WIFI_SSID
                                       (strlen(WIFI_SSID) > 0) ||
                                       #endif
                                       false;
        unsigned long wifi_timeout = wifi_explicitly_enabled ? 10000 : 500; // 10s if enabled, 500ms otherwise
        while (WiFi.status() != WL_CONNECTED && (millis() - wifi_start) < wifi_timeout) {
          delay(50);
        }
        wifi_connected = (WiFi.status() == WL_CONNECTED);
      }
      
      if (wifi_connected) {
        // WiFi connected - use WiFi
        WiFi.mode(WIFI_STA); // Ensure WiFi is on
        wifi_interface.begin(TCP_PORT);
        serial_interface_ptr = &wifi_interface;
        g_wifi_sta_connected = true;
        g_wifi_attempt_start = 0; // Clear timeout - WiFi connected successfully
        g_wifi_fallback_to_ble = false;
        // BLE not initialized when WiFi is connected (pin still available via getBLEPin())
      } else {
        // WiFi not connected or no credentials - use BLE
        // Only disable WiFi if it's explicitly disabled or no credentials
        bool should_disable_wifi = true;
        // Check if WiFi should keep trying: either enabled in prefs OR compile-time WIFI_SSID is set
        bool wifi_should_try = false;
        if (prefs && prefs->wifi_enabled && strlen(prefs->wifi_ssid) > 0) {
          wifi_should_try = true;
        }
        #ifdef WIFI_SSID
          if (!wifi_should_try && strlen(WIFI_SSID) > 0) {
            // Compile-time WiFi SSID is set - treat as enabled
            wifi_should_try = true;
          }
        #endif
        
        if (wifi_should_try) {
          // WiFi is enabled (via prefs or compile-time) but didn't connect yet - keep it trying in background
          // Note: On ESP32, we can't run BLE and WiFi simultaneously, so we must choose one
          // If WiFi is enabled, we'll keep trying WiFi and not use BLE
          // This means if WiFi doesn't connect, the device won't have any interface until WiFi connects
          should_disable_wifi = false;
        }
        
        if (should_disable_wifi) {
          g_wifi_sta_connected = false;
          // Ensure WiFi is fully disabled before initializing BLE
          if (WiFi.getMode() != WIFI_OFF) {
            WiFi.disconnect(true);
            delay(200); // Give time for disconnect
          }
          WiFi.mode(WIFI_OFF);
          delay(500); // Longer delay to ensure WiFi is fully off before BLE init
          
          // Initialize BLE (this can hang if WiFi isn't fully off)
          if (pin != 0) {
            ble_interface.begin(dev_name, pin);
            serial_interface_ptr = &ble_interface;
          } else {
            // BLE pin not set, fall back to WiFi
            wifi_interface.begin(TCP_PORT);
            serial_interface_ptr = &wifi_interface;
          }
        } else {
          // WiFi is enabled but not connected yet - keep WiFi trying and use WiFi interface
          // The interface will work once WiFi connects
          // Start timeout timer for fallback to BLE
          g_wifi_sta_connected = false; // Not connected yet, but trying
          g_wifi_attempt_start = millis(); // Track when we started trying
          g_wifi_fallback_to_ble = false;
          wifi_interface.begin(TCP_PORT);
          serial_interface_ptr = &wifi_interface;
        }
      }
    #else
      // No BLE available, try WiFi if credentials exist
      if (use_wifi) {
        WiFi.mode(WIFI_STA);
        if (prefs && prefs->wifi_enabled && strlen(prefs->wifi_ssid) > 0) {
          WiFi.begin(prefs->wifi_ssid, prefs->wifi_password);
        }
        #ifdef WIFI_SSID
          else if (strlen(WIFI_SSID) > 0) {
            WiFi.begin(WIFI_SSID, WIFI_PWD);
          }
        #endif
        g_wifi_sta_connected = (WiFi.status() == WL_CONNECTED);
      } else {
        g_wifi_sta_connected = false;
      }
      serial_interface_ptr = &wifi_interface;
    #endif
  #elif defined(BLE_PIN_CODE)
    // No WiFi interface, use BLE
    char dev_name[32+16];
    sprintf(dev_name, "%s%s", BLE_NAME_PREFIX, the_mesh.getNodeName());
    ble_interface.begin(dev_name, the_mesh.getBLEPin());
    serial_interface_ptr = &ble_interface;
  #elif defined(USE_SERIAL_INTERFACE)
    #if defined(SERIAL_RX)
      companion_serial.setPins(SERIAL_RX, SERIAL_TX);
      companion_serial.begin(115200);
      serial_interface.begin(companion_serial);
    #else
      serial_interface.begin(Serial);
    #endif
    serial_interface_ptr = &serial_interface;
  #endif
  
  the_mesh.startInterface(*serial_interface_ptr);
  
  // Update UI to use the active interface for connection status (for ESP32 with runtime interface selection)
  #ifdef DISPLAY_CLASS
    #ifdef ESP32
      #if defined(WIFI_SSID) || defined(WITH_WIFI_INTERFACE) || defined(BLE_PIN_CODE)
        // UI was initialized with BLE interface (if available) to ensure pin code is always shown
        // Now update it to use the active interface for connection status checking
        // The pin code display uses the_mesh.getBLEPin() which works regardless of active interface
        ui_task.setSerialInterface(serial_interface_ptr);
      #endif
    #endif
  #endif
#else
  #error "need to define filesystem"
#endif

  sensors.begin();

#ifdef DISPLAY_CLASS
  ui_task.begin(disp, &sensors, the_mesh.getNodePrefs());  // still want to pass this in as dependency, as prefs might be moved
#endif
}

void loop() {
  // Periodically check WiFi connection status if WiFi is enabled
  #if defined(ESP32) && (defined(WIFI_SSID) || defined(WITH_WIFI_INTERFACE)) && defined(BLE_PIN_CODE)
    static unsigned long last_wifi_check = 0;
    if (millis() - last_wifi_check > 1000) { // Check every second
      last_wifi_check = millis();
      NodePrefs* prefs = the_mesh.getNodePrefs();
      bool wifi_should_try = false;
      if (prefs && prefs->wifi_enabled && strlen(prefs->wifi_ssid) > 0) {
        wifi_should_try = true;
      }
      #ifdef WIFI_SSID
        if (!wifi_should_try && strlen(WIFI_SSID) > 0) {
          // Compile-time WiFi SSID is set - treat as enabled
          wifi_should_try = true;
        }
      #endif
      
      if (wifi_should_try) {
        // WiFi is enabled with valid credentials - check if it has connected
        if (WiFi.status() == WL_CONNECTED) {
          if (!g_wifi_sta_connected) {
            // WiFi just connected - switch from BLE to WiFi interface
            g_wifi_sta_connected = true;
            g_wifi_attempt_start = 0; // Clear timeout
            g_wifi_fallback_to_ble = false;
            
            // If currently using BLE, switch to WiFi
            if (serial_interface_ptr == &ble_interface) {
              // Disable BLE first
              ble_interface.disable();
              
              // Ensure WiFi is in STA mode
              WiFi.mode(WIFI_STA);
              
              // Start WiFi interface
              wifi_interface.begin(TCP_PORT);
              serial_interface_ptr = &wifi_interface;
              
              // Restart mesh interface with WiFi
              the_mesh.startInterface(*serial_interface_ptr);
              
              // Update UI to use WiFi interface
              #ifdef DISPLAY_CLASS
                ui_task.setSerialInterface(serial_interface_ptr);
              #endif
            } else if (serial_interface_ptr == &wifi_interface) {
              // Already using WiFi interface, ensure it's started (begin() is safe to call multiple times)
              wifi_interface.begin(TCP_PORT);
            }
          }
        } else {
          // WiFi not connected yet
          g_wifi_sta_connected = false;
          
          // Check if timeout has been exceeded
          if (g_wifi_attempt_start > 0 && 
              (millis() - g_wifi_attempt_start) > WIFI_CONNECTION_TIMEOUT_MS) {
            // WiFi connection timeout exceeded - fall back to BLE
            if (!g_wifi_fallback_to_ble && serial_interface_ptr == &wifi_interface) {
              g_wifi_fallback_to_ble = true;
              
              // Disable WiFi
              if (WiFi.getMode() != WIFI_OFF) {
                WiFi.disconnect(true);
                delay(200);
              }
              WiFi.mode(WIFI_OFF);
              delay(500); // Ensure WiFi is fully off before BLE init
              
              // Initialize BLE
              char dev_name[32+16];
              sprintf(dev_name, "%s%s", BLE_NAME_PREFIX, the_mesh.getNodeName());
              uint32_t pin = the_mesh.getBLEPin();
              if (pin != 0) {
                ble_interface.begin(dev_name, pin);
                serial_interface_ptr = &ble_interface;
                the_mesh.startInterface(*serial_interface_ptr);
                
                // Update UI to use BLE interface
                #ifdef DISPLAY_CLASS
                  ui_task.setSerialInterface(serial_interface_ptr);
                #endif
              }
            }
          }
        }
      } else {
        // WiFi is enabled but has no valid credentials - ensure BLE is initialized
        // This handles the case where wifi_enabled=1 but SSID is empty/invalid
        if (serial_interface_ptr == &wifi_interface && !g_wifi_fallback_to_ble) {
          // We're using WiFi interface but WiFi shouldn't be trying - switch to BLE
          g_wifi_fallback_to_ble = true;
          
          // Disable WiFi
          if (WiFi.getMode() != WIFI_OFF) {
            WiFi.disconnect(true);
            delay(200);
          }
          WiFi.mode(WIFI_OFF);
          delay(500);
          
          // Initialize BLE
          char dev_name[32+16];
          sprintf(dev_name, "%s%s", BLE_NAME_PREFIX, the_mesh.getNodeName());
          uint32_t pin = the_mesh.getBLEPin();
          if (pin != 0) {
            ble_interface.begin(dev_name, pin);
            serial_interface_ptr = &ble_interface;
            the_mesh.startInterface(*serial_interface_ptr);
            
            // Update UI to use BLE interface
            #ifdef DISPLAY_CLASS
              ui_task.setSerialInterface(serial_interface_ptr);
            #endif
          }
        }
      }
    }
  #endif
  
  the_mesh.loop();
  sensors.loop();
#ifdef DISPLAY_CLASS
  ui_task.loop();
#endif
  rtc_clock.tick();
}

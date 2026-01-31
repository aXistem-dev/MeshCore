# M5Stack Cardputer-Adv with Cap LoRa-1262

This variant adds support for the M5Stack Cardputer-Adv with the Cap LoRa-1262 expansion module.

> **📚 For detailed hardware documentation, see:**
> - [Cardputer-Adv Documentation](../../../Cardputer/Cardputer-Adv-Documentation.md) - Complete hardware specs and pin mappings
> - [Cap LoRa-1262 Documentation](../../../Cardputer/Cap_LoRa-1262.md) - Expansion module specifications
> - [Firmware Comparison](../../../Cardputer/FIRMWARE_COMPARISON.md) - Integration details and implementation strategy

## Hardware Specifications

- **Main Board**: M5Stack Cardputer-Adv (ESP32-S3FN8)
- **LoRa Module**: Cap LoRa-1262 with SX1262 chip
- **GPS Module**: ATGM336H-6N@AT6668 GNSS module with ceramic antenna
- **Frequency Band**: 868-923 MHz (Europe)
- **Display**: Built-in ST7789V2 TFT display (240 x 135px)
- **Keyboard**: Built-in 56-key QWERTY keyboard

## Pin Configuration

### LoRa SX1262 Pins (Cap LoRa-1262)

| Function | G# Format | GPIO Number | Notes |
|----------|-----------|-------------|-------|
| **NSS (Chip Select)** | G5 | 5 | SPI Chip Select |
| **MOSI** | G14 | 14 | SPI Master Out Slave In |
| **MISO** | G39 | 39 | SPI Master In Slave Out |
| **SCLK** | G40 | 40 | SPI Clock |
| **IRQ (DIO1)** | G4 | 4 | Interrupt pin |
| **RESET** | G3 | 3 | Reset pin |
| **BUSY** | G6 | 6 | Busy signal |

### GPS Module Pins (Cap LoRa-1262 - ATGM336H-6N@AT6668)

| Function | G# Format | GPIO Number | Notes |
|----------|-----------|-------------|-------|
| **RX** | G13 | 13 | GPS UART Receive - Verified |
| **TX** | G15 | 15 | GPS UART Transmit - Verified on hardware |

**GPS UART Configuration**: 115200bps, 8N1 (8 data bits, no parity, 1 stop bit)

### I2C Bus (Cardputer-Adv Main Bus)

| Function | G# Format | GPIO Number | Notes |
|----------|-----------|-------------|-------|
| **SDA** | G8 | 8 | I2C Data - Shared with keyboard, audio codec, IMU |
| **SCL** | G9 | 9 | I2C Clock - Shared with keyboard, audio codec, IMU |

> **Note**: The I2C bus (G8/G9) is shared between multiple peripherals:
> - Keyboard (TCA8418RTWR)
> - Audio codec (ES8311)
> - IMU (BMI270)
> - Cap LoRa-1262 HY2.0-4P port (if used)

### User Button

| Function | G# Format | GPIO Number | Notes |
|----------|-----------|-------------|-------|
| **Boot Button** | G0 | 0 | Used for download mode entry |

### Display (ST7789V2)

| Function | G# Format | GPIO Number | Notes |
|----------|-----------|-------------|-------|
| **RST** | G33 | 33 | Display reset pin |
| **DC/RS** | G34 | 34 | Display data/command pin (Register Select) |
| **CS** | G37 | 37 | Display chip select |
| **DAT/MOSI** | G35 | 35 | Display SPI data line (MOSI) |
| **SCK** | G36 | 36 | Display SPI clock |
| **Backlight** | G38 | 38 | Display backlight control (also RGB LED power) |

### Battery Monitoring

| Function | G# Format | GPIO Number | Notes |
|----------|-----------|-------------|-------|
| **ADC** | G10 | 10 | Battery voltage monitoring (2:1 voltage divider) |

### Other Peripherals

| Function | G# Format | GPIO Number | Notes |
|----------|-----------|-------------|-------|
| **microSD CS** | G12 | 12 | microSD card chip select |
| **Keyboard INT** | G11 | 11 | Keyboard interrupt |
| **IR Emitter** | G44 | 44 | Infrared transmit |
| **LoRa Cap Power** | G46 | 46 | Power enable for LoRa Cap I/O expander |

### Cap LoRa-1262 HY2.0-4P Grove Port

| Color | Function | Cap LoRa-1262 | Cardputer-Adv GPIO | Notes |
|-------|----------|---------------|-------------------|-------|
| Black | GND | GND | GND | Ground |
| Red | 5V | 5V | 5V | Power |
| Yellow | SDA | SDA | G8 | I2C Data (shared with main I2C bus) |
| White | SCL | SCL | G9 | I2C Clock (shared with main I2C bus) |

> **Note**: The HY2.0-4P port on the Cap LoRa-1262 shares the I2C bus (SDA/SCL) with the Cardputer-Adv's internal I2C bus (G8/G9), which is also used by the keyboard, audio codec, and IMU.

## Hardware Initialization Sequence

The board initialization follows this sequence (from `M5CardputerBoard.h`):

1. **Enable I/O Expander Power** (GPIO 46)
   - Powers up the PI4IOE I/O expander on the LoRa Cap
   - Delay: 100ms

2. **Initialize Main I2C Bus** (G8/G9)
   - Required before M5Cardputer keyboard initialization

3. **Initialize M5Cardputer Hardware**
   - Display, keyboard, RTC, speaker, microphone
   - Delay: 100ms

4. **Configure PI4IOE I/O Expander** (I2C address 0x43)
   - Register 0x03: Configure all pins as outputs
   - Register 0x01: Set all outputs HIGH (enables LoRa module power)
   - Delay: 200ms for LoRa module to power up

5. **Radio Initialization**
   - Hardware reset sequence (10ms LOW, 100ms HIGH)
   - SPI initialization
   - Radio configuration

## Available Firmware Variants

The following firmware variants are available:

1. **M5Stack_Cardputer_ADV_repeater** - Simple mesh repeater with GPS
2. **M5Stack_Cardputer_ADV_room_server** - BBS-style chat room server
3. **M5Stack_Cardputer_ADV_companion_radio_usb** - Companion radio via USB
4. **M5Stack_Cardputer_ADV_companion_radio_ble** - Companion radio via Bluetooth LE
5. **M5Stack_Cardputer_ADV_companion_radio_wifi** - Companion radio via WiFi
6. **M5Stack_Cardputer_ADV_sensor** - Sensor node with GPS telemetry

## Building the Firmware

### Using PlatformIO directly

```bash
# Build repeater variant
pio run -e M5Stack_Cardputer_ADV_repeater

# Build BLE companion radio
pio run -e M5Stack_Cardputer_ADV_companion_radio_ble

# Upload to device
pio run -e M5Stack_Cardputer_ADV_companion_radio_ble --target upload
```

## Configuration

### Changing WiFi Credentials (for WiFi variant)

Edit `platformio.ini` and modify:
```ini
-D WIFI_SSID='"your_ssid"'
-D WIFI_PWD='"your_password"'
```

### Changing Admin Password

Edit `platformio.ini` and modify:
```ini
-D ADMIN_PASSWORD='"your_password"'
```

### Setting Your Location (for repeater/sensor variants)

Edit `platformio.ini` and set your GPS coordinates:
```ini
-D ADVERT_LAT=52.2297  ; Latitude
-D ADVERT_LON=21.0122  ; Longitude
```

### Production / Public Builds (disable debug)

Edit `platformio.ini` and ensure debug is off:
```ini
-D CORE_DEBUG_LEVEL=0
```

## Features

- LoRa mesh networking with SX1262 radio
- GPS location tracking with ATGM336H-6N@AT6668 module
- **Automatic GPS time synchronization** - Clock syncs when GPS gets a valid lock
- **Automatic BLE time synchronization** - Clock syncs when connected via Bluetooth (companion radio BLE mode)
- Support for multiple firmware modes (repeater, room server, companion radio, sensor)
- 868 MHz frequency band (European ISM band)
- USB connectivity via Cardputer's built-in USB-C port
- Bluetooth LE and WiFi support for companion radio mode
- ST7789 display with 240x135 resolution
- 56-key QWERTY keyboard support
- Brightness control (Dim, Low, Normal, Bright)

## Hardware References

- [M5Stack Cardputer-Adv](https://shop.m5stack.com/products/m5stack-cardputer-adv-version-esp32-s3)
- **[Cap LoRa-1262 Documentation](../../../Cardputer/Cap_LoRa-1262.md)** - Complete Cap LoRa-1262 expansion module specifications and pin mapping
- **[Cardputer-Adv Documentation](../../../Cardputer/Cardputer-Adv-Documentation.md)** - Complete Cardputer-Adv hardware documentation and pin mapping
- **[Firmware Comparison](../../../Cardputer/FIRMWARE_COMPARISON.md)** - Detailed comparison of third-party Cardputer firmwares and integration strategy

## Notes

- The Cap LoRa-1262 is the current recommended module (replaces older Cap LoRa868)
- The GPS module supports multi-constellation GNSS (GPS, BeiDou, GLONASS, Galileo, QZSS)
- Default frequency is 868.0 MHz (EU863-870 ISM band)
- Maximum TX power is 22 dBm
- The Cardputer's built-in display and keyboard can be used for UI enhancements
- I2C bus (G8/G9) is shared between multiple peripherals - ensure proper addressing

## Clock Synchronization

The Cardputer-Adv firmware supports automatic clock synchronization from two sources:

### GPS Time Sync
- **Automatic**: When GPS gets a valid lock (after ~2-3 seconds of valid NMEA data), the clock automatically syncs
- **Requires**: GPS enabled firmware variant (e.g., `companion_radio_ble`, `sensor`)
- **Behavior**: Clock syncs once GPS has valid time data (time_valid > 2)
- **Manual trigger**: Use `clock sync` command via serial console to force GPS time sync

### BLE Time Sync (Companion Radio)
- **Automatic**: When connected via Bluetooth LE, the smartphone/tablet app automatically syncs the device clock
- **Requires**: `companion_radio_ble` firmware variant
- **Behavior**: Clock syncs when the BLE client sends time sync command (typically on connection)
- **Prevents clock drift**: Ensures device time stays synchronized with your phone/tablet

### Manual Time Setting
- **Serial console**: Use `time <epoch_seconds>` command to manually set time
- **Clock query**: Use `clock` command to display current time
- **Note**: Clock cannot be set backwards (prevents time travel issues)

## Troubleshooting

- **If the LoRa module is not detected**: Check that the Cap LoRa-1262 is properly seated on the Cardputer-Adv
- **Ensure the SMA antenna is connected** before transmitting
- **GPS may take several minutes** to acquire a fix when first powered on (cold start: ~23s)
- **For GPS issues**: Ensure you have a clear view of the sky
- **GPS time sync not working**: Ensure GPS has a valid lock (check GPS info screen), time sync requires 2-3 seconds of valid NMEA data
- **BLE time sync not working**: Ensure you're using `companion_radio_ble` firmware and the BLE client (app) supports time sync
- **I2C conflicts**: If experiencing I2C issues, check that all devices on the bus (G8/G9) have unique addresses

## Pin Number Reference

For reference, here are the GPIO pin numbers without the "G" prefix:

- LoRa: NSS=5, MOSI=14, MISO=39, SCK=40, IRQ=4, RST=3, BUSY=6
- GPS: RX=13, TX=15
- I2C: SDA=8, SCL=9
- Display: RST=33, DC=34, CS=37, DAT=35, SCK=36, Backlight=38
- Battery: ADC=10
- User Button: 0
- LoRa Power: 46

See [FIRMWARE_COMPARISON.md](../../../Cardputer/FIRMWARE_COMPARISON.md) for detailed pin mapping reference table.

---

**Last Updated**: January 24, 2026

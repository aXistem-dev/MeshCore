# Complete Weather Station for RAK4631

Integrated weather station combining BME280 environmental sensor with SparkFun Weather Meter (rain gauge, anemometer, and wind vane).

## Hardware Setup

### BME280 Sensor
- Connect the BME280 sensor module to the RAK4631 via the Qwic connector
- The Qwic connector uses the main I2C bus (Wire) on pins 13 (SDA) and 14 (SCL)
- Default I2C address: 0x76 (will try 0x77 if not found)

### Weather Meter (via RAK13002 WisBlock IO Module)
- **Rain Gauge** -> WB_IO1 (pin 17) and GND
- **Anemometer** -> WB_IO3 (pin 21) and GND  
- **Wind Vane** -> WB_A0 (pin 5) and GND

## Features

### BME280 Environmental Sensor
- Temperature (°C)
- Barometric Pressure (hPa)
- Relative Humidity (%)
- Altitude (m ASL) - calibrated for 50.5m ASL location

### Weather Station
- **Rain Gauge**: Measures rainfall in mm (0.2794mm per tip)
- **Anemometer**: Measures wind speed in km/h
- **Wind Vane**: Measures wind direction in degrees (0-359°) with cardinal direction

## Building and Uploading

1. Connect your RAK4631 to your computer via USB (should appear as `/dev/cu.usbmodem1101` or similar)

2. Build and upload using PlatformIO:
   ```bash
   pio run -e RAK_4631_bme280_reader -t upload --upload-port /dev/cu.usbmodem1101
   ```

3. Monitor the serial output:
   ```bash
   pio device monitor --port /dev/cu.usbmodem1101 --baud 115200
   ```
   Or use CoolTerm or any serial monitor at 115200 baud.

## Serial Commands

- **`c`** - Enter calibration mode (for wind vane)
  - Shows live ADC readings and estimated direction
  - Rotate wind vane to each direction and note ADC values
  - Update `vaneMap[]` array in code with your calibrated values
  
- **`r`** - Run normal mode (default)
  - Displays complete weather data every 5 seconds
  
- **`s`** - Show current statistics
  - Displays all sensor readings and totals
  
- **`z`** - Zero rainfall counter
  - Resets the rain gauge counter to 0

## Output Format

### Normal Mode (every 5 seconds):
```
========================================
=== COMPLETE WEATHER DATA ===
========================================
--- Environmental (BME280) ---
Temperature: 22.28 °C
Pressure:    1022.12 hPa
Humidity:    44.67 %
Altitude:    50.50 m ASL
--- Weather Station ---
Rainfall:    5.58 mm
Wind Speed:  12.3 km/h @ 225° (SW)
========================================
```

### Calibration Mode:
```
ADC: 456    | Voltage: 1.467V    | Estimated Dir: 45° (NE)
ADC: 789    | Voltage: 2.534V    | Estimated Dir: 225° (SW)
```

## Calibration

### Wind Vane Calibration

The wind vane uses a voltage divider with internal pull-up resistor. The default `vaneMap[]` values are examples and should be calibrated for your specific hardware:

1. Send `c` to enter calibration mode
2. Manually rotate the wind vane to each cardinal direction (N, NE, E, SE, S, SW, W, NW)
3. Note the ADC value for each position
4. Update the `vaneMap[]` array in the code with your actual values
5. Re-upload the code
6. Send `r` to return to normal mode

### Altitude Calibration

The altitude is calibrated for 50.5m ASL. To adjust for your location, modify `SEA_LEVEL_PRESSURE_HPA` in the code.

## Configuration

- **BME280 I2C Address**: Default is `0x76`. If your sensor uses `0x77`, the code will automatically detect it.
- **Sea Level Pressure**: Set to `1028.6` hPa for 50.5m ASL location
- **Rain Gauge**: 0.2794mm per tip (standard tipping bucket)
- **Wind Speed Factor**: 2.4 km/h per click/second (may need adjustment for your anemometer)
- **Update Interval**: 5 seconds in normal mode

## Troubleshooting

### No BME280 Data
- Check I2C connections (SDA: 13, SCL: 14)
- Verify sensor is powered
- Try both I2C addresses (0x76 and 0x77)

### No Weather Station Data
- Verify connections to WB_IO1, WB_IO2, and WB_A0
- Check that RAK13002 IO module is properly connected
- Ensure sensors are properly grounded

### Wind Direction Not Accurate
- Enter calibration mode (`c`) and calibrate the `vaneMap[]` array
- Verify internal pull-up resistor value matches your hardware
- Check for loose connections

### Rain Gauge Not Counting
- Verify connection to WB_IO1
- Check that the rain gauge is properly grounded
- Ensure interrupt is working (check Serial output for tips)

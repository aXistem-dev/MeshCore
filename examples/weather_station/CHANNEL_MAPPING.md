# Weather Station Telemetry Channel Mapping

This document describes the CayenneLPP telemetry channels and data types used by the Weather Station sensor.

## Channel 1 (TELEM_CHANNEL_SELF = 1) - BME280 Environmental Data

Multiple data types can share the same channel in CayenneLPP. All BME280 readings are on Channel 1:

1. **Type 116 (VOLTAGE)** - Battery voltage in Volts
   - Added by `SensorMesh::handleRequest()` before querying sensors
   - Represents the device's battery voltage (e.g., 3.98V)

2. **Type 103 (TEMPERATURE)** - Temperature in Celsius
   - BME280 temperature reading
   - Range: typically -40°C to +85°C

3. **Type 115 (BAROMETRIC_PRESSURE)** - Atmospheric pressure in hPa
   - BME280 pressure reading (converted from Pa to hPa)
   - Typical range: 300-1100 hPa

4. **Type 104 (RELATIVE_HUMIDITY)** - Relative humidity in %
   - BME280 humidity reading
   - Range: 0-100%

5. **Type 121 (ALTITUDE)** - Altitude in meters above sea level
   - Calculated from BME280 pressure using calibrated sea level pressure (1028.6 hPa)
   - Calibrated for 50.5m ASL location

## Channel 2 (TELEM_CHANNEL_SELF + 1 = 2) - Rainfall Data

All rainfall measurements use **Type 130 (DISTANCE)** in meters. Multiple values are sent on the same channel:

1. **Rainfall - Last 10 minutes** (in meters)
   - Rainfall accumulated in the past 10 minutes
   - Converted from mm to meters (mm / 1000.0)

2. **Rainfall - Last 1 hour** (in meters)
   - Rainfall accumulated in the past hour
   - Converted from mm to meters

3. **Rainfall - Last 24 hours** (in meters)
   - Rainfall accumulated in the past 24 hours
   - Converted from mm to meters

4. **Rainfall - Current day** (in meters)
   - Rainfall from midnight (local time) to now
   - Uses timezone offset for local midnight calculation
   - Converted from mm to meters

**Note:** Each rain tip = 0.2794mm of rainfall. The circular buffer stores up to 10,000 tips (~2.8m of rainfall) to handle extreme 24-hour scenarios.

## Channel 3 (TELEM_CHANNEL_SELF + 2 = 3) - Wind Speed

- **Type 2 (ANALOG_INPUT)** - Wind speed in km/h
  - Resolution: 0.01 km/h (CayenneLPP multiplies by 100 internally)
  - Range: 0-327.67 km/h (signed 16-bit limit)
  - Calculated from anemometer clicks using factor: 2.4 km/h per click/second
  - Always sent, even if 0, to ensure channel is present

## Channel 4 (TELEM_CHANNEL_SELF + 3 = 4) - Wind Direction

- **Type 132 (DIRECTION)** - Wind direction in degrees
  - Resolution: 1 degree
  - Range: 0-359 degrees (0° = North, 90° = East, 180° = South, 270° = West)
  - Read from wind vane via analog input (WB_A1, pin 31) with 10-bit ADC resolution
  - Uses calibrated lookup table (`vaneMap`) to convert ADC values to degrees
  - Always sent (defaults to 0° North if ADC reading is out of range)

## Data Type Reference

| Type ID | Name | Size | Resolution | Signed |
|---------|------|------|------------|--------|
| 116 | VOLTAGE | 2 bytes | 0.01V | No |
| 103 | TEMPERATURE | 2 bytes | 0.1°C | Yes |
| 115 | BAROMETRIC_PRESSURE | 2 bytes | 0.1 hPa | No |
| 104 | RELATIVE_HUMIDITY | 1 byte | 0.5% | No |
| 121 | ALTITUDE | 2 bytes | 0.01m | Yes |
| 130 | DISTANCE | 4 bytes | 0.001m | No |
| 2 | ANALOG_INPUT | 2 bytes | 0.01 | Yes |
| 132 | DIRECTION | 2 bytes | 1° | No |

## Example Telemetry Buffer Structure

```
Position 0-1:   Channel 1, Type 116 (VOLTAGE) - Battery voltage
Position 2-3:   Channel 1, Type 103 (TEMPERATURE) - Temperature
Position 4-5:   Channel 1, Type 115 (BAROMETRIC_PRESSURE) - Pressure
Position 6:     Channel 1, Type 104 (RELATIVE_HUMIDITY) - Humidity
Position 7-8:   Channel 1, Type 121 (ALTITUDE) - Altitude
Position 9-12:  Channel 2, Type 130 (DISTANCE) - Rainfall 10min
Position 13-16: Channel 2, Type 130 (DISTANCE) - Rainfall 1hr
Position 17-20: Channel 2, Type 130 (DISTANCE) - Rainfall 24hr
Position 21-24: Channel 2, Type 130 (DISTANCE) - Rainfall today
Position 25-26: Channel 3, Type 2 (ANALOG_INPUT) - Wind speed
Position 27-28: Channel 4, Type 132 (DIRECTION) - Wind direction
```


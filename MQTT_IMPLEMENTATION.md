# MQTT Bridge Implementation for MeshCore

This document describes the MQTT bridge implementation that allows MeshCore repeaters to uplink packet data to multiple MQTT brokers.

## Overview

The MQTT bridge implementation provides:
- Multiple MQTT broker support (up to 3 brokers)
- Automatic reconnection with exponential backoff
- JSON message formatting for status, packets, and raw data
- Configurable topics and QoS levels
- Packet queuing during connection issues

## Files Added

### Core Implementation
- `src/helpers/bridges/MQTTBridge.h` - MQTT bridge class definition
- `src/helpers/bridges/MQTTBridge.cpp` - MQTT bridge implementation
- `src/helpers/MQTTMessageBuilder.h` - JSON message formatting utilities
- `src/helpers/MQTTMessageBuilder.cpp` - JSON message formatting implementation

### Integration
- Updated `examples/simple_repeater/MyMesh.h` - Added MQTT bridge support
- Updated `examples/simple_repeater/MyMesh.cpp` - Added MQTT bridge integration
- Updated `src/helpers/CommonCLI.h` - Added MQTT configuration fields
- Updated `src/helpers/CommonCLI.cpp` - Added MQTT CLI commands
- Updated `variants/heltec_v3/platformio.ini` - Added MQTT build configuration

## Build Configuration

To build the MQTT bridge firmware:

```bash
pio run -e Heltec_v3_repeater_bridge_mqtt
```

## Default Configuration

The MQTT bridge comes with the following defaults:
- **Origin**: "MeshCore-Repeater"
- **IATA**: "SEA"
- **Status Messages**: Enabled
- **Packet Messages**: Enabled
- **Raw Messages**: Disabled
- **Status Interval**: 5 minutes (300000 ms)
- **Default Broker**: meshtastic.pugetmesh.org:1883 (username: meshdev, password: large4cats)

## CLI Commands

### Get Commands
- `get mqtt.origin` - Get device origin name
- `get mqtt.iata` - Get IATA code
- `get mqtt.status` - Get status message setting (on/off)
- `get mqtt.packets` - Get packet message setting (on/off)
- `get mqtt.raw` - Get raw message setting (on/off)
- `get mqtt.interval` - Get status publish interval (ms)

### Set Commands
- `set mqtt.origin <name>` - Set device origin name
- `set mqtt.iata <code>` - Set IATA code
- `set mqtt.status on|off` - Enable/disable status messages
- `set mqtt.packets on|off` - Enable/disable packet messages
- `set mqtt.raw on|off` - Enable/disable raw messages
- `set mqtt.interval <ms>` - Set status publish interval (1000-3600000 ms)

## MQTT Topics

The bridge publishes to three main topics:

### Status Topic: `meshcore/{IATA}/status`
Device connection status and metadata (retained messages).

### Packets Topic: `meshcore/{IATA}/packets`
Full packet data with RF characteristics and metadata.

### Raw Topic: `meshcore/{IATA}/raw`
Minimal raw packet data for map integration.

## JSON Message Formats

### Status Message
```json
{
  "status": "online|offline",
  "timestamp": "2024-01-01T12:00:00.000000",
  "origin": "Device Name",
  "origin_id": "DEVICE_PUBLIC_KEY",
  "model": "device_model",
  "firmware_version": "firmware_version",
  "radio": "radio_info",
  "client_version": "packet_capture_version"
}
```

### Packet Message
```json
{
  "origin": "MeshCore-HOWL",
  "origin_id": "A1B2C3D4E5F67890...",
  "timestamp": "2024-01-01T12:00:00.000000",
  "type": "PACKET",
  "direction": "rx|tx",
  "time": "12:00:00",
  "date": "01/01/2024",
  "len": "45",
  "packet_type": "4",
  "route": "F|D|T|U",
  "payload_len": "32",
  "raw": "F5930103807E5F1E...",
  "SNR": "12.5",
  "RSSI": "-65",
  "hash": "A1B2C3D4E5F67890",
  "path": "node1,node2,node3"
}
```

### Raw Message
```json
{
  "origin": "MeshCore-HOWL",
  "origin_id": "A1B2C3D4E5F67890...",
  "timestamp": "2024-01-01T12:00:00.000000",
  "type": "RAW",
  "data": "F5930103807E5F1E..."
}
```

## Testing

1. Flash the MQTT bridge firmware to your device
2. Connect to the device via serial console
3. Configure WiFi connection (if not already connected)
4. Check MQTT settings: `get mqtt.origin`
5. Monitor MQTT broker for incoming messages

## Dependencies

- **PubSubClient**: MQTT client library
- **ArduinoJson**: JSON message formatting
- **WiFi**: ESP32 WiFi functionality

## Future Enhancements

- Device-signed JWTs for WebSocket MQTT servers
- Multiple broker configuration via CLI
- Advanced packet filtering
- Custom topic templates
- TLS/SSL support for secure connections

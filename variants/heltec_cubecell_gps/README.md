# Heltec CubeCell GPS-6502 (HTCC-AB02S)

Experimental MeshCore port on branch `feature/cubecell-gps-6502`.

## Board

- MCU: ASR6502 (48 MHz, **16 KB RAM**, **112 KB application flash** after bootloader)
- Radio: SX1262 (pins from Heltec `board-config.h`)
- GPS: onboard AIR530Z (not enabled in current minimal repeater build)
- No BLE / WiFi — supported firmware types: repeater, room server, sensor, companion USB only

## Build environments

| Environment | Status |
|-------------|--------|
| `Heltec_CubeCell_GPS_repeater` | Compiles sources; **link fails** (ROM ~144 KB needed, ~106 KB available; RAM overflow) |
| `Heltec_CubeCell_GPS_room_server` | Same platform constraints |
| `Heltec_CubeCell_GPS_sensor` | Same |
| `Heltec_CubeCell_GPS_companion_radio_usb` | Same (plus larger RAM for contacts) |

```bash
cd MeshCore
pio run -e Heltec_CubeCell_GPS_repeater
```

## Flash (when a build succeeds)

```bash
pio run -e Heltec_CubeCell_GPS_repeater -t upload --upload-port /dev/cu.usbserial-0001
```

## Current blocker

Standard MeshCore repeater firmware is too large for this MCU. A dedicated slim profile (or separate project such as [CubeCellMeshCore](https://github.com/atomozero/CubeCellMeshCore)) is required before flashing will work.

## What is implemented

- `cubecell_base` PlatformIO platform (`heltec-cubecell`, board `cubecell_gps`)
- `CUBECELL_PLATFORM` integration (filesystem, serial companion, identity store)
- CubeCell internal flash LittleFS at end of application partition
- Variant pin/radio setup and build targets listed above

# SlunseCore Companion Firmware v1.17.0

SlunseCore companion builds track **MeshCore v1.17.0** and add SlunseCore custom features on top.

**Official upstream release:** [MeshCore v1.17.0 — 9 Aug 2026](https://blog.meshcore.io/2026/08/09/release-1-17-0)

## SlunseCore changes

Retained from earlier SlunseCore work: on-device **Settings** (`ui-new`) for buzzer, path hash, auto-add contacts, screen/timezone/advert settings, and boot logo — now re-themed onto MeshCore's new semantic `UIColor` display system (see below) and re-registered against MeshCore's new self-describing prefs format, so all SlunseCore preferences continue to persist correctly. Also retained: Packet FWD toggle (now wired to MeshCore's `repeat.disable_fwd`), battery/USB indicator, and nRF52 idle sleep from upstream.

## Official MeshCore v1.17.0 release notes

The following is condensed from the [official MeshCore v1.17.0 announcement](https://blog.meshcore.io/2026/08/09/release-1-17-0) (9 Aug 2026).

### New Features

- MCU temperature telemetry for companion devices
- LR2021 radio support, including the Meshnology W12
- New `room.post` CLI command for server-originated posts
- Repeater power-off via extended button press
- CAD toggling, boot-reason retrieval for ESP targets
- FEM RX gain control for select Heltec boards

### Enhancements

- Fixes for missing preamble-detect IRQ bit and refined timeout logic
- nRF52 targets now use CC310 hardware crypto
- Preferences storage rewritten to a self-describing JSON-like config format (auto-migrates from the old binary prefs file)
- Companion UI received a new semantic color scheme (`UIColor`); monochrome titlebars no longer invert; new light theme for color displays
- Display drivers accelerated; room server gained boosted-gain support; power consumption optimized
- Companion interfaces refactored to support multiple simultaneous connection modes (USB / BLE / Ethernet)

### New Device Support

Sensecap MeshTracker X1 and variants, ThinkNode M7/M9, Heltec RC32 and V4 R8, RAK ethernet modules, Elecrow devices, Nibble Zero Connect, Heltec Tower V2, LilyGo TETH Elite.

### Bug Fixes

Companion CAD issues, GPS initialization, OTA failure reporting, USB backpressure handling, BLE reconnection and synchronization, display fixes, watchdog timer implementation, and radio gain calibration across multiple device variants. IRQ/timeout and preamble-detection fixes across LR1110/SX1262/SX1268/LLCC68/STM32WLx radios.

### Key Improvements

The "listen-before-talk" collision-avoidance system was significantly refined to handle stuck IRQ flags and improve preamble detection.

See the [full MeshCore v1.17.0 changelog](https://blog.meshcore.io/2026/08/09/release-1-17-0) for complete details and every fixed device.

## Flashing

Download from the release assets → [flasher.meshcore.io](https://flasher.meshcore.io/) → **Custom firmware**.

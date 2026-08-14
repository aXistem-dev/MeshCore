# SlunseCore Room Server Firmware v1.17.0

SlunseCore room server builds track **MeshCore v1.17.0** with SlunseCore custom features preserved.

**Official upstream release:** [MeshCore v1.17.0 — 9 Aug 2026](https://blog.meshcore.io/2026/08/09/release-1-17-0)

## SlunseCore changes

- SlunseCore integration branch merged with upstream v1.17.0; no additional room-server-specific UI changes in this release.

## Official MeshCore v1.17.0 release notes

The following is condensed from the [official MeshCore v1.17.0 announcement](https://blog.meshcore.io/2026/08/09/release-1-17-0) (9 Aug 2026).

### New Features

- New `room.post` CLI command for server-originated posts
- LR2021 radio support, including the Meshnology W12
- Boot-reason retrieval for ESP targets

### Enhancements

- Preferences storage rewritten to a self-describing JSON-like config format (auto-migrates from the old binary prefs file)
- nRF52 targets now use CC310 hardware crypto
- Room server gained boosted-gain support; power consumption optimized
- Fixes for missing preamble-detect IRQ bit and refined timeout logic

### New Device Support

Sensecap MeshTracker X1 and variants, ThinkNode M7/M9, Heltec RC32 and V4 R8, RAK ethernet modules, Elecrow devices, Nibble Zero Connect, Heltec Tower V2, LilyGo TETH Elite.

### Bug Fixes

GPS initialization, OTA failure reporting, watchdog timer implementation, and radio gain calibration across multiple device variants. IRQ/timeout and preamble-detection fixes across LR1110/SX1262/SX1268/LLCC68/STM32WLx radios.

### Key Improvements

The "listen-before-talk" collision-avoidance system was significantly refined to handle stuck IRQ flags and improve preamble detection.

See the [full MeshCore v1.17.0 changelog](https://blog.meshcore.io/2026/08/09/release-1-17-0) for complete details and every fixed device.

## Flashing

Download from the release assets → [flasher.meshcore.io](https://flasher.meshcore.io/) → **Custom firmware**.

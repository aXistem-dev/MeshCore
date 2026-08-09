# SlunseCore Repeater Firmware v1.17.0

SlunseCore repeater builds track **MeshCore v1.17.0** with SlunseCore custom features preserved.

**Official upstream release:** [MeshCore v1.17.0 — 9 Aug 2026](https://blog.meshcore.io/2026/08/09/release-1-17-0)

## SlunseCore changes

- **GPS saver** — SlunseCore GPS power-save CLI commands (`gps saver`, `gps hold`, `gps interval`, `gps telem`, etc.; see `docs/cli_commands.md`), now re-registered against MeshCore's new self-describing prefs format.
- **SenseCap Solar** — Headless operation with remapped LoRa TX LED and USR/PWR button actions, alongside upstream's new Ethernet support.

## MeshCore v1.17.0 (included)

Highlights from the [official release notes](https://blog.meshcore.io/2026/08/09/release-1-17-0):

- Preferences storage rewritten to a self-describing config format (auto-migrates from the old binary prefs file)
- Ethernet support added (initial CH390 support on ThinkNode M7)
- LR2021 radio: multi-SF side-detector support, auto-LDRO, hardware CAD
- IRQ/timeout and preamble-detection fixes across LR1110/SX1262/SX1268/LLCC68/STM32WLx radios
- nRF52 hardware crypto (CC310) enabled by default
- New board support: SenseCAP MeshTracker X1, Meshnology W12, ThinkNode M7/M9, Heltec RC32/Tower V2/V4 R8, LilyGo TETH Elite

See the [full MeshCore v1.17.0 changelog](https://blog.meshcore.io/2026/08/09/release-1-17-0) for all fixes and new devices.

## Flashing

Download from the release assets → [flasher.meshcore.io](https://flasher.meshcore.io/) → **Custom firmware**.

# SlunseCore Companion Firmware v1.17.0

SlunseCore companion builds track **MeshCore v1.17.0** and add SlunseCore custom features on top.

**Official upstream release:** [MeshCore v1.17.0 — 9 Aug 2026](https://blog.meshcore.io/2026/08/09/release-1-17-0)

## SlunseCore changes

Retained from earlier SlunseCore work: on-device **Settings** (`ui-new`) for buzzer, path hash, auto-add contacts, screen/timezone/advert settings, and boot logo — now re-themed onto MeshCore's new semantic `UIColor` display system (see below) and re-registered against MeshCore's new self-describing prefs format, so all SlunseCore preferences continue to persist correctly. Also retained: Packet FWD toggle (now wired to MeshCore's `repeat.disable_fwd`), battery/USB indicator, and nRF52 idle sleep from upstream.

## MeshCore v1.17.0 (included)

Highlights from the [official release notes](https://blog.meshcore.io/2026/08/09/release-1-17-0):

- Preferences storage rewritten to a self-describing config format (auto-migrates from the old binary prefs file)
- Display color system reworked to semantic `UIColor` roles; monochrome titlebars no longer invert; new light theme for color displays
- Companion interfaces refactored to support multiple simultaneous connection modes (USB / BLE / Ethernet)
- LR2021 radio: multi-SF side-detector support, auto-LDRO, hardware CAD
- IRQ/timeout and preamble-detection fixes across LR1110/SX1262/SX1268/LLCC68/STM32WLx radios
- nRF52 hardware crypto (CC310) enabled by default; various BLE reliability fixes
- New board support: SenseCAP MeshTracker X1, Meshnology W12, ThinkNode M7/M9, Heltec RC32/Tower V2/V4 R8, LilyGo TETH Elite

See the [full MeshCore v1.17.0 changelog](https://blog.meshcore.io/2026/08/09/release-1-17-0) for all fixes and new devices.

## Flashing

Download from the release assets → [flasher.meshcore.io](https://flasher.meshcore.io/) → **Custom firmware**.

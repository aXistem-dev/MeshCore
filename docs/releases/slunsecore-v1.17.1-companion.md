# SlunseCore Companion Firmware v1.17.1

SlunseCore companion builds track **MeshCore v1.17.1** and add SlunseCore custom features on top.

**Official upstream release:** [MeshCore v1.17.1 — 14 Aug 2026](https://blog.meshcore.io/2026/08/14/release-1-17-1)

## SlunseCore changes

Retained from earlier SlunseCore work: on-device **Settings** (`ui-new`) for buzzer, path hash, auto-add contacts, screen/timezone/advert settings, and boot logo, themed onto MeshCore's semantic `UIColor` display system with a working selection highlight on monochrome displays. Also retained: Packet FWD toggle, battery/USB indicator, and nRF52 idle sleep from upstream.

R1 Neo and Minewsemi ME25LS01 companion/repeater/room-server builds now use upstream's own v1.17.1 build fixes directly (see below) rather than SlunseCore's earlier ad-hoc patches for the same gaps.

**Known gap:** `ThinkNode_M5_companion_radio_serial` is disabled in this release — its `SERIAL_TX`/`SERIAL_RX` pins reference Arduino `D6`/`D7` aliases that don't exist for this board, and no schematic-verified GPIO pair is available yet (upstream has the same unresolved gap as of v1.17.1). All other ThinkNode M5 variants (BLE/USB/WiFi/repeater/room-server/etc.) are unaffected.

## Official MeshCore v1.17.1 release notes

The following is condensed from the [official MeshCore v1.17.1 announcement](https://blog.meshcore.io/2026/08/14/release-1-17-1) (14 Aug 2026). This is a point release focused on fixes on top of v1.17.0.

### New Features

- New `radio.fem.rxgain` / `radio.fem.txgain` CLI commands (txgain support currently limited to Station G3)
- Missing KISS radio roles added, plus KISS radio build support

### Enhancements

- Modified login-response behavior for direct login scenarios (prevents flood messages)
- LR2021 preamble-detection and IRQ-timeout logic updates
- nRF52 CC310 optimization: `nRFCrypto` now initialized only once
- Debug print flags added for noise-floor monitoring
- Improved RNG: combined entropy from radio + CC310 sources

### Critical Fixes

- Heltec T096 transmit failure, caused by an out-of-bounds `PIN_SPI1_MISO` configuration
- T-Echo Lite pin alignment and TCXO voltage adjustment (3.0V), matching revised Lilygo schematics
- R1 Neo and Minewsemi ME25LS01 repeater build issues
- Rx boosted gain no longer resets to the compiled-in default after an AGC reset
- T-Beam Supreme S3 display functionality restored
- Multiple unused-pin corrections across nRF52 boards

### FEM Configuration Note

`radio.fem.rxgain` was not being persisted properly in earlier versions. **Repeater administrators with FEM-equipped devices (e.g. Station G3) should verify their FEM gain settings after updating.**

See the [full MeshCore v1.17.1 changelog](https://blog.meshcore.io/2026/08/14/release-1-17-1) for complete details.

## Flashing

Download from the release assets → [flasher.meshcore.io](https://flasher.meshcore.io/) → **Custom firmware**.

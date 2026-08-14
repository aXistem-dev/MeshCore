# SlunseCore Room Server Firmware v1.17.1

SlunseCore room server builds track **MeshCore v1.17.1** with SlunseCore custom features preserved.

**Official upstream release:** [MeshCore v1.17.1 — 14 Aug 2026](https://blog.meshcore.io/2026/08/14/release-1-17-1)

## SlunseCore changes

- SlunseCore integration branch merged with upstream v1.17.1; no additional room-server-specific UI changes in this release. R1 Neo and Minewsemi ME25LS01 room-server builds now use upstream's own v1.17.1 build fixes directly rather than SlunseCore's earlier ad-hoc patches for the same gaps.

## Official MeshCore v1.17.1 release notes

The following is condensed from the [official MeshCore v1.17.1 announcement](https://blog.meshcore.io/2026/08/14/release-1-17-1) (14 Aug 2026). This is a point release focused on fixes on top of v1.17.0.

### New Features

- New `radio.fem.rxgain` / `radio.fem.txgain` CLI commands (txgain support currently limited to Station G3)
- Missing KISS radio roles added, plus KISS radio build support

### Enhancements

- LR2021 preamble-detection and IRQ-timeout logic updates
- nRF52 CC310 optimization: `nRFCrypto` now initialized only once
- Improved RNG: combined entropy from radio + CC310 sources

### Critical Fixes

- Heltec T096 transmit failure, caused by an out-of-bounds `PIN_SPI1_MISO` configuration
- T-Echo Lite pin alignment and TCXO voltage adjustment (3.0V), matching revised Lilygo schematics
- R1 Neo and Minewsemi ME25LS01 build issues
- Rx boosted gain no longer resets to the compiled-in default after an AGC reset
- Multiple unused-pin corrections across nRF52 boards

### FEM Configuration Note

`radio.fem.rxgain` was not being persisted properly in earlier versions. Administrators with FEM-equipped devices should verify their FEM gain settings after updating.

See the [full MeshCore v1.17.1 changelog](https://blog.meshcore.io/2026/08/14/release-1-17-1) for complete details.

## Flashing

Download from the release assets → [flasher.meshcore.io](https://flasher.meshcore.io/) → **Custom firmware**.

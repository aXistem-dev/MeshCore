# SlunseCore Repeater Firmware v1.17.1

SlunseCore repeater builds track **MeshCore v1.17.1** with SlunseCore custom features preserved.

**Official upstream release:** [MeshCore v1.17.1 — 14 Aug 2026](https://blog.meshcore.io/2026/08/14/release-1-17-1)

## SlunseCore changes

- **GPS saver** — SlunseCore GPS power-save CLI commands (`gps saver`, `gps hold`, `gps interval`, `gps telem`, etc.; see `docs/cli_commands.md`), unaffected by this release.
- **SenseCap Solar** — Headless operation with remapped LoRa TX LED and USR/PWR button actions, carried forward unchanged.
- R1 Neo and Minewsemi ME25LS01 repeater builds now use upstream's own v1.17.1 build fixes directly rather than SlunseCore's earlier ad-hoc patches for the same gaps.

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
- Multiple unused-pin corrections across nRF52 boards

### FEM Configuration Note

`radio.fem.rxgain` was not being persisted properly in earlier versions. **Repeater administrators with FEM-equipped devices (e.g. Station G3) should verify their `radio.fem.rxgain`/`radio.fem.txgain` settings after updating.**

See the [full MeshCore v1.17.1 changelog](https://blog.meshcore.io/2026/08/14/release-1-17-1) for complete details.

## Flashing

Download from the release assets → [flasher.meshcore.io](https://flasher.meshcore.io/) → **Custom firmware**.

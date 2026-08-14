# SlunseCore Repeater Firmware v1.17.1

SlunseCore repeater builds track **MeshCore v1.17.1** with SlunseCore custom features preserved.

**Official upstream release:** [MeshCore v1.17.1 — 14 Aug 2026](https://blog.meshcore.io/2026/08/14/release-1-17-1)

## SlunseCore changes

- **GPS saver** — SlunseCore GPS power-save CLI commands (`gps saver`, `gps hold`, `gps interval`, `gps telem`, etc.; see `docs/cli_commands.md`), unaffected by this release.
- **SenseCap Solar** — Headless operation with remapped LoRa TX LED and USR/PWR button actions, carried forward unchanged.
- R1 Neo and Minewsemi ME25LS01 repeater builds now use upstream's own v1.17.1 build fixes directly rather than SlunseCore's earlier ad-hoc patches for the same gaps.

## MeshCore v1.17.1 (included)

Highlights from the [official release notes](https://blog.meshcore.io/2026/08/14/release-1-17-1):

- New `radio.fem.rxgain`/`radio.fem.txgain` CLI commands (txgain currently Station G3-only), plus a fix for FEM gain prefs not persisting correctly
- Additional KISS radio roles and build support
- Refined login-response handling to prevent flood messages; improved LR2021 preamble-detection logic
- Rx boosted gain no longer resets to the compiled-in default after an AGC reset
- Heltec T096 transmit failure fixed (out-of-bounds pin mapping); T-Echo Lite pin/TCXO updates matching revised schematics
- R1 Neo and Minewsemi ME25LS01 build fixes; ProMicro pinmap cleanup; Heltec T1/MeshPocket pin fixes
- nRF52: combined RNG entropy from radio + CC310, single-init crypto

See the [full MeshCore v1.17.1 changelog](https://blog.meshcore.io/2026/08/14/release-1-17-1) for all fixes and new devices.

**Note for FEM-equipped devices (e.g. Station G3):** verify your `radio.fem.rxgain`/`radio.fem.txgain` settings after updating, as this release fixes a persistence issue affecting those preferences.

## Flashing

Download from the release assets → [flasher.meshcore.io](https://flasher.meshcore.io/) → **Custom firmware**.

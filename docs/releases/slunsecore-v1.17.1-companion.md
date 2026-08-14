# SlunseCore Companion Firmware v1.17.1

SlunseCore companion builds track **MeshCore v1.17.1** and add SlunseCore custom features on top.

**Official upstream release:** [MeshCore v1.17.1 — 14 Aug 2026](https://blog.meshcore.io/2026/08/14/release-1-17-1)

## SlunseCore changes

Retained from earlier SlunseCore work: on-device **Settings** (`ui-new`) for buzzer, path hash, auto-add contacts, screen/timezone/advert settings, and boot logo, themed onto MeshCore's semantic `UIColor` display system with a working selection highlight on monochrome displays. Also retained: Packet FWD toggle, battery/USB indicator, and nRF52 idle sleep from upstream.

R1 Neo and Minewsemi ME25LS01 companion/repeater/room-server builds now use upstream's own v1.17.1 build fixes directly (see below) rather than SlunseCore's earlier ad-hoc patches for the same gaps.

**Known gap:** `ThinkNode_M5_companion_radio_serial` is disabled in this release — its `SERIAL_TX`/`SERIAL_RX` pins reference Arduino `D6`/`D7` aliases that don't exist for this board, and no schematic-verified GPIO pair is available yet (upstream has the same unresolved gap as of v1.17.1). All other ThinkNode M5 variants (BLE/USB/WiFi/repeater/room-server/etc.) are unaffected.

## MeshCore v1.17.1 (included)

Highlights from the [official release notes](https://blog.meshcore.io/2026/08/14/release-1-17-1):

- New `radio.fem.rxgain`/`radio.fem.txgain` CLI commands (txgain currently Station G3-only), plus a fix for FEM gain prefs not persisting correctly
- Additional KISS radio roles and build support
- Refined login-response handling to prevent flood messages; improved LR2021 preamble-detection logic
- Rx boosted gain no longer resets to the compiled-in default after an AGC reset
- Heltec T096 transmit failure fixed (out-of-bounds pin mapping); T-Echo Lite pin/TCXO updates matching revised schematics; T-Beam Supreme S3 display fix
- R1 Neo and Minewsemi ME25LS01 build fixes; ProMicro pinmap cleanup; Heltec T1/MeshPocket pin fixes
- nRF52: combined RNG entropy from radio + CC310, single-init crypto

See the [full MeshCore v1.17.1 changelog](https://blog.meshcore.io/2026/08/14/release-1-17-1) for all fixes and new devices.

**Note for FEM-equipped devices:** verify your `radio.fem.rxgain` and `radio.rxgain` settings after updating, as this release fixes a persistence issue affecting those preferences.

## Flashing

Download from the release assets → [flasher.meshcore.io](https://flasher.meshcore.io/) → **Custom firmware**.

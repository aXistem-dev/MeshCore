# SlunseCore Repeater Firmware v1.16.0

SlunseCore repeater builds track **MeshCore v1.16.0** with SlunseCore custom features preserved.

**Official upstream release:** [MeshCore v1.16.0 — 6 Jun 2026](https://blog.meshcore.io/2026/06/06/release-1-16-0)

## SlunseCore changes

- **GPS saver** — SlunseCore GPS power-save CLI commands (`gps saver`, `gps hold`, `gps interval`, `gps telem`, etc.; see `docs/cli_commands.md`).
- **SenseCap Solar** — Headless operation with remapped LoRa TX LED and USR/PWR button actions.

## MeshCore v1.16.0 (included)

Highlights from the [official release notes](https://blog.meshcore.io/2026/06/06/release-1-16-0):

- New CLI vars `flood.max.unscoped` and `flood.max.advert`
- ESP repeater power-saving improvements
- Extended ACK support, `region def` CLI, path hash fixes
- Longer preamble for lower spreading factors; new board support

See the [full MeshCore v1.16.0 changelog](https://blog.meshcore.io/2026/06/06/release-1-16-0) for all fixes and new devices.

## Flashing

Download from the release assets → [flasher.meshcore.io](https://flasher.meshcore.io/) → **Custom firmware**.

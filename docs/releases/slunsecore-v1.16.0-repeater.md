# SlunseCore Repeater Firmware v1.16.0

SlunseCore repeater builds are based on **MeshCore v1.16.0** with SlunseCore custom features preserved.

**Upstream release:** [MeshCore v1.16.0](https://github.com/meshcore-dev/MeshCore)

## SlunseCore changes

- **MeshCore v1.16.0 sync**: Flood limits, region defaults, power-saving CLI, path hash support, and related upstream fixes merged into `dev-slunsecore`.
- **GPS saver**: SlunseCore GPS power-save CLI commands retained (see `docs/cli_commands.md`).
- **SenseCap Solar**: Headless operation with remapped LoRa TX LED and button actions preserved.

## Flashing

Download the firmware from the release assets below, go to [flasher.meshcore.io](https://flasher.meshcore.io/), choose **Custom firmware** (at the bottom of the page), and flash the downloaded bin or zip.

---

## Full SlunseCore features

- SenseCap Solar headless button/LED behaviour
- GPS power-save boot-only and periodic modes
- GPS prefs sanitization on upgrade
- Telemetry location policy (`gps telem` CLI)

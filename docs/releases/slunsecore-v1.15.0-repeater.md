# SlunseCore Repeater Firmware v1.15.0

SlunseCore repeater builds are based on **MeshCore v1.15.0** with some extra features and tweaks.

**Upstream release:** [MeshCore v1.15.0 — 19 Apr 2026](https://blog.meshcore.io/2026/04/19/release-1-15-0)

## SlunseCore changes

- **SenseCap Solar:** Fixed remapped LEDs (LoRa TX = blue); PWR and USR buttons enabled — PWR holds to power off, USR taps for advert/GPS.
- **GPS saver (repeaters with GPS):** Extra CLI commands to configure GPS power-save behaviour on repeaters (see `docs/cli_commands.md` on this branch).

### New GPS CLI commands

| Command | Description |
|---|---|
| `gps saver [off\|bootonly\|periodic]` | Get or set power-save mode |
| `gps hold [5–240]` | Get or set GPS hold time (seconds) after fix |
| `gps timeout [1–15]` | Get or set how long (minutes) to search for a GPS fix before giving up and powering down the GPS |
| `gps interval [Nm\|Nh\|Nd\|sec]` | Get or set periodic wake interval (e.g. `30m`, `4h`, `1d`) — only applies when `gps saver` is `periodic` |
| `gps telem [deny\|allow\|always]` | Get or set whether GPS coords are included in telemetry responses — `deny`: nobody, `allow`: trusted users only, `always`: everybody |

## Flashing

Download the firmware from the release assets below, go to [flasher.meshcore.io](https://flasher.meshcore.io/), choose **Custom firmware** (at the bottom of the page), and flash the downloaded bin or zip.

---

## Full SlunseCore features

- SenseCap Solar headless:
  - LoRa TX LED remapped to pin 12 (blue)
  - Button actions:
    - USR double-tap: sends advert
    - USR triple-tap: toggles GPS on/off
    - PWR 1.5 s hold: powers off the device
  - LED feedback:
    - Power-on: white + blue solid 5 s
    - GPS searching: white slow blink
    - GPS fix acquired: white solid 3 s then off
    - GPS powered on: white 2 fast blinks
    - GPS powered off: white 3 fast blinks then off
    - Power-off: white + blue 5× fast blink
- GPS power-save boot-only mode: GPS stays on after boot until a fix is obtained or timeout elapses, then powers down
- GPS power-save periodic mode: powers GPS up at a configurable interval to refresh the fix (and clock sync)
- GPS prefs sanitization: hold/timeout/interval clamped to valid ranges on first boot after upgrade
- Telemetry location policy: control whether GPS coordinates are shared in telemetry responses (`gps telem`)

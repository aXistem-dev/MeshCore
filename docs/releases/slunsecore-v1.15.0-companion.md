# SlunseCore Companion Firmware v1.15.0

SlunseCore companion builds are based on **MeshCore v1.15.0** with some extra features and tweaks.

**Upstream release:** [MeshCore v1.15.0 — 19 Apr 2026](https://blog.meshcore.io/2026/04/19/release-1-15-0)

## SlunseCore changes

- **Packet FWD setting:** Added a settings menu entry to enable packet repeat/forward mode on the companion. Works on most frequencies — verify your frequency is correct after enabling.

## Flashing

Download the firmware from the release assets below, go to [flasher.meshcore.io](https://flasher.meshcore.io/), choose **Custom firmware** (at the bottom of the page), and flash the downloaded bin or zip.

---

## Full SlunseCore features

- SlunseCore boot screen logo
- Versioning: `sc-` prefix on firmware version; commit hash included in filenames for dev builds
- SenseCap Solar headless:
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
- Settings screen (`ui-new`):
  - Screen always on toggle
  - Screensaver toggle (visible only when screen always on is enabled)
  - Screen brightness: Dim / Low / Normal / Bright
  - Timezone selection
  - Share position in advertisements toggle
  - Packet FWD toggle (enables `client_repeat` on-device)
- Battery indicator: percentage as text with color coding (green/yellow/red) + charging/USB power detection

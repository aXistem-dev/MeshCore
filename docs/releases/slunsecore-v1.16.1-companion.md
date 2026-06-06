# SlunseCore Companion Firmware v1.16.1

SlunseCore companion builds are based on **MeshCore v1.16.0** with SlunseCore custom features and new Settings menu items.

**Upstream release:** [MeshCore v1.16.0](https://github.com/meshcore-dev/MeshCore)

## SlunseCore changes (v1.16.1)

- **Buzzer setting** (`ui-new`, boards with `PIN_BUZZER`): ON/OFF toggle in Settings. Triple-click shortcut on the home screen still works.
- **Path hash setting**: Choose 1-byte (default), 2-byte, or 3-byte path hash mode. A compatibility warning is shown before enabling multibyte modes — older repeaters may drop messages.
- **Auto-add contacts setting**: Configure automatic contact discovery without the phone app:
  - Mode: auto-add all types, or manual type selection
  - Types (manual mode): Chat, Repeater, Room, Sensor
  - Overwrite oldest when contact list is full
  - Max hops limit (unlimited, direct only, or 2–64 hops)
- **nRF52 power saving**: Upstream idle sleep is included (`board.sleep()` when no pending work). Device wakes for BLE traffic and outbound messages.

## MeshCore v1.16.0 upstream sync

This release includes the full MeshCore v1.16.0 merge: flood limits, region defaults, repeater power-saving CLI, path hash protocol support, auto-add prefs, and related fixes.

## Flashing

Download the firmware from the release assets below, go to [flasher.meshcore.io](https://flasher.meshcore.io/), choose **Custom firmware** (at the bottom of the page), and flash the downloaded bin or zip.

---

## Full SlunseCore features

- SlunseCore boot screen logo
- Versioning: `sc-` prefix on firmware version; commit hash included in filenames for dev builds
- SenseCap Solar headless button/LED behaviour (where applicable)
- Settings screen (`ui-new`):
  - Screen always on / screensaver / brightness / timezone
  - Share position in advertisements
  - Packet FWD toggle
  - Buzzer ON/OFF (when hardware supports it)
  - Path hash mode
  - Auto-add contacts
- Battery indicator with charging/USB detection

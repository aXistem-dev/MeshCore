# SlunseCore Companion Firmware v1.16.0

SlunseCore companion builds track **MeshCore v1.16.0** and add SlunseCore custom features on top.

**Official upstream release:** [MeshCore v1.16.0 — 6 Jun 2026](https://blog.meshcore.io/2026/06/06/release-1-16-0)

## SlunseCore changes

On-device **Settings** (`ui-new`) for preferences that were already in firmware but only reachable via the app or CLI:

- **Buzzer** — ON/OFF toggle (boards with `PIN_BUZZER`). Triple-click on the home screen still works.
- **Path hash** — 1-byte (default), 2-byte, or 3-byte mode, with a compatibility warning before non-default values.
- **Auto-add contacts** — mode (auto-all vs manual), type toggles (Chat / Repeater / Room / Sensor), overwrite-oldest when full, max-hops cap.

Also retained from earlier SlunseCore work: boot logo, Packet FWD, screen/timezone/advert settings, SenseCap Solar headless behaviour, battery/USB indicator, and nRF52 idle sleep from upstream.

## MeshCore v1.16.0 (included)

Highlights from the [official release notes](https://blog.meshcore.io/2026/06/06/release-1-16-0):

- New CLI vars `flood.max.unscoped` and `flood.max.advert`
- nRF52 companion power saving; ESP repeater power-saving improvements
- Extended ACK support, `region def` CLI, anonymous non-contact requests
- Companion raw packet send, default scope override, auto-shutdown on battery
- Longer preamble for lower spreading factors; new board support

See the [full MeshCore v1.16.0 changelog](https://blog.meshcore.io/2026/06/06/release-1-16-0) for all fixes and new devices.

## Flashing

Download from the release assets → [flasher.meshcore.io](https://flasher.meshcore.io/) → **Custom firmware**.

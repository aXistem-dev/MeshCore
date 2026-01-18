#!/usr/bin/env python3
import argparse
import re
import subprocess
import sys
import time

try:
    import serial
except ImportError:
    print("Missing dependency: pyserial. Install with: pip install pyserial", file=sys.stderr)
    sys.exit(1)

RX_LINE_RE = re.compile(
    r"RX len=(?P<len>\d+) hash=0x(?P<hash>[0-9A-Fa-f]+) SNR=(?P<snr>-?\d+(?:\.\d+)?) RSSI=(?P<rssi>-?\d+(?:\.\d+)?)"
)
RAW_LEN_RE = re.compile(r"RX raw_len=(?P<raw_len>\d+)")
RAW_TRUNC_RE = re.compile(r"RX raw_len=(?P<raw_len>\d+) truncated_to=(?P<len>\d+)")
HEAD_TAIL_RE = re.compile(r"RX head=(?P<head>[0-9A-Fa-f]+) tail=(?P<tail>[0-9A-Fa-f]+)")
RAW_DROP_RE = re.compile(r"RX raw_len dropped from (?P<prev>\d+) to (?P<curr>\d+)")
RAW_HEX_RE = re.compile(r"RAW:\s*(?P<hex>[0-9A-Fa-f]+)")


def main() -> int:
    parser = argparse.ArgumentParser(description="Monitor RadioLib RX debug output for duplicate/suspicious events.")
    parser.add_argument("port", help="Serial port (e.g. /dev/cu.usbmodem...)")
    parser.add_argument("--baud", type=int, default=115200, help="Serial baud rate (default: 115200)")
    parser.add_argument("--snr-drop", type=float, default=6.0, help="SNR drop threshold for duplicate alert (dB)")
    parser.add_argument("--alert-raw-drop", action="store_true", help="Alert on raw length drops (noisy)")
    parser.add_argument("--decode", action="store_true", help="Decode MeshCore packets on alerts (requires meshcore-decoder)")
    parser.add_argument("--decode-notes", action="store_true", help="Also decode packets for notes (implies --decode)")
    parser.add_argument("--decoder-cmd", default="meshcore-decoder", help="Decoder command (default: meshcore-decoder)")
    parser.add_argument("--decode-max-lines", type=int, default=20, help="Max decoder output lines to print")
    parser.add_argument("--log-file", help="Append alerts/notes to this file")
    parser.add_argument("--no-color", action="store_true", help="Disable ANSI colors")
    parser.add_argument("--min-gap-ms", type=int, default=200, help="Minimum time gap for duplicate alert")
    parser.add_argument("--show-all", action="store_true", help="Print all RX lines (default: only alerts)")
    args = parser.parse_args()

    if args.decode_notes:
        args.decode = True

    last_packet = None
    last_raw_len = None
    last_event_ts = 0.0
    current_packet = {}
    pending_raw_drop = False
    use_color = (not args.no_color) and sys.stdout.isatty()

    def colorize(text: str, color: str) -> str:
        if not use_color:
            return text
        colors = {
            "red": "\033[31m",
            "yellow": "\033[33m",
            "cyan": "\033[36m",
            "reset": "\033[0m",
        }
        return f"{colors.get(color, '')}{text}{colors['reset']}"

    def log_to_file(lines):
        if not args.log_file:
            return
        try:
            with open(args.log_file, "a", encoding="utf-8") as handle:
                for line in lines:
                    handle.write(line + "\n")
        except Exception as exc:
            print(f"Failed to write log file: {exc}", file=sys.stderr)

    def decode_packet(packet: dict):
        if not args.decode:
            return []
        raw_hex = packet.get("raw_hex")
        if not raw_hex:
            return ["Decoder skipped: no raw hex available (enable MESH_PACKET_LOGGING)."]
        try:
            result = subprocess.run(
                [args.decoder_cmd, "--structure", raw_hex],
                capture_output=True,
                text=True,
                timeout=2,
                check=False,
            )
        except FileNotFoundError:
            return [f"Decoder not found: {args.decoder_cmd} (install meshcore-decoder)."]
        except subprocess.TimeoutExpired:
            return ["Decoder timed out."]
        output = (result.stdout or result.stderr or "").strip()
        if not output:
            return ["Decoder returned no output."]
        lines = output.splitlines()
        if len(lines) > args.decode_max_lines:
            lines = lines[: args.decode_max_lines] + ["... (truncated)"]
        return [f"[decoder] {line}" for line in lines]

    def maybe_alert(reason: str, packet: dict):
        nonlocal last_event_ts
        now = time.time()
        if (now - last_event_ts) * 1000 < args.min_gap_ms:
            return
        last_event_ts = now
        header = colorize(f"[ALERT] {reason}", "red")
        line1 = f"  len={packet.get('len')} hash={packet.get('hash')} snr={packet.get('snr')} rssi={packet.get('rssi')}"
        print(f"\n{header}")
        print(line1)
        if packet.get("raw_len") is not None:
            head = packet.get("head")
            tail = packet.get("tail")
            if head and tail:
                line2 = f"  raw_len={packet.get('raw_len')} head={head} tail={tail}"
                print(line2)
            else:
                line2 = f"  raw_len={packet.get('raw_len')}"
                print(line2)
        else:
            line2 = None
        print("", flush=True)
        lines = [f"[ALERT] {reason}", line1]
        if line2:
            lines.append(line2)
        if args.decode:
            lines.extend(decode_packet(packet))
        log_to_file(lines)

    def note(reason: str, packet: dict):
        header = colorize(f"[NOTE] {reason}", "yellow")
        line1 = f"  len={packet.get('len')} hash={packet.get('hash')} snr={packet.get('snr')} rssi={packet.get('rssi')}"
        print(f"\n{header}")
        print(line1)
        if packet.get("raw_len") is not None:
            head = packet.get("head")
            tail = packet.get("tail")
            if head and tail:
                line2 = f"  raw_len={packet.get('raw_len')} head={head} tail={tail}"
                print(line2)
            else:
                line2 = f"  raw_len={packet.get('raw_len')}"
                print(line2)
        else:
            line2 = None
        print("", flush=True)
        lines = [f"[NOTE] {reason}", line1]
        if line2:
            lines.append(line2)
        if args.decode and args.decode_notes:
            lines.extend(decode_packet(packet))
        log_to_file(lines)

    with serial.Serial(args.port, args.baud, timeout=1) as ser:
        banner = colorize(f"Listening on {args.port} @ {args.baud} baud...", "cyan")
        print(banner, flush=True)
        while True:
            line = ser.readline()
            if not line:
                continue
            try:
                text = line.decode(errors="replace").strip()
            except Exception:
                continue

            if "RadioLibWrapper:" not in text:
                continue

            if args.show_all:
                print(text)

            match = RX_LINE_RE.search(text)
            if match:
                # Finalize previous packet if a raw drop was observed without head/tail
                if pending_raw_drop and current_packet:
                    if args.alert_raw_drop:
                        maybe_alert("Raw length drop detected", current_packet)
                    else:
                        note("Raw length drop detected (likely normal size variation)", current_packet)
                    pending_raw_drop = False
                current_packet = {
                    "len": int(match.group("len")),
                    "hash": match.group("hash").upper(),
                    "snr": float(match.group("snr")),
                    "rssi": float(match.group("rssi")),
                }
                if last_packet:
                    if (current_packet["len"] == last_packet["len"] and
                            current_packet["hash"] == last_packet["hash"]):
                        snr_drop = last_packet["snr"] - current_packet["snr"]
                        if snr_drop >= args.snr_drop:
                            maybe_alert(f"Duplicate hash/len with SNR drop {snr_drop:.1f} dB", current_packet)
                last_packet = current_packet.copy()
                continue

            match = RAW_TRUNC_RE.search(text)
            if match:
                raw_len = int(match.group("raw_len"))
                len_trunc = int(match.group("len"))
                current_packet["raw_len"] = raw_len
                if raw_len != len_trunc:
                    maybe_alert(f"Raw length truncated ({raw_len} -> {len_trunc})", current_packet)
                last_raw_len = raw_len
                continue

            match = RAW_LEN_RE.search(text)
            if match:
                raw_len = int(match.group("raw_len"))
                current_packet["raw_len"] = raw_len
                last_raw_len = raw_len
                continue

            match = RAW_DROP_RE.search(text)
            if match:
                prev = int(match.group("prev"))
                curr = int(match.group("curr"))
                current_packet["raw_len"] = curr
                current_packet["raw_drop"] = (prev, curr)
                pending_raw_drop = True
                last_raw_len = curr
                continue

            match = HEAD_TAIL_RE.search(text)
            if match:
                current_packet["head"] = match.group("head")
                current_packet["tail"] = match.group("tail")
                # If we have a previous packet with same head but different hash/len, flag.
                if last_packet and "head" in current_packet:
                    if (current_packet.get("head") == last_packet.get("head") and
                            (current_packet.get("hash") != last_packet.get("hash") or
                             current_packet.get("len") != last_packet.get("len"))):
                        maybe_alert("Same head with different hash/len (possible stale tail)", current_packet)
                if pending_raw_drop:
                    prev, curr = current_packet.get("raw_drop", (None, None))
                    if args.alert_raw_drop:
                        maybe_alert(f"Raw length drop detected ({prev} -> {curr})", current_packet)
                    else:
                        note(f"Raw length drop detected ({prev} -> {curr}) (likely normal size variation)", current_packet)
                    pending_raw_drop = False
                continue

            match = RAW_HEX_RE.search(text)
            if match:
                raw_hex = match.group("hex")
                if len(raw_hex) % 2 == 0:
                    current_packet["raw_hex"] = raw_hex.upper()
                continue

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

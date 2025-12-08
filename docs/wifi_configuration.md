# WiFi Configuration

This document describes the WiFi configuration capabilities available in MeshCore companion radio firmware, including runtime configuration of WiFi credentials and network settings.

## Overview

MeshCore companion radio devices support runtime configuration of WiFi settings through binary protocol commands. WiFi can be configured without requiring firmware recompilation or device reboot in most cases.

## Binary Protocol Commands

Companion radio devices use a binary protocol for WiFi configuration. All WiFi commands use command code 44 with subcommands in the second byte.

### Command Format

All WiFi commands follow this two-byte format:
- Byte 0: Command code `44`
- Byte 1: Subcommand (varies by operation)

### Command Codes

| Command | Code | Description |
|---------|------|-------------|
| `CMD_SET_WIFI_SSID` | 44 | Set WiFi SSID (2-byte command: code + subcommand) |
| `CMD_SET_WIFI_PASSWORD` | 44 | Set WiFi password (2-byte command: code + subcommand) |
| `CMD_GET_WIFI_SSID` | 44 | Get WiFi SSID (2-byte command: code + subcommand) |
| `CMD_GET_WIFI_PASSWORD` | 44 | Get WiFi password (2-byte command: code + subcommand) |
| `CMD_GET_WIFI_CONFIG` | 44 | Get WiFi network configuration (2-byte command: code + subcommand) |
| `CMD_SET_WIFI_ENABLED` | 44 | Enable/disable WiFi (2-byte command: code + subcommand) |
| `CMD_SET_WIFI_CONFIG` | 44 | Set static IP configuration (2-byte command: code + subcommand) |

### WiFi Command Sub-Commands

All WiFi commands use command code 44 with subcommands in byte 1:

| Subcommand | Code | Command | Description |
|------------|------|---------|-------------|
| `WIFI_SUBCMD_SET_SSID` | 0 | `CMD_SET_WIFI_SSID` | Set WiFi SSID |
| `WIFI_SUBCMD_SET_PASSWORD` | 1 | `CMD_SET_WIFI_PASSWORD` | Set WiFi password |
| `WIFI_SUBCMD_GET_SSID` | 2 | `CMD_GET_WIFI_SSID` | Get WiFi SSID |
| `WIFI_SUBCMD_GET_PASSWORD` | 3 | `CMD_GET_WIFI_PASSWORD` | Get WiFi password |
| `WIFI_SUBCMD_GET_CONFIG` | 4 | `CMD_GET_WIFI_CONFIG` | Get WiFi network configuration |
| `WIFI_SUBCMD_SET_ENABLED` | 5 | `CMD_SET_WIFI_ENABLED` | Enable/disable WiFi |
| `WIFI_SUBCMD_SET_CONFIG` | 6 | `CMD_SET_WIFI_CONFIG` | Set static IP configuration |

## Response Codes

| Response | Code | Description |
|----------|------|-------------|
| `RESP_CODE_WIFI` | 25 | WiFi response (2-byte response: code + subcommand) |

### WiFi Response Sub-Commands

The `RESP_CODE_WIFI` response uses a 2-byte header structure:
- **Byte 0:** `RESP_CODE_WIFI` (25)
- **Byte 1:** Response subcommand:
  - `0` - SSID data response
  - `1` - Password data response
  - `2` - Network configuration response

---

## Binary Frame Structures

All multi-byte integers use little-endian byte order unless otherwise specified (IP addresses use network byte order).

### CMD_SET_WIFI_SSID (44, 0)

**Total Frame Size:** 3 + ssid_len bytes (minimum 4 bytes, maximum 34 bytes)

| Offset | Size | Type | Field Name | Description | Range/Notes |
|--------|------|------|------------|-------------|-------------|
| 0 | 1 | uint8_t | command_code | Always `0x2C` (44) | - |
| 1 | 1 | uint8_t | subcommand | Always `0x00` (SET_SSID) | - |
| 2 | 1 | uint8_t | ssid_len | Length of SSID string | 1 - 31 |
| 3 | variable | char[] | ssid_data | SSID string bytes | UTF-8, null-terminated optional |

**Response:**
- **Success:** `[0x00]` (RESP_CODE_OK)
- **Error:** `[0x01, error_code]` (RESP_CODE_ERR + error code)

**Example Structure (C/C++):**
```c
struct CmdSetWifiSsid {
    uint8_t command_code;  // 0x2C
    uint8_t subcommand;    // 0x00
    uint8_t ssid_len;
    char    ssid[32];      // Variable length, max 31 chars
} __attribute__((packed));
```

---

### CMD_SET_WIFI_PASSWORD (44, 1)

**Total Frame Size:** 3 + pwd_len bytes (minimum 4 bytes, maximum 67 bytes)

| Offset | Size | Type | Field Name | Description | Range/Notes |
|--------|------|------|------------|-------------|-------------|
| 0 | 1 | uint8_t | command_code | Always `0x2C` (44) | - |
| 1 | 1 | uint8_t | subcommand | Always `0x01` (SET_PASSWORD) | - |
| 2 | 1 | uint8_t | pwd_len | Length of password string | 1 - 63 |
| 3 | variable | char[] | pwd_data | Password string bytes | UTF-8, null-terminated optional |

**Response:**
- **Success:** `[0x00]` (RESP_CODE_OK)
- **Error:** `[0x01, error_code]` (RESP_CODE_ERR + error code)

**Example Structure (C/C++):**
```c
struct CmdSetWifiPassword {
    uint8_t command_code;  // 0x2C
    uint8_t subcommand;    // 0x01
    uint8_t pwd_len;
    char    password[64];  // Variable length, max 63 chars
} __attribute__((packed));
```

---

### CMD_GET_WIFI_SSID (44, 2)

**Command Frame Size:** 2 bytes

| Offset | Size | Type | Field Name | Description | Range/Notes |
|--------|------|------|------------|-------------|-------------|
| 0 | 1 | uint8_t | command_code | Always `0x2C` (44) | - |
| 1 | 1 | uint8_t | subcommand | Always `0x02` (GET_SSID) | - |

**Response: RESP_CODE_WIFI + SSID (25, 0)**

**Total Frame Size:** 3 + ssid_len bytes (minimum 3 bytes, maximum 34 bytes)

| Offset | Size | Type | Field Name | Description | Range/Notes |
|--------|------|------|------------|-------------|-------------|
| 0 | 1 | uint8_t | response_code | Always `0x19` (25) | - |
| 1 | 1 | uint8_t | subcommand | Always `0x00` (SSID response) | - |
| 2 | 1 | uint8_t | ssid_len | Length of SSID string | 0 - 31 |
| 3 | variable | char[] | ssid_data | SSID string bytes | UTF-8, may be empty |

**Example Structure (C/C++):**
```c
struct RespWifiSsid {
    uint8_t response_code;  // 0x19
    uint8_t subcommand;     // 0x00
    uint8_t ssid_len;
    char    ssid[32];       // Variable length, max 31 chars
} __attribute__((packed));
```

---

### CMD_GET_WIFI_PASSWORD (44, 3)

**Command Frame Size:** 2 bytes

| Offset | Size | Type | Field Name | Description | Range/Notes |
|--------|------|------|------------|-------------|-------------|
| 0 | 1 | uint8_t | command_code | Always `0x2C` (44) | - |
| 1 | 1 | uint8_t | subcommand | Always `0x03` (GET_PASSWORD) | - |

**Response: RESP_CODE_WIFI + PASSWORD (25, 1)**

**Total Frame Size:** 3 + pwd_len bytes (minimum 3 bytes, maximum 67 bytes)

| Offset | Size | Type | Field Name | Description | Range/Notes |
|--------|------|------|------------|-------------|-------------|
| 0 | 1 | uint8_t | response_code | Always `0x19` (25) | - |
| 1 | 1 | uint8_t | subcommand | Always `0x01` (PASSWORD response) | - |
| 2 | 1 | uint8_t | pwd_len | Length of password string | 0 - 63 |
| 3 | variable | char[] | pwd_data | Password string bytes | UTF-8, may be empty |

**Example Structure (C/C++):**
```c
struct RespWifiPassword {
    uint8_t response_code;  // 0x19
    uint8_t subcommand;     // 0x01
    uint8_t pwd_len;
    char    password[64];   // Variable length, max 63 chars
} __attribute__((packed));
```

---

### CMD_GET_WIFI_CONFIG (44, 4)

**Command Frame Size:** 2 bytes

| Offset | Size | Type | Field Name | Description | Range/Notes |
|--------|------|------|------------|-------------|-------------|
| 0 | 1 | uint8_t | command_code | Always `0x2C` (44) | - |
| 1 | 1 | uint8_t | subcommand | Always `0x04` (GET_CONFIG) | - |

**Response: RESP_CODE_WIFI + CONFIG (25, 2)**

**Total Frame Size:** 24 + ssid_len bytes (minimum 24 bytes, maximum 56 bytes)

| Offset | Size | Type | Field Name | Description | Range/Notes |
|--------|------|------|------------|-------------|-------------|
| 0 | 1 | uint8_t | response_code | Always `0x19` (25) | - |
| 1 | 1 | uint8_t | subcommand | Always `0x02` (CONFIG response) | - |
| 2 | 1 | uint8_t | status | WiFi connection status | wl_status_t enum (see below) |
| 3 | 4 | uint8_t[4] | ip | Local IP address | IPv4 address, network byte order |
| 7 | 4 | uint8_t[4] | subnet | Subnet mask | IPv4 address, network byte order |
| 11 | 4 | uint8_t[4] | gateway | Gateway IP address | IPv4 address, network byte order |
| 15 | 4 | uint8_t[4] | dns1 | Primary DNS server | IPv4 address, network byte order |
| 19 | 4 | uint8_t[4] | dns2 | Secondary DNS server | IPv4 address, network byte order |
| 23 | 2 | int16_t | rssi | Signal strength | dBm, signed (-128 to +127) |
| 25 | 1 | uint8_t | ssid_len | Length of current SSID | 0 - 31 |
| 26 | variable | char[] | ssid_data | Current SSID string | UTF-8, may be empty |

**WiFi Status Values (wl_status_t):**

| Value | Name | Description |
|-------|------|-------------|
| 0 | `WL_IDLE_STATUS` | WiFi is in idle state |
| 1 | `WL_NO_SSID_AVAIL` | No SSID available |
| 2 | `WL_SCAN_COMPLETED` | Scan completed |
| 3 | `WL_CONNECTED` | Connected to WiFi network |
| 4 | `WL_CONNECT_FAILED` | Connection failed |
| 5 | `WL_CONNECTION_LOST` | Connection lost |
| 6 | `WL_DISCONNECTED` | Disconnected |
| 7 | `WL_AP_LISTENING` | Access point listening |
| 8 | `WL_AP_CONNECTED` | Access point connected |
| 9 | `WL_AP_FAILED` | Access point failed |

**Notes:**
- IP addresses are in network byte order (big-endian)
- If WiFi is not connected, IP addresses will be `0.0.0.0`
- RSSI value of -128 typically indicates no signal or not connected
- SSID field contains the currently connected network SSID, not the configured SSID

**Example Structure (C/C++):**
```c
struct RespWifiConfig {
    uint8_t  response_code;  // 0x19
    uint8_t  subcommand;     // 0x02
    uint8_t  status;
    uint8_t  ip[4];
    uint8_t  subnet[4];
    uint8_t  gateway[4];
    uint8_t  dns1[4];
    uint8_t  dns2[4];
    int16_t  rssi;
    uint8_t  ssid_len;
    char     ssid[32];       // Variable length, max 31 chars
} __attribute__((packed));
```

---

### CMD_SET_WIFI_ENABLED (44, 5)

**Total Frame Size:** 3 bytes

| Offset | Size | Type | Field Name | Description | Range/Notes |
|--------|------|------|------------|-------------|-------------|
| 0 | 1 | uint8_t | command_code | Always `0x2C` (44) | - |
| 1 | 1 | uint8_t | subcommand | Always `0x05` (SET_ENABLED) | - |
| 2 | 1 | uint8_t | enabled | Enable/disable flag | 0 = disable, 1 = enable |

**Response:**
- **Success:** `[0x00]` (RESP_CODE_OK)
- **Error:** `[0x01, error_code]` (RESP_CODE_ERR + error code)

**Behavior:**
- When enabled: WiFi enters station mode and attempts to connect if credentials are configured
- When disabled: WiFi disconnects and is turned off

**Example Structure (C/C++):**
```c
struct CmdSetWifiEnabled {
    uint8_t command_code;  // 0x2C
    uint8_t subcommand;    // 0x05
    uint8_t enabled;       // 0 or 1
} __attribute__((packed));
```

---

### CMD_SET_WIFI_CONFIG (44, 6)

**Total Frame Size:** 22 bytes

| Offset | Size | Type | Field Name | Description | Range/Notes |
|--------|------|------|------------|-------------|-------------|
| 0 | 1 | uint8_t | command_code | Always `0x2C` (44) | - |
| 1 | 1 | uint8_t | subcommand | Always `0x06` (SET_CONFIG) | - |
| 2 | 4 | uint8_t[4] | ip | Desired local IP address | IPv4 address, network byte order |
| 6 | 4 | uint8_t[4] | subnet | Subnet mask | IPv4 address, network byte order |
| 10 | 4 | uint8_t[4] | gateway | Gateway IP address | IPv4 address, network byte order |
| 14 | 4 | uint8_t[4] | dns1 | Primary DNS server | IPv4 address, network byte order |
| 18 | 4 | uint8_t[4] | dns2 | Secondary DNS server | IPv4 address, network byte order |

**Response:**
- **Success:** `[0x00]` (RESP_CODE_OK)
- **Error:** `[0x01, error_code]` (RESP_CODE_ERR + error code)

**Notes:**
- Configures static IP address, disabling DHCP
- IP addresses must be in network byte order (big-endian)
- Settings are applied immediately if WiFi is connected
- To re-enable DHCP, set all IP addresses to `0.0.0.0` (not officially supported, may require reboot)

**Example Structure (C/C++):**
```c
struct CmdSetWifiConfig {
    uint8_t command_code;  // 0x2C
    uint8_t subcommand;     // 0x06
    uint8_t ip[4];
    uint8_t subnet[4];
    uint8_t gateway[4];
    uint8_t dns1[4];
    uint8_t dns2[4];
} __attribute__((packed));
```

---

## Code Examples

### Python Examples

```python
import struct
import socket

# Constants
CMD_WIFI = 44
WIFI_SUBCMD_SET_SSID = 0
WIFI_SUBCMD_SET_PASSWORD = 1
WIFI_SUBCMD_GET_SSID = 2
WIFI_SUBCMD_GET_PASSWORD = 3
WIFI_SUBCMD_GET_CONFIG = 4
WIFI_SUBCMD_SET_ENABLED = 5
WIFI_SUBCMD_SET_CONFIG = 6

RESP_CODE_WIFI = 25
RESP_CODE_OK = 0
RESP_CODE_ERR = 1

def send_set_wifi_ssid(serial_interface, ssid: str):
    """Send command to set WiFi SSID"""
    ssid_bytes = ssid.encode('utf-8')
    if len(ssid_bytes) > 31:
        raise ValueError("SSID too long (max 31 bytes)")
    cmd = struct.pack('<BB B', CMD_WIFI, WIFI_SUBCMD_SET_SSID, len(ssid_bytes)) + ssid_bytes
    serial_interface.write(cmd)

def send_set_wifi_password(serial_interface, password: str):
    """Send command to set WiFi password"""
    pwd_bytes = password.encode('utf-8')
    if len(pwd_bytes) > 63:
        raise ValueError("Password too long (max 63 bytes)")
    cmd = struct.pack('<BB B', CMD_WIFI, WIFI_SUBCMD_SET_PASSWORD, len(pwd_bytes)) + pwd_bytes
    serial_interface.write(cmd)

def send_get_wifi_ssid(serial_interface):
    """Send command to get WiFi SSID"""
    cmd = struct.pack('<BB', CMD_WIFI, WIFI_SUBCMD_GET_SSID)
    serial_interface.write(cmd)

def send_get_wifi_password(serial_interface):
    """Send command to get WiFi password"""
    cmd = struct.pack('<BB', CMD_WIFI, WIFI_SUBCMD_GET_PASSWORD)
    serial_interface.write(cmd)

def send_get_wifi_config(serial_interface):
    """Send command to get WiFi configuration"""
    cmd = struct.pack('<BB', CMD_WIFI, WIFI_SUBCMD_GET_CONFIG)
    serial_interface.write(cmd)

def send_set_wifi_enabled(serial_interface, enabled: bool):
    """Send command to enable/disable WiFi"""
    cmd = struct.pack('<BB B', CMD_WIFI, WIFI_SUBCMD_SET_ENABLED, 1 if enabled else 0)
    serial_interface.write(cmd)

def send_set_wifi_config(serial_interface, ip: str, subnet: str, gateway: str, dns1: str, dns2: str):
    """Send command to set static IP configuration"""
    ip_bytes = socket.inet_aton(ip)
    subnet_bytes = socket.inet_aton(subnet)
    gateway_bytes = socket.inet_aton(gateway)
    dns1_bytes = socket.inet_aton(dns1)
    dns2_bytes = socket.inet_aton(dns2)
    cmd = struct.pack('<BB', CMD_WIFI, WIFI_SUBCMD_SET_CONFIG) + \
          ip_bytes + subnet_bytes + gateway_bytes + dns1_bytes + dns2_bytes
    serial_interface.write(cmd)

def parse_wifi_ssid_response(frame):
    """Parse RESP_CODE_WIFI + SSID response"""
    response_code, subcommand, ssid_len = struct.unpack('<B B B', frame[:3])
    if response_code != RESP_CODE_WIFI or subcommand != 0:
        raise ValueError("Invalid response type")
    ssid = frame[3:3+ssid_len].decode('utf-8', errors='ignore')
    return ssid

def parse_wifi_password_response(frame):
    """Parse RESP_CODE_WIFI + PASSWORD response"""
    response_code, subcommand, pwd_len = struct.unpack('<B B B', frame[:3])
    if response_code != RESP_CODE_WIFI or subcommand != 1:
        raise ValueError("Invalid response type")
    password = frame[3:3+pwd_len].decode('utf-8', errors='ignore')
    return password

def parse_wifi_config_response(frame):
    """Parse RESP_CODE_WIFI + CONFIG response"""
    response_code, subcommand, status = struct.unpack('<B B B', frame[:3])
    if response_code != RESP_CODE_WIFI or subcommand != 2:
        raise ValueError("Invalid response type")
    
    ip = socket.inet_ntoa(frame[3:7])
    subnet = socket.inet_ntoa(frame[7:11])
    gateway = socket.inet_ntoa(frame[11:15])
    dns1 = socket.inet_ntoa(frame[15:19])
    dns2 = socket.inet_ntoa(frame[19:23])
    rssi = struct.unpack('<h', frame[23:25])[0]
    ssid_len = frame[25]
    ssid = frame[26:26+ssid_len].decode('utf-8', errors='ignore')
    
    return {
        'status': status,
        'ip': ip,
        'subnet': subnet,
        'gateway': gateway,
        'dns1': dns1,
        'dns2': dns2,
        'rssi': rssi,
        'ssid': ssid
    }
```

### JavaScript/TypeScript Examples

```typescript
// Constants
const CMD_WIFI = 44;
const WIFI_SUBCMD_SET_SSID = 0;
const WIFI_SUBCMD_SET_PASSWORD = 1;
const WIFI_SUBCMD_GET_SSID = 2;
const WIFI_SUBCMD_GET_PASSWORD = 3;
const WIFI_SUBCMD_GET_CONFIG = 4;
const WIFI_SUBCMD_SET_ENABLED = 5;
const WIFI_SUBCMD_SET_CONFIG = 6;

const RESP_CODE_WIFI = 25;
const RESP_CODE_OK = 0;
const RESP_CODE_ERR = 1;

interface WifiConfig {
    status: number;
    ip: string;
    subnet: string;
    gateway: string;
    dns1: string;
    dns2: string;
    rssi: number;
    ssid: string;
}

function sendSetWifiSsid(serialInterface: SerialPort, ssid: string): void {
    const ssidBytes = new TextEncoder().encode(ssid);
    if (ssidBytes.length > 31) {
        throw new Error("SSID too long (max 31 bytes)");
    }
    const cmd = new Uint8Array([CMD_WIFI, WIFI_SUBCMD_SET_SSID, ssidBytes.length, ...ssidBytes]);
    serialInterface.write(cmd);
}

function sendSetWifiPassword(serialInterface: SerialPort, password: string): void {
    const pwdBytes = new TextEncoder().encode(password);
    if (pwdBytes.length > 63) {
        throw new Error("Password too long (max 63 bytes)");
    }
    const cmd = new Uint8Array([CMD_WIFI, WIFI_SUBCMD_SET_PASSWORD, pwdBytes.length, ...pwdBytes]);
    serialInterface.write(cmd);
}

function sendGetWifiSsid(serialInterface: SerialPort): void {
    const cmd = new Uint8Array([CMD_WIFI, WIFI_SUBCMD_GET_SSID]);
    serialInterface.write(cmd);
}

function sendGetWifiPassword(serialInterface: SerialPort): void {
    const cmd = new Uint8Array([CMD_WIFI, WIFI_SUBCMD_GET_PASSWORD]);
    serialInterface.write(cmd);
}

function sendGetWifiConfig(serialInterface: SerialPort): void {
    const cmd = new Uint8Array([CMD_WIFI, WIFI_SUBCMD_GET_CONFIG]);
    serialInterface.write(cmd);
}

function sendSetWifiEnabled(serialInterface: SerialPort, enabled: boolean): void {
    const cmd = new Uint8Array([CMD_WIFI, WIFI_SUBCMD_SET_ENABLED, enabled ? 1 : 0]);
    serialInterface.write(cmd);
}

function sendSetWifiConfig(
    serialInterface: SerialPort,
    ip: string,
    subnet: string,
    gateway: string,
    dns1: string,
    dns2: string
): void {
    const ipBytes = ipToBytes(ip);
    const subnetBytes = ipToBytes(subnet);
    const gatewayBytes = ipToBytes(gateway);
    const dns1Bytes = ipToBytes(dns1);
    const dns2Bytes = ipToBytes(dns2);
    const cmd = new Uint8Array([
        CMD_WIFI,
        WIFI_SUBCMD_SET_CONFIG,
        ...ipBytes,
        ...subnetBytes,
        ...gatewayBytes,
        ...dns1Bytes,
        ...dns2Bytes
    ]);
    serialInterface.write(cmd);
}

function ipToBytes(ip: string): Uint8Array {
    return new Uint8Array(ip.split('.').map(Number));
}

function parseWifiSsidResponse(buffer: ArrayBuffer): string {
    const view = new DataView(buffer);
    const response_code = view.getUint8(0);
    const subcommand = view.getUint8(1);
    if (response_code !== RESP_CODE_WIFI || subcommand !== 0) {
        throw new Error('Invalid response type');
    }
    const ssid_len = view.getUint8(2);
    const ssid_bytes = new Uint8Array(buffer, 3, ssid_len);
    return new TextDecoder('utf-8', { fatal: false }).decode(ssid_bytes);
}

function parseWifiPasswordResponse(buffer: ArrayBuffer): string {
    const view = new DataView(buffer);
    const response_code = view.getUint8(0);
    const subcommand = view.getUint8(1);
    if (response_code !== RESP_CODE_WIFI || subcommand !== 1) {
        throw new Error('Invalid response type');
    }
    const pwd_len = view.getUint8(2);
    const pwd_bytes = new Uint8Array(buffer, 3, pwd_len);
    return new TextDecoder('utf-8', { fatal: false }).decode(pwd_bytes);
}

function parseWifiConfigResponse(buffer: ArrayBuffer): WifiConfig {
    const view = new DataView(buffer);
    const response_code = view.getUint8(0);
    const subcommand = view.getUint8(1);
    if (response_code !== RESP_CODE_WIFI || subcommand !== 2) {
        throw new Error('Invalid response type');
    }
    
    const status = view.getUint8(2);
    const ip = bytesToIp(new Uint8Array(buffer, 3, 4));
    const subnet = bytesToIp(new Uint8Array(buffer, 7, 4));
    const gateway = bytesToIp(new Uint8Array(buffer, 11, 4));
    const dns1 = bytesToIp(new Uint8Array(buffer, 15, 4));
    const dns2 = bytesToIp(new Uint8Array(buffer, 19, 4));
    const rssi = view.getInt16(23, true);
    const ssid_len = view.getUint8(25);
    const ssid_bytes = new Uint8Array(buffer, 26, ssid_len);
    const ssid = new TextDecoder('utf-8', { fatal: false }).decode(ssid_bytes);
    
    return {
        status,
        ip,
        subnet,
        gateway,
        dns1,
        dns2,
        rssi,
        ssid
    };
}

function bytesToIp(bytes: Uint8Array): string {
    return Array.from(bytes).join('.');
}
```

---

## WiFi Initialization

On device startup, WiFi initialization follows this priority:

1. **Runtime Preferences** (if `wifi_enabled` is set and SSID is configured)
2. **Compile-time Flags** (legacy support for `WIFI_SSID` and `WIFI_PWD` defines)

WiFi will only connect automatically if:
- `wifi_enabled` is set to 1 (for runtime prefs)
- A valid SSID is configured
- WiFi credentials are available

## Data Persistence

WiFi settings are stored in the companion radio's preferences file (`/new_prefs`, NodePrefs structure). Settings persist across reboots and power cycles.

## Platform Support

WiFi configuration commands are available on ESP32-based devices. Other platforms (NRF52, RP2040, STM32) do not support WiFi functionality.

## Error Codes

When a command fails, the response is `[RESP_CODE_ERR (1), error_code]`:

| Error Code | Name | Description |
|------------|------|-------------|
| 1 | `ERR_CODE_UNSUPPORTED_CMD` | Command not supported on this platform |
| 6 | `ERR_CODE_ILLEGAL_ARG` | Invalid parameter (e.g., SSID/password too long, invalid IP address) |

## Field Size Considerations

- **SSID:** Maximum 31 bytes (UTF-8 encoded)
- **Password:** Maximum 63 bytes (UTF-8 encoded)
- **IP Addresses:** 4 bytes each, network byte order (big-endian)
- **RSSI:** Signed 16-bit integer, range -128 to +127 dBm
- **Status:** 8-bit enum, see wl_status_t values above

## Notes

- WiFi SSID and password fields support up to 31 and 63 characters respectively
- Static IP configuration overrides DHCP settings
- WiFi must be enabled (`wifi_enabled = 1`) for automatic connection on startup
- Companion radio devices apply WiFi settings immediately when using the enable command
- IP addresses in binary frames use network byte order (big-endian), while other multi-byte integers use little-endian

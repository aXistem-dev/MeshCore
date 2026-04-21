#pragma once

// Dedicated internal-heap arena for mbedtls allocations.
//
// On non-PSRAM ESP32 targets running two concurrent MQTTS sessions (~61 KB
// steady-state per session), the main internal heap accumulates fragmentation
// from repeated TLS handshake allocate/free cycles. This module carves out a
// contiguous internal-heap region at boot, wraps it in a multi_heap handle,
// and installs application-level mbedtls calloc/free hooks via
// mbedtls_platform_set_calloc_free().
//
// The goal is isolation, not shrinkage: steady-state TLS memory cost is
// unchanged, but fragmentation is confined to the arena where it cannot
// degrade main-heap largest-free-block measurements that gate new TLS
// handshakes.
//
// Arena size is controlled by the MQTT_MBEDTLS_ARENA_BYTES build flag. When
// unset the module compiles to inert stubs — no arena, no hook installation,
// identical behaviour to pre-arena builds.
//
// Safe fallback: if the boot-time reservation or hook installation fails,
// mbedtls continues to use its default (ESP-IDF-provided) calloc/free against
// the main internal heap. Existing runtime code paths are unchanged.

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Reserve a contiguous internal-heap region of arena_bytes and install the
// mbedtls allocation hooks. Must be called exactly once, before any TLS
// client performs a handshake. Subsequent calls are no-ops and return the
// current activation state.
bool mbedtlsArenaInit(size_t arena_bytes);

// True once the arena has been initialised and the mbedtls hooks are live.
bool mbedtlsArenaIsActive(void);

// Total arena size in bytes as passed to mbedtlsArenaInit().
size_t mbedtlsArenaCapacity(void);

// Sum of currently unallocated bytes inside the arena.
size_t mbedtlsArenaFreeSize(void);

// Largest contiguous free block inside the arena. This is the arena-local
// equivalent of heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL) and is
// the key metric for diagnosing arena-internal fragmentation.
size_t mbedtlsArenaLargestFreeBlock(void);

// Historical minimum of free bytes inside the arena (watermark).
size_t mbedtlsArenaMinimumFreeSize(void);

#ifdef __cplusplus
}
#endif

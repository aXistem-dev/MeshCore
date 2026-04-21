#include "MbedtlsArena.h"

#if defined(ESP_PLATFORM) && defined(MQTT_MBEDTLS_ARENA_BYTES)

#include <string.h>
#include <Arduino.h>
#include <esp_heap_caps.h>
#include <multi_heap.h>
#include <mbedtls/platform.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Mirror MQTTBridge's logging macro locally so the arena can log before
// MQTTBridge::begin() finishes wiring up its own debug path.
#if defined(MQTT_DEBUG)
  #define ARENA_LOG(fmt, ...) Serial.printf("MQTT: " fmt "\n", ##__VA_ARGS__)
#else
  #define ARENA_LOG(...) ((void)0)
#endif

namespace {

void*                s_arena_storage = nullptr;
size_t               s_arena_bytes   = 0;
multi_heap_handle_t  s_arena_heap    = nullptr;
SemaphoreHandle_t    s_arena_lock    = nullptr;
bool                 s_active        = false;

inline void arena_lock() {
  if (s_arena_lock != nullptr) {
    xSemaphoreTakeRecursive(s_arena_lock, portMAX_DELAY);
  }
}

inline void arena_unlock() {
  if (s_arena_lock != nullptr) {
    xSemaphoreGiveRecursive(s_arena_lock);
  }
}

extern "C" void* arena_calloc(size_t n, size_t sz) {
  if (n == 0 || sz == 0) return nullptr;
  size_t total = n * sz;
  if (sz != 0 && total / sz != n) return nullptr;  // overflow guard

  arena_lock();
  void* p = multi_heap_malloc(s_arena_heap, total);
  arena_unlock();

  if (p != nullptr) {
    memset(p, 0, total);
  }
  return p;
}

extern "C" void arena_free(void* p) {
  if (p == nullptr) return;
  arena_lock();
  multi_heap_free(s_arena_heap, p);
  arena_unlock();
}

}  // namespace

extern "C" bool mbedtlsArenaInit(size_t arena_bytes) {
  if (s_active) return true;

  // A minimum reservation large enough for a single TLS handshake + session
  // context. Below this the arena would exhaust during the first connect and
  // degrade worse than the default behaviour.
  constexpr size_t kMinArenaBytes = 48 * 1024;
  if (arena_bytes < kMinArenaBytes) {
    ARENA_LOG("MbedtlsArena: requested %u bytes below minimum %u, skipping",
              (unsigned)arena_bytes, (unsigned)kMinArenaBytes);
    return false;
  }

  size_t largest_before = heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL);
  size_t free_before    = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);

  if (arena_bytes > largest_before) {
    ARENA_LOG("MbedtlsArena: cannot reserve %u, internal largest_free=%u free=%u",
              (unsigned)arena_bytes, (unsigned)largest_before, (unsigned)free_before);
    return false;
  }

  s_arena_storage = heap_caps_malloc(arena_bytes, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  if (s_arena_storage == nullptr) {
    ARENA_LOG("MbedtlsArena: heap_caps_malloc(%u) returned null", (unsigned)arena_bytes);
    return false;
  }

  s_arena_heap = multi_heap_register(s_arena_storage, arena_bytes);
  if (s_arena_heap == nullptr) {
    ARENA_LOG("MbedtlsArena: multi_heap_register failed");
    heap_caps_free(s_arena_storage);
    s_arena_storage = nullptr;
    return false;
  }

  s_arena_lock = xSemaphoreCreateRecursiveMutex();
  if (s_arena_lock == nullptr) {
    ARENA_LOG("MbedtlsArena: xSemaphoreCreateRecursiveMutex failed");
    // We cannot safely proceed without serialization — mbedtls allocations
    // can originate from multiple tasks (MQTT task, OTA task, WiFi task).
    heap_caps_free(s_arena_storage);
    s_arena_storage = nullptr;
    s_arena_heap    = nullptr;
    return false;
  }

  int rc = mbedtls_platform_set_calloc_free(arena_calloc, arena_free);
  if (rc != 0) {
    ARENA_LOG("MbedtlsArena: mbedtls_platform_set_calloc_free rc=%d", rc);
    vSemaphoreDelete(s_arena_lock);
    s_arena_lock    = nullptr;
    heap_caps_free(s_arena_storage);
    s_arena_storage = nullptr;
    s_arena_heap    = nullptr;
    return false;
  }

  s_arena_bytes = arena_bytes;
  s_active      = true;

  size_t largest_after = heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL);
  size_t arena_free_initial = multi_heap_free_size(s_arena_heap);
  ARENA_LOG("MbedtlsArena: active size=%u arena_free=%u main_largest=%u->%u main_free=%u",
            (unsigned)arena_bytes,
            (unsigned)arena_free_initial,
            (unsigned)largest_before,
            (unsigned)largest_after,
            (unsigned)free_before);
  return true;
}

extern "C" bool mbedtlsArenaIsActive(void) { return s_active; }

extern "C" size_t mbedtlsArenaCapacity(void) { return s_active ? s_arena_bytes : 0; }

extern "C" size_t mbedtlsArenaFreeSize(void) {
  if (!s_active) return 0;
  arena_lock();
  size_t v = multi_heap_free_size(s_arena_heap);
  arena_unlock();
  return v;
}

extern "C" size_t mbedtlsArenaLargestFreeBlock(void) {
  if (!s_active) return 0;
  multi_heap_info_t info = {};
  arena_lock();
  multi_heap_get_info(s_arena_heap, &info);
  arena_unlock();
  return info.largest_free_block;
}

extern "C" size_t mbedtlsArenaMinimumFreeSize(void) {
  if (!s_active) return 0;
  arena_lock();
  size_t v = multi_heap_minimum_free_size(s_arena_heap);
  arena_unlock();
  return v;
}

#else  // !(ESP_PLATFORM && MQTT_MBEDTLS_ARENA_BYTES)

extern "C" bool   mbedtlsArenaInit(size_t) { return false; }
extern "C" bool   mbedtlsArenaIsActive(void)          { return false; }
extern "C" size_t mbedtlsArenaCapacity(void)          { return 0; }
extern "C" size_t mbedtlsArenaFreeSize(void)          { return 0; }
extern "C" size_t mbedtlsArenaLargestFreeBlock(void)  { return 0; }
extern "C" size_t mbedtlsArenaMinimumFreeSize(void)   { return 0; }

#endif

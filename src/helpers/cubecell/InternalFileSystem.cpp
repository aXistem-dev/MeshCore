#include <Arduino.h>
#include <CyFlash.h>
#include <string.h>
#include "InternalFileSystem.h"

extern int FLASH_update(uint32_t dst_addr, const void *data, uint32_t size);
extern int FLASH_read_at(uint32_t address, uint8_t *pData, uint32_t len_bytes);

static int _cubecell_flash_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size) {
  if (!buffer || !size) return LFS_ERR_INVAL;
  uint32_t address = LFS_FLASH_ADDR_BASE + (block * LFS_BLOCK_SIZE + off);
  FLASH_read_at(address, (uint8_t *)buffer, size);
  return LFS_ERR_OK;
}

static int _cubecell_flash_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size) {
  uint32_t address = LFS_FLASH_ADDR_BASE + (block * LFS_BLOCK_SIZE + off);
  if (address < LFS_FLASH_ADDR_BASE || (address + size) > CUBECELL_FLASH_SIZE) {
    return LFS_ERR_INVAL;
  }
  return FLASH_update(address, buffer, size) == 0 ? LFS_ERR_OK : LFS_ERR_IO;
}

static int _cubecell_flash_erase(const struct lfs_config *c, lfs_block_t block) {
  uint32_t address = LFS_FLASH_ADDR_BASE + (block * LFS_BLOCK_SIZE);
  if (address < LFS_FLASH_ADDR_BASE || address >= CUBECELL_FLASH_SIZE) {
    return LFS_ERR_INVAL;
  }
  uint8_t page_cache[CY_FLASH_SIZEOF_ROW];
  memset(page_cache, 0xFF, sizeof(page_cache));
  uint32_t row = address / CY_FLASH_SIZEOF_ROW;
  return CySysFlashWriteRow(row, page_cache) == CY_SYS_FLASH_SUCCESS ? LFS_ERR_OK : LFS_ERR_IO;
}

static int _cubecell_flash_sync(const struct lfs_config *c) {
  return LFS_ERR_OK;
}

struct lfs_config _InternalFSConfig = {
  .context = NULL,
  .read = _cubecell_flash_read,
  .prog = _cubecell_flash_prog,
  .erase = _cubecell_flash_erase,
  .sync = _cubecell_flash_sync,
  .read_size = LFS_BLOCK_SIZE,
  .prog_size = LFS_BLOCK_SIZE,
  .block_size = LFS_BLOCK_SIZE,
  .block_count = LFS_FLASH_TOTAL_SIZE / LFS_BLOCK_SIZE,
  .lookahead = 64,
  .read_buffer = NULL,
  .prog_buffer = NULL,
  .lookahead_buffer = NULL,
  .file_buffer = NULL
};

InternalFileSystem InternalFS;

InternalFileSystem::InternalFileSystem(void)
  : Adafruit_LittleFS(&_InternalFSConfig) {
}

bool InternalFileSystem::begin(void) {
  bool format_fs = false;
#ifdef FORMAT_FS
  format_fs = true;
#endif
  if (format_fs || !Adafruit_LittleFS::begin()) {
    this->format();
    if (!Adafruit_LittleFS::begin()) return false;
  }
  return true;
}

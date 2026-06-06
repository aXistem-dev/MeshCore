#ifndef INTERNALFILESYSTEM_H_
#define INTERNALFILESYSTEM_H_

#include "Adafruit_LittleFS.h"

#ifndef LFS_FLASH_TOTAL_SIZE
  #define LFS_FLASH_TOTAL_SIZE (12 * 1024)
#endif
#define LFS_BLOCK_SIZE (256)
#define CUBECELL_FLASH_SIZE 0x1C000
#define LFS_FLASH_ADDR_BASE (CUBECELL_FLASH_SIZE - LFS_FLASH_TOTAL_SIZE)

class InternalFileSystem : public Adafruit_LittleFS {
public:
  InternalFileSystem(void);
  bool begin(void);
};

extern InternalFileSystem InternalFS;

#endif

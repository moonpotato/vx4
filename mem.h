#pragma once

#include "error.h"

#include <stdint.h>
#include <stdbool.h>

/////////////////////////////////////////////////////////////////////////
// Type declarations
/////////////////////////////////////////////////////////////////////////

typedef uint32_t maddr_t;
typedef uint32_t msize_t;
typedef uint8_t mblock_t;

/////////////////////////////////////////////////////////////////////////
// Constants + helper macros
/////////////////////////////////////////////////////////////////////////

#define MEM_NUM_BLKS 4096 // 2^12
#define MEM_BLK_SIZE (1u << 20) // 1MiB blocks

#define MEM_BLOCK_IN(addr) ((addr) >> 20)
#define MEM_BLOCK_MASK(addr) ((addr) & 0xFFFFF) // Last 20 bits

/////////////////////////////////////////////////////////////////////////
// Function declarations
/////////////////////////////////////////////////////////////////////////

extern error_t mem_read_byte(maddr_t base, uint8_t *dest);
extern error_t mem_read_dbyte(maddr_t base, uint16_t *dest);
extern error_t mem_read_word(maddr_t base, uint32_t *dest);

extern error_t mem_write_byte(maddr_t base, uint8_t val);
extern error_t mem_write_dbyte(maddr_t base, uint16_t val);
extern error_t mem_write_word(maddr_t base, uint32_t val);

extern uint32_t mem_read_string(maddr_t base, char *dest, msize_t max);
extern uint32_t mem_read_mem(maddr_t base, void *dest, msize_t max);

extern uint32_t mem_write_string(maddr_t base, const char *src);
extern uint32_t mem_write_mem(maddr_t base, const void *src, msize_t num);

extern error_t mem_set_bytes(maddr_t base, uint8_t val, msize_t num);
extern error_t mem_set_dbytes(maddr_t base, uint16_t val, msize_t num);
extern error_t mem_set_words(maddr_t base, uint32_t val, msize_t num);

extern error_t mem_map_device(maddr_t base, mblock_t *mem);
extern error_t mem_unmap_device(maddr_t base);

extern mblock_t *mem_raw_block(maddr_t base, bool create);
extern void mem_dump();


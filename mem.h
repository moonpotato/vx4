#pragma once

#include "error.h"

#include <stdint.h>
#include <stdbool.h>

/////////////////////////////////////////////////////////////////////////
// Type declarations
/////////////////////////////////////////////////////////////////////////

typedef uint32_t mem_addr; // A virtual CPU memory address
typedef uint32_t mem_size; // Size type for virtual CPU memory
typedef uint8_t mem_block; // Byte type to allow byte-wise memory access

/////////////////////////////////////////////////////////////////////////
// Constants + helper macros
/////////////////////////////////////////////////////////////////////////

// MEM_NUM_BLKS * MEM_BLK_SIZE should be 2^32
#define MEM_NUM_BLKS 4096 // 2^12
#define MEM_BLK_SIZE (1u << 20) // 1MiB blocks

#define MEM_BLOCK_IN(addr) ((addr) >> 20)
#define MEM_BLOCK_MASK(addr) ((addr) & 0xFFFFF) // Last 20 bits

/////////////////////////////////////////////////////////////////////////
// Function declarations
/////////////////////////////////////////////////////////////////////////

/**
 * Reads size-aligned data from memory and stores the value. Previously
 * untouched memory is initialized to 0 by default, unless it is part
 * of a virtual device mapping.
 *
 * IN base: The address from which to read the data.
 * OUT dest: A location to store the read data.
 *
 * Returns:
 * ERR_NOERR: Data was successfully read.
 * ERR_INVAL: The provided address was not correctly aligned for the
 * requested size of data.
 */
extern error_t mem_read_byte(mem_addr base, uint8_t *dest);
extern error_t mem_read_dbyte(mem_addr base, uint16_t *dest);
extern error_t mem_read_word(mem_addr base, uint32_t *dest);

/**
 * Writes data to a size-aligned location in memory.
 *
 * IN base: The address at which to store the data.
 * IN val: The data to write.
 *
 * Returns:
 * ERR_NOERR: Data was successfully written.
 * ERR_INVAL: The provided address was not correctly aligned for the
 * size of data provided.
 */
extern error_t mem_write_byte(mem_addr base, uint8_t val);
extern error_t mem_write_dbyte(mem_addr base, uint16_t val);
extern error_t mem_write_word(mem_addr base, uint32_t val);

/**
 * Copies a null-terminated string from memory to a buffer. If the
 * string is too long, it is truncated in order to preserve the
 * null termination.
 *
 * IN base: The address to begin reading.
 * OUT dest: A buffer large enough to store num read bytes.
 * IN num: Maximum number of bytes to copy, including the final NULL.
 *
 * Returns: The number of bytes read.
 */
extern uint32_t mem_read_string(mem_addr base, char *dest, mem_size max);

/**
 * Reads from a set span of memory into a buffer.
 *
 * IN base: The address to begin reading.
 * OUT dest: A buffer large enough to store num read bytes.
 * IN num: Number of bytes to copy.
 *
 * Returns: The number of bytes read. A value != num indicates error.
 */
extern uint32_t mem_read_mem(mem_addr base, void *dest, mem_size num);

/**
 * Writes a null-terminated string into memory, including the
 * null terminator.
 *
 * IN base: The address to begin writing.
 * IN src: The source buffer, can be of any length.
 *
 * Returns: The number of bytes written.
 */
extern uint32_t mem_write_string(mem_addr base, const char *src);

/**
 * Writes from a given buffer into memory at a specific location.
 *
 * IN base: The address to begin writing.
 * IN src: The source buffer, must be of at least length num.
 * IN num: Number of bytes to copy.
 *
 * Returns: The number of bytes written. A value != num indicates error.
 */
extern uint32_t mem_write_mem(mem_addr base, const void *src, mem_size num);

/**
 * Fills a size-aligned block of memory with a given value.
 *
 * IN base: The address to begin filling.
 * IN val: The value to repeat.
 * IN num: Number of writes to do.
 *
 * A total of sizeof(val) * num bytes will be written.
 *
 * Returns:
 * ERR_NOERR: The writes completed successfully.
 * ERR_INVAL: The provided address was not correctly aligned for the
 * requested size.
 */
extern error_t mem_set_bytes(mem_addr base, uint8_t val, mem_size num);
extern error_t mem_set_dbytes(mem_addr base, uint16_t val, mem_size num);
extern error_t mem_set_words(mem_addr base, uint32_t val, mem_size num);

/**
 * Maps a custom block of memory into the virtual address space.
 * Facilitates virtual memory-mapped devices. The memory block provided
 * MUST remain allocated until the correspoding call to mem_unamp_device.
 *
 * IN base: The block-aligned address to begin the mapping.
 * IN mem: The block of memory to map, must be of at least length MEM_BLK_SIZE.
 *
 * Returns:
 * ERR_NOERR: The memory was successfully mapped.
 * ERR_INVAL: The address specified was not a block boundary.
 * ERR_PCOND: The address specified refers to a block that is already mapped.
 */
extern error_t mem_map_device(mem_addr base, mem_block *mem);

/**
 * Unmaps a custom memory mapping from the virtual address space, returning
 * it to main system memory. The caller is now free to delete the memory used.
 *
 * IN base: The block-aligned address of the mapping to be delete.
 *
 * Returns:
 * ERR_NOERR: The memory was successfully unmapped.
 * ERR_INVAL: The address specified was not a block boundary.
 * ERR_PCOND: The address specified is not currently part of a mapping.
 */
extern error_t mem_unmap_device(mem_addr base);

/**
 * Retrieves the memory currently being used to hold a given block.
 *
 * IN base: The starting address of a block.
 * IN create: Load new memory if the block is currently unloaded?
 *
 * Returns: A pointer to the memory used to store the block, or NULL if:
 * - The base address given refers to no block (is not block aligned).
 * - The block is unloaded and mem_raw_block was called with create = false.
 */
extern mem_block *mem_raw_block(mem_addr base, bool create);

/**
 * Cause all loaded blocks to be written to files. Each block is written
 * to a file with name XXXX.dump, where XXXX is the block number in base-10.
 */
extern void mem_dump();


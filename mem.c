#include "mem.h"

#include "error.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/////////////////////////////////////////////////////////////////////////
// Internal constants + helper macros
/////////////////////////////////////////////////////////////////////////

#define IS_BLOCK_ALIGNED(addr) (MEM_BLOCK_MASK(addr) == 0)
#define IS_DBYTE_ALIGNED(addr) (((addr) & 0x1) == 0)
#define IS_WORD_ALIGNED(addr) (((addr) & 0x3) == 0)

/////////////////////////////////////////////////////////////////////////
// Module internal declarations
/////////////////////////////////////////////////////////////////////////

/**
 * Each entry defaults to being unmapped, and is mapped only
 * when required.
 */
typedef struct _mem_blk_entry {
	enum _mem_type {
		MAP_NONE,
		MAP_SYSTEM,
		MAP_DEVICE,
	} type;

	mblock_t *base; // Pointer to the beginning of a MEM_BLK_SIZE array
} mem_blk_entry;

// Every entry is prefilled to MAP_NONE (unloaded).
static mem_blk_entry memory[MEM_NUM_BLKS];

/**
 * Sets an empty block to be part of main system memory, and allocates
 * memory to store the block. The block type after successful completion
 * of this function is MAP_SYSTEM.
 *
 * IN block: The block to operate on.
 *
 * Returns:
 * ERR_NOERR: The block was successfully created.
 * ERR_PCOND: The block wasn't empty (MAP_NONE).
 */
static error_t create_system_block(mem_blk_entry *block);

/**
 * Deletes a block of memory previously allocated as system memory,
 * and sets it to be unused (MAP_NONE).
 *
 * IN block: The block to clear.
 *
 * Returns:
 * ERR_NOERR: The block was successfully deleted.
 * ERR_PCOND: The block wasn't MAP_SYSTEM.
 */
static error_t delete_system_block(mem_blk_entry *block);

/**
 * Sets an empty block to be part of a memory-mapped virtual device.
 * The memory to map must be provided, and remain allocated until
 * a corresponding call to remove_device_block. On successful completion,
 * the block type is MAP_DEVICE.
 *
 * IN block: The block to operate on.
 * IN mem: A block of memory at least MEM_BLK_SIZE in length.
 *
 * Returns:
 * ERR_NOERR: The block was successfully mapped.
 * ERR_PCOND: The block wasn't empty (MAP_NONE).
 */
static error_t install_device_block(mem_blk_entry *block, mblock_t *mem);

/**
 * Deletes a block of memory previously allocated as virtual device
 * memory, and sets it to be unused (MAP_NONE). After successful
 * completion, the memory used to map the block IS NOT delete, this
 * is the responsibility of the caller.
 *
 * IN block: The block to clear.
 *
 * Returns:
 * ERR_NOERR: The block was successfully deleted.
 * ERR_PCOND: The block wasn't MAP_SYSTEM.
 */
static error_t remove_device_block(mem_blk_entry *block);

/////////////////////////////////////////////////////////////////////////
// Interface functions
/////////////////////////////////////////////////////////////////////////

error_t mem_read_byte(maddr_t base, uint8_t *dest)
{
	mem_blk_entry *blk = &memory[MEM_BLOCK_IN(base)];
	maddr_t off = MEM_BLOCK_MASK(base);

	create_system_block(blk);

	*dest = blk->base[off];

	return ERR_NOERR;
}

error_t mem_read_dbyte(maddr_t base, uint16_t *dest)
{
	if (!IS_DBYTE_ALIGNED(base)) {
		return ERR_INVAL;
	}

	mem_blk_entry *blk = &memory[MEM_BLOCK_IN(base)];
	maddr_t off = MEM_BLOCK_MASK(base);

	create_system_block(blk);

	*dest = *(uint16_t *)&blk->base[off];

	return ERR_NOERR;
}

error_t mem_read_word(maddr_t base, uint32_t *dest)
{
	if (!IS_WORD_ALIGNED(base)) {
		return ERR_INVAL;
	}

	mem_blk_entry *blk = &memory[MEM_BLOCK_IN(base)];
	maddr_t off = MEM_BLOCK_MASK(base);

	create_system_block(blk);

	*dest = *(uint32_t *)&blk->base[off];

	return ERR_NOERR;
}

error_t mem_write_byte(maddr_t base, uint8_t val)
{
	mem_blk_entry *blk = &memory[MEM_BLOCK_IN(base)];
	maddr_t off = MEM_BLOCK_MASK(base);

	create_system_block(blk);

	blk->base[off] = val;

	return ERR_NOERR;
}

error_t mem_write_dbyte(maddr_t base, uint16_t val)
{
	if (!IS_DBYTE_ALIGNED(base)) {
		return ERR_INVAL;
	}

	mem_blk_entry *blk = &memory[MEM_BLOCK_IN(base)];
	maddr_t off = MEM_BLOCK_MASK(base);

	create_system_block(blk);

	*(uint16_t *)&blk->base[off] = val;

	return ERR_NOERR;
}

error_t mem_write_word(maddr_t base, uint32_t val)
{
	if (!IS_WORD_ALIGNED(base)) {
		return ERR_INVAL;
	}

	mem_blk_entry *blk = &memory[MEM_BLOCK_IN(base)];
	maddr_t off = MEM_BLOCK_MASK(base);

	create_system_block(blk);

	*(uint32_t *)&blk->base[off] = val;

	return ERR_NOERR;
}

uint32_t mem_read_string(maddr_t base, char *dest, msize_t max)
{
	uint8_t *udest = (uint8_t *)dest;
	msize_t read = 0;

	while (read < (max - 1)) {
		error_t stat = mem_read_byte(base, udest);

		if (stat != ERR_NOERR) {
			break;
		}

		if (*udest == 0) {
			break;
		}

		++base, ++udest, ++read;
	}

	*udest = 0;
	return read;
}

uint32_t mem_read_mem(maddr_t base, void *dest, msize_t num)
{
	uint8_t *udest = (uint8_t *)dest;
	msize_t read = 0;

	while (read < num) {
		error_t stat = mem_read_byte(base, udest);

		if (stat != ERR_NOERR) {
			break;
		}

		++base, ++udest, ++read;
	}

	return read;
}

uint32_t mem_write_string(maddr_t base, const char *src)
{
	const uint8_t *usrc = (uint8_t *)src;
	msize_t written = 0;

	do {
		error_t stat = mem_write_byte(base, *usrc);

		if (stat != ERR_NOERR) {
			break;
		}

		++base, ++written;
	} while (*usrc++ != 0);

	return written;
}

uint32_t mem_write_mem(maddr_t base, const void *src, msize_t num)
{
	const uint8_t *usrc = (uint8_t *)src;
	msize_t written = 0;

	while (written < num) {
		error_t stat = mem_write_byte(base, *usrc);

		if (stat != ERR_NOERR) {
			return written;
		}

		++base, ++usrc, ++written;
	}

	return written;
}

error_t mem_set_bytes(maddr_t base, uint8_t val, msize_t num)
{
	msize_t written = 0;

	while (written < num) {
		error_t stat = mem_write_byte(base, val);

		if (stat != ERR_NOERR) {
			return stat;
		}

		++base, ++written;
	}

	return ERR_NOERR;
}

error_t mem_set_dbytes(maddr_t base, uint16_t val, msize_t num)
{
	msize_t written = 0;

	while (written < num) {
		error_t stat = mem_write_dbyte(base, val);

		if (stat != ERR_NOERR) {
			return stat;
		}

		base += 2;
		++written;
	}

	return ERR_NOERR;
}


error_t mem_set_words(maddr_t base, uint32_t val, msize_t num)
{
	msize_t written = 0;

	while (written < num) {
		error_t stat = mem_write_word(base, val);

		if (stat != ERR_NOERR) {
			return stat;
		}

		base += 4;
		++written;
	}

	return ERR_NOERR;
}

error_t mem_map_device(maddr_t base, mblock_t *mem)
{
	if (!IS_BLOCK_ALIGNED(base)) {
		return ERR_INVAL;
	}

	mem_blk_entry *blk = &memory[MEM_BLOCK_IN(base)];

	if (blk->type == MAP_SYSTEM) {
		delete_system_block(blk);
	}

	return install_device_block(blk, mem);
}

error_t mem_unmap_device(maddr_t base)
{
	if (!IS_BLOCK_ALIGNED(base)) {
		return ERR_INVAL;
	}

	mem_blk_entry *blk = &memory[MEM_BLOCK_IN(base)];
	return remove_device_block(blk);
}

mblock_t *mem_raw_block(maddr_t base, bool create)
{
	if (!IS_BLOCK_ALIGNED(base)) {
		return NULL;
	}

	mem_blk_entry *blk = &memory[MEM_BLOCK_IN(base)];

	if (create) {
		create_system_block(blk);
	}

	return blk->base;
}

void mem_dump()
{
	for (msize_t i = 0; i < MEM_NUM_BLKS; ++i) {
		if (memory[i].base != NULL) {
			char fname[12];
			snprintf(fname, 12, "%04u.dump", i * MEM_BLK_SIZE);

			FILE *dump = fopen(fname, "wb");
			fwrite(memory[i].base, 1, MEM_BLK_SIZE, dump);
			fclose(dump);
		}
	}
}

/////////////////////////////////////////////////////////////////////////
// Module internal functions
/////////////////////////////////////////////////////////////////////////

error_t create_system_block(mem_blk_entry *block)
{
	if (block->type != MAP_NONE) {
		return ERR_PCOND;
	}

	block->base = aligned_alloc(4096, MEM_BLK_SIZE);

	if (block->base == NULL) {
		DIE_ON(ERR_NOMEM);
	}

	block->type = MAP_SYSTEM;
	return ERR_NOERR;
}

error_t delete_system_block(mem_blk_entry *block)
{
	if (block->type != MAP_SYSTEM) {
		return ERR_PCOND;
	}

	free(block->base);
	block->base = NULL;

	block->type = MAP_NONE;
	return ERR_NOERR;
}

error_t install_device_block(mem_blk_entry *block, mblock_t *mem)
{
	if (block->type != MAP_NONE) {
		return ERR_PCOND;
	}

	block->base = mem;
	block->type = MAP_DEVICE;
	return ERR_NOERR;
}

error_t remove_device_block(mem_blk_entry *block)
{
	if (block->type != MAP_DEVICE) {
		return ERR_PCOND;
	}

	block->base = NULL;
	block->type = MAP_NONE;
	return ERR_NOERR;
}


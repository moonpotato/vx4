#pragma once

#include "error.h"
#include "mem.h"

#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////
// Type declarations
////////////////////////////////////////////////////////////////////////////////

typedef uint8_t reg_id;

////////////////////////////////////////////////////////////////////////////////
// Constants + helper macros
////////////////////////////////////////////////////////////////////////////////

enum _reg_id {
	REG_R0,  REG_R1,  REG_R2,  REG_R3,
	REG_R4,  REG_R5,  REG_R6,  REG_R7,
	REG_R8,  REG_R9,  REG_R10, REG_R11,
	REG_R12, REG_R13, REG_R14, REG_R15,

	REG_NUM_REGS
};

#define IS_VALID_REGISTER(reg) ((reg) < REG_NUM_REGS)

/**
 * Each register is divided as follows:
 *
 * 0                                                31
 * |                       word                      |
 * |       low dbyte        |       high dbyte       |
 * |  low byte  | high byte |
 * 0            7          15
 *
 * (e.g. the low dbyte is bits [0, 15] of the word.)
 */

////////////////////////////////////////////////////////////////////////////////
// Function declarations
////////////////////////////////////////////////////////////////////////////////

/**
 * Reads a part of a register and retrieves the result.
 *
 * IN which: A constant selecting the register to read.
 * OUT dest: The location to save the read value.
 *
 * Returns:
 * ERR_NOERR: The read completed successfully.
 * ERR_INVAL: The register specified by which doesn't exist.
 */
extern error_t reg_read_low_byte(reg_id which, uint8_t *dest);
extern error_t reg_read_high_byte(reg_id which, uint8_t *dest);
extern error_t reg_read_low_dbyte(reg_id which, uint16_t *dest);
extern error_t reg_read_high_dbyte(reg_id which, uint16_t *dest);
extern error_t reg_read_word(reg_id which, uint32_t *dest);


/**
 * Writes a value into a part of a register. All other components
 * of the register remain unchanged.
 *
 * (e.g. if the register contains 0xFFFFFFFF and 0x0DAE is written
 * into the high dbyte, the register then contains 0xFFFF0DAE.)
 *
 * IN which: A constant selecting the register to write.
 * IN val: The value to write into the register.
 *
 * Returns:
 * ERR_NOERR: The write completed successfully.
 * ERR_INVAL: The register specified by which doesn't exist.
 */
extern error_t reg_write_low_byte(reg_id which, uint8_t val);
extern error_t reg_write_high_byte(reg_id which, uint8_t val);
extern error_t reg_write_low_dbyte(reg_id which, uint16_t val);
extern error_t reg_write_high_dbyte(reg_id which, uint16_t val);
extern error_t reg_write_word(reg_id which, uint32_t val);

/**
 * Causes all the register values to be read from/written to memory beginning
 * at a specified address.
 *
 * IN start: The memory address to begin writing.
 *
 * Returns:
 * ERR_NOERR: Writing/reading completed successfully.
 * ERR_EXTERN: All of the registers couldn't be written/read.
 */
extern error_t reg_write_all_mem(mem_addr start);
extern error_t reg_read_all_mem(mem_addr start);

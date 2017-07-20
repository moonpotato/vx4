#pragma once

#include "error.h"

#include <stdint.h>

/////////////////////////////////////////////////////////////////////////
// Constants + helper macros
/////////////////////////////////////////////////////////////////////////

typedef enum _reg_name {
	REG_R0,  REG_R1,  REG_R2,  REG_R3,
	REG_R4,  REG_R5,  REG_R6,  REG_R7,
	REG_R8,  REG_R9,  REG_R10, REG_R11,
	REG_R12, REG_R13, REG_R14, REG_R15,

	REG_NUM_REGS
} reg_name;

/////////////////////////////////////////////////////////////////////////
// Function declarations
/////////////////////////////////////////////////////////////////////////

extern error_t reg_read_low_byte(reg_name which, uint8_t *dest);
extern error_t reg_read_high_byte(reg_name which, uint8_t *dest);
extern error_t reg_read_low_dbyte(reg_name which, uint16_t *dest);
extern error_t reg_read_high_dbyte(reg_name which, uint16_t *dest);
extern error_t reg_read_word(reg_name which, uint32_t *dest);

extern error_t reg_write_low_byte(reg_name which, uint8_t val);
extern error_t reg_write_high_byte(reg_name which, uint8_t val);
extern error_t reg_write_low_dbyte(reg_name which, uint16_t val);
extern error_t reg_write_high_dbyte(reg_name which, uint16_t val);
extern error_t reg_write_word(reg_name which, uint32_t val);

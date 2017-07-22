#include "register.h"

#include "error.h"

#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////
// Internal constants + helper macros
////////////////////////////////////////////////////////////////////////////////

// Enums are typically represented by signed types, so check positive
#define IS_VALID_REGISTER(reg) (((reg) < REG_NUM_REGS) && ((reg) >= 0))

////////////////////////////////////////////////////////////////////////////////
// Module internal declarations
////////////////////////////////////////////////////////////////////////////////

static uint32_t registers[REG_NUM_REGS];

////////////////////////////////////////////////////////////////////////////////
// Interface functions
////////////////////////////////////////////////////////////////////////////////

/**
 * Caution: here lies quite a bit of pointer arithmetic, though the
 * basics are quite similar in each case. Taking reg_read_high_byte
 * as an example:
 *
 * &registers[which] takes the address of register in question.
 * Then we cast it with (uint8_t *) and treat it as an array of
 * 4 bytes. The high byte is bits [8, 15] and is thus at position
 * 1 in the new "array".
 */

error_t reg_read_low_byte(reg_name which, uint8_t *dest)
{
	if (!IS_VALID_REGISTER(which)) {
		return ERR_INVAL;
	}

	uint8_t *reg = (uint8_t *)&registers[which];

	*dest = reg[0];
	return ERR_NOERR;
}

error_t reg_read_high_byte(reg_name which, uint8_t *dest)
{
	if (!IS_VALID_REGISTER(which)) {
		return ERR_INVAL;
	}

	uint8_t *reg = (uint8_t *)&registers[which];

	*dest = reg[1];
	return ERR_NOERR;
}

error_t reg_read_low_dbyte(reg_name which, uint16_t *dest)
{
	if (!IS_VALID_REGISTER(which)) {
		return ERR_INVAL;
	}

	uint16_t *reg = (uint16_t *)&registers[which];

	*dest = reg[0];
	return ERR_NOERR;
}

error_t reg_read_high_dbyte(reg_name which, uint16_t *dest)
{
	if (!IS_VALID_REGISTER(which)) {
		return ERR_INVAL;
	}

	uint16_t *reg = (uint16_t *)&registers[which];

	*dest = reg[1];
	return ERR_NOERR;
}

error_t reg_read_word(reg_name which, uint32_t *dest)
{
	if (!IS_VALID_REGISTER(which)) {
		return ERR_INVAL;
	}

	*dest = registers[which];
	return ERR_NOERR;
}

error_t reg_write_low_byte(reg_name which, uint8_t val)
{
	if (!IS_VALID_REGISTER(which)) {
		return ERR_INVAL;
	}

	uint8_t *reg = (uint8_t *)&registers[which];

	reg[0] = val;
	return ERR_NOERR;
}

error_t reg_write_high_byte(reg_name which, uint8_t val)
{
	if (!IS_VALID_REGISTER(which)) {
		return ERR_INVAL;
	}

	uint8_t *reg = (uint8_t *)&registers[which];

	reg[1] = val;
	return ERR_NOERR;
}

error_t reg_write_low_dbyte(reg_name which, uint16_t val)
{
	if (!IS_VALID_REGISTER(which)) {
		return ERR_INVAL;
	}

	uint16_t *reg = (uint16_t *)&registers[which];

	reg[0] = val;
	return ERR_NOERR;
}

error_t reg_write_high_dbyte(reg_name which, uint16_t val)
{
	if (!IS_VALID_REGISTER(which)) {
		return ERR_INVAL;
	}

	uint16_t *reg = (uint16_t *)&registers[which];

	reg[1] = val;
	return ERR_NOERR;
}

error_t reg_write_word(reg_name which, uint32_t val)
{
	if (!IS_VALID_REGISTER(which)) {
		return ERR_INVAL;
	}

	registers[which] = val;
	return ERR_NOERR;
}


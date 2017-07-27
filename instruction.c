#include "instruction.h"

#include "error.h"
#include "mem.h"
#include "register.h"
#include "cpu.h"

#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////
// Module internal declarations
////////////////////////////////////////////////////////////////////////////////

static error_t instruction_nop(void *data);
static error_t instruction_hlt(void *data);
static error_t instruction_jmpc(void *data);
static error_t instruction_movrc(void *data);
static error_t instruction_movmr(void *data);
static error_t instruction_addrc(void *data);

instruction_info instructions[INS_NUM_INS] = {
    {instruction_nop, 0},
    {instruction_hlt, 0},
    {instruction_jmpc, 4},
    {instruction_movrc, 5},
    {instruction_movmr, 5},
    {instruction_addrc, 5},
};

////////////////////////////////////////////////////////////////////////////////
// Module internal functions
////////////////////////////////////////////////////////////////////////////////

static error_t instruction_nop(void *data)
{
	(void)data;
	return ERR_NOERR;
}

static error_t instruction_hlt(void *data)
{
	(void)data;

	cpu_queue_halt();
	return ERR_NOERR;
}

static error_t instruction_jmpc(void *data)
{
	cpu_queue_jump(*(mem_addr *)data);
	return ERR_NOERR;
}

static error_t instruction_movrc(void *data)
{
	reg_id *dest = (reg_id *)data;
    uint32_t *src = (uint32_t *)(dest + 1);

    if (!IS_VALID_REGISTER(*dest)) {
		return ERR_INVAL;
    }

    reg_write_word(*dest, *src);
    return ERR_NOERR;
}

static error_t instruction_movmr(void *data)
{
	mem_addr *dest = (mem_addr *)data;
	reg_id *src = (reg_id *)(dest + 1);

	if (!IS_VALID_REGISTER(*src)) {
		return ERR_INVAL;
	}

	uint32_t word;
    reg_read_word(*src, &word);
    return mem_write_word(*dest, word);
}

static error_t instruction_addrc(void *data)
{
	reg_id *dest = (reg_id *)data;
    uint32_t *src = (uint32_t *)(dest + 1);

    if (!IS_VALID_REGISTER(*dest)) {
		return ERR_INVAL;
    }

    uint32_t word;
    reg_read_word(*dest, &word);
    word += *src;
    reg_write_word(*dest, word);

    return ERR_NOERR;
}

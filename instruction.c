#include "instruction.h"

#include "error.h"
#include "mem.h"
#include "port.h"
#include "register.h"
#include "cpu.h"

#include <stdint.h>
#include <stdbool.h>

////////////////////////////////////////////////////////////////////////////////
// Module internal declarations
////////////////////////////////////////////////////////////////////////////////

static error_t instruction_nop(void *data);
static error_t instruction_hlt(void *data);
static error_t instruction_jmpc(void *data);
static error_t instruction_movrc(void *data);
static error_t instruction_movpcr(void *data);
static error_t instruction_addrc(void *data);
static error_t instruction_movprr(void *data);
static error_t instruction_outscr(void *data);
static error_t instruction_inxrsc(void *data);
static error_t instruction_cli(void *data);
static error_t instruction_sti(void *data);

instruction_info instructions[] = {
    {instruction_nop, 0},
    {instruction_hlt, 0},
    {instruction_jmpc, 4},
    {instruction_movrc, 6},
    {instruction_movpcr, 6},
    {instruction_addrc, 6},
    {instruction_movprr, 2},
    {instruction_outscr, 4},
	{instruction_inxrsc, 4},
	{instruction_cli, 0},
	{instruction_sti, 0},
};

#define INS_NUM_INS (sizeof (instructions) / sizeof (instruction_info))
#define IS_VALID_INSTRUCTION(id) ((id) < INS_NUM_INS)

////////////////////////////////////////////////////////////////////////////////
// Interface functions
////////////////////////////////////////////////////////////////////////////////

bool valid_instruction(instruction_id ins)
{
	return IS_VALID_INSTRUCTION(ins);
}

////////////////////////////////////////////////////////////////////////////////
// Module internal functions
////////////////////////////////////////////////////////////////////////////////

error_t instruction_nop(void *data)
{
	(void)data;
	return ERR_NOERR;
}

error_t instruction_hlt(void *data)
{
	(void)data;

	cpu_queue_halt();
	return ERR_NOERR;
}

error_t instruction_jmpc(void *data)
{
	cpu_queue_jump(*(mem_addr *)data);
	return ERR_NOERR;
}

error_t instruction_movrc(void *data)
{
	reg_id *dest = (reg_id *)data;
    uint32_t *src = (uint32_t *)(dest + 1);

    if (!IS_VALID_REGISTER(*dest)) {
		return ERR_INVAL;
    }

    reg_write_word(*dest, *src);
    return ERR_NOERR;
}

error_t instruction_movpcr(void *data)
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

error_t instruction_addrc(void *data)
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

error_t instruction_movprr(void *data)
{
	reg_id *dest = (reg_id *)data;
	reg_id *src = (reg_id *)(dest + 1);

	if (!IS_VALID_REGISTER(*dest)) {
		return ERR_INVAL;
    }
    if (!IS_VALID_REGISTER(*src)) {
		return ERR_INVAL;
    }

    mem_addr where;
    reg_read_word(*dest, &where);
    uint32_t word;
    reg_read_word(*src, &word);

    return mem_write_word(where, word);
}

error_t instruction_outscr(void *data)
{
	port_id *dest = (port_id *)data;
	reg_id *src = (reg_id *)(dest + 1);

	if (!IS_VALID_PORT(*dest)) {
		return ERR_INVAL;
    }
    if (!IS_VALID_REGISTER(*src)) {
		return ERR_INVAL;
    }

    uint32_t word;
    reg_read_word(*src, &word);
    port_write(*dest, word);

    return ERR_NOERR;
}

error_t instruction_inxrsc(void *data)
{
	reg_id *dest = (reg_id *)data;
	port_id *src = (port_id *)(dest + 1);

	if (!IS_VALID_REGISTER(*dest)) {
		return ERR_INVAL;
    }
    if (!IS_VALID_PORT(*src)) {
		return ERR_INVAL;
    }

    uint32_t word;
    port_read(*src, &word);
    reg_write_word(*dest, word);

    return ERR_NOERR;
}

error_t instruction_cli(void *data)
{
	(void)data;
	cpu_interrupt_set(false);

	return ERR_NOERR;
}

error_t instruction_sti(void *data)
{
	(void)data;
	cpu_interrupt_set(true);

	return ERR_NOERR;
}

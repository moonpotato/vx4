#include "instruction.h"

#include "error.h"
#include "cpu.h"

////////////////////////////////////////////////////////////////////////////////
// Module internal declarations
////////////////////////////////////////////////////////////////////////////////

static error_t instruction_nop(void *data);
static error_t instruction_halt(void *data);
static error_t instruction_jmpc(void *data);

instruction_info instructions[INS_NUM_INS] = {
    {instruction_nop, 0},
    {instruction_halt, 0},
    {instruction_jmpc, 4},
};

////////////////////////////////////////////////////////////////////////////////
// Module internal functions
////////////////////////////////////////////////////////////////////////////////

static error_t instruction_nop(void *data)
{
	(void)data;
	return ERR_NOERR;
}

static error_t instruction_halt(void *data)
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

#include "stack.h"

#include "error.h"
#include "mem.h"

#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////
// Internal constants + helper macros
////////////////////////////////////////////////////////////////////////////////

#define IS_ALIGNED(ptr) (((ptr) & 0x3) == 0)

////////////////////////////////////////////////////////////////////////////////
// Module internal declarations
////////////////////////////////////////////////////////////////////////////////

mem_addr reg_sp; // Stack pointer
mem_addr reg_bp; // Base pointer

////////////////////////////////////////////////////////////////////////////////
// Interface functions
////////////////////////////////////////////////////////////////////////////////

error_t stack_enter_frame()
{
    if (!IS_ALIGNED(reg_sp)) {
        return ERR_PCOND;
    }

    stack_push(reg_bp);
    reg_bp = reg_sp;

    return ERR_NOERR;
}

error_t stack_leave_frame()
{
    if (!IS_ALIGNED(reg_bp)) {
        return ERR_PCOND;
    }

    reg_sp = reg_bp;
    stack_pop(&reg_bp);

    return ERR_NOERR;
}

error_t stack_push(uint32_t word)
{
    if (!IS_ALIGNED(reg_sp)) {
        return ERR_PCOND;
    }

    reg_sp -= 4;
    mem_write_word(reg_sp, word);

    return ERR_NOERR;
}

error_t stack_push_multi(const uint32_t *words, mem_size num)
{
    if (!IS_ALIGNED(reg_sp)) {
        return ERR_PCOND;
    }

    reg_sp -= num * 4;
    mem_write_mem(reg_sp, words, num * 4);

    return ERR_NOERR;
}

error_t stack_pop(uint32_t *word)
{
    if (!IS_ALIGNED(reg_sp)) {
        return ERR_PCOND;
    }

    mem_read_word(reg_sp, word);
    reg_sp += 4;

    return ERR_NOERR;
}

error_t stack_pop_multi(uint32_t *words, mem_size num)
{
    if (!IS_ALIGNED(reg_sp)) {
        return ERR_PCOND;
    }

    mem_read_mem(reg_sp, words, num * 4);
    reg_sp += num * 4;

    return ERR_NOERR;
}

error_t stack_skip(mem_size num)
{
    if (!IS_ALIGNED(reg_sp)) {
        return ERR_PCOND;
    }

    reg_sp -= num * 4;

    return ERR_NOERR;
}

error_t stack_unskip(mem_size num)
{
    if (!IS_ALIGNED(reg_sp)) {
        return ERR_PCOND;
    }

    reg_sp += num * 4;

    return ERR_NOERR;
}

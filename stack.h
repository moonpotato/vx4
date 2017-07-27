#pragma once

#include "error.h"
#include "mem.h"

#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////
// Global variable declarations
////////////////////////////////////////////////////////////////////////////////

extern mem_addr reg_sp; // Stack pointer
extern mem_addr reg_bp; // Base pointer

////////////////////////////////////////////////////////////////////////////////
// Function declarations
////////////////////////////////////////////////////////////////////////////////

extern error_t stack_enter_frame();
extern error_t stack_leave_frame();

extern error_t stack_push(uint32_t word);
extern error_t stack_push_multi(const uint32_t *words, mem_size num);

extern error_t stack_pop(uint32_t *word);
extern error_t stack_pop_multi(uint32_t *words, mem_size num);

extern error_t stack_skip(mem_size num);
extern error_t stack_unskip(mem_size num);

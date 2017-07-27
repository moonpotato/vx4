#pragma once

#include "error.h"
#include "mem.h"

#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////
// Global variable declarations
////////////////////////////////////////////////////////////////////////////////

// These are made global because the CPU may need to directly edit them
// Should not be touched except by functions in cpu.c or stack.c
extern mem_addr reg_sp; // Stack pointer
extern mem_addr reg_bp; // Base pointer

////////////////////////////////////////////////////////////////////////////////
// Function declarations
////////////////////////////////////////////////////////////////////////////////

/**
 * Create and enter a new stack frame.
 *
 * Returns:
 * ERR_NOERR: The operation completed successfully.
 * ERR_PCOND: The stack is currently unaligned and thus in an invalid state.
 */
extern error_t stack_enter_frame();

/**
 * Leave and destroy the bottom stack frame.
 *
 * Returns:
 * ERR_NOERR: The operation completed successfully.
 * ERR_PCOND: The operation would cause the stack to become unaligned.
 */
extern error_t stack_leave_frame();

/**
 * Push a word onto the bottom of the stack.
 *
 * IN word: The word to push.
 *
 * Returns:
 * ERR_NOERR: The operation completed successfully.
 * ERR_PCOND: The stack is currently unaligned and thus in an invalid state.
 */
extern error_t stack_push(uint32_t word);

/**
 * Push a number of word onto the bottom of the stack.
 *
 * IN word: A pointer to an array of words to push.
 * IN num: The number of words in the array.
 *
 * Returns:
 * ERR_NOERR: The operation completed successfully.
 * ERR_PCOND: The stack is currently unaligned and thus in an invalid state.
 */
extern error_t stack_push_multi(const uint32_t *words, mem_size num);

/**
 * Pop a word from the bottom of the stack.
 *
 * OUT word: A pointer to a location to write the popped word.
 *
 * Returns:
 * ERR_NOERR: The operation completed successfully.
 * ERR_PCOND: The stack is currently unaligned and thus in an invalid state.
 */
extern error_t stack_pop(uint32_t *word);

/**
 * Pop a number of words from the bottom of the stack.
 *
 * OUT word: A pointer to a location to write the popped word.
 * IN num: The number of words in the array to be popped.
 *
 * Returns:
 * ERR_NOERR: The operation completed successfully.
 * ERR_PCOND: The stack is currently unaligned and thus in an invalid state.
 */
extern error_t stack_pop_multi(uint32_t *words, mem_size num);

/**
 * Skip a number of stack slots, leaving holes in the stack.
 *
 * IN num: The number of word-sized slots to skip.
 *
 * Returns:
 * ERR_NOERR: The operation completed successfully.
 * ERR_PCOND: The stack is currently unaligned and thus in an invalid state.
 */
extern error_t stack_skip(mem_size num);

/**
 * Jump back a number of stack slots, causing anything in those slots to
 * be discarded.
 *
 * IN num: The number of word-sized slots to jump back.
 *
 * Returns:
 * ERR_NOERR: The operation completed successfully.
 * ERR_PCOND: The stack is currently unaligned and thus in an invalid state.
 */
extern error_t stack_unskip(mem_size num);

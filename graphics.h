#pragma once

#include "error.h"
#include "disk.h" // For DISK_MMAP_START constant

#include <stdint.h>
#include <stdbool.h>

////////////////////////////////////////////////////////////////////////////////
// Constants + helper macros
////////////////////////////////////////////////////////////////////////////////

#define GFX_MEM_MAX (8u * 1024 * 1024) // Reserve 8MiB for pixel data

// Put the graphics memory right before the disk buffers
#define GFX_MMAP_START (DISK_MMAP_START - GFX_MEM_MAX)

typedef enum _gfx_action {
    GA_NONE, // No action to perform
    GA_ADDR, // Get the base address of the graphics mmap
    GA_BUFSZ, // Get the size (in bytes) of the graphics mmap
    GA_RES, // Get/set the current graphics resolution as width | height << 16
} gfx_action;

typedef enum _gfx_state {
    GS_OK,
    GS_WAIT,
    GS_ERROR,
} gfx_state;

////////////////////////////////////////////////////////////////////////////////
// Function declarations
////////////////////////////////////////////////////////////////////////////////

/**
 * Start the graphics system, including creating a window, framebuffer
 * and control ports.
 *
 * IN width, height: The dimensions of the window to create.
 *
 * Returns:
 * ERR_NOERR: All operations completed successfully.
 * ERR_INVAL: The window dimensions specified were too large.
 * ERR_EXTERN: An error occurred while attempting to initialize the
 * rendering subsystem.
 * ERR_NOMEM: The framebuffer couldn't be allocated.
 * ERR_PORT: Enought ports couldn't be allocated for the graphics system.
 */
extern error_t graphics_begin(int width, int height);

/**
 * Reinitialize the rendering subsystem with a different window size.
 *
 * IN width, height: The dimensions of the window to create.
 *
 * Returns:
 * ERR_NOERR: All operations completed successfully.
 * ERR_PCOND: The graphics system wasn't already initialized, use
 * graphics_begin instead.
 * ERR_INVAL: The window dimensions specified were too large.
 * ERR_EXTERN: An error occurred while attempting to initialize the
 * rendering subsystem.
 */
extern error_t graphics_restart(int width, int height);

/**
 * Process all frame-wise and event loop actions for the graphics subsystem.
 */
extern void graphics_step();

/**
 * Draw the graphics framebuffer to the window, and update the window on
 * screen.
 */
extern void graphics_render();

/**
 * Clean up all resources related to the graphics subsystem and shut it down.
 * Must be called on application exit to avoid leaking graphics resources.
 */
extern void graphics_end();

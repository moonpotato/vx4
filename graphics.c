#include "graphics.h"

#include "error.h"
#include "mem.h"
#include "port.h"
#include "kbd.h"
#include "intr.h"

#include <SDL2/SDL.h>

#include <stdint.h>
#include <stdbool.h>

////////////////////////////////////////////////////////////////////////////////
// Internal constants + helper macros
////////////////////////////////////////////////////////////////////////////////

#define RECT_BYTE_SIZE(width, height) ((width) * (height) * 4u)
#define IS_VALID_MODE(width, height) (RECT_BYTE_SIZE((width), (height)) < GFX_MEM_MAX)

////////////////////////////////////////////////////////////////////////////////
// Module internal declarations
////////////////////////////////////////////////////////////////////////////////

// Pointers to resources used by SDL for rendering.
static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;

// The current dimensions of the window
static int win_width;
static int win_height;

/**
 * Initialize all resources for the SDL rendering subsystem, and create
 * the rendering window.
 *
 * IN width, height: The dimensions of the window to create.
 *
 * Returns:
 * ERR_NOERR: All operations completed successfully.
 * ERR_INVAL: The window dimensions specified were too large.
 * ERR_EXTERN: An error occurred while trying to acquire one of the resources
 * required for rendering.
 */
static error_t sdl_subsys_init(int width, int height);

/**
 * Clean up the all resources of the SDL rendering subsystem, and close
 * the window.
 */
static void sdl_subsys_quit();

// The memory-mapped framebuffer
static uint8_t *gfx_buffer;

// State for the command ports
static gfx_action act; // The current queued action
static gfx_state res; // The success/failure to be reported via command_reply
static uint32_t port_data; // Auxiliary data used by the port functions

/**
 * Sets the command to be executed on subsequent reads/writes to the data port.
 *
 * IN num: Ignored, part of the callback signature.
 * IN command: The command to set.
 */
static void command_recv(port_id num, uint32_t command);

/**
 * Fetch the status of the last read/write to the data port.
 *
 * IN num: Ignored, part of the callback signature.
 *
 * Returns: That status.
 */
static uint32_t command_reply(port_id num);

/**
 * Executes the set command, if it requires data being written to the port,
 * or doesn't involve returning data to be read on the port.
 *
 * IN num: Ignored, part of the callback signature.
 * IN data: The data that was written to the port that triggered the callback.
 */
static void data_write(port_id num, uint32_t data);

/**
 * Executes the set command, if it involves reading of the data port to
 * return data.
 *
 * IN num: Ignored, part of the callback signature.
 *
 * Returns: Whatever data was requested to be read from the port.
 */
static uint32_t data_read(port_id num);

static port_entry graphics_port[] = {
	{"Graphics v1 command", command_recv, command_reply},
	{"Graphics v1 data", data_write, data_read}
};

static port_id cmd_port;
static port_id data_port;

// Has the graphics subsystem been initialized yet?
static bool gfx_init;

////////////////////////////////////////////////////////////////////////////////
// Interface functions
////////////////////////////////////////////////////////////////////////////////

error_t graphics_begin(int width, int height)
{
	int stat = sdl_subsys_init(width, height);
	if (stat != ERR_NOERR) {
		return stat;
	}

    win_width = width;
    win_height = height;

	gfx_buffer = malloc(GFX_MEM_MAX);
	if (gfx_buffer == NULL) {
        return ERR_NOMEM;
	}

	for (unsigned i = 0; i < (GFX_MEM_MAX / MEM_BLK_SIZE); ++i) {
		mem_addr off = i * MEM_BLK_SIZE;
		mem_addr blk = GFX_MMAP_START + off;
		mem_map_device(blk, &gfx_buffer[off]);
	}

	stat = port_install(&graphics_port[0], &cmd_port);
	if (stat != ERR_NOERR) {
		return ERR_PORT;
	}

	stat = port_install(&graphics_port[1], &data_port);
	if (stat != ERR_NOERR) {
		return ERR_PORT;
	}

	gfx_init = true;
	return ERR_NOERR;
}

error_t graphics_restart(int width, int height)
{
	if (!gfx_init) {
		return ERR_PCOND;
	}

	sdl_subsys_quit();

	win_width = 0;
	win_height = 0;

	int stat = sdl_subsys_init(width, height);

	if (stat == ERR_NOERR) {
		win_width = width;
		win_height = height;
	}
	else {
        gfx_init = false;
	}

    return stat;
}

void graphics_step()
{
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				interrupt_raise(INTR_HALT);
				break;

			case SDL_KEYDOWN:
                keyboard_queue_press((event.key.keysym.mod << 16) | (event.key.keysym.scancode & 0xFFFF));
                break;
		}
    }
}

void graphics_render()
{
	void *pixels;
	int pitch;

	if (SDL_LockTexture(texture, NULL, &pixels, &pitch) == 0) {
		memcpy(pixels, gfx_buffer, RECT_BYTE_SIZE(win_width, win_height));
		SDL_UnlockTexture(texture);
	}

	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

void graphics_end()
{
	gfx_init = false;

	if (cmd_port) {
		port_remove(cmd_port);
	}

	if (data_port) {
		port_remove(data_port);
	}

	if (gfx_buffer) {
		for (unsigned i = 0; i < (GFX_MEM_MAX / MEM_BLK_SIZE); ++i) {
			mem_unmap_device(GFX_MMAP_START + (i * MEM_BLK_SIZE));
		}

		free(gfx_buffer);
	}

	sdl_subsys_quit();
}

////////////////////////////////////////////////////////////////////////////////
// Module internal functions
////////////////////////////////////////////////////////////////////////////////

error_t sdl_subsys_init(int width, int height)
{
	if (!IS_VALID_MODE(width, height)) {
		return ERR_INVAL;
	}

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return ERR_EXTERN;
	}

	// From here, we have an SDL context which must be destroyed on exit
	// To avoid leaking graphics resources

	window = SDL_CreateWindow("vx4", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0);
	if (!window) {
		graphics_end();
		return ERR_EXTERN;
	}

	renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer) {
		graphics_end();
		return ERR_EXTERN;
	}

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, width, height);
	if (!texture) {
		return ERR_EXTERN;
	}

	return ERR_NOERR;
}

void sdl_subsys_quit()
{
	if (texture) {
		SDL_DestroyTexture(texture);
	}

	if (renderer) {
		SDL_DestroyRenderer(renderer);
	}

	if (window) {
		SDL_DestroyWindow(window);
	}

	SDL_Quit();
}

void command_recv(port_id num, uint32_t command)
{
	(void)num;
    act = (int)command;

    if (act == GA_NONE) {
		res = GS_OK;
    }
    else {
		res = GS_WAIT;
    }
}

uint32_t command_reply(port_id num)
{
	(void)num;
    return (uint32_t)res;
}

void data_write(port_id num, uint32_t data)
{
	(void)num;
	port_data = data;

	switch (act) {
		default:
		case GA_NONE:
			res = GS_ERROR;
			break;

		case GA_RES:
            if (graphics_restart(data & 0xFFFF, data >> 16) == ERR_NOERR) {
				res = GS_OK;
            }
            else {
				res = GS_ERROR;
            }
            break;
	}
}

uint32_t data_read(port_id num)
{
	(void)num;
	uint32_t ret;

	switch (act) {
		default:
		case GA_NONE:
			res = GS_ERROR;
			ret = 0;
			break;

		case GA_ADDR:
			res = GS_OK;
			ret = GFX_MMAP_START;
			break;

		case GA_BUFSZ:
			res = GS_OK;
			ret = GFX_MEM_MAX;
			break;

		case GA_RES:
			res = GS_OK;
			ret = ((uint32_t)win_width) | (((uint32_t)win_height) << 16);
			break;
	}

	return ret;
}

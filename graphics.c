#include "graphics.h"

#include "error.h"
#include "mem.h"

#include <SDL2/SDL.h>

#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////
// Internal constants + helper macros
////////////////////////////////////////////////////////////////////////////////

#define IS_VALID_MODE(width, height) (((width) * (height) * 4u) < GFX_MEM_MAX)

////////////////////////////////////////////////////////////////////////////////
// Module internal declarations
////////////////////////////////////////////////////////////////////////////////

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;

static error_t sdl_subsys_init(int width, int height);
static void sdl_subsys_quit();

static uint8_t *gfx_buffer;

////////////////////////////////////////////////////////////////////////////////
// Interface functions
////////////////////////////////////////////////////////////////////////////////

error_t graphics_begin(int width, int height)
{
	int stat = sdl_subsys_init(width, height);
	if (stat != ERR_NOERR) {
		return stat;
	}

	gfx_buffer = malloc(GFX_MEM_MAX);
	if (gfx_buffer == NULL) {
        return ERR_NOMEM;
	}

	for (unsigned i = 0; i < (GFX_MEM_MAX / MEM_BLK_SIZE); ++i) {
		mem_addr off = i * MEM_BLK_SIZE;
		mem_addr blk = GFX_MMAP_START + off;
		mem_map_device(blk, &gfx_buffer[off]);
	}

	return ERR_NOERR;
}

void graphics_render()
{

}

void graphics_end()
{
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

	return ERR_NOERR;

	renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer) {
		graphics_end();
		return ERR_EXTERN;
	}

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);
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

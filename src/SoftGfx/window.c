/*
 * SoftGfx - 1.0 - public domain
 * window.c : Creates a new window
 */

#include <stdio.h>
#include <SDL2/SDL.h>
#include <SoftGfx/gfx.h>

/*
 * #define DISP_W		640u
 * #define DISP_H		360u
 */

#define DISP_W		480u
#define DISP_H		480u


/* User defined functions */
extern void appSetup(void);
extern void appUpdate();
extern int appDraw();
extern void appQuit();
extern void appKeyboard(SDL_Scancode key, u32 state);
/* Functions for the display */
extern void gfxDisplayQuit(void);
extern u32* gfxDisplayGet(void);
extern void gfxDisplayInit(u32 width, u32 height, u32 win_width, u32 win_height);

static SDL_Renderer *renderer;
static SDL_Window *window;


int
main(int argc, char **argv)
{

	/* Initialize the builder and create window */
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Couldn't start SDL: %s\n", SDL_GetError());
		exit(1);
	}

	window = SDL_CreateWindow("SoftGraph", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, DISP_W, DISP_H, 0);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	/* Initialize the renderer */
	gfxDisplayInit(DISP_W, DISP_H, DISP_W, DISP_H);
	gfxClearColor(0x00u, 0x00u, 0x00u);

	/* Call the users functions */
	appSetup();
	appDraw();
	SDL_Texture *main_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, DISP_W, DISP_H);
	SDL_Rect screen_dst = {.x = 0, .y = 0, .w = DISP_W, .h = DISP_H};
	/* Start application */
	while (1) {
		SDL_Event ev;

		while (SDL_PollEvent(&ev)) {
			switch (ev.type) {
				case SDL_QUIT: {
					appQuit();
					gfxDisplayQuit();
					exit(0);
				} break;

				case SDL_KEYDOWN: {
					appKeyboard(ev.key.keysym.scancode, ev.key.state);
				} break;

				case SDL_KEYUP: {
					appKeyboard(ev.key.keysym.scancode, ev.key.state);
				} break;

				default:
					break;
			}
		}

		SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
		SDL_RenderClear(renderer);
		int *pix, pitch;
		SDL_LockTexture(main_tex, NULL, (void**) &pix, &pitch);
		appUpdate();
		appDraw();
		memcpy(pix, gfxDisplayGet(), pitch * DISP_H);
		SDL_UnlockTexture(main_tex);
		SDL_RenderCopy(renderer, main_tex, NULL, &screen_dst);
		SDL_RenderPresent(renderer);
	}
	return 0;
}


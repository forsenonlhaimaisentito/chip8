/*
 * chip8
 * Copyright (C) 2016  forsenonlhaimaisentito <titor@catafratta.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <SDL.h>

#include "util.h"
#include "chip8.h"

/* Dimensione in pixel reali dello schermo CHIP-8
 * 1: 64x32
 * 5: 320x160
 * 8: 512x256
 * 10: 640x320
 * 15: 960x480
 * 16: 1024x512
 * 20: 1280x640 */
#define GFX_SCALE 16

/* Tabella di conversione tasti PC a tasti CHIP-8
 *
 * CHIP-8:    PC:
 * 1 2 3 C    1 2 3 4
 * 4 5 6 D     q w e r
 * 7 8 9 E      a s d f
 * A 0 B F       z x c v
 */
static int keymap[16] = {
	SDL_SCANCODE_X, /* FIXME: Apparentemente X non funziona */
	SDL_SCANCODE_1,
	SDL_SCANCODE_2,
	SDL_SCANCODE_3,
	SDL_SCANCODE_Q,
	SDL_SCANCODE_W,
	SDL_SCANCODE_E,
	SDL_SCANCODE_A,
	SDL_SCANCODE_S,
	SDL_SCANCODE_D,
	SDL_SCANCODE_Z,
	SDL_SCANCODE_C,
	SDL_SCANCODE_4,
	SDL_SCANCODE_R,
	SDL_SCANCODE_F,
	SDL_SCANCODE_V
};

static int init_sdl(SDL_Window **win, SDL_Renderer **ren, SDL_Texture **tex);
static void emulation_loop(chip8_machine_t *chip8);

static SDL_Window *win;
static SDL_Renderer *ren;
static SDL_Texture *tex;

static uint32_t fg, bg;

int main(int argc, char **argv){
	uint8_t buf[0xE00];
	size_t count;
	chip8_machine_t chip8;

	if (argc < 2){
		fprintf(stderr, "Usage: %s FILE.ch8 [FGCOLOR [BGCOLOR]]\n", argv[0]);
		return 1;
	}

	if (argc > 2){
		fg = (uint32_t) ((strtol(argv[2], NULL, 16) << 8) | 0xFF);
	} else {
		fg = 0xFFFFFFFF;
	}

	if (argc > 3){
		bg = (uint32_t) ((strtol(argv[3], NULL, 16) << 8) | 0xFF);
	} else {
		bg = 0x000000FF;
	}
	
	if (!(count = read_file(argv[1], buf, 0xE00))){
		return 1;
	}

	chip8_init(&chip8);
	chip8_load(&chip8, buf, 0xE00);
	
	if (init_sdl(&win, &ren, &tex)){
		return 1;
	}
	
	emulation_loop(&chip8);

	SDL_DestroyTexture(tex);
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();
	
	return 0;
}

static int init_sdl(SDL_Window **win, SDL_Renderer **ren, SDL_Texture **tex){
	*win = NULL;
	*ren = NULL;
	*tex = NULL;
	
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)){
		fprintf(stderr, "Errore init SDL: %s\n", SDL_GetError());
		goto error;
	}

	*win = SDL_CreateWindow("CHIP-8",
							SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
							64 * GFX_SCALE, 32 * GFX_SCALE,
							SDL_WINDOW_SHOWN);
	if (!win){
		fprintf(stderr, "Errore creazione finestra: %s\n", SDL_GetError());
		goto error;
	}

	*ren = SDL_CreateRenderer(*win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!ren){
		fprintf(stderr, "Errore creazione renderer: %s\n", SDL_GetError());
		goto error;
	}

	*tex = SDL_CreateTexture(*ren, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
	if (!tex){
		fprintf(stderr, "Errore creazione texture: %s\n", SDL_GetError());
		goto error;
	}

	/* TODO: Init audio */

	return 0;

 error:
	if (*ren){
		SDL_DestroyRenderer(*ren);
	}
	if (*win){
		SDL_DestroyWindow(*win);
	}
	SDL_Quit();
	return 1;
}

/* TODO: File separato per gestire SDL2 */
static void emulation_loop(chip8_machine_t *chip8){
	SDL_Event ev;
	int i, j, beep;
	long last, delta, cdelta;
	uint8_t keys[16];
	const Uint8 *sdl_keys;
    uint32_t *pixels;
	int pitch;

	sdl_keys = SDL_GetKeyboardState(NULL);
	last = SDL_GetTicks();
	cdelta = beep = 0;

	while (1){
		while (SDL_PollEvent(&ev)){
			switch (ev.type){
			case SDL_QUIT:
				goto quit_loop;
			case SDL_KEYDOWN:
				if (ev.key.repeat){
					break;
				}
				logd("KEYDOWN");
				if (ev.key.keysym.sym == SDLK_ESCAPE){
				    chip8->pc = chip8->sp = 0;
					memset(chip8->vram, 0, 256);
				}
				
				for (i=0; i<16; i++){
					if (ev.key.keysym.scancode == keymap[i]){
						chip8_pressed(chip8, i);
						break;
					}
				}
				/* fall-through */
			case SDL_KEYUP:
				for (i=0; i<16; i++){
					keys[i] = sdl_keys[keymap[i]];
				}
				chip8_update_keys(chip8, keys);
				break;
			}
		}

		delta = SDL_GetTicks() - last;
		cdelta += delta;
		last = delta + last;

		/* La differenza va accumulata finché non arriva almeno a 16ms,
		 * altrimenti verrà sempre arrotondata a zero per i timer che
		 * contano intervalli di 16.6666ms */
		if (cdelta > 16){
			beep = chip8_update_timers(chip8, cdelta);
			cdelta = 0;
		}

		if (beep){
			logd("BEEP\n");
		}

		chip8_exec(chip8);
	    
		if (chip8->drawn){
			SDL_LockTexture(tex, NULL, (void **) &pixels, &pitch);
			/* Il display CHIP-8 è grande 64x32 pixel, ogni pixel è
			 * monocromatico ed è rappresentato da un singolo bit,
			 * dunque un byte contiene una fila di 8 pixel, per un
			 * totale di 8 byte per riga, 256 per l'intero display. */

			/* Per ogni byte */
			for (i=0; i<256; i++){
				/* Per ogni bit */
				for (j=0; j<8; j++){
					pixels[(i * 8 + j)] = (chip8->vram[i] & (1 << (7 - j))) ? fg : bg;
				}
			}
		
			SDL_UnlockTexture(tex);
			SDL_RenderCopy(ren, tex, NULL, NULL);
			SDL_RenderPresent(ren);

			/* Qui non c'è sleep perché in init_sdl() abbiamo chiesto
			 * un renderer con VSYNC, questo significa che avremo una
			 * frequenza del loop minore o uguale a quella di refresh
			 * dello schermo (tipicamente 60Hz) se bisogna disegnare */
		} else {
			/* Evitiamo 100% CPU */
			SDL_Delay(8);
		}
	}

	/* TODO: Soluzione più pulita */
 quit_loop: {}
}

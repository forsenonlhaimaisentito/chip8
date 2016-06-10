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
#include "ui.h"

static void emulation_loop(chip8_machine_t *chip8);

int main(int argc, char **argv){
	uint8_t buf[0xE00];
	uint32_t fg, bg;
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
	
	if (ui_init_sdl()){
		return 1;
	}

	ui_set_colors(fg, bg);
	
	emulation_loop(&chip8);

	ui_quit_sdl();
	
	return 0;
}

static void emulation_loop(chip8_machine_t *chip8){
	int beep;
	long last, delta, cdelta;
	
	last = SDL_GetTicks();
	cdelta = beep = 0;

	while (1){
		if (ui_input(chip8)){
			break;
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
			ui_render(chip8);

			/* Qui non c'è sleep perché in init_sdl() abbiamo chiesto
			 * un renderer con VSYNC, questo significa che avremo una
			 * frequenza del loop minore o uguale a quella di refresh
			 * dello schermo (tipicamente 60Hz) se bisogna disegnare */
		} else {
			/* Evitiamo 100% CPU */
			SDL_Delay(8);
		}
	}
}

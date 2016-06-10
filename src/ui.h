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
#ifndef _UI_H_
#define _UI_H_

#include "chip8.h"

extern int ui_init_sdl();
extern void ui_quit_sdl();
extern void ui_set_colors(uint32_t _fg, uint32_t _bg);
extern int ui_input(chip8_machine_t *chip8);
extern void ui_render(chip8_machine_t *chip8);

#endif /* _UI_H_ */

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
#ifndef _CHIP8_H_
#define _CHIP8_H_

#include <stdint.h>

#define FONT_ADDR 0x000

typedef struct {
	uint8_t v[16];      /* Registri V0-VF */
	uint16_t i;         /* Registro I */
	uint8_t dt, st;     /* Delay timer e sound timer */
	unsigned sp, pc;    /* Stack pointer e program counter */
	uint16_t stack[16]; /* Stack */
	uint8_t ram[4096];  /* RAM */
	uint8_t vram[256];  /* Memoria video (VRAM) */
	int wait;           /* Non zero se in attesa di input */
	int drawn;          /* Non zero se lo schermo va aggiornato */
	uint8_t last_key;   /* Primo tasto premuto se in attesa */
	uint8_t keys[16];   /* Stato della tastiera */
} chip8_machine_t;

extern const uint8_t font[80];

/* Funzioni da cpu.c */
extern void chip8_init(chip8_machine_t *ctx);
extern int chip8_load(chip8_machine_t *ctx, const void *prog, size_t len);
extern void chip8_pressed(chip8_machine_t *ctx, uint8_t key);
extern void chip8_update_keys(chip8_machine_t *ctx, const uint8_t *keys);
extern int chip8_update_timers(chip8_machine_t *ctx, long delta);
extern int chip8_exec(chip8_machine_t *ctx);

#endif /* _CHIP8_H_ */

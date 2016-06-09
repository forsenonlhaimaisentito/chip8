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
#ifndef _AS_H_
#define _AS_H_

#include <stdint.h>

typedef struct asm_instr {
	uint16_t opcode;
	char *label;
} asm_instr_t;

extern void push_label(const char *label);
extern void push_instr(asm_instr_t instr);
extern void push_resb(uint16_t count);
extern void push_byte(uint8_t byte);
extern void chip8_decode(uint16_t opcode, char *buf, size_t len);

#endif /* _AS_H_ */

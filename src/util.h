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
#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdint.h>
#include <stddef.h>

extern int popcount(uint8_t b);
extern void logd(const char *fmt, ...);
extern void err(const char *fmt, ...);
extern size_t read_file(const char *path, void *buf, size_t len);

#endif /* _UTIL_H_ */
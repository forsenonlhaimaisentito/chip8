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
#include <stdlib.h> /* srand, rand */
#include <string.h> /* memset, memcpy */
#include <stdint.h> /* uint8_t, uint16_t */
#include <time.h> /* time */

#include "chip8.h"
#include "util.h"

const uint8_t font[80] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0,
	0x20, 0x60, 0x20, 0x20, 0x70,
	0xF0, 0x10, 0xF0, 0x80, 0xF0,
	0xF0, 0x10, 0xF0, 0x10, 0xF0,
	0x90, 0x90, 0xF0, 0x10, 0x10,
	0xF0, 0x80, 0xF0, 0x10, 0xF0,
	0xF0, 0x80, 0xF0, 0x90, 0xF0,
	0xF0, 0x10, 0x20, 0x40, 0x40,
	0xF0, 0x90, 0xF0, 0x90, 0xF0,
	0xF0, 0x90, 0xF0, 0x10, 0xF0,
	0xF0, 0x90, 0xF0, 0x90, 0x90,
	0xE0, 0x90, 0xE0, 0x90, 0xE0,
	0xF0, 0x80, 0x80, 0x80, 0xF0,
	0xE0, 0x90, 0x90, 0x90, 0xE0,
	0xF0, 0x80, 0xF0, 0x80, 0xF0,
	0xF0, 0x80, 0xF0, 0x80, 0x80
};

/* Inizializza una macchina CHIP-8 */
void chip8_init(chip8_machine_t *ctx){
	memset(ctx, 0, sizeof(chip8_machine_t));

	/* I programmi CHIP-8 iniziano all'indirizzo 0x200 */
	ctx->pc = 0x200;

	/* Il font di sistema può avere una posizione in memoria in base
	 * all'implementazione, nel nostro caso si troverà a 0x000 */
	memcpy(ctx->ram + FONT_ADDR, font, sizeof(font));
}

/* Carica un programma CHIP-8 in memoria, tagliandolo se necessario
 * Ritorna 0 se il programma è stato caricato interamente,
 * o 1 se è stato tagliato */
int chip8_load(chip8_machine_t *ctx, const void *prog, size_t len){
	size_t actual;

	/* Abbiamo solo 0x1000 - 0x200 = 0xE00 byte di RAM,
	 * se len è maggiore limitiamoci a quelli. */
	actual = (len > 0x0E00) ? 0x0E00 : len;

	memcpy(ctx->ram + 0x200, prog, actual);

	/* Ritorniamo 1 per segnalare che il programma è stato tagliato */
	return (actual == len);
}

/* Imposta l'ultimo tasto premuto */
void chip8_pressed(chip8_machine_t *ctx, uint8_t key){
	/* Se in attesa e senza nuovi tasti, imposta */
	if (ctx->wait && !ctx->last_key){
		ctx->last_key = key;
	}
}

/* Aggiorna lo stato dei tasti, il secondo argomento è un
 * array di 16 byte, uno per tasto, dove 1 indica premuto e 0 non */
void chip8_update_keys(chip8_machine_t *ctx, const uint8_t *keys){
	memcpy(ctx->keys, keys, 16);
}

/* Aggiorna i valori dei timer in base al tempo trascorso in millisecondi delta,
 * ritorna non zero se è necessario produrre un suono */
int chip8_update_timers(chip8_machine_t *ctx, long delta){
	unsigned ticks;
	
	/* Ogni timer decrementa di 1 con una frequenza di 60 volte al secondo */
	ticks = (unsigned) ((float) delta / 16.666666f);
	
	if (ctx->dt){
		ctx->dt = (ticks >= ctx->dt) ? 0 : ctx->dt - ticks;
	}

	if (ctx->st){
		ctx->st = (ticks >= ctx->st) ? 0 : ctx->st - ticks;

		/* Se il sound timer è ancora non zero, ritorniamo non zero */
		if (ctx->st){
			return ctx->st;
		}
	}

	return 0;
}

/* Esegue la prossima istruzione in memoria
 * Ritorna:
 * 0 in caso di successo
 * 1 in caso di istruzione 0xxx non valida
 * 2 in caso di istruzione 8xxx non valida
 * 3 in caso di istruzione 9xxx non valida 
 * 4 in caso di istruzione Exxx non valida
 * 5 in caso di istruzione Fxxx non valida */
int chip8_exec(chip8_machine_t *ctx){
	uint8_t x, y, n, nn, tmp8, offset;
	uint16_t opcode, nnn, tmp;
	int jump, ret, c;

	/* Se siamo in attesa di input */
	if (ctx->wait){
		/* e non c'è input */
		if (!ctx->last_key){
			/* Ritorniamo subito */
			return 0;
		} else { /* altrimenti */
			/* passiamo il valore al programma e terminiamo l'attesa */
			ctx->v[ctx->wait - 1] = ctx->last_key;
			ctx->wait = 0;
		}
	}

	ctx->drawn = 0;
	
	/* Gli opcode CHIP-8 sono a 16 bit big-endian,
	 * quindi vanno letti in maniera indipendente
	 * dall'architettura dell'host */
	opcode = ((ctx->ram[ctx->pc & 0x0FFF] << 8)
			  | (ctx->ram[(ctx->pc + 1) & 0x0FFF] & 0xFF));

	x = (opcode >> 8) & 0x0F;
	y = (opcode >> 4) & 0x0F;
	n = opcode & 0x0F;
	nn = opcode & 0xFF;
	nnn = opcode & 0x0FFF;
	jump = ret = 0;
	
	/* Il primo nibble (4 bit) dell'opcode specifica il tipo di istruzione */
	switch (opcode & 0xF000){
	case 0x0000:
		/* Pulisci schermo e return */
		switch (opcode & 0x0FFF){
		case 0x00E0:
			/* Pulisci schermo */
			memset(ctx->vram, 0, sizeof(ctx->vram));
			ctx->drawn = 1;
			break;
		case 0x00EE:
			/* Ritorna da procedura */
			ctx->pc = ctx->stack[--ctx->sp];
			jump = 1;
			break;
		default:
			/* Esegui programma RCA1802 a NNN (obsoleto) */
			ret = 1;
			break;
		}
		break;
	case 0x1000:
		/* Salto incondizionato */
		ctx->pc = nnn;
		jump = 1;
		break;
	case 0x2000:
		/* Chiamata a procedura */
		ctx->stack[ctx->sp++] = (ctx->pc + 2) & 0x0FFF;
		ctx->pc = nnn;
		jump = 1;
		break;
	case 0x3000:
		/* Salta la prossima istruzione se V[x] == NN */
		if (ctx->v[x] == nn){
			ctx->pc = (ctx->pc + 2) & 0x0FFF;
		}
		break;
	case 0x4000:
		/* Salta la prossima istruzione se V[x] != NN */
		if (ctx->v[x] != nn){
			ctx->pc = (ctx->pc + 2) & 0x0FFF;
		}
		break;
	case 0x5000:
		/* Salta la prossima istruzione se V[x] == V[y] */
		if (ctx->v[x] == ctx->v[y]){
			ctx->pc = (ctx->pc + 2) & 0x0FFF;
		}
		break;
	case 0x6000:
		/* Imposta V[x] a NN */
		ctx->v[x] = nn;
		break;
	case 0x7000:
		/* Somma NN a V[x] */
		ctx->v[x] += nn;
		break;
	case 0x8000:
		/* Operazioni tra registri */
		switch (opcode & 0x000F){
		case 0x00:
			/* Imposta il valore di V[x] uguale a quello di V[y] */
			ctx->v[x] = ctx->v[y];
			break;
		case 0x01:
			/* Imposta il valore di V[x] uguale all'OR bitwise tra V[x] e V[y] */
			ctx->v[x] |= ctx->v[y];
			break;
		case 0x02:
			/* Imposta il valore di V[x] uguale all'AND bitwise tra V[x] e V[y] */
			ctx->v[x] &= ctx->v[y];
			break;
		case 0x03:
			/* Imposta il valore di V[x] uguale allo XOR bitwise tra V[x] e V[y] */
			ctx->v[x] ^= ctx->v[y];
			break;
		case 0x04:
			/* Somma V[y] a V[x], imposta V[0xF] a 1 se c'è resto, altrimenti a zero */
			tmp = ctx->v[x] + ctx->v[y];
			ctx->v[x] = tmp & 0xFF;
			ctx->v[0x0F] = (tmp & 0x100) >> 8;
			break;
		case 0x05:
			/* Sottrai V[y] da V[x], imposta V[0xF] a zero se c'è prestito, altrimenti a 1 */
			tmp = (ctx->v[x] <= ctx->v[y]);
			ctx->v[x] -= ctx->v[y];
			ctx->v[0x0F] = tmp;
			break;
		case 0x06:
			/* Shift a destra di uno V[x], imposta V[0xF] al valore del bit meno significativo di V[x] */
			ctx->v[0x0F] = ctx->v[x] & 0x01;
			ctx->v[x] >>= 1;
			break;
		case 0x07:
			/* Imposta V[x] come V[y] meno V[x], imposta V[0xF] a zero se c'è prestito, altrimenti a 1 */
			tmp = (ctx->v[y] <= ctx->v[x]);
			ctx->v[x] = ctx->v[y] - ctx->v[x];
			ctx->v[0x0F] = tmp;
			break;
		case 0x0E:
			/* Shift a sinistra di uno V[x], imposta V[0xF] al valore del bit più significativo di V[x] */
			ctx->v[0x0F] = (ctx->v[x] & 0x80) >> 7;
			ctx->v[x] <<= 1;
			break;
		default:
			/* Istruzione 8xxx non valida */
			ret = 2;
			break;
		}
		break;
	case 0x9000:
		switch (n){
		case 0x00:
			/* Salta la prossima istruzione se V[x] != V[y] */
			if (ctx->v[x] != ctx->v[y]){
				ctx->pc = (ctx->pc + 2) & 0x0FFF;
			}
			break;
		default:
			/* Istruzione 9xxx non valida */
			ret = 3;
			break;
		}
	case 0xA000:
		/* Imposta I a NNN */
		ctx->i = nnn;
		break;
	case 0xB000:
		/* Salta a NNN + V0 */
		ctx->pc = (nnn + ctx->v[0]) & 0x0FFF;
		jump = 1;
		break;
	case 0xC000:
		/* Imposta V[x] al risultato di AND logico tra NN ed un numero casuale */
		srand(time(NULL));
		ctx->v[x] = nn & (rand() & 0xFF);
		break;
	case 0xD000:
		/* Disegna lo sprite 8xN puntato da I alla posizione (V[x], V[y])
		 * L'operazione consiste in uno XOR bitwise tra i byte dello schermo
		 * e dello sprite; se durante l'operazione draw viene cancellato un pixel
		 * (portato da uno a zero), il registro VF avrà valore uno, altrimenti zero,
		 * questo serve per implementare una rudimentale forma di  collision detection */

		// x = ctx->v[x]
		// y = ctx->v[y]
		// r = n

		logd("DRAW %02Xh %02Xh %02Xh", ctx->v[x], ctx->v[y], n);
		
		/* Per ogni riga */
		for (tmp=0; tmp<n; tmp++){
			/* Leggo il byte della riga */
			tmp8 = ctx->ram[(ctx->i + tmp) & 0x0FFF];

			/* Trovo la posizione nella memoria video */
			offset = (ctx->v[y] + tmp) * 8 + ctx->v[x] / 8;

			/* Se lo sprite sta "in mezzo" a due byte, applico uno alla volta */
			if (ctx->v[x] % 8){
				c = popcount(ctx->vram[offset]) + popcount(ctx->vram[offset + 1]);
				ctx->vram[offset] ^= tmp8 >> (ctx->v[x] % 8); /* XOR col primo byte */
				ctx->vram[offset + 1] ^= tmp8 << (8 - (ctx->v[x] % 8)); /* XOR coi rimanenti */
				ctx->v[0x0F] = (c > (popcount(ctx->vram[offset]) + popcount(ctx->vram[offset + 1])));
			} else {
				/* Altrimenti applico direttamente */
				c = popcount(ctx->vram[offset]);
				ctx->vram[offset] ^= tmp8;
				ctx->v[0x0F] = (c > popcount(ctx->vram[offset]));
			}
		}
		
		ctx->drawn = 1;
		break;
	case 0xE000:
		/* Salti condizionati in base all'input */
		switch (opcode & 0x00FF){
		case 0x9E:
			/* Salta la prossima istruzione se il tasto con valore V[x] è premuto */
			if (ctx->keys[ctx->v[x] & 0x0F]){
				ctx->pc = (ctx->pc + 2) & 0x0FFF;
			}
			break;
		case 0xA1:
			/* Salta la prossima istruzione se il tasto con valore V[x] NON è premuto */
			if (!ctx->keys[ctx->v[x] & 0x0F]){
				ctx->pc = (ctx->pc + 2) & 0x0FFF;
			}
			break;
		default:
			/* Istruzione Exxx non valida */
			ret = 4;
			break;
		}
		break;
	case 0xF000:
		/* Funzioni miste input, timer, BCD e memoria */
		switch (opcode & 0x00FF){
		case 0x07:
			/* Imposta V[x] con valore uguale al delay timer */
			ctx->v[x] = ctx->dt;
			break;
		case 0x0A:
			/* Attendi la pressione di un tasto e scrivi il valore in V[x] */
			ctx->last_key = 0;

			/* Usiamo wait per ricordare in quale registro mettere l'input,
			 * al momento di scrivere il valore, verrà sottratto 1 da wait,
			 * questo ci permette di indicare anche il registro zero senza
			 * che wait risulti falso */
			ctx->wait = x + 1; 
			break;
		case 0x15:
			/* Imposta il delay timer con valore uguale a V[x] */
			ctx->dt = ctx->v[x];
			break;
		case 0x18:
			/* Imposta il sound timer con valore uguale a V[x] */
			ctx->st = ctx->v[x];
			break;
		case 0x1E:
			/* Somma V[x] ad I */
			ctx->i = (ctx->i + ctx->v[x]) & 0x0FFF;
			break;
		case 0x29:
			/* Imposta I con valore uguale all'indirizzo dello sprite 4x5
			 * per il carattere hex di V[x] */
			ctx->i = FONT_ADDR + (ctx->v[x] & 0x0F) * 5;
			break;
		case 0x33:
			/* Scrivi in memoria all'indirizzo contenuto in I
			 * la rappresentazione NBCD unpacked di V[x],
			 * partendo dalla cifra più significativa */
			ctx->ram[ctx->i & 0x0FFF] = ctx->v[x] / 100;
			ctx->ram[(ctx->i + 1) & 0x0FFF] = (ctx->v[x] / 10) % 10;
			ctx->ram[(ctx->i + 2) & 0x0FFF] = ctx->v[x] % 10;
			break;
		case 0x55:
			/* Scrivi i valori dei registri da V[0] a V[x] in memoria all'indirizzo contenuto in I */
			for (tmp=0; tmp<=x; tmp++){
				ctx->ram[(ctx->i + tmp) & 0x0FFF] = ctx->v[tmp];
			}
			break;
		case 0x65:
			/* Scrivi i valori in memoria all'indirizzo contenuto in I nei registri da V[0] a V[x] */
			for (tmp=0; tmp<=x; tmp++){
				 ctx->v[tmp] = ctx->ram[(ctx->i + tmp) & 0x0FFF];
			}
			break;
		default:
			/* Istruzione Fxxx non valida */
			ret = 5;
			break;
		}
		break;
	}

	if (!jump){
		ctx->pc = (ctx->pc + 2) & 0x0FFF;
	}

	return ret;
}

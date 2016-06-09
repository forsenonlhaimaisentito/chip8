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
#include <stdio.h>
#include <stdint.h>
#include <string.h>

void chip8_decode(uint16_t opcode, char *buf, size_t len){
	uint8_t x, y, n, nn;
	uint16_t nnn;
	
	x = (opcode >> 8) & 0x0F;
	y = (opcode >> 4) & 0x0F;
	n = opcode & 0x0F;
	nn = opcode & 0xFF;
	nnn = opcode & 0x0FFF;
	
	/* Il primo nibble (4 bit) dell'opcode specifica il tipo di istruzione */
	switch (opcode & 0xF000){
	case 0x0000:
		/* Pulisci schermo e return */
		switch (opcode & 0x0FFF){
		case 0x00E0:
			/* Pulisci schermo */
			snprintf(buf, len, "CLS");
			break;
		case 0x00EE:
			/* Ritorna da procedura */
			snprintf(buf, len, "RET");
			break;
		default:
			/* Esegui programma RCA1802 a NNN (obsoleto) */
			snprintf(buf, len, "EXEC %03Xh", nnn);
			break;
		}
		break;
	case 0x1000:
		/* Salto incondizionato */
		snprintf(buf, len, "JP %03Xh", nnn);
		/* TODO: label? */
		break;
	case 0x2000:
		/* Chiamata a procedura */
		snprintf(buf, len, "CALL %03Xh", nnn);
		/* TODO: label? */
		break;
	case 0x3000:
		/* Salta la prossima istruzione se V[x] == NN */
		snprintf(buf, len, "SKIPE V%1X, %02Xh", x, nn);
		break;
	case 0x4000:
		/* Salta la prossima istruzione se V[x] != NN */
		snprintf(buf, len, "SKIPNE V%1X, %02Xh", x, nn);
		break;
	case 0x5000:
		/* Salta la prossima istruzione se V[x] == V[y] */
		snprintf(buf, len, "SKIPE V%1X, V%1X", x, y);
		break;
	case 0x6000:
		/* Imposta V[x] a NN */
		snprintf(buf, len, "LD V%1X, %02Xh", x, nn);
		break;
	case 0x7000:
		/* Somma NN a V[x] */
		snprintf(buf, len, "ADD V%1X, %02Xh", x, nn);
		break;
	case 0x8000:
		/* Operazioni tra registri */
		switch (opcode & 0x000F){
		case 0x00:
			/* Imposta il valore di V[x] uguale a quello di V[y] */
			snprintf(buf, len, "LD V%1X, V%1X", x, y);
			break;
		case 0x01:
			/* Imposta il valore di V[x] uguale all'OR bitwise tra V[x] e V[y] */
			snprintf(buf, len, "OR V%1X, V%1X", x, y);
			break;
		case 0x02:
			/* Imposta il valore di V[x] uguale all'AND bitwise tra V[x] e V[y] */
			snprintf(buf, len, "AND V%1X, V%1X", x, y);
			break;
		case 0x03:
			/* Imposta il valore di V[x] uguale allo XOR bitwise tra V[x] e V[y] */
			snprintf(buf, len, "XOR V%1X, V%1X", x, y);
			break;
		case 0x04:
			/* Somma V[y] a V[x], imposta V[0xF] a 1 se c'è resto, altrimenti a zero */
			snprintf(buf, len, "ADD V%1X, V%1X", x, y);
			break;
		case 0x05:
			/* Sottrai V[y] da V[x], imposta V[0xF] a zero se c'è prestito, altrimenti a 1 */
			snprintf(buf, len, "SUB V%1X, V%1X", x, y);
			break;
		case 0x06:
			/* Shift a destra di uno V[x], imposta V[0xF] al valore del bit meno significativo di V[x] */
			snprintf(buf, len, "SHR V%1X", x);
			break;
		case 0x07:
			/* Imposta V[x] come V[y] meno V[x], imposta V[0xF] a zero se c'è prestito, altrimenti a 1 */
			snprintf(buf, len, "RSB V%1X, V%1X", x, y);
			break;
		case 0x0E:
			/* Shift a sinistra di uno V[x], imposta V[0xF] al valore del bit più significativo di V[x] */
			snprintf(buf, len, "SHL V%1X", x);
			break;
		default:
			/* Istruzione 8xxx non valida */
			snprintf(buf, len, "; Invalid %04Xh", opcode);
			break;
		}
		break;
	case 0x9000:
		if (n){
			/* Istruzione 9xxx non valida */
			snprintf(buf, len, "; Invalid %04Xh", opcode);
			break;
		}

		/* Salta la prossima istruzione se V[x] != V[y] */
		snprintf(buf, len, "SKIPNE V%1X, V%1X", x, y);
		break;
	case 0xA000:
		/* Imposta I a NNN */
	    snprintf(buf, len, "LD I, %03Xh", nnn);
		break;
	case 0xB000:
		/* Salta a NNN + V0 */
		snprintf(buf, len, "JP %03Xh + V0", nnn);
		/* TODO: label? */
		break;
	case 0xC000:
		/* Imposta V[x] al risultato di AND logico tra NN ed un numero casuale */
		snprintf(buf, len, "RAND V%1X", x);
		break;
	case 0xD000:
		/* Disegna N righe dello sprite puntato da I alla posizione (V[x], V[y]) */
		snprintf(buf, len, "DRAW V%1X, V%1X, %1Xh", x, y, n);
		break;
	case 0xE000:
		/* Salti condizionati in base all'input */
		switch (opcode & 0x00FF){
		case 0x9E:
			/* Salta la prossima istruzione se il tasto con valore V[x] è premuto */
			snprintf(buf, len, "SKIPDN V%1X", x);
			break;
		case 0xA1:
			/* Salta la prossima istruzione se il tasto con valore V[x] NON è premuto */
			snprintf(buf, len, "SKIPUP V%1X", x);
			break;
		default:
			/* Istruzione Exxx non valida */
			snprintf(buf, len, "; Invalid %04Xh", opcode);
			break;
		}
		break;
	case 0xF000:
		/* Funzioni miste input, timer, BCD e memoria */
		switch (opcode & 0x00FF){
		case 0x07:
			/* Imposta V[x] con valore uguale al delay timer */
			snprintf(buf, len, "LD V%1X, DT", x);
			break;
		case 0x0A:
			/* Attendi la pressione di un tasto e scrivi il valore in V[x] */
			snprintf(buf, len, "IN V%1X", x);
			break;
		case 0x15:
			/* Imposta il delay timer con valore uguale a V[x] */
			snprintf(buf, len, "LD DT, V%1X", x);
			break;
		case 0x18:
			/* Imposta il sound timer con valore uguale a V[x] */
			snprintf(buf, len, "LD ST, V%1X", x);
			break;
		case 0x1E:
			/* Somma V[x] ad I */
			snprintf(buf, len, "ADD I, V%1X", x);
			break;
		case 0x29:
			/* Imposta I con valore uguale all'indirizzo dello sprite 4x5
			 * per il carattere hex di V[x] */
			snprintf(buf, len, "SPRITE V%1X", x);
			break;
		case 0x33:
			/* Scrivi in memoria all'indirizzo contenuto in I
			 * la rappresentazione NBCD unpacked di V[x],
			 * partendo dalla cifra più significativa */
			snprintf(buf, len, "BCD V%1X", x);
			break;
		case 0x55:
			/* Scrivi i valori dei registri da V[0] a V[x] in memoria all'indirizzo contenuto in I */
			snprintf(buf, len, "STOR V%1X", x);
			break;
		case 0x65:
			/* Scrivi i valori in memoria all'indirizzo contenuto in I nei registri da V[0] a V[x] */
			snprintf(buf, len, "LOAD V%1X", x);
			break;
		default:
			/* Istruzione Fxxx non valida */
			snprintf(buf, len, "; Invalid %04Xh", opcode);
			break;
		}
		break;
	}
}

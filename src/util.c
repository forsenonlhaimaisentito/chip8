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
#include <stdio.h> /* FILE, fopen, fread, fclose, fprintf */
#include <stddef.h> /* size_t */
#include <stdint.h> /* uint8_t */
#include <errno.h> /* errno */
#include <string.h> /* strerror */
#include <stdarg.h> /* va_list */

/* Conta il numero di bit impostati ad 1 in un byte */
int popcount(uint8_t b){
	int c;

	/* Finché ci sono bit nel byte */
	for (c=0; b; c++){
		/* Rimuovo un bit dal byte e lo conto */
		b &= b - 1;
	}
	
	return c;
}

/* Funzione generica di logging, funzionante
 * solo se la costante DEBUG è impostata a
 * compile-time */
void logd(const char *fmt, ...){
#ifdef DEBUG
	va_list ap;

	fprintf(stderr, "DEBUG: ");
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
#endif
}

/* Segnala un errore aggiungendo il messaggio specifico */
void err(const char *fmt, ...){
	va_list ap;

	fprintf(stderr, "ERRORE: ");
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, ": %s\n", strerror(errno));
}

/* Legge il file indicato da path, memorizzando il contenuto nel
 * buffer buf, al massimo len byte */
size_t read_file(const char *path, void *buf, size_t len){
	FILE *fp;
	size_t count;

	if ((fp = fopen(path, "rb")) == NULL){
		err("Impossibile aprire il file %s", path);
		goto fail;
	}
	
	count = fread(buf, 1, len, fp);

	if (((count < len) && !feof(fp)) || ferror(fp)){
		err("Errore di lettura per %s", path);
		goto fail;
	} else if ((count == len) && !feof(fp)){
		fprintf(stderr, "Attenzione: file letto solo per %ld byte\n", len);
	}

	fclose(fp);
	return count;
	
 fail:
	if (fp){
		fclose(fp);
	}
	return 0;
}

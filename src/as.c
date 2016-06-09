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
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "as.h"

#ifndef MAX_LABELS
#define MAX_LABELS 256
#endif

extern int yyparse();
extern FILE *yyin;

struct asm_label {
	uint16_t addr;
	char *name;
};

enum asm_state {
	LABEL_SCAN, ASSEMBLY
};

static uint8_t *prog;
static size_t bufsize, used;
static struct asm_label labels[MAX_LABELS];
static unsigned nlabels;
static enum asm_state asm_state;

static void disas(const char *file);

int main(int argc, char **argv){
	char *infile, *outfile;
	FILE *out;

	if (argc < 2){
		fprintf(stderr, "Assembler: %s INFILE OUTFILE\n", argv[0]);
		fprintf(stderr, "Disassembler: %s -d INFILE\n", argv[0]);
		return 0;
	} else if (argc < 3){
		disas(argv[1]);
		return 0;
	}

	infile = argv[1];
	outfile = argv[2];
	
	if ((yyin = fopen(infile, "r")) == NULL){
		err("impossibile leggere il file %s", infile);
		return EXIT_FAILURE;
	}
	
	asm_state = LABEL_SCAN;
	if (yyparse()){
		fclose(yyin);
		return EXIT_FAILURE;
	}
	
	asm_state = ASSEMBLY;
	bufsize = 4096;
	used = 0;
	prog = malloc(bufsize);
	fseek(yyin, 0, SEEK_SET);
	if (yyparse()){
		fclose(yyin);
		return EXIT_FAILURE;
	}

	fclose(yyin);

	if ((out = fopen(outfile, "wb")) == NULL){
		err("impossibile scrivere il file %s", outfile);
		return EXIT_FAILURE;
	}

	if (fwrite(prog, 1, used, out) < used){
		err("impossibile scrivere il file %s", outfile);
		fclose(out);
		free(prog);
		return EXIT_FAILURE;
	}
	
	fclose(out);
	free(prog);

	fprintf(stderr, "Scritti %ld bytes\n", used);
	
	return 0;
}

static void disas(const char *file){
	char buf[128];
	unsigned index;
	size_t count;
	uint16_t opcode;

	prog = malloc(4096);
	
	if (!(count = read_file(file, prog, 4096))){
		exit(EXIT_FAILURE);
	}

	for (index=0; index<count; index+=2){
		opcode = ((prog[index & 0x0FFF] << 8)
				  | (prog[(index + 1) & 0x0FFF] & 0xFF));
		chip8_decode(opcode, buf, 128);
		printf("%04X\t%02X %02X\t%s\n", index+0x200, prog[index & 0x0FFF], prog[(index + 1) & 0x0FFF], buf);
	}
}

static void check_buffer(int needed){
	if (used + needed >= bufsize){
		bufsize += needed + (bufsize / 2);
		if ((prog = realloc(prog, bufsize)) == NULL){
			err("impossibile allocare memoria");
			fclose(yyin);
			exit(EXIT_FAILURE);
		}
	}
}

void push_resb(uint16_t count){
	int i;

	if (asm_state != ASSEMBLY){
		used += count;
		return;
	}
	
	check_buffer(count);

	for (i=0; i<count; i++){
		prog[used + i] = 0;
		used++;
	}
	logd("RESB %ud\n", count);
}

void push_byte(uint8_t byte){
	if (asm_state != ASSEMBLY){
		used++;
		return;
	}
	
	check_buffer(0);
	prog[used++] = byte;
	logd("PUSHb %02X\n", byte);
}

void push_label(const char *label){
	if (asm_state != LABEL_SCAN){
		return;
	}
	
	if (nlabels == MAX_LABELS){
		fprintf(stderr, "Errore: limite di %d label raggiunto", MAX_LABELS);
		fclose(yyin);
		free(prog);
		exit(EXIT_FAILURE);
	}
	
	labels[nlabels++] = (struct asm_label) { 0x200 + used, label };
	logd("PUSHl %s = %04Xh\n", label, 0x200 + used);
}

void push_instr(asm_instr_t instr){
	unsigned i;

	if (asm_state != ASSEMBLY){
		used += 2;
		return;
	}
	
	if (instr.label != NULL){
		for (i=0; i<nlabels; i++){
			if (!strcmp(labels[i].name, instr.label)){
				instr.opcode |= labels[i].addr;
				instr.label = NULL;
				break;
			}
		}

		if (instr.label){
			fprintf(stderr, "Errore: label sconosciuto: %s", instr.label);
			fclose(yyin);
			exit(EXIT_FAILURE);
		}
	}

    check_buffer(0);

	prog[used++] = (instr.opcode >> 8) & 0xFF;
	prog[used++] = instr.opcode & 0xFF;
	
	logd("PUSHi %04Xh\n", instr.opcode);
}

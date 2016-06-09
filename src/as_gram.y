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
%{
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "as.h"

extern int yylex();
extern int yyparse();
extern FILE *yyin;
extern int linenum;
extern int yylineno;

void yyerror(const char *s) {
	printf("Error at line %d: %s\n", yylineno, s);
} 
%}

%define parse.lac full
%define parse.error verbose

%union {
	uint8_t byte;
	uint16_t word;
	char *text;
	asm_instr_t instr;
}

%token					T_COMMA T_CLS T_RET T_JP T_CALL T_SKIPE T_SKIPNE
						T_LD T_ADD T_SUB T_RSB T_OR T_AND T_XOR T_SHR T_SHL
						T_RAND T_DRAW T_SKIPDN T_SKIPUP T_IN T_SPRITE T_BCD T_PLUS
						T_STOR T_LOAD T_IREG T_DT T_ST T_COLON T_DB T_RESB T_QUOTE
%token	<text>			T_LITERAL T_ASCII
%token	<byte>			T_BYTE T_DREG
%token	<word>			T_WORD

%type	<text>			label
%type	<word>			resb
%type	<instr>			command ret jp call skip ld mathop bitop memop misc
						
%%

program:		stmts
		;

stmts:			stmt
		|		stmts stmt
		;

stmt:			label { push_label($1); }
		|		command { push_instr($1); }
		|		data
		;

label:			T_LITERAL T_COLON { $$ = $1; }
		;

command:		ret
		|		jp
		|		call
		|		skip
		|		ld
		|		mathop
		|		bitop
		|		memop
		|		misc
		;

ret:			T_RET { $$ = (asm_instr_t) { 0x00EE, NULL }; }
		;

jp:				T_JP T_WORD { $$ = (asm_instr_t) { 0x1000 | $2, NULL }; }
		|		T_JP T_LITERAL { $$ = (asm_instr_t) { 0x1000, $2 }; }
		|		T_JP T_WORD T_PLUS T_DREG
				{
					if ($4 != 0){
						yyerror("only V0 is valid for offset jump");
						YYABORT;
					}
					$$ = (asm_instr_t) { 0xB000 | $2, NULL };
				}
		|		T_JP T_LITERAL T_PLUS T_DREG
				{
					if ($4 != 0){
						yyerror("only V0 is valid for offset jump");
						YYABORT;
					}
					$$ = (asm_instr_t) { 0xB000, $2 };
				}
		;

call:			T_CALL T_WORD { $$ = (asm_instr_t) { 0x2000 | $2, NULL }; }
		|		T_CALL T_LITERAL { $$ = (asm_instr_t) { 0x2000, $2 }; }
		;

skip:			T_SKIPE T_DREG T_COMMA T_BYTE { $$ = (asm_instr_t) { 0x3000 | ($2 << 8) | $4, NULL }; }
		|		T_SKIPNE T_DREG T_COMMA T_BYTE { $$ = (asm_instr_t) { 0x4000 | ($2 << 8) | $4, NULL }; }
		|		T_SKIPE T_DREG T_COMMA T_DREG { $$ = (asm_instr_t) { 0x5000 | ($2 << 8) | ($4 << 4), NULL }; }
		|		T_SKIPNE T_DREG T_COMMA T_DREG { $$ = (asm_instr_t) { 0x9000 | ($2 << 8) | ($4 << 4), NULL }; }
		|		T_SKIPDN T_DREG { $$ = (asm_instr_t) { 0xE09E | ($2 << 8), NULL }; }
		|		T_SKIPUP T_DREG { $$ = (asm_instr_t) { 0xE0A1 | ($2 << 8), NULL }; }
		;

ld:				T_LD T_DREG T_COMMA T_BYTE { $$ = (asm_instr_t) { 0x6000 | ($2 << 8) | $4, NULL }; }
		|		T_LD T_DREG T_COMMA T_WORD
				{
					if ($4 > 255){
						yyerror("integer value too large for a byte");
						YYABORT;
					}
					$$ = (asm_instr_t) { 0x6000 | ($2 << 8) | $4, NULL };
				}
		|		T_LD T_DREG T_COMMA T_DREG { $$ = (asm_instr_t) { 0x8000 | ($2 << 8) | ($4 << 4), NULL }; }
		|		T_LD T_IREG T_COMMA T_WORD { $$ = (asm_instr_t) { 0xA000 | $4, NULL }; }
		|		T_LD T_IREG T_COMMA T_LITERAL { $$ = (asm_instr_t) { 0xA000, $4 }; }
		|		T_LD T_DT T_COMMA T_DREG { $$ = (asm_instr_t) { 0xF015 | ($4 << 8) , NULL }; }
		|		T_LD T_DREG T_COMMA T_DT { $$ = (asm_instr_t) { 0xF007 | ($2 << 8) , NULL }; }
		|		T_LD T_ST T_COMMA T_DREG { $$ = (asm_instr_t) { 0xF018 | ($4 << 8) , NULL }; }
		;

mathop:			T_ADD T_DREG T_COMMA T_BYTE { $$ = (asm_instr_t) { 0x7000 | ($2 << 8) | $4, NULL }; }
		|		T_ADD T_DREG T_COMMA T_DREG { $$ = (asm_instr_t) { 0x8004 | ($2 << 8) | ($4 << 4), NULL }; }
		|		T_SUB T_DREG T_COMMA T_DREG { $$ = (asm_instr_t) { 0x8005 | ($2 << 8) | ($4 << 4), NULL }; }
		|		T_RSB T_DREG T_COMMA T_DREG { $$ = (asm_instr_t) { 0x8005 | ($2 << 8) | ($4 << 4), NULL }; }
		|		T_ADD T_IREG T_COMMA T_DREG { $$ = (asm_instr_t) { 0xF01E | ($4 << 8), NULL }; }
		;

bitop:			T_OR T_DREG T_COMMA T_DREG  { $$ = (asm_instr_t) { 0x8001 | ($2 << 8) | ($4 << 4), NULL }; }
		|		T_AND T_DREG T_COMMA T_DREG  { $$ = (asm_instr_t) { 0x8002 | ($2 << 8) | ($4 << 4), NULL }; }
		|		T_XOR T_DREG T_COMMA T_DREG  { $$ = (asm_instr_t) { 0x8003 | ($2 << 8) | ($4 << 4), NULL }; }
		|		T_SHR T_DREG { $$ = (asm_instr_t) { 0x8006 | ($2 << 8), NULL }; }
		|		T_SHL T_DREG { $$ = (asm_instr_t) { 0x800E | ($2 << 8), NULL }; }
		|		T_RAND T_DREG T_COMMA T_BYTE  { $$ = (asm_instr_t) { 0xC000 | ($2 << 8) | $4, NULL }; }
		;

memop:			T_STOR T_DREG { $$ = (asm_instr_t) { 0xF055 | ($2 << 8), NULL }; }
		|		T_LOAD T_DREG { $$ = (asm_instr_t) { 0xF065 | ($2 << 8), NULL }; }
		;

misc:			T_CLS { $$ = (asm_instr_t) { 0x00E0, NULL }; }
		|		T_BCD T_DREG { $$ = (asm_instr_t) { 0xF033 | ($2 << 8), NULL }; }
		|		T_IN T_DREG { $$ = (asm_instr_t) { 0xF00A | ($2 << 8), NULL }; }
		|		T_SPRITE T_DREG { $$ = (asm_instr_t) { 0xF029 | ($2 << 8), NULL }; }
		|		T_DRAW T_DREG T_COMMA T_DREG T_COMMA T_BYTE { $$ = (asm_instr_t) { 0xD000 | ($2 << 8) | ($4 << 4) | ($6 & 0x0F), NULL }; }
		;

data:			db
		|		resb { push_resb($1); }
		;

db:				T_DB bytes
		;

bytes:			bytes T_BYTE { push_byte($2); }
		|		T_BYTE { push_byte($1); }
		|		T_QUOTE T_ASCII T_QUOTE
				{
					int i;
					for (i=0; i<strlen($2); i++){
						push_byte((uint8_t) $2[i]);
					}
				}
		;

resb:			T_RESB T_BYTE { $$ = $2; }
		|		T_RESB T_WORD { $$ = $2; }
		;

%%

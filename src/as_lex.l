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
#include <string.h>
#include <stdint.h>
#include "as.h"
#include "as_gram.h"
%}

%option noyywrap
%option yylineno

%x str

%%

[ \t]						;
\n				 			;
;[^\n]*						;
\'							{ BEGIN(str); return T_QUOTE; }
<str>\'						{ BEGIN(INITIAL); return T_QUOTE; }
,							return T_COMMA;
:							return T_COLON;
\+							return T_PLUS;
0x[0-9A-Fa-f]{1,2}			{ yylval.byte = (uint8_t) strtol(yytext, NULL, 16); return T_BYTE; }
[0-9A-Fa-f]{1,2}h			{ yylval.byte = (uint8_t) strtol(yytext, NULL, 16); return T_BYTE; }
0x[0-9A-Fa-f]{3}			{ yylval.word = (uint16_t) strtol(yytext, NULL, 16) & 0x0FFF; return T_WORD; }
[0-9A-Fa-f]{3}h				{ yylval.word = (uint16_t) strtol(yytext, NULL, 16) & 0x0FFF; return T_WORD; }
[0-9]{1,4}					{ yylval.word = (uint16_t) atoi(yytext) & 0x0FFF; return T_WORD; }
(?i:V[0-9A-F])				{ yylval.byte = strtol(yytext+1, NULL, 16); return T_DREG; }
(?i:i)						return T_IREG;
(?i:dt)						return T_DT;
(?i:st)						return T_ST;
(?i:"CLS")					return T_CLS;
(?i:"RET")					return T_RET;
(?i:"JP")					return T_JP;
(?i:"CALL")					return T_CALL;
(?i:"SKIPE")				return T_SKIPE;
(?i:"SKIPNE")				return T_SKIPNE;
(?i:"LD")					return T_LD;
(?i:"ADD")					return T_ADD;
(?i:"SUB")					return T_SUB;
(?i:"RSB")					return T_RSB;
(?i:"OR")					return T_OR;
(?i:"AND")					return T_AND;
(?i:"XOR")					return T_XOR;
(?i:"SHR")					return T_SHR;
(?i:"SHL")					return T_SHL;
(?i:"RAND")					return T_RAND;
(?i:"DRAW")					return T_DRAW;
(?i:"SKIPDW")				return T_SKIPDN;
(?i:"SKIPUP")				return T_SKIPUP;
(?i:"IN")					return T_IN;
(?i:"SPRITE")				return T_SPRITE;
(?i:"BCD")					return T_BCD;
(?i:"STOR")					return T_STOR;
(?i:"LOAD")					return T_LOAD;
(?i:"DB")					return T_DB;
(?i:"RESB")					return T_RESB;
[A-Za-z_.][A-Za-z0-9_.]*	{ yylval.text = strdup(yytext); return T_LITERAL; }
<str>[^\']+					{ yylval.text = strdup(yytext); return T_ASCII; }

%%

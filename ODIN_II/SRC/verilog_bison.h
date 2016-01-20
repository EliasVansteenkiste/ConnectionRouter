/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     vSYMBOL_ID = 258,
     vNUMBER_ID = 259,
     vDELAY_ID = 260,
     vALWAYS = 261,
     vINITIAL = 262,
     vSPECIFY = 263,
     vAND = 264,
     vASSIGN = 265,
     vBEGIN = 266,
     vCASE = 267,
     vDEFAULT = 268,
     vDEFINE = 269,
     vELSE = 270,
     vEND = 271,
     vENDCASE = 272,
     vENDMODULE = 273,
     vENDSPECIFY = 274,
     vENDFUNCTION = 275,
     vIF = 276,
     vINOUT = 277,
     vINPUT = 278,
     vMODULE = 279,
     vFUNCTION = 280,
     vNAND = 281,
     vNEGEDGE = 282,
     vNOR = 283,
     vNOT = 284,
     vOR = 285,
     vFOR = 286,
     vOUTPUT = 287,
     vPARAMETER = 288,
     vPOSEDGE = 289,
     vREG = 290,
     vWIRE = 291,
     vXNOR = 292,
     vXOR = 293,
     vDEFPARAM = 294,
     voANDAND = 295,
     voOROR = 296,
     voLTE = 297,
     voGTE = 298,
     voPAL = 299,
     voSLEFT = 300,
     voSRIGHT = 301,
     voEQUAL = 302,
     voNOTEQUAL = 303,
     voCASEEQUAL = 304,
     voCASENOTEQUAL = 305,
     voXNOR = 306,
     voNAND = 307,
     voNOR = 308,
     vWHILE = 309,
     vINTEGER = 310,
     vNOT_SUPPORT = 311,
     UOR = 312,
     UAND = 313,
     UNOT = 314,
     UNAND = 315,
     UNOR = 316,
     UXNOR = 317,
     UXOR = 318,
     ULNOT = 319,
     UADD = 320,
     UMINUS = 321,
     LOWER_THAN_ELSE = 322
   };
#endif
/* Tokens.  */
#define vSYMBOL_ID 258
#define vNUMBER_ID 259
#define vDELAY_ID 260
#define vALWAYS 261
#define vINITIAL 262
#define vSPECIFY 263
#define vAND 264
#define vASSIGN 265
#define vBEGIN 266
#define vCASE 267
#define vDEFAULT 268
#define vDEFINE 269
#define vELSE 270
#define vEND 271
#define vENDCASE 272
#define vENDMODULE 273
#define vENDSPECIFY 274
#define vENDFUNCTION 275
#define vIF 276
#define vINOUT 277
#define vINPUT 278
#define vMODULE 279
#define vFUNCTION 280
#define vNAND 281
#define vNEGEDGE 282
#define vNOR 283
#define vNOT 284
#define vOR 285
#define vFOR 286
#define vOUTPUT 287
#define vPARAMETER 288
#define vPOSEDGE 289
#define vREG 290
#define vWIRE 291
#define vXNOR 292
#define vXOR 293
#define vDEFPARAM 294
#define voANDAND 295
#define voOROR 296
#define voLTE 297
#define voGTE 298
#define voPAL 299
#define voSLEFT 300
#define voSRIGHT 301
#define voEQUAL 302
#define voNOTEQUAL 303
#define voCASEEQUAL 304
#define voCASENOTEQUAL 305
#define voXNOR 306
#define voNAND 307
#define voNOR 308
#define vWHILE 309
#define vINTEGER 310
#define vNOT_SUPPORT 311
#define UOR 312
#define UAND 313
#define UNOT 314
#define UNAND 315
#define UNOR 316
#define UXNOR 317
#define UXOR 318
#define ULNOT 319
#define UADD 320
#define UMINUS 321
#define LOWER_THAN_ELSE 322




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 61 "SRC/verilog_bison.y"
{
	char *id_name;
	char *num_value;
	ast_node_t *node;
}
/* Line 1529 of yacc.c.  */
#line 189 "SRC/verilog_bison.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;


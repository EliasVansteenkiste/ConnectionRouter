/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



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




/* Copy the first part of user declarations.  */
#line 1 "SRC/verilog_bison.y"

/*
Copyright (c) 2009 Peter Andrew Jamieson (jamieson.peter@gmail.com)

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/ 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "parse_making_ast.h"
 
#define PARSE {printf("here\n");}

#ifndef YYLINENO
extern int yylineno;
#define YYLINENO yylineno
#else
extern int yylineno;
#endif

void yyerror(const char *str);
int yywrap();
int yyparse();
int yylex(void);

 // RESPONCE in an error
void yyerror(const char *str)
{
	fprintf(stderr,"error in parsing: %s - on line number %d\n",str, yylineno);
	exit(-1);
}
 
// point of continued file reading
int yywrap()
{
	return 1;
}



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 61 "SRC/verilog_bison.y"
{
	char *id_name;
	char *num_value;
	ast_node_t *node;
}
/* Line 193 of yacc.c.  */
#line 296 "SRC/verilog_bison.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 309 "SRC/verilog_bison.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  9
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1880

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  94
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  62
/* YYNRULES -- Number of rules.  */
#define YYNRULES  181
/* YYNRULES -- Number of states.  */
#define YYNSTATES  421

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   322

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    76,     2,    92,     2,    68,    61,     2,
      69,    70,    66,    64,    89,    65,    90,    67,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    58,    88,
      62,    91,    63,    57,    93,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    73,     2,    74,    60,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    71,    59,    72,    75,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     8,    11,    13,    15,    19,    28,
      38,    46,    49,    51,    53,    55,    57,    59,    61,    63,
      65,    67,    69,    71,    73,    75,    77,    79,    85,    88,
      92,    95,    97,    99,   101,   103,   105,   107,   109,   111,
     115,   119,   123,   127,   131,   135,   139,   143,   147,   149,
     151,   158,   162,   166,   168,   172,   174,   176,   183,   195,
     204,   208,   210,   217,   221,   225,   229,   231,   235,   239,
     243,   247,   251,   255,   259,   263,   265,   269,   271,   278,
     284,   293,   301,   305,   307,   311,   315,   317,   320,   324,
     329,   338,   342,   350,   354,   356,   360,   362,   368,   370,
     374,   376,   382,   384,   388,   390,   392,   395,   398,   404,
     412,   419,   429,   435,   437,   440,   442,   449,   453,   458,
     462,   467,   471,   474,   476,   480,   484,   488,   491,   493,
     498,   503,   507,   509,   511,   514,   517,   519,   522,   525,
     528,   531,   534,   537,   540,   543,   546,   549,   553,   557,
     561,   565,   569,   573,   577,   581,   585,   589,   593,   597,
     601,   605,   609,   613,   617,   621,   625,   629,   633,   637,
     641,   647,   649,   653,   660,   667,   669,   671,   676,   683,
     687,   691
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      95,     0,    -1,    96,    -1,    96,    98,    -1,    96,    97,
      -1,    98,    -1,    97,    -1,    14,     3,     4,    -1,    24,
       3,    69,   116,    70,    88,    99,    18,    -1,    24,     3,
      69,   116,    89,    70,    88,    99,    18,    -1,    24,     3,
      69,    70,    88,    99,    18,    -1,    99,   100,    -1,   100,
      -1,   107,    -1,   109,    -1,   110,    -1,   111,    -1,   112,
      -1,   113,    -1,   120,    -1,   122,    -1,   128,    -1,   101,
      -1,   102,    -1,   138,    -1,   108,    -1,   103,    -1,    25,
     114,    88,   104,    20,    -1,     7,   140,    -1,     8,   141,
      19,    -1,   104,   105,    -1,   105,    -1,   107,    -1,   106,
      -1,   112,    -1,   113,    -1,   120,    -1,   108,    -1,   139,
      -1,    23,   116,    88,    -1,    33,   116,    88,    -1,    39,
     116,    88,    -1,    23,   116,    88,    -1,    32,   116,    88,
      -1,    22,   116,    88,    -1,    36,   116,    88,    -1,    35,
     116,    88,    -1,    55,   117,    88,    -1,   115,    -1,     3,
      -1,    73,   153,    58,   153,    74,     3,    -1,   116,    89,
     118,    -1,   116,    90,   118,    -1,   118,    -1,   117,    89,
     119,    -1,   119,    -1,     3,    -1,    73,   153,    58,   153,
      74,     3,    -1,    73,   153,    58,   153,    74,     3,    73,
     153,    58,   153,    74,    -1,    73,   153,    58,   153,    74,
       3,    91,   153,    -1,     3,    91,   154,    -1,     3,    -1,
       3,    73,   153,    58,   153,    74,    -1,     3,    91,   154,
      -1,    10,   121,    88,    -1,   121,    89,   143,    -1,   143,
      -1,     9,   123,    88,    -1,    26,   123,    88,    -1,    28,
     123,    88,    -1,    29,   124,    88,    -1,    30,   123,    88,
      -1,    37,   123,    88,    -1,    38,   123,    88,    -1,   123,
      89,   126,    -1,   126,    -1,   124,    89,   125,    -1,   125,
      -1,     3,    69,   153,    89,   153,    70,    -1,    69,   153,
      89,   153,    70,    -1,     3,    69,   153,    89,   153,    89,
     127,    70,    -1,    69,   153,    89,   153,    89,   127,    70,
      -1,   127,    89,   153,    -1,   153,    -1,     3,   129,    88,
      -1,   129,    89,   132,    -1,   132,    -1,     3,   131,    -1,
      69,   133,    70,    -1,     3,    69,   134,    70,    -1,    92,
      69,   136,    70,     3,    69,   134,    70,    -1,     3,    69,
      70,    -1,    92,    69,   136,    70,     3,    69,    70,    -1,
     133,    89,   135,    -1,   135,    -1,   134,    89,   135,    -1,
     135,    -1,    90,     3,    69,   153,    70,    -1,   153,    -1,
     136,    89,   137,    -1,   137,    -1,    90,     3,    69,   153,
      70,    -1,   153,    -1,     6,   150,   140,    -1,   140,    -1,
     148,    -1,   143,    88,    -1,   144,    88,    -1,    21,    69,
     153,    70,   140,    -1,    21,    69,   153,    70,   140,    15,
     140,    -1,    12,    69,   153,    70,   146,    17,    -1,    31,
      69,   143,    88,   153,    88,   143,    70,   140,    -1,    54,
      69,   153,    70,   140,    -1,    88,    -1,   141,   142,    -1,
     142,    -1,    69,   145,    70,    91,   154,    88,    -1,   154,
      91,   153,    -1,   154,    91,     5,   153,    -1,   154,    42,
     153,    -1,   154,    42,     5,   153,    -1,   154,    44,   153,
      -1,   146,   147,    -1,   147,    -1,   153,    58,   140,    -1,
      13,    58,   140,    -1,    11,   149,    16,    -1,   149,   140,
      -1,   140,    -1,    93,    69,   151,    70,    -1,    93,    69,
      66,    70,    -1,   151,    30,   152,    -1,   152,    -1,   154,
      -1,    34,     3,    -1,    27,     3,    -1,   154,    -1,    64,
     153,    -1,    65,   153,    -1,    75,   153,    -1,    61,   153,
      -1,    59,   153,    -1,    52,   153,    -1,    53,   153,    -1,
      51,   153,    -1,    76,   153,    -1,    60,   153,    -1,   153,
      60,   153,    -1,   153,    66,   153,    -1,   153,    67,   153,
      -1,   153,    68,   153,    -1,   153,    64,   153,    -1,   153,
      65,   153,    -1,   153,    61,   153,    -1,   153,    59,   153,
      -1,   153,    52,   153,    -1,   153,    53,   153,    -1,   153,
      51,   153,    -1,   153,    62,   153,    -1,   153,    63,   153,
      -1,   153,    46,   153,    -1,   153,    45,   153,    -1,   153,
      47,   153,    -1,   153,    48,   153,    -1,   153,    42,   153,
      -1,   153,    43,   153,    -1,   153,    49,   153,    -1,   153,
      50,   153,    -1,   153,    41,   153,    -1,   153,    40,   153,
      -1,   153,    57,   153,    58,   153,    -1,   130,    -1,    69,
     153,    70,    -1,    71,   153,    71,   153,    72,    72,    -1,
      71,   153,    71,   155,    72,    72,    -1,     4,    -1,     3,
      -1,     3,    73,   153,    74,    -1,     3,    73,   153,    58,
     153,    74,    -1,    71,   155,    72,    -1,   155,    89,   153,
      -1,   153,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   126,   126,   129,   139,   140,   141,   144,   147,   148,
     149,   152,   153,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   173,   177,   181,
     184,   185,   188,   189,   190,   191,   192,   193,   194,   197,
     200,   203,   206,   209,   212,   215,   216,   218,   220,   222,
     223,   225,   226,   227,   230,   231,   233,   234,   235,   237,
     239,   242,   244,   247,   250,   252,   253,   257,   258,   259,
     260,   261,   262,   263,   267,   268,   270,   271,   273,   274,
     278,   279,   284,   285,   289,   294,   295,   298,   303,   305,
     306,   307,   308,   310,   311,   313,   314,   317,   318,   321,
     322,   325,   326,   330,   333,   334,   335,   336,   337,   338,
     339,   340,   341,   342,   345,   346,   349,   351,   352,   355,
     356,   359,   360,   361,   364,   365,   368,   371,   372,   375,
     376,   380,   381,   384,   385,   386,   389,   390,   391,   392,
     393,   394,   395,   396,   397,   398,   399,   400,   401,   402,
     403,   404,   405,   406,   407,   408,   409,   410,   411,   412,
     413,   414,   415,   416,   417,   418,   419,   420,   421,   422,
     423,   424,   425,   429,   430,   433,   434,   435,   436,   437,
     440,   441
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "vSYMBOL_ID", "vNUMBER_ID", "vDELAY_ID",
  "vALWAYS", "vINITIAL", "vSPECIFY", "vAND", "vASSIGN", "vBEGIN", "vCASE",
  "vDEFAULT", "vDEFINE", "vELSE", "vEND", "vENDCASE", "vENDMODULE",
  "vENDSPECIFY", "vENDFUNCTION", "vIF", "vINOUT", "vINPUT", "vMODULE",
  "vFUNCTION", "vNAND", "vNEGEDGE", "vNOR", "vNOT", "vOR", "vFOR",
  "vOUTPUT", "vPARAMETER", "vPOSEDGE", "vREG", "vWIRE", "vXNOR", "vXOR",
  "vDEFPARAM", "voANDAND", "voOROR", "voLTE", "voGTE", "voPAL", "voSLEFT",
  "voSRIGHT", "voEQUAL", "voNOTEQUAL", "voCASEEQUAL", "voCASENOTEQUAL",
  "voXNOR", "voNAND", "voNOR", "vWHILE", "vINTEGER", "vNOT_SUPPORT", "'?'",
  "':'", "'|'", "'^'", "'&'", "'<'", "'>'", "'+'", "'-'", "'*'", "'/'",
  "'%'", "'('", "')'", "'{'", "'}'", "'['", "']'", "'~'", "'!'", "UOR",
  "UAND", "UNOT", "UNAND", "UNOR", "UXNOR", "UXOR", "ULNOT", "UADD",
  "UMINUS", "LOWER_THAN_ELSE", "';'", "','", "'.'", "'='", "'#'", "'@'",
  "$accept", "source_text", "items", "define", "module",
  "list_of_module_items", "module_item", "function_declaration",
  "initial_block", "specify_block", "list_of_function_items",
  "function_item", "function_input_declaration", "parameter_declaration",
  "defparam_declaration", "input_declaration", "output_declaration",
  "inout_declaration", "net_declaration", "integer_declaration",
  "function_output_variable", "function_id_and_output_variable",
  "variable_list", "integer_type_variable_list", "variable",
  "integer_type_variable", "continuous_assign",
  "list_of_blocking_assignment", "gate_declaration",
  "list_of_multiple_inputs_gate_declaration_instance",
  "list_of_single_input_gate_declaration_instance",
  "single_input_gate_instance", "multiple_inputs_gate_instance",
  "list_of_multiple_inputs_gate_connections", "module_instantiation",
  "list_of_module_instance", "function_instantiation", "function_instance",
  "module_instance", "list_of_function_connections",
  "list_of_module_connections", "module_connection",
  "list_of_module_parameters", "module_parameter", "always",
  "function_statement", "statement", "list_of_specify_statement",
  "specify_statement", "blocking_assignment", "non_blocking_assignment",
  "parallel_connection", "case_item_list", "case_items", "seq_block",
  "stmt_list", "delay_control", "event_expression_list",
  "event_expression", "expression", "primary", "expression_list", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,    63,    58,   124,
      94,    38,    60,    62,    43,    45,    42,    47,    37,    40,
      41,   123,   125,    91,    93,   126,    33,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,    59,    44,
      46,    61,    35,    64
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    94,    95,    96,    96,    96,    96,    97,    98,    98,
      98,    99,    99,   100,   100,   100,   100,   100,   100,   100,
     100,   100,   100,   100,   100,   100,   100,   101,   102,   103,
     104,   104,   105,   105,   105,   105,   105,   105,   105,   106,
     107,   108,   109,   110,   111,   112,   112,   113,   114,   115,
     115,   116,   116,   116,   117,   117,   118,   118,   118,   118,
     118,   119,   119,   119,   120,   121,   121,   122,   122,   122,
     122,   122,   122,   122,   123,   123,   124,   124,   125,   125,
     126,   126,   127,   127,   128,   129,   129,   130,   131,   132,
     132,   132,   132,   133,   133,   134,   134,   135,   135,   136,
     136,   137,   137,   138,   139,   140,   140,   140,   140,   140,
     140,   140,   140,   140,   141,   141,   142,   143,   143,   144,
     144,   145,   146,   146,   147,   147,   148,   149,   149,   150,
     150,   151,   151,   152,   152,   152,   153,   153,   153,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   153,   153,   154,   154,   154,   154,   154,
     155,   155
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     2,     1,     1,     3,     8,     9,
       7,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     5,     2,     3,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     1,     1,
       6,     3,     3,     1,     3,     1,     1,     6,    11,     8,
       3,     1,     6,     3,     3,     3,     1,     3,     3,     3,
       3,     3,     3,     3,     3,     1,     3,     1,     6,     5,
       8,     7,     3,     1,     3,     3,     1,     2,     3,     4,
       8,     3,     7,     3,     1,     3,     1,     5,     1,     3,
       1,     5,     1,     3,     1,     1,     2,     2,     5,     7,
       6,     9,     5,     1,     2,     1,     6,     3,     4,     3,
       4,     3,     2,     1,     3,     3,     3,     2,     1,     4,
       4,     3,     1,     1,     2,     2,     1,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       5,     1,     3,     6,     6,     1,     1,     4,     6,     3,
       3,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     2,     6,     5,     0,     0,     1,
       4,     3,     7,     0,    56,     0,     0,     0,    53,     0,
       0,   176,   175,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   171,     0,   136,     0,     0,
       0,   176,     0,    60,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    22,    23,    26,
      13,    25,    14,    15,    16,    17,    18,    19,    20,    21,
      24,     0,     0,    87,   144,   142,   143,   141,   146,   140,
     137,   138,     0,   181,     0,   139,   145,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    51,    52,   181,     0,     0,     0,
      86,     0,     0,     0,     0,     0,     0,     0,   113,    28,
       0,     0,   105,     0,     0,     0,   115,     0,     0,     0,
      75,     0,    66,     0,     0,     0,    49,     0,     0,    48,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
       0,     0,     0,     0,    61,     0,    55,    10,    11,     0,
       0,    94,    98,     0,   172,     0,   179,     0,   169,   168,
     164,   165,   161,   160,   162,   163,   166,   167,   157,   155,
     156,     0,     0,   154,   147,   153,   158,   159,   151,   152,
     148,   149,   150,     0,     0,     0,     0,    84,     0,     0,
     103,   128,     0,     0,     0,     0,     0,   106,   107,     0,
       0,     0,     0,    29,   114,     0,     0,    67,     0,    64,
       0,    44,     0,    42,     0,     0,    68,    69,     0,     0,
      70,     0,    71,    43,    40,    46,    45,    72,    73,    41,
       0,     0,    47,     0,     0,    88,     0,     0,   177,   181,
       0,   180,     0,     0,     8,     0,    91,     0,    96,     0,
       0,   100,   102,    85,     0,     0,     0,     0,   132,   133,
     126,   127,     0,     0,     0,     0,     0,   119,     0,   117,
       0,     0,     0,     0,    74,    65,     0,     0,     0,    31,
      33,    32,    37,    34,    35,    36,    38,   104,     0,     0,
      76,     0,    63,    54,     0,    93,     0,     0,     0,   170,
      57,     9,    89,     0,     0,     0,     0,   135,   134,   130,
       0,   129,     0,     0,     0,     0,   120,   118,     0,   121,
       0,     0,     0,     0,    27,    30,     0,     0,     0,     0,
     178,   173,   174,     0,     0,    95,     0,     0,    99,   131,
       0,     0,   123,     0,   108,     0,   112,     0,     0,     0,
       0,    39,     0,    79,     0,    97,     0,    59,     0,     0,
       0,   110,   122,     0,     0,     0,   116,     0,     0,    83,
      50,    78,    62,     0,   101,    92,     0,   125,   124,   109,
       0,     0,    81,     0,     0,    90,     0,    80,    82,    58,
     111
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     3,     4,     5,     6,    65,    66,    67,    68,    69,
     308,   309,   310,    70,    71,    72,    73,    74,    75,    76,
     158,   159,    17,   175,    18,   176,    77,   151,    78,   149,
     164,   165,   150,   398,    79,   129,    35,    83,   130,   180,
     277,   278,   280,   281,    80,   316,   317,   145,   146,   140,
     141,   231,   371,   372,   142,   222,   132,   287,   288,   182,
      37,    94
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -192
static const yytype_int16 yypact[] =
{
      16,    33,    35,    67,    16,  -192,  -192,    76,    62,  -192,
    -192,  -192,  -192,     3,    37,    49,   605,   -31,  -192,    48,
     789,    -4,  -192,   605,   605,   605,   605,   605,   605,   605,
     605,   605,   605,   605,   605,  -192,  1501,  -192,    52,    42,
      40,    69,   605,  -192,    18,    50,   372,    97,    53,    48,
      40,    40,    41,    53,    53,    54,    53,    40,    40,    40,
      40,    53,    53,    40,   167,   676,  -192,  -192,  -192,  -192,
    -192,  -192,  -192,  -192,  -192,  -192,  -192,  -192,  -192,  -192,
    -192,   376,   605,  -192,  -192,  -192,  -192,  -192,  -192,  -192,
    -192,  -192,  1253,  1221,    63,  -192,  -192,   605,   605,   605,
     605,   605,   605,   605,   605,   605,   605,   605,   605,   605,
     605,   605,   605,   605,   605,   605,   605,   605,   605,   605,
     605,   605,   789,    92,  -192,  -192,  1675,   113,   118,   -18,
    -192,   123,   372,   372,   130,   132,   140,   142,  -192,  -192,
     145,   152,  -192,   -13,    48,     8,  -192,   183,   605,   -14,
    -192,   116,  -192,    94,    87,   101,  -192,   605,   166,  -192,
     133,   136,   189,   605,   154,  -192,   195,   127,   149,   160,
     181,   203,   205,   184,   -38,   210,  -192,  -192,  -192,   215,
     -47,  -192,  1675,  1008,  -192,   605,  -192,   605,  1731,  1704,
      93,    93,   200,   200,   459,   459,   459,   459,  1785,  1812,
    1785,  1530,  1038,  1758,  1785,  1812,    93,    93,   163,   163,
    -192,  -192,  -192,   714,   789,   103,   403,  -192,    18,   117,
    -192,  -192,   285,   605,   605,    48,   605,  -192,  -192,   515,
     551,   191,   242,  -192,  -192,   605,   790,  -192,    53,  -192,
      48,  -192,    40,  -192,  1559,   224,  -192,  -192,   605,   821,
    -192,    54,  -192,  -192,  -192,  -192,  -192,  -192,  -192,  -192,
     605,    48,  -192,   167,   231,  -192,   376,   605,  -192,  1188,
      64,  1675,   605,   299,  -192,   752,  -192,   -29,  -192,   301,
      57,  -192,  1675,  -192,   302,   304,   240,    -2,  -192,  -192,
    -192,  -192,  1284,  1315,   223,  1346,   605,  1675,   605,  1675,
     222,   605,   852,   605,  -192,  -192,   605,    40,   477,  -192,
    -192,  -192,  -192,  -192,  -192,  -192,  -192,  -192,   883,   605,
    -192,  1588,  -192,  -192,   605,  -192,  1068,   243,   245,  1675,
     -27,  -192,  -192,   376,   249,   311,   403,  -192,  -192,  -192,
      45,  -192,   579,   372,   605,   372,  1675,  1675,    48,  1675,
     605,   914,  1098,   187,  -192,  -192,   605,  1377,   605,  1408,
    -192,  -192,  -192,   605,   605,  -192,   605,   253,  -192,  -192,
     265,   482,  -192,  1617,   309,   976,  -192,   237,   945,   605,
     324,  -192,  1439,  -192,  1128,  -192,  1646,  1675,  1470,   349,
     372,  -192,  -192,   372,   372,    48,  -192,   605,    59,  1675,
    -192,  -192,  -192,   605,  -192,  -192,    60,  -192,  -192,  -192,
     259,    95,  -192,   605,  1158,  -192,   372,  -192,  1675,  -192,
    -192
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -192,  -192,  -192,   326,   327,  -103,   -63,  -192,  -192,  -192,
    -192,    24,  -192,  -191,  -190,  -192,  -192,  -192,  -184,  -183,
    -192,  -192,   -26,  -192,   -36,    70,  -182,  -192,  -192,   141,
    -192,    84,    98,   -60,  -192,  -192,  -192,  -192,   120,  -192,
     -46,   -80,  -192,     9,  -192,  -192,   -24,  -192,   209,   -44,
    -192,  -192,  -192,   -12,  -192,  -192,  -192,  -192,    17,   -16,
       1,   173
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint16 yytable[] =
{
      36,   181,   178,   124,   125,   152,    14,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    95,    96,   213,
      43,   127,   139,   265,   154,   155,   126,   233,   340,   229,
       1,   167,   168,   169,   170,   260,     7,   173,     8,    38,
       2,   332,   266,    14,   156,    14,   363,   143,    41,    22,
     153,    41,    22,   261,   311,   312,   147,   162,    39,    40,
     333,   313,   314,   315,   364,    81,   183,     9,   341,    82,
     217,   218,   284,    15,   237,   238,    16,   144,   230,   285,
      12,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   211,   212,    21,    22,   220,   221,
     128,   275,   123,    16,   157,    16,    42,   311,   312,    42,
      41,    22,   148,   163,   313,   314,   315,   335,    19,   412,
     415,    13,   236,   143,   143,   186,   328,    20,   101,   102,
     122,   244,    82,   131,   284,   232,   336,   249,   413,   333,
     178,   285,   187,   187,    23,    24,    25,   117,   118,   119,
     120,   121,    26,    27,    28,   417,   144,    29,    30,   269,
     174,   271,    31,   276,    32,   241,   242,    40,    33,    34,
     214,   294,   215,   286,   413,   230,   325,   216,    42,   243,
     242,    40,   219,   179,   160,   161,   305,   166,   291,   223,
     282,   224,   171,   172,   239,   240,   124,   292,   293,   225,
     295,   226,   178,   297,   299,   253,   242,    40,   264,   302,
     289,   246,   238,   143,   247,   238,   153,    41,    22,   119,
     120,   121,   318,   227,    49,   133,   134,   254,   242,    40,
     228,   153,   250,   251,   321,   135,   143,   307,   255,   242,
      40,   326,   235,   365,   245,   136,   329,    58,   248,    59,
      60,   300,   322,    63,   117,   118,   119,   120,   121,   256,
     242,    40,   259,   242,    40,   381,   242,    40,   137,    64,
     346,   353,   347,   252,   238,   349,   301,   351,    41,    22,
     352,   257,   238,   258,   238,    42,   133,   134,   262,   263,
     324,   290,   330,   357,   334,   337,   135,   338,   359,   143,
     339,   344,   138,   348,   367,   361,   136,   362,   366,   374,
     282,   376,   389,   390,   394,   396,   373,   400,   375,   416,
      10,    11,   355,   323,   378,   320,   304,   411,   283,   137,
     382,   289,   384,   406,   143,   368,   143,   386,   387,   377,
     388,   410,    21,    22,   234,   373,    42,   369,   270,   392,
       0,     0,     0,   399,     0,     0,   407,     0,     0,   408,
     409,     0,     0,   138,     0,    41,    22,     0,     0,    21,
      22,   399,     0,   133,   134,     0,     0,   414,     0,     0,
       0,   143,   420,   135,   143,   143,   153,   418,     0,     0,
      23,    24,    25,   136,     0,     0,    21,    22,    26,    27,
      28,     0,     0,    29,    30,     0,     0,   143,    31,   405,
      32,     0,     0,     0,    33,    34,   137,    23,    24,    25,
       0,     0,     0,     0,     0,    26,    27,    28,     0,   179,
      29,    30,     0,    42,     0,    31,     0,    32,     0,     0,
       0,    33,    34,     0,    23,    24,    25,     0,     0,     0,
     138,     0,    26,    27,    28,     0,   179,    29,    30,     0,
       0,     0,    31,     0,    32,     0,     0,     0,    33,    34,
      41,    22,     0,     0,     0,    21,    22,    49,   133,   134,
       0,     0,     0,   279,     0,   370,     0,   354,   135,   391,
     307,    99,   100,     0,   101,   102,     0,     0,   136,     0,
      58,     0,    59,    60,     0,     0,    63,     0,    21,    22,
     296,   115,   116,   117,   118,   119,   120,   121,     0,     0,
       0,   137,    64,    23,    24,    25,     0,     0,     0,     0,
       0,    26,    27,    28,     0,     0,    29,    30,    42,     0,
       0,    31,     0,    32,    21,    22,   298,    33,    34,     0,
       0,     0,     0,     0,     0,   138,    23,    24,    25,     0,
       0,     0,     0,     0,    26,    27,    28,     0,     0,    29,
      30,     0,    21,    22,    31,     0,    32,     0,     0,     0,
      33,    34,   370,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    23,    24,    25,     0,     0,     0,    21,    22,
      26,    27,    28,     0,     0,    29,    30,     0,     0,     0,
      31,     0,    32,     0,     0,     0,    33,    34,     0,     0,
      23,    24,    25,     0,     0,     0,     0,     0,    26,    27,
      28,     0,     0,    29,    30,     0,     0,     0,    31,     0,
      32,     0,     0,     0,    33,    34,    23,    24,    25,     0,
       0,     0,     0,     0,    26,    27,    28,     0,     0,    29,
      30,     0,     0,     0,    31,     0,    32,     0,     0,    44,
      33,    34,    45,    46,    47,    48,    49,     0,     0,     0,
       0,     0,     0,     0,   177,     0,     0,     0,    50,    51,
       0,    52,    53,     0,    54,    55,    56,     0,    57,    58,
       0,    59,    60,    61,    62,    63,     0,    44,     0,     0,
      45,    46,    47,    48,    49,     0,     0,     0,     0,     0,
       0,    64,   274,     0,     0,     0,    50,    51,     0,    52,
      53,     0,    54,    55,    56,     0,    57,    58,     0,    59,
      60,    61,    62,    63,     0,    44,     0,     0,    45,    46,
      47,    48,    49,     0,     0,     0,     0,     0,     0,    64,
     331,     0,     0,     0,    50,    51,     0,    52,    53,     0,
      54,    55,    56,     0,    57,    58,     0,    59,    60,    61,
      62,    63,    44,     0,     0,    45,    46,    47,    48,    49,
       0,     0,     0,     0,     0,     0,     0,    64,     0,     0,
       0,    50,    51,     0,    52,    53,     0,    54,    55,    56,
       0,    57,    58,     0,    59,    60,    61,    62,    63,     0,
      97,    98,    99,   100,     0,   101,   102,   103,   104,   105,
     106,   107,   108,   109,    64,     0,     0,   110,     0,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,     0,
       0,    97,    98,    99,   100,     0,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,     0,     0,   110,   303,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
       0,     0,    97,    98,    99,   100,     0,   101,   102,   103,
     104,   105,   106,   107,   108,   109,     0,     0,     0,   110,
     319,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,     0,     0,    97,    98,    99,   100,     0,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,     0,     0,
     110,   350,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,     0,     0,    97,    98,    99,   100,     0,   101,
     102,   103,   104,   105,   106,   107,   108,   109,     0,     0,
       0,   110,   356,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,     0,     0,    97,    98,    99,   100,     0,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,     0,   110,   379,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,     0,     0,    97,    98,    99,   100,
       0,   101,   102,   103,   104,   105,   106,   107,   108,   109,
       0,     0,     0,   110,   397,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,     0,     0,     0,    97,    98,
      99,   100,     0,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,   395,   110,   267,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,     0,    97,    98,
      99,   100,   268,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,     0,   110,     0,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,     0,    97,    98,
      99,   100,   273,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,     0,   110,     0,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,     0,    97,    98,
      99,   100,   360,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,     0,   110,     0,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,     0,    97,    98,
      99,   100,   380,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,     0,   110,     0,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,     0,    97,    98,
      99,   100,   402,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,     0,   110,     0,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,     0,    97,    98,
      99,   100,   419,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,     0,   110,     0,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,     0,     0,     0,
     327,    97,    98,    99,   100,     0,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,     0,     0,   110,     0,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
       0,     0,   185,    97,    98,    99,   100,     0,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,     0,     0,
     110,     0,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,     0,   184,    97,    98,    99,   100,     0,   101,
     102,   103,   104,   105,   106,   107,   108,   109,     0,     0,
       0,   110,     0,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,     0,   342,    97,    98,    99,   100,     0,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,     0,   110,     0,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,     0,   343,    97,    98,    99,   100,
       0,   101,   102,   103,   104,   105,   106,   107,   108,   109,
       0,     0,     0,   110,     0,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,     0,   345,    97,    98,    99,
     100,     0,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,     0,   110,     0,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,     0,   383,    97,    98,
      99,   100,     0,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,     0,   110,     0,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,     0,   385,    97,
      98,    99,   100,     0,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,     0,   110,     0,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,     0,   401,
      97,    98,    99,   100,     0,   101,   102,   103,   104,   105,
     106,   107,   108,   109,     0,     0,     0,   110,     0,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,     0,
     404,    97,    98,    99,   100,     0,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,     0,     0,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
      97,    98,    99,   100,     0,   101,   102,   103,   104,   105,
     106,   107,   108,   109,     0,     0,     0,   110,   272,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,    97,
      98,    99,   100,     0,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,     0,   110,   306,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,    97,    98,
      99,   100,     0,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,     0,   110,   358,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,    97,    98,    99,
     100,     0,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,     0,   110,   393,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,    97,    98,    99,   100,
       0,   101,   102,   103,   104,   105,   106,   107,   108,   109,
       0,     0,     0,   110,   403,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,    97,    98,    99,   100,     0,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,     0,   110,     0,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,    97,     0,    99,   100,     0,   101,
     102,   103,   104,   105,   106,   107,   108,   109,     0,     0,
       0,     0,     0,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,    99,   100,     0,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,     0,     0,     0,     0,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
      99,   100,     0,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,     0,     0,     0,     0,   113,   114,
     115,   116,   117,   118,   119,   120,   121,    99,   100,     0,
     101,   102,   103,   104,   105,   106,     0,   108,     0,     0,
       0,     0,     0,     0,     0,     0,   114,   115,   116,   117,
     118,   119,   120,   121,    99,   100,     0,   101,   102,   103,
     104,   105,   106,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   115,   116,   117,   118,   119,   120,
     121
};

static const yytype_int16 yycheck[] =
{
      16,    81,    65,    39,    40,    49,     3,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,   122,
      19,     3,    46,    70,    50,    51,    42,    19,    30,    42,
      14,    57,    58,    59,    60,    73,     3,    63,     3,    70,
      24,    70,    89,     3,     3,     3,    73,    46,     3,     4,
      49,     3,     4,    91,   245,   245,     3,     3,    89,    90,
      89,   245,   245,   245,    91,    69,    82,     0,    70,    73,
      88,    89,    27,    70,    88,    89,    73,    69,    91,    34,
       4,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,     3,     4,   132,   133,
      92,   214,    70,    73,    73,    73,    71,   308,   308,    71,
       3,     4,    69,    69,   308,   308,   308,    70,    91,    70,
      70,    69,   148,   132,   133,    72,    72,    88,    45,    46,
      88,   157,    73,    93,    27,   144,    89,   163,    89,    89,
     213,    34,    89,    89,    51,    52,    53,    64,    65,    66,
      67,    68,    59,    60,    61,    70,    69,    64,    65,   185,
       3,   187,    69,    70,    71,    88,    89,    90,    75,    76,
      88,   225,    69,    66,    89,    91,   266,    69,    71,    88,
      89,    90,    69,    90,    53,    54,   240,    56,   222,    69,
     216,    69,    61,    62,    88,    89,   242,   223,   224,    69,
     226,    69,   275,   229,   230,    88,    89,    90,     3,   235,
     219,    88,    89,   222,    88,    89,   225,     3,     4,    66,
      67,    68,   248,    88,    10,    11,    12,    88,    89,    90,
      88,   240,    88,    89,   260,    21,   245,    23,    88,    89,
      90,   267,    69,   333,    88,    31,   272,    33,    69,    35,
      36,    70,   261,    39,    64,    65,    66,    67,    68,    88,
      89,    90,    88,    89,    90,    88,    89,    90,    54,    55,
     296,   307,   298,    88,    89,   301,    44,   303,     3,     4,
     306,    88,    89,    88,    89,    71,    11,    12,    88,    89,
      69,    16,     3,   319,     3,     3,    21,     3,   324,   308,
      70,    88,    88,    91,     3,    72,    31,    72,    69,   343,
     336,   345,    69,    58,    15,    88,   342,     3,   344,    70,
       4,     4,   308,   263,   350,   251,   238,   397,   218,    54,
     356,   340,   358,   389,   343,   336,   345,   363,   364,   348,
     366,   395,     3,     4,   145,   371,    71,   340,   185,   371,
      -1,    -1,    -1,   379,    -1,    -1,   390,    -1,    -1,   393,
     394,    -1,    -1,    88,    -1,     3,     4,    -1,    -1,     3,
       4,   397,    -1,    11,    12,    -1,    -1,   403,    -1,    -1,
      -1,   390,   416,    21,   393,   394,   395,   413,    -1,    -1,
      51,    52,    53,    31,    -1,    -1,     3,     4,    59,    60,
      61,    -1,    -1,    64,    65,    -1,    -1,   416,    69,    70,
      71,    -1,    -1,    -1,    75,    76,    54,    51,    52,    53,
      -1,    -1,    -1,    -1,    -1,    59,    60,    61,    -1,    90,
      64,    65,    -1,    71,    -1,    69,    -1,    71,    -1,    -1,
      -1,    75,    76,    -1,    51,    52,    53,    -1,    -1,    -1,
      88,    -1,    59,    60,    61,    -1,    90,    64,    65,    -1,
      -1,    -1,    69,    -1,    71,    -1,    -1,    -1,    75,    76,
       3,     4,    -1,    -1,    -1,     3,     4,    10,    11,    12,
      -1,    -1,    -1,    90,    -1,    13,    -1,    20,    21,    17,
      23,    42,    43,    -1,    45,    46,    -1,    -1,    31,    -1,
      33,    -1,    35,    36,    -1,    -1,    39,    -1,     3,     4,
       5,    62,    63,    64,    65,    66,    67,    68,    -1,    -1,
      -1,    54,    55,    51,    52,    53,    -1,    -1,    -1,    -1,
      -1,    59,    60,    61,    -1,    -1,    64,    65,    71,    -1,
      -1,    69,    -1,    71,     3,     4,     5,    75,    76,    -1,
      -1,    -1,    -1,    -1,    -1,    88,    51,    52,    53,    -1,
      -1,    -1,    -1,    -1,    59,    60,    61,    -1,    -1,    64,
      65,    -1,     3,     4,    69,    -1,    71,    -1,    -1,    -1,
      75,    76,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    51,    52,    53,    -1,    -1,    -1,     3,     4,
      59,    60,    61,    -1,    -1,    64,    65,    -1,    -1,    -1,
      69,    -1,    71,    -1,    -1,    -1,    75,    76,    -1,    -1,
      51,    52,    53,    -1,    -1,    -1,    -1,    -1,    59,    60,
      61,    -1,    -1,    64,    65,    -1,    -1,    -1,    69,    -1,
      71,    -1,    -1,    -1,    75,    76,    51,    52,    53,    -1,
      -1,    -1,    -1,    -1,    59,    60,    61,    -1,    -1,    64,
      65,    -1,    -1,    -1,    69,    -1,    71,    -1,    -1,     3,
      75,    76,     6,     7,     8,     9,    10,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    18,    -1,    -1,    -1,    22,    23,
      -1,    25,    26,    -1,    28,    29,    30,    -1,    32,    33,
      -1,    35,    36,    37,    38,    39,    -1,     3,    -1,    -1,
       6,     7,     8,     9,    10,    -1,    -1,    -1,    -1,    -1,
      -1,    55,    18,    -1,    -1,    -1,    22,    23,    -1,    25,
      26,    -1,    28,    29,    30,    -1,    32,    33,    -1,    35,
      36,    37,    38,    39,    -1,     3,    -1,    -1,     6,     7,
       8,     9,    10,    -1,    -1,    -1,    -1,    -1,    -1,    55,
      18,    -1,    -1,    -1,    22,    23,    -1,    25,    26,    -1,
      28,    29,    30,    -1,    32,    33,    -1,    35,    36,    37,
      38,    39,     3,    -1,    -1,     6,     7,     8,     9,    10,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,    -1,
      -1,    22,    23,    -1,    25,    26,    -1,    28,    29,    30,
      -1,    32,    33,    -1,    35,    36,    37,    38,    39,    -1,
      40,    41,    42,    43,    -1,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    55,    -1,    -1,    57,    -1,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      -1,    40,    41,    42,    43,    -1,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    -1,    -1,    57,    89,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    -1,    40,    41,    42,    43,    -1,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    -1,    -1,    57,
      89,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    -1,    40,    41,    42,    43,    -1,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    -1,    -1,    -1,
      57,    89,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    -1,    40,    41,    42,    43,    -1,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    -1,
      -1,    57,    89,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    -1,    40,    41,    42,    43,    -1,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      -1,    -1,    57,    89,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    -1,    40,    41,    42,    43,
      -1,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    -1,    -1,    57,    89,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    -1,    -1,    40,    41,
      42,    43,    -1,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    -1,    88,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    40,    41,
      42,    43,    74,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    -1,    -1,    57,    -1,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    40,    41,
      42,    43,    74,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    -1,    -1,    57,    -1,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    40,    41,
      42,    43,    74,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    -1,    -1,    57,    -1,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    40,    41,
      42,    43,    74,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    -1,    -1,    57,    -1,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    40,    41,
      42,    43,    74,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    -1,    -1,    57,    -1,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    40,    41,
      42,    43,    74,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    -1,    -1,    57,    -1,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    -1,    -1,
      72,    40,    41,    42,    43,    -1,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    -1,    -1,    57,    -1,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    -1,    71,    40,    41,    42,    43,    -1,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    -1,    -1,    -1,
      57,    -1,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    40,    41,    42,    43,    -1,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    -1,
      -1,    57,    -1,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    40,    41,    42,    43,    -1,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      -1,    -1,    57,    -1,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    40,    41,    42,    43,
      -1,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    -1,    -1,    57,    -1,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    40,    41,    42,
      43,    -1,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    -1,    -1,    -1,    57,    -1,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    40,    41,
      42,    43,    -1,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    -1,    -1,    57,    -1,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    40,
      41,    42,    43,    -1,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,    -1,    -1,    57,    -1,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      40,    41,    42,    43,    -1,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    -1,    -1,    57,    -1,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    40,    41,    42,    43,    -1,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    -1,    -1,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      40,    41,    42,    43,    -1,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    -1,    -1,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    40,
      41,    42,    43,    -1,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,    -1,    -1,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    40,    41,
      42,    43,    -1,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    -1,    -1,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    40,    41,    42,
      43,    -1,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    -1,    -1,    -1,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    40,    41,    42,    43,
      -1,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    -1,    -1,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    40,    41,    42,    43,    -1,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      -1,    -1,    57,    -1,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    40,    -1,    42,    43,    -1,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    -1,
      -1,    -1,    -1,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    42,    43,    -1,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    -1,    -1,    -1,    -1,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      42,    43,    -1,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    -1,    -1,    -1,    -1,    -1,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    42,    43,    -1,
      45,    46,    47,    48,    49,    50,    -1,    52,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    61,    62,    63,    64,
      65,    66,    67,    68,    42,    43,    -1,    45,    46,    47,
      48,    49,    50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    62,    63,    64,    65,    66,    67,
      68
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    14,    24,    95,    96,    97,    98,     3,     3,     0,
      97,    98,     4,    69,     3,    70,    73,   116,   118,    91,
      88,     3,     4,    51,    52,    53,    59,    60,    61,    64,
      65,    69,    71,    75,    76,   130,   153,   154,    70,    89,
      90,     3,    71,   154,     3,     6,     7,     8,     9,    10,
      22,    23,    25,    26,    28,    29,    30,    32,    33,    35,
      36,    37,    38,    39,    55,    99,   100,   101,   102,   103,
     107,   108,   109,   110,   111,   112,   113,   120,   122,   128,
     138,    69,    73,   131,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   153,   155,   153,   153,    40,    41,    42,
      43,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    88,    70,   118,   118,   153,     3,    92,   129,
     132,    93,   150,    11,    12,    21,    31,    54,    88,   140,
     143,   144,   148,   154,    69,   141,   142,     3,    69,   123,
     126,   121,   143,   154,   116,   116,     3,    73,   114,   115,
     123,   123,     3,    69,   124,   125,   123,   116,   116,   116,
     116,   123,   123,   116,     3,   117,   119,    18,   100,    90,
     133,   135,   153,   153,    70,    71,    72,    89,   153,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   153,    99,    88,    69,    69,    88,    89,    69,
     140,   140,   149,    69,    69,    69,    69,    88,    88,    42,
      91,   145,   154,    19,   142,    69,   153,    88,    89,    88,
      89,    88,    89,    88,   153,    88,    88,    88,    69,   153,
      88,    89,    88,    88,    88,    88,    88,    88,    88,    88,
      73,    91,    88,    89,     3,    70,    89,    58,    74,   153,
     155,   153,    58,    74,    18,    99,    70,   134,   135,    90,
     136,   137,   153,   132,    27,    34,    66,   151,   152,   154,
      16,   140,   153,   153,   143,   153,     5,   153,     5,   153,
      70,    44,   153,    89,   126,   143,    58,    23,   104,   105,
     106,   107,   108,   112,   113,   120,   139,   140,   153,    89,
     125,   153,   154,   119,    69,   135,   153,    72,    72,   153,
       3,    18,    70,    89,     3,    70,    89,     3,     3,    70,
      30,    70,    70,    70,    88,    70,   153,   153,    91,   153,
      89,   153,   153,   116,    20,   105,    89,   153,    58,   153,
      74,    72,    72,    73,    91,   135,    69,     3,   137,   152,
      13,   146,   147,   153,   140,   153,   140,   154,   153,    89,
      74,    88,   153,    70,   153,    70,   153,   153,   153,    69,
      58,    17,   147,    58,    15,    88,    88,    89,   127,   153,
       3,    70,    74,    58,    70,    70,   134,   140,   140,   140,
     143,   127,    70,    89,   153,    70,    70,    70,   153,    74,
     140
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 126 "SRC/verilog_bison.y"
    {next_parsed_verilog_file((yyvsp[(1) - (1)].node));;}
    break;

  case 3:
#line 129 "SRC/verilog_bison.y"
    {
											if ((yyvsp[(1) - (2)].node) != NULL)
											{
												(yyval.node) = newList_entry((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node));
											}
											else
											{
												(yyval.node) = newList(FILE_ITEMS, (yyvsp[(2) - (2)].node));
											}
										;}
    break;

  case 4:
#line 139 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(1) - (2)].node);;}
    break;

  case 5:
#line 140 "SRC/verilog_bison.y"
    {(yyval.node) = newList(FILE_ITEMS, (yyvsp[(1) - (1)].node));;}
    break;

  case 6:
#line 141 "SRC/verilog_bison.y"
    {(yyval.node) = NULL;;}
    break;

  case 7:
#line 144 "SRC/verilog_bison.y"
    {(yyval.node) = NULL; newConstant((yyvsp[(2) - (3)].id_name), (yyvsp[(3) - (3)].num_value), yylineno);;}
    break;

  case 8:
#line 147 "SRC/verilog_bison.y"
    {(yyval.node) = newModule((yyvsp[(2) - (8)].id_name), (yyvsp[(4) - (8)].node), (yyvsp[(7) - (8)].node), yylineno);;}
    break;

  case 9:
#line 148 "SRC/verilog_bison.y"
    {(yyval.node) = newModule((yyvsp[(2) - (9)].id_name), (yyvsp[(4) - (9)].node), (yyvsp[(8) - (9)].node), yylineno);;}
    break;

  case 10:
#line 149 "SRC/verilog_bison.y"
    {(yyval.node) = newModule((yyvsp[(2) - (7)].id_name), NULL, (yyvsp[(6) - (7)].node), yylineno);;}
    break;

  case 11:
#line 152 "SRC/verilog_bison.y"
    {(yyval.node) = newList_entry((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node));;}
    break;

  case 12:
#line 153 "SRC/verilog_bison.y"
    {(yyval.node) = newList(MODULE_ITEMS, (yyvsp[(1) - (1)].node));;}
    break;

  case 13:
#line 156 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 14:
#line 157 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 15:
#line 158 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 16:
#line 159 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 17:
#line 160 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 18:
#line 161 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 19:
#line 162 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 20:
#line 163 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 21:
#line 164 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 22:
#line 165 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 23:
#line 166 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 24:
#line 167 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 25:
#line 168 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 26:
#line 169 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 27:
#line 173 "SRC/verilog_bison.y"
    {(yyval.node) = newFunction((yyvsp[(2) - (5)].node), (yyvsp[(4) - (5)].node), yylineno); ;}
    break;

  case 28:
#line 177 "SRC/verilog_bison.y"
    {(yyval.node) = newInitial((yyvsp[(2) - (2)].node), yylineno); ;}
    break;

  case 29:
#line 181 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(2) - (3)].node);;}
    break;

  case 30:
#line 184 "SRC/verilog_bison.y"
    {(yyval.node) = newList_entry((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node));;}
    break;

  case 31:
#line 185 "SRC/verilog_bison.y"
    {(yyval.node) = newList(FUNCTION_ITEMS, (yyvsp[(1) - (1)].node));;}
    break;

  case 32:
#line 188 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 33:
#line 189 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 34:
#line 190 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 35:
#line 191 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 36:
#line 192 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 37:
#line 193 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 38:
#line 194 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 39:
#line 197 "SRC/verilog_bison.y"
    {(yyval.node) = markAndProcessSymbolListWith(FUNCTION, INPUT, (yyvsp[(2) - (3)].node));;}
    break;

  case 40:
#line 200 "SRC/verilog_bison.y"
    {(yyval.node) = markAndProcessSymbolListWith(MODULE,PARAMETER, (yyvsp[(2) - (3)].node));;}
    break;

  case 41:
#line 203 "SRC/verilog_bison.y"
    {(yyval.node) = newDefparam(MODULE_PARAMETER_LIST, (yyvsp[(2) - (3)].node), yylineno);;}
    break;

  case 42:
#line 206 "SRC/verilog_bison.y"
    {(yyval.node) = markAndProcessSymbolListWith(MODULE,INPUT, (yyvsp[(2) - (3)].node));;}
    break;

  case 43:
#line 209 "SRC/verilog_bison.y"
    {(yyval.node) = markAndProcessSymbolListWith(MODULE,OUTPUT, (yyvsp[(2) - (3)].node));;}
    break;

  case 44:
#line 212 "SRC/verilog_bison.y"
    {(yyval.node) = markAndProcessSymbolListWith(MODULE,INOUT, (yyvsp[(2) - (3)].node));;}
    break;

  case 45:
#line 215 "SRC/verilog_bison.y"
    {(yyval.node) = markAndProcessSymbolListWith(MODULE, WIRE, (yyvsp[(2) - (3)].node));;}
    break;

  case 46:
#line 216 "SRC/verilog_bison.y"
    {(yyval.node) = markAndProcessSymbolListWith(MODULE, REG, (yyvsp[(2) - (3)].node));;}
    break;

  case 47:
#line 218 "SRC/verilog_bison.y"
    {(yyval.node) = markAndProcessSymbolListWith(MODULE,INTEGER, (yyvsp[(2) - (3)].node));;}
    break;

  case 48:
#line 220 "SRC/verilog_bison.y"
    {(yyval.node) = newList(VAR_DECLARE_LIST, (yyvsp[(1) - (1)].node));;}
    break;

  case 49:
#line 222 "SRC/verilog_bison.y"
    {(yyval.node) = newVarDeclare((yyvsp[(1) - (1)].id_name), NULL, NULL, NULL, NULL, NULL, yylineno);;}
    break;

  case 50:
#line 223 "SRC/verilog_bison.y"
    {(yyval.node) = newVarDeclare((yyvsp[(6) - (6)].id_name), (yyvsp[(2) - (6)].node), (yyvsp[(4) - (6)].node), NULL, NULL, NULL, yylineno);;}
    break;

  case 51:
#line 225 "SRC/verilog_bison.y"
    {(yyval.node) = newList_entry((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));;}
    break;

  case 52:
#line 226 "SRC/verilog_bison.y"
    {(yyval.node) = newList_entry((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));;}
    break;

  case 53:
#line 227 "SRC/verilog_bison.y"
    {(yyval.node) = newList(VAR_DECLARE_LIST, (yyvsp[(1) - (1)].node));;}
    break;

  case 54:
#line 230 "SRC/verilog_bison.y"
    {(yyval.node) = newList_entry((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));;}
    break;

  case 55:
#line 231 "SRC/verilog_bison.y"
    {(yyval.node) = newList(VAR_DECLARE_LIST, (yyvsp[(1) - (1)].node));;}
    break;

  case 56:
#line 233 "SRC/verilog_bison.y"
    {(yyval.node) = newVarDeclare((yyvsp[(1) - (1)].id_name), NULL, NULL, NULL, NULL, NULL, yylineno);;}
    break;

  case 57:
#line 234 "SRC/verilog_bison.y"
    {(yyval.node) = newVarDeclare((yyvsp[(6) - (6)].id_name), (yyvsp[(2) - (6)].node), (yyvsp[(4) - (6)].node), NULL, NULL, NULL, yylineno);;}
    break;

  case 58:
#line 235 "SRC/verilog_bison.y"
    {(yyval.node) = newVarDeclare((yyvsp[(6) - (11)].id_name), (yyvsp[(2) - (11)].node), (yyvsp[(4) - (11)].node), (yyvsp[(8) - (11)].node), (yyvsp[(10) - (11)].node), NULL, yylineno);;}
    break;

  case 59:
#line 237 "SRC/verilog_bison.y"
    {(yyval.node) = newVarDeclare((yyvsp[(6) - (8)].id_name), (yyvsp[(2) - (8)].node), (yyvsp[(4) - (8)].node), NULL, NULL, (yyvsp[(8) - (8)].node), yylineno);;}
    break;

  case 60:
#line 239 "SRC/verilog_bison.y"
    {(yyval.node) = newVarDeclare((yyvsp[(1) - (3)].id_name), NULL, NULL, NULL, NULL, (yyvsp[(3) - (3)].node), yylineno);;}
    break;

  case 61:
#line 242 "SRC/verilog_bison.y"
    {(yyval.node) = newIntegerTypeVarDeclare((yyvsp[(1) - (1)].id_name), NULL, NULL, NULL, NULL, NULL, yylineno);;}
    break;

  case 62:
#line 244 "SRC/verilog_bison.y"
    {(yyval.node) = newIntegerTypeVarDeclare((yyvsp[(1) - (6)].id_name), NULL, NULL, (yyvsp[(3) - (6)].node), (yyvsp[(5) - (6)].node), NULL, yylineno);;}
    break;

  case 63:
#line 247 "SRC/verilog_bison.y"
    {(yyval.node) = newIntegerTypeVarDeclare((yyvsp[(1) - (3)].id_name), NULL, NULL, NULL, NULL, (yyvsp[(3) - (3)].node), yylineno);;}
    break;

  case 64:
#line 250 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(2) - (3)].node);;}
    break;

  case 65:
#line 252 "SRC/verilog_bison.y"
    {(yyval.node) = newList_entry((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));;}
    break;

  case 66:
#line 253 "SRC/verilog_bison.y"
    {(yyval.node) = newList(ASSIGN, (yyvsp[(1) - (1)].node));;}
    break;

  case 67:
#line 257 "SRC/verilog_bison.y"
    {(yyval.node) = newGate(BITWISE_AND, (yyvsp[(2) - (3)].node), yylineno);;}
    break;

  case 68:
#line 258 "SRC/verilog_bison.y"
    {(yyval.node) = newGate(BITWISE_NAND, (yyvsp[(2) - (3)].node), yylineno);;}
    break;

  case 69:
#line 259 "SRC/verilog_bison.y"
    {(yyval.node) = newGate(BITWISE_NOR, (yyvsp[(2) - (3)].node), yylineno);;}
    break;

  case 70:
#line 260 "SRC/verilog_bison.y"
    {(yyval.node) = newGate(BITWISE_NOT, (yyvsp[(2) - (3)].node), yylineno);;}
    break;

  case 71:
#line 261 "SRC/verilog_bison.y"
    {(yyval.node) = newGate(BITWISE_OR, (yyvsp[(2) - (3)].node), yylineno);;}
    break;

  case 72:
#line 262 "SRC/verilog_bison.y"
    {(yyval.node) = newGate(BITWISE_XNOR, (yyvsp[(2) - (3)].node), yylineno);;}
    break;

  case 73:
#line 263 "SRC/verilog_bison.y"
    {(yyval.node) = newGate(BITWISE_XOR, (yyvsp[(2) - (3)].node), yylineno);;}
    break;

  case 74:
#line 267 "SRC/verilog_bison.y"
    {(yyval.node) = newList_entry((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));;}
    break;

  case 75:
#line 268 "SRC/verilog_bison.y"
    {(yyval.node) = newList(ONE_GATE_INSTANCE, (yyvsp[(1) - (1)].node));;}
    break;

  case 76:
#line 270 "SRC/verilog_bison.y"
    {(yyval.node) = newList_entry((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));;}
    break;

  case 77:
#line 271 "SRC/verilog_bison.y"
    {(yyval.node) = newList(ONE_GATE_INSTANCE, (yyvsp[(1) - (1)].node));;}
    break;

  case 78:
#line 273 "SRC/verilog_bison.y"
    {(yyval.node) = newGateInstance((yyvsp[(1) - (6)].id_name), (yyvsp[(3) - (6)].node), (yyvsp[(5) - (6)].node), NULL, yylineno);;}
    break;

  case 79:
#line 274 "SRC/verilog_bison.y"
    {(yyval.node) = newGateInstance(NULL, (yyvsp[(2) - (5)].node), (yyvsp[(4) - (5)].node), NULL, yylineno);;}
    break;

  case 80:
#line 278 "SRC/verilog_bison.y"
    {(yyval.node) = newMultipleInputsGateInstance((yyvsp[(1) - (8)].id_name), (yyvsp[(3) - (8)].node), (yyvsp[(5) - (8)].node), (yyvsp[(7) - (8)].node), yylineno);;}
    break;

  case 81:
#line 279 "SRC/verilog_bison.y"
    {(yyval.node) = newMultipleInputsGateInstance(NULL, (yyvsp[(2) - (7)].node), (yyvsp[(4) - (7)].node), (yyvsp[(6) - (7)].node), yylineno);;}
    break;

  case 82:
#line 284 "SRC/verilog_bison.y"
    {(yyval.node) = newList_entry((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));;}
    break;

  case 83:
#line 285 "SRC/verilog_bison.y"
    {(yyval.node) = newModuleConnection(NULL, (yyvsp[(1) - (1)].node), yylineno);;}
    break;

  case 84:
#line 289 "SRC/verilog_bison.y"
    {(yyval.node) = newModuleInstance((yyvsp[(1) - (3)].id_name), (yyvsp[(2) - (3)].node), yylineno);;}
    break;

  case 85:
#line 294 "SRC/verilog_bison.y"
    {(yyval.node) = newList_entry((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));;}
    break;

  case 86:
#line 295 "SRC/verilog_bison.y"
    {(yyval.node) = newList(ONE_MODULE_INSTANCE, (yyvsp[(1) - (1)].node));;}
    break;

  case 87:
#line 298 "SRC/verilog_bison.y"
    {(yyval.node) = newFunctionInstance((yyvsp[(1) - (2)].id_name), (yyvsp[(2) - (2)].node), yylineno);;}
    break;

  case 88:
#line 303 "SRC/verilog_bison.y"
    {(yyval.node) = newFunctionNamedInstance((yyvsp[(2) - (3)].node), NULL, yylineno);;}
    break;

  case 89:
#line 305 "SRC/verilog_bison.y"
    {(yyval.node) = newModuleNamedInstance((yyvsp[(1) - (4)].id_name), (yyvsp[(3) - (4)].node), NULL, yylineno);;}
    break;

  case 90:
#line 306 "SRC/verilog_bison.y"
    {(yyval.node) = newModuleNamedInstance((yyvsp[(5) - (8)].id_name), (yyvsp[(7) - (8)].node), (yyvsp[(3) - (8)].node), yylineno); ;}
    break;

  case 91:
#line 307 "SRC/verilog_bison.y"
    {(yyval.node) = newModuleNamedInstance((yyvsp[(1) - (3)].id_name), NULL, NULL, yylineno);;}
    break;

  case 92:
#line 308 "SRC/verilog_bison.y"
    {(yyval.node) = newModuleNamedInstance((yyvsp[(5) - (7)].id_name), NULL, (yyvsp[(3) - (7)].node), yylineno);;}
    break;

  case 93:
#line 310 "SRC/verilog_bison.y"
    {(yyval.node) = newList_entry((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));;}
    break;

  case 94:
#line 311 "SRC/verilog_bison.y"
    {(yyval.node) = newfunctionList(MODULE_CONNECT_LIST, (yyvsp[(1) - (1)].node));;}
    break;

  case 95:
#line 313 "SRC/verilog_bison.y"
    {(yyval.node) = newList_entry((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));;}
    break;

  case 96:
#line 314 "SRC/verilog_bison.y"
    {(yyval.node) = newList(MODULE_CONNECT_LIST, (yyvsp[(1) - (1)].node));;}
    break;

  case 97:
#line 317 "SRC/verilog_bison.y"
    {(yyval.node) = newModuleConnection((yyvsp[(2) - (5)].id_name), (yyvsp[(4) - (5)].node), yylineno);;}
    break;

  case 98:
#line 318 "SRC/verilog_bison.y"
    {(yyval.node) = newModuleConnection(NULL, (yyvsp[(1) - (1)].node), yylineno);;}
    break;

  case 99:
#line 321 "SRC/verilog_bison.y"
    {(yyval.node) = newList_entry((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));;}
    break;

  case 100:
#line 322 "SRC/verilog_bison.y"
    {(yyval.node) = newList(MODULE_PARAMETER_LIST, (yyvsp[(1) - (1)].node));;}
    break;

  case 101:
#line 325 "SRC/verilog_bison.y"
    {(yyval.node) = newModuleParameter((yyvsp[(2) - (5)].id_name), (yyvsp[(4) - (5)].node), yylineno);;}
    break;

  case 102:
#line 326 "SRC/verilog_bison.y"
    {(yyval.node) = newModuleParameter(NULL, (yyvsp[(1) - (1)].node), yylineno);;}
    break;

  case 103:
#line 330 "SRC/verilog_bison.y"
    {(yyval.node) = newAlways((yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].node), yylineno);;}
    break;

  case 104:
#line 333 "SRC/verilog_bison.y"
    {(yyval.node) = newAlways(NULL, (yyvsp[(1) - (1)].node), yylineno);;}
    break;

  case 105:
#line 334 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 106:
#line 335 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(1) - (2)].node);;}
    break;

  case 107:
#line 336 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(1) - (2)].node);;}
    break;

  case 108:
#line 337 "SRC/verilog_bison.y"
    {(yyval.node) = newIf((yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].node), NULL, yylineno);;}
    break;

  case 109:
#line 338 "SRC/verilog_bison.y"
    {(yyval.node) = newIf((yyvsp[(3) - (7)].node), (yyvsp[(5) - (7)].node), (yyvsp[(7) - (7)].node), yylineno);;}
    break;

  case 110:
#line 339 "SRC/verilog_bison.y"
    {(yyval.node) = newCase((yyvsp[(3) - (6)].node), (yyvsp[(5) - (6)].node), yylineno);;}
    break;

  case 111:
#line 340 "SRC/verilog_bison.y"
    {(yyval.node) = newFor((yyvsp[(3) - (9)].node), (yyvsp[(5) - (9)].node), (yyvsp[(7) - (9)].node), (yyvsp[(9) - (9)].node), yylineno);;}
    break;

  case 112:
#line 341 "SRC/verilog_bison.y"
    {(yyval.node) = newWhile((yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].node), yylineno);;}
    break;

  case 113:
#line 342 "SRC/verilog_bison.y"
    {(yyval.node) = NULL;;}
    break;

  case 114:
#line 345 "SRC/verilog_bison.y"
    {(yyval.node) = newList_entry((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node));;}
    break;

  case 115:
#line 346 "SRC/verilog_bison.y"
    {(yyval.node) = newList(SPECIFY_PAL_CONNECT_LIST, (yyvsp[(1) - (1)].node));;}
    break;

  case 116:
#line 349 "SRC/verilog_bison.y"
    {(yyval.node) = newParallelConnection((yyvsp[(2) - (6)].node), (yyvsp[(5) - (6)].node), yylineno);;}
    break;

  case 117:
#line 351 "SRC/verilog_bison.y"
    {(yyval.node) = newBlocking((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), yylineno);;}
    break;

  case 118:
#line 352 "SRC/verilog_bison.y"
    {(yyval.node) = newBlocking((yyvsp[(1) - (4)].node), (yyvsp[(4) - (4)].node), yylineno);;}
    break;

  case 119:
#line 355 "SRC/verilog_bison.y"
    {(yyval.node) = newNonBlocking((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), yylineno);;}
    break;

  case 120:
#line 356 "SRC/verilog_bison.y"
    {(yyval.node) = newNonBlocking((yyvsp[(1) - (4)].node), (yyvsp[(4) - (4)].node), yylineno);;}
    break;

  case 121:
#line 359 "SRC/verilog_bison.y"
    {(yyval.node) = NULL;;}
    break;

  case 122:
#line 360 "SRC/verilog_bison.y"
    {(yyval.node) = newList_entry((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node));;}
    break;

  case 123:
#line 361 "SRC/verilog_bison.y"
    {(yyval.node) = newList(CASE_LIST, (yyvsp[(1) - (1)].node));;}
    break;

  case 124:
#line 364 "SRC/verilog_bison.y"
    {(yyval.node) = newCaseItem((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), yylineno);;}
    break;

  case 125:
#line 365 "SRC/verilog_bison.y"
    {(yyval.node) = newDefaultCase((yyvsp[(3) - (3)].node), yylineno);;}
    break;

  case 126:
#line 368 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(2) - (3)].node);;}
    break;

  case 127:
#line 371 "SRC/verilog_bison.y"
    {(yyval.node) = newList_entry((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node));;}
    break;

  case 128:
#line 372 "SRC/verilog_bison.y"
    {(yyval.node) = newList(BLOCK, (yyvsp[(1) - (1)].node));;}
    break;

  case 129:
#line 375 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(3) - (4)].node);;}
    break;

  case 130:
#line 376 "SRC/verilog_bison.y"
    {(yyval.node) = NULL;;}
    break;

  case 131:
#line 380 "SRC/verilog_bison.y"
    {(yyval.node) = newList_entry((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));;}
    break;

  case 132:
#line 381 "SRC/verilog_bison.y"
    {(yyval.node) = newList(DELAY_CONTROL, (yyvsp[(1) - (1)].node));;}
    break;

  case 133:
#line 384 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 134:
#line 385 "SRC/verilog_bison.y"
    {(yyval.node) = newPosedgeSymbol((yyvsp[(2) - (2)].id_name), yylineno);;}
    break;

  case 135:
#line 386 "SRC/verilog_bison.y"
    {(yyval.node) = newNegedgeSymbol((yyvsp[(2) - (2)].id_name), yylineno);;}
    break;

  case 136:
#line 389 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 137:
#line 390 "SRC/verilog_bison.y"
    {(yyval.node) = newUnaryOperation(ADD, (yyvsp[(2) - (2)].node), yylineno);;}
    break;

  case 138:
#line 391 "SRC/verilog_bison.y"
    {(yyval.node) = newUnaryOperation(MINUS, (yyvsp[(2) - (2)].node), yylineno);;}
    break;

  case 139:
#line 392 "SRC/verilog_bison.y"
    {(yyval.node) = newUnaryOperation(BITWISE_NOT, (yyvsp[(2) - (2)].node), yylineno);;}
    break;

  case 140:
#line 393 "SRC/verilog_bison.y"
    {(yyval.node) = newUnaryOperation(BITWISE_AND, (yyvsp[(2) - (2)].node), yylineno);;}
    break;

  case 141:
#line 394 "SRC/verilog_bison.y"
    {(yyval.node) = newUnaryOperation(BITWISE_OR, (yyvsp[(2) - (2)].node), yylineno);;}
    break;

  case 142:
#line 395 "SRC/verilog_bison.y"
    {(yyval.node) = newUnaryOperation(BITWISE_NAND, (yyvsp[(2) - (2)].node), yylineno);;}
    break;

  case 143:
#line 396 "SRC/verilog_bison.y"
    {(yyval.node) = newUnaryOperation(BITWISE_NOR, (yyvsp[(2) - (2)].node), yylineno);;}
    break;

  case 144:
#line 397 "SRC/verilog_bison.y"
    {(yyval.node) = newUnaryOperation(BITWISE_XNOR, (yyvsp[(2) - (2)].node), yylineno);;}
    break;

  case 145:
#line 398 "SRC/verilog_bison.y"
    {(yyval.node) = newUnaryOperation(LOGICAL_NOT, (yyvsp[(2) - (2)].node), yylineno);;}
    break;

  case 146:
#line 399 "SRC/verilog_bison.y"
    {(yyval.node) = newUnaryOperation(BITWISE_XOR, (yyvsp[(2) - (2)].node), yylineno);;}
    break;

  case 147:
#line 400 "SRC/verilog_bison.y"
    {(yyval.node) = newBinaryOperation(BITWISE_XOR, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), yylineno);;}
    break;

  case 148:
#line 401 "SRC/verilog_bison.y"
    {(yyval.node) = newBinaryOperation(MULTIPLY, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), yylineno);;}
    break;

  case 149:
#line 402 "SRC/verilog_bison.y"
    {(yyval.node) = newBinaryOperation(DIVIDE, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), yylineno);;}
    break;

  case 150:
#line 403 "SRC/verilog_bison.y"
    {(yyval.node) = newBinaryOperation(MODULO, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), yylineno);;}
    break;

  case 151:
#line 404 "SRC/verilog_bison.y"
    {(yyval.node) = newBinaryOperation(ADD, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), yylineno);;}
    break;

  case 152:
#line 405 "SRC/verilog_bison.y"
    {(yyval.node) = newBinaryOperation(MINUS, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), yylineno);;}
    break;

  case 153:
#line 406 "SRC/verilog_bison.y"
    {(yyval.node) = newBinaryOperation(BITWISE_AND, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), yylineno);;}
    break;

  case 154:
#line 407 "SRC/verilog_bison.y"
    {(yyval.node) = newBinaryOperation(BITWISE_OR, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), yylineno);;}
    break;

  case 155:
#line 408 "SRC/verilog_bison.y"
    {(yyval.node) = newBinaryOperation(BITWISE_NAND, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), yylineno);;}
    break;

  case 156:
#line 409 "SRC/verilog_bison.y"
    {(yyval.node) = newBinaryOperation(BITWISE_NOR, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), yylineno);;}
    break;

  case 157:
#line 410 "SRC/verilog_bison.y"
    {(yyval.node) = newBinaryOperation(BITWISE_XNOR, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), yylineno);;}
    break;

  case 158:
#line 411 "SRC/verilog_bison.y"
    {(yyval.node) = newBinaryOperation(LT, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), yylineno);;}
    break;

  case 159:
#line 412 "SRC/verilog_bison.y"
    {(yyval.node) = newBinaryOperation(GT, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), yylineno);;}
    break;

  case 160:
#line 413 "SRC/verilog_bison.y"
    {(yyval.node) = newBinaryOperation(SR, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), yylineno);;}
    break;

  case 161:
#line 414 "SRC/verilog_bison.y"
    {(yyval.node) = newBinaryOperation(SL, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), yylineno);;}
    break;

  case 162:
#line 415 "SRC/verilog_bison.y"
    {(yyval.node) = newBinaryOperation(LOGICAL_EQUAL, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), yylineno);;}
    break;

  case 163:
#line 416 "SRC/verilog_bison.y"
    {(yyval.node) = newBinaryOperation(NOT_EQUAL, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), yylineno);;}
    break;

  case 164:
#line 417 "SRC/verilog_bison.y"
    {(yyval.node) = newBinaryOperation(LTE, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), yylineno);;}
    break;

  case 165:
#line 418 "SRC/verilog_bison.y"
    {(yyval.node) = newBinaryOperation(GTE, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), yylineno);;}
    break;

  case 166:
#line 419 "SRC/verilog_bison.y"
    {(yyval.node) = newBinaryOperation(CASE_EQUAL, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), yylineno);;}
    break;

  case 167:
#line 420 "SRC/verilog_bison.y"
    {(yyval.node) = newBinaryOperation(CASE_NOT_EQUAL, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), yylineno);;}
    break;

  case 168:
#line 421 "SRC/verilog_bison.y"
    {(yyval.node) = newBinaryOperation(LOGICAL_OR, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), yylineno);;}
    break;

  case 169:
#line 422 "SRC/verilog_bison.y"
    {(yyval.node) = newBinaryOperation(LOGICAL_AND, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), yylineno);;}
    break;

  case 170:
#line 423 "SRC/verilog_bison.y"
    {(yyval.node) = newIfQuestion((yyvsp[(1) - (5)].node), (yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].node), yylineno);;}
    break;

  case 171:
#line 424 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 172:
#line 425 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(2) - (3)].node);;}
    break;

  case 173:
#line 429 "SRC/verilog_bison.y"
    {(yyval.node) = newListReplicate( (yyvsp[(2) - (6)].node), (yyvsp[(4) - (6)].node) ); ;}
    break;

  case 174:
#line 430 "SRC/verilog_bison.y"
    {(yyval.node) = newListReplicate( (yyvsp[(2) - (6)].node), (yyvsp[(4) - (6)].node) ); ;}
    break;

  case 175:
#line 433 "SRC/verilog_bison.y"
    {(yyval.node) = newNumberNode((yyvsp[(1) - (1)].num_value), yylineno);;}
    break;

  case 176:
#line 434 "SRC/verilog_bison.y"
    {(yyval.node) = newSymbolNode((yyvsp[(1) - (1)].id_name), yylineno);;}
    break;

  case 177:
#line 435 "SRC/verilog_bison.y"
    {(yyval.node) = newArrayRef((yyvsp[(1) - (4)].id_name), (yyvsp[(3) - (4)].node), yylineno);;}
    break;

  case 178:
#line 436 "SRC/verilog_bison.y"
    {(yyval.node) = newRangeRef((yyvsp[(1) - (6)].id_name), (yyvsp[(3) - (6)].node), (yyvsp[(5) - (6)].node), yylineno);;}
    break;

  case 179:
#line 437 "SRC/verilog_bison.y"
    {(yyval.node) = (yyvsp[(2) - (3)].node); ((yyvsp[(2) - (3)].node))->types.concat.num_bit_strings = -1;;}
    break;

  case 180:
#line 440 "SRC/verilog_bison.y"
    {(yyval.node) = newList_entry((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); /* note this will be in order lsb = greatest to msb = 0 in the node child list */;}
    break;

  case 181:
#line 441 "SRC/verilog_bison.y"
    {(yyval.node) = newList(CONCATENATE, (yyvsp[(1) - (1)].node));;}
    break;


/* Line 1267 of yacc.c.  */
#line 3102 "SRC/verilog_bison.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 444 "SRC/verilog_bison.y"



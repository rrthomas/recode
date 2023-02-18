/* Conversion of files between different charsets and surfaces.
   Copyright © 1990-2018 Free Software Foundation, Inc.
   Contributed by François Pinard <pinard@iro.umontreal.ca>, 1988.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 3 of the
   License, or (at your option) any later version.

   This library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the Recode Library; see the file `COPYING.LIB'.
   If not, see <https://www.gnu.org/licenses/>.
*/

#include "config.h"
#include "common.h"
#include "decsteps.h"

static const char *const translation_table[128] =
  {
    "^5",			/*   0 */
    "^6",			/*   1 */
    "^7",			/*   2 */
    "^8",			/*   3 */
    "^9",			/*   4 */
    "^+",			/*   5 */
    "^-",			/*   6 */
    "^*",			/*   7 */
    "^/",			/*   8 */
    "^(",			/*   9 */
    "\n",			/*  10, would have been "^)" */
    "^$",			/*  11 */
    "^=",			/*  12 */
    "^ ",			/*  13 */
    "^,",			/*  14 */
    "^.",			/*  15 */
    "^#",			/*  16 */
    "^[",			/*  17 */
    "^]",			/*  18 */
    "^%",			/*  19 */
    "^\"",			/*  20 */
    "^_",			/*  21 */
    "^!",			/*  22 */
    "^&",			/*  23 */
    "^'",			/*  24 */
    "^?",			/*  25 */
    "^<",			/*  26 */
    "^>",			/*  27 */
    "^@",			/*  28 */
    "^\\",			/*  29 */
    "^^",			/*  30 */
    "^;",			/*  31 */
    " ",			/*  32 */
    "!",			/*  33 */
    "\"",			/*  34 */
    "#",			/*  35 */
    "$",			/*  36 */
    "%",			/*  37 */
    "&",			/*  38 */
    "'",			/*  39 */
    "(",			/*  40 */
    ")",			/*  41 */
    "*",			/*  42 */
    "+",			/*  43 */
    ",",			/*  44 */
    "-",			/*  45 */
    ".",			/*  46 */
    "/",			/*  47 */
    "0",			/*  48 */
    "1",			/*  49 */
    "2",			/*  50 */
    "3",			/*  51 */
    "4",			/*  52 */
    "5",			/*  53 */
    "6",			/*  54 */
    "7",			/*  55 */
    "8",			/*  56 */
    "9",			/*  57 */
    "@D",			/*  58 */
    ";",			/*  59 */
    "<",			/*  60 */
    "=",			/*  61 */
    ">",			/*  62 */
    "?",			/*  63 */
    "@A",			/*  64 */
    "A",			/*  65 */
    "B",			/*  66 */
    "C",			/*  67 */
    "D",			/*  68 */
    "E",			/*  69 */
    "F",			/*  70 */
    "G",			/*  71 */
    "H",			/*  72 */
    "I",			/*  73 */
    "J",			/*  74 */
    "K",			/*  75 */
    "L",			/*  76 */
    "M",			/*  77 */
    "N",			/*  78 */
    "O",			/*  79 */
    "P",			/*  80 */
    "Q",			/*  81 */
    "R",			/*  82 */
    "S",			/*  83 */
    "T",			/*  84 */
    "U",			/*  85 */
    "V",			/*  86 */
    "W",			/*  87 */
    "X",			/*  88 */
    "Y",			/*  89 */
    "Z",			/*  90 */
    "[",			/*  91 */
    "\\",			/*  92 */
    "]",			/*  93 */
    "@B",			/*  94 */
    "_",			/*  95 */
    "@G",			/*  96 */
    "^A",			/*  97 */
    "^B",			/*  98 */
    "^C",			/*  99 */
    "^D",			/* 100 */
    "^E",			/* 101 */
    "^F",			/* 102 */
    "^G",			/* 103 */
    "^H",			/* 104 */
    "^I",			/* 105 */
    "^J",			/* 106 */
    "^K",			/* 107 */
    "^L",			/* 108 */
    "^M",			/* 109 */
    "^N",			/* 110 */
    "^O",			/* 111 */
    "^P",			/* 112 */
    "^Q",			/* 113 */
    "^R",			/* 114 */
    "^S",			/* 115 */
    "^T",			/* 116 */
    "^U",			/* 117 */
    "^V",			/* 118 */
    "^W",			/* 119 */
    "^X",			/* 120 */
    "^Y",			/* 121 */
    "^Z",			/* 122 */
    "^0",			/* 123 */
    "^1",			/* 124 */
    "^2",			/* 125 */
    "^3",			/* 126 */
    "^4",			/* 127 */
  };

static bool
init_ascii_cdcnos (RECODE_STEP step,
		   RECODE_CONST_REQUEST request,
		   RECODE_CONST_OPTION_LIST before_options,
		   RECODE_CONST_OPTION_LIST after_options)
{
  RECODE_OUTER outer = request->outer;
  const char **table;
  int counter;

  if (before_options || after_options)
    return false;

  if (!ALLOC (table, 256, const char *))
    return false;
  for (counter = 0; counter < 128; counter++)
    table[counter] = translation_table[counter];
  for (; counter < 256; counter++)
    table[counter] = NULL;

  step->step_type = RECODE_BYTE_TO_STRING;
  step->step_table = table;
  step->step_table_term_routine = free;
  return true;
}

/* Previous obsolete lex code:

@A			{ recode_put_byte ('@', subtask); }
@B			{ recode_put_byte ('^', subtask); }
@D			{ recode_put_byte (':', subtask); }
@G			{ recode_put_byte ('`', subtask); }

\^" "			{ recode_put_byte ( 13, subtask); }
\^!			{ recode_put_byte ( 22, subtask); }
\^\"			{ recode_put_byte ( 20, subtask); }
\^#			{ recode_put_byte ( 16, subtask); }
\^$			{ recode_put_byte ( 11, subtask); }
\^\%			{ recode_put_byte ( 19, subtask); }
\^&			{ recode_put_byte ( 23, subtask); }
\^'			{ recode_put_byte ( 24, subtask); }
\^\(			{ recode_put_byte (  9, subtask); }
\^\)			{ recode_put_byte ( 10, subtask); }
\^\*			{ recode_put_byte (  7, subtask); }
\^\+			{ recode_put_byte (  5, subtask); }
\^\,			{ recode_put_byte ( 14, subtask); }
\^-			{ recode_put_byte (  6, subtask); }
\^\.			{ recode_put_byte ( 15, subtask); }
\^\/			{ recode_put_byte (  8, subtask); }

\^0			{ recode_put_byte ('{', subtask); }
\^1			{ recode_put_byte ('|', subtask); }
\^2			{ recode_put_byte ('}', subtask); }
\^3			{ recode_put_byte ('~', subtask); }
\^4			{ recode_put_byte (127, subtask); }

\^5			{ recode_put_byte (  0, subtask); }
\^6			{ recode_put_byte (  1, subtask); }
\^7			{ recode_put_byte (  2, subtask); }
\^8			{ recode_put_byte (  3, subtask); }
\^9			{ recode_put_byte (  4, subtask); }

\^;			{ recode_put_byte ( 31, subtask); }
\^<			{ recode_put_byte ( 26, subtask); }
\^=			{ recode_put_byte ( 12, subtask); }
\^>			{ recode_put_byte ( 27, subtask); }
\^?			{ recode_put_byte ( 25, subtask); }
\^@			{ recode_put_byte ( 28, subtask); }

\^[A-Z]			{ recode_put_byte (yytext[1]-'A'+'a', subtask); }

\^\[			{ recode_put_byte ( 17, subtask); }
\^\\			{ recode_put_byte ( 29, subtask); }
\^]			{ recode_put_byte ( 18, subtask); }
\^\^			{ recode_put_byte ( 30, subtask); }
\^_			{ recode_put_byte ( 21, subtask); }

\^[a-z]			{ recode_put_byte (yytext[1], subtask); }

*/

static bool
transform_cdcnos_ascii (RECODE_SUBTASK subtask)
{
  int input_char;		/* current character */

  while (input_char = recode_get_byte (subtask), input_char != EOF)
    {
      switch (input_char)
	{
        case '@':
	  switch (input_char = recode_get_byte (subtask), input_char)
	    {
	    case 'A': case 'a': input_char = '@'; break;
	    case 'B': case 'b': input_char = '^'; break;
	    case 'D': case 'd': input_char = ':'; break;
	    case 'G': case 'g': input_char = '`'; break;

	    default:
	      RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
	      recode_put_byte ('@', subtask);
	      if (input_char == EOF)
		SUBTASK_RETURN (subtask);
	    }
	  break;

	case '^':
	  input_char = recode_get_byte (subtask);
	  if (input_char >= 'A' && input_char <= 'Z')
	    input_char += 'a' - 'A';
	  else if (input_char < 'a' || input_char > 'z')
	    switch (input_char)
	      {
	      case ' ': input_char = 13; break;
	      case '!': input_char = 22; break;
	      case '"': input_char = 20; break;
	      case '#': input_char = 16; break;
	      case '$': input_char = 11; break;
	      case '%': input_char = 19; break;
	      case '&': input_char = 23; break;
	      case '\'': input_char = 24; break;
	      case '(': input_char = 9; break;

	      case ')':
		RETURN_IF_NOGO (RECODE_NOT_CANONICAL, subtask);
		input_char = '\n'; /* 10 */
		break;

	      case '*': input_char = 7; break;
	      case '+': input_char = 5; break;
	      case ',': input_char = 14; break;
	      case '-': input_char = 6; break;
	      case '.': input_char = 15; break;
	      case '/': input_char = 8; break;
	      case '0': input_char = '{'; break;
	      case '1': input_char = '|'; break;
	      case '2': input_char = '}'; break;
	      case '3': input_char = '~'; break;
	      case '4': input_char = 127; break;
	      case '5': input_char = 0; break;
	      case '6': input_char = 1; break;
	      case '7': input_char = 2; break;
	      case '8': input_char = 3; break;
	      case '9': input_char = 4; break;
	      case ';': input_char = 31; break;
	      case '<': input_char = 26; break;
	      case '=': input_char = 12; break;
	      case '>': input_char = 27; break;
	      case '?': input_char = 25; break;
	      case '@': input_char = 28; break;
	      case '[': input_char = 17; break;
	      case '\\': input_char = 29; break;
	      case ']': input_char = 18; break;
	      case '^': input_char = 30; break;
	      case '_': input_char = 21; break;

	      default:
		RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
	        recode_put_byte ('^', subtask);
	        if (input_char == EOF)
		  SUBTASK_RETURN (subtask);
	      }
	  break;

        default:
          break;
	}
      recode_put_byte (input_char, subtask);
    }
  SUBTASK_RETURN (subtask);
}

bool
module_cdcnos (RECODE_OUTER outer)
{
  return
    recode_declare_single (outer, "ASCII-BS", "CDC-NOS",
		    outer->quality_byte_to_variable,
		    init_ascii_cdcnos, recode_transform_byte_to_variable)
    && recode_declare_single (outer, "CDC-NOS", "ASCII-BS",
		       outer->quality_variable_to_byte,
		       NULL, transform_cdcnos_ascii)
    && recode_declare_alias (outer, "NOS", "CDC-NOS");
}

void
delmodule_cdcnos (_GL_UNUSED RECODE_OUTER outer)
{
}

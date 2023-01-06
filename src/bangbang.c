/* Conversion of files between different charsets and surfaces.
   Copyright © 1990-2000 Free Software Foundation, Inc.
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

static const char *const translation_table[256] =
  {
    "!!@",			/*   0 */
    "!!a",			/*   1 */
    "!!b",			/*   2 */
    "!!c",			/*   3 */
    "!!d",			/*   4 */
    "!!e",			/*   5 */
    "!!f",			/*   6 */
    "!!g",			/*   7 */
    "!!h",			/*   8 */
    "!!i",			/*   9 */
    "\n",			/*  10, would have been "!!j" */
    "!!k",			/*  11 */
    "!!l",			/*  12 */
    "!!m",			/*  13 */
    "!!n",			/*  14 */
    "!!o",			/*  15 */
    "!!p",			/*  16 */
    "!!q",			/*  17 */
    "!!r",			/*  18 */
    "!!s",			/*  19 */
    "!!t",			/*  20 */
    "!!u",			/*  21 */
    "!!v",			/*  22 */
    "!!w",			/*  23 */
    "!!x",			/*  24 */
    "!!y",			/*  25 */
    "!!z",			/*  26 */
    "!![",			/*  27 */
    "!!\\",			/*  28 */
    "!!]",			/*  29 */
    "!!^",			/*  30 */
    "!!_",			/*  31 */
    " ",			/*  32 */
    "!\"",			/*  33 */
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
    ":",			/*  58 */
    ";",			/*  59 */
    "<",			/*  60 */
    "=",			/*  61 */
    ">",			/*  62 */
    "?",			/*  63 */
    "@",			/*  64 */
    "!a",			/*  65 */
    "!b",			/*  66 */
    "!c",			/*  67 */
    "!d",			/*  68 */
    "!e",			/*  69 */
    "!f",			/*  70 */
    "!g",			/*  71 */
    "!h",			/*  72 */
    "!i",			/*  73 */
    "!j",			/*  74 */
    "!k",			/*  75 */
    "!l",			/*  76 */
    "!m",			/*  77 */
    "!n",			/*  78 */
    "!o",			/*  79 */
    "!p",			/*  80 */
    "!q",			/*  81 */
    "!r",			/*  82 */
    "!s",			/*  83 */
    "!t",			/*  84 */
    "!u",			/*  85 */
    "!v",			/*  86 */
    "!w",			/*  87 */
    "!x",			/*  88 */
    "!y",			/*  89 */
    "!z",			/*  90 */
    "[",			/*  91 */
    "\\",			/*  92 */
    "]",			/*  93 */
    "^",			/*  94 */
    "_",			/*  95 */
    "!@",			/*  96 */
    "a",			/*  97 */
    "b",			/*  98 */
    "c",			/*  99 */
    "d",			/* 100 */
    "e",			/* 101 */
    "f",			/* 102 */
    "g",			/* 103 */
    "h",			/* 104 */
    "i",			/* 105 */
    "j",			/* 106 */
    "k",			/* 107 */
    "l",			/* 108 */
    "m",			/* 109 */
    "n",			/* 110 */
    "o",			/* 111 */
    "p",			/* 112 */
    "q",			/* 113 */
    "r",			/* 114 */
    "s",			/* 115 */
    "t",			/* 116 */
    "u",			/* 117 */
    "v",			/* 118 */
    "w",			/* 119 */
    "x",			/* 120 */
    "y",			/* 121 */
    "z",			/* 122 */
    "![",			/* 123 */
    "!\\",			/* 124 */
    "!]",			/* 125 */
    "!^",			/* 126 */
    "!_",			/* 127 */

    NULL,			/* 128 */
    NULL,			/* 129 */
    NULL,			/* 130 */
    NULL,			/* 131 */
    NULL,			/* 132 */
    NULL,			/* 133 */
    NULL,			/* 134 */
    NULL,			/* 135 */
    NULL,			/* 136 */
    NULL,			/* 137 */
    NULL,			/* 138 */
    NULL,			/* 139 */
    NULL,			/* 140 */
    NULL,			/* 141 */
    NULL,			/* 142 */
    NULL,			/* 143 */
    NULL,			/* 144 */
    NULL,			/* 145 */
    NULL,			/* 146 */
    NULL,			/* 147 */
    NULL,			/* 148 */
    NULL,			/* 149 */
    NULL,			/* 150 */
    NULL,			/* 151 */
    NULL,			/* 152 */
    NULL,			/* 153 */
    NULL,			/* 154 */
    NULL,			/* 155 */
    NULL,			/* 156 */
    NULL,			/* 157 */
    NULL,			/* 158 */
    NULL,			/* 159 */

    " ",			/* 160 no-break space */
    NULL,			/* 161 inverted exclamation mark */
    NULL,			/* 162 cent sign */
    NULL,			/* 163 pound sign */
    NULL,			/* 164 currency sign */
    NULL,			/* 165 yen sign */
    NULL,			/* 166 broken bar */
    NULL,			/* 167 paragraph sign, section sign */
    NULL,			/* 168 diaeresis */
    NULL,			/* 169 copyright sign */
    NULL,			/* 170 feminine ordinal indicator */
    "!>",			/* 171 left angle quotation mark */
    NULL,			/* 172 not sign */
    NULL,			/* 173 soft hyphen */
    NULL,			/* 174 registered trade mark sign */
    NULL,			/* 175 macron */
    NULL,			/* 176 degree sign */
    NULL,			/* 177 plus-minus sign */
    NULL,			/* 178 superscript two */
    NULL,			/* 179 superscript three */
    NULL,			/* 180 acute accent */
    NULL,			/* 181 small greek mu, micro sign */
    NULL,			/* 182 pilcrow sign */
    NULL,			/* 183 middle dot */
    NULL,			/* 184 cedilla */
    NULL,			/* 185 superscript one */
    NULL,			/* 186 masculine ordinal indicator */
    "!?",			/* 187 right angle quotation mark */
    NULL,			/* 188 vulgar fraction one quarter */
    NULL,			/* 189 vulgar fraction one half */
    NULL,			/* 190 vulgar fraction three quarters */
    NULL,			/* 191 inverted question mark */
    NULL,			/* 192 capital A with grave accent */
    NULL,			/* 193 capital A with acute accent */
    NULL,			/* 194 capital A with circumflex accent */
    NULL,			/* 195 capital A with tilde */
    NULL,			/* 196 capital A diaeresis */
    NULL,			/* 197 capital A with ring above */
    NULL,			/* 198 capital diphthong A with E */
    NULL,			/* 199 capital C with cedilla */
    NULL,			/* 200 capital E with grave accent */
    NULL,			/* 201 capital E with acute accent */
    NULL,			/* 202 capital E with circumflex accent */
    NULL,			/* 203 capital E with diaeresis */
    NULL,			/* 204 capital I with grave accent */
    NULL,			/* 205 capital I with acute accent */
    NULL,			/* 206 capital I with circumflex accent */
    NULL,			/* 207 capital I with diaeresis */
    NULL,			/* 208 capital icelandic ETH */
    NULL,			/* 209 capital N with tilde */
    NULL,			/* 210 capital O with grave accent */
    NULL,			/* 211 capital O with acute accent */
    NULL,			/* 212 capital O with circumflex accent */
    NULL,			/* 213 capital O with tilde */
    NULL,			/* 214 capital O with diaeresis */
    NULL,			/* 215 multiplication sign */
    NULL,			/* 216 capital O with oblique stroke */
    NULL,			/* 217 capital U with grave accent */
    NULL,			/* 218 capital U with acute accent */
    NULL,			/* 219 capital U with circumflex accent */
    NULL,			/* 220 capital U with diaeresis */
    NULL,			/* 221 capital Y with acute accent */
    NULL,			/* 222 capital icelandic THORN */
    NULL,			/* 223 small german sharp s */
    "!0",			/* 224 small a with grave accent */
    NULL,			/* 225 small a with acute accent */
    "!1",			/* 226 small a with circumflex accent */
    NULL,			/* 227 small a with tilde */
    NULL,			/* 228 small a with diaeresis */
    NULL,			/* 229 small a with ring above */
    "!;",			/* 230 small diphthong a with e */
    "!=",			/* 231 small c with cedilla */
    "!3",			/* 232 small e with grave accent */
    "!2",			/* 233 small e with acute accent */
    "!5",			/* 234 small e with circumflex accent */
    "!4",			/* 235 small e with diaeresis */
    NULL,			/* 236 small i with grave accent */
    NULL,			/* 237 small i with acute accent */
    "!7",			/* 238 small i with circumflex accent */
    "!6",			/* 239 small i with diaeresis */
    NULL,			/* 240 small icelandic eth */
    NULL,			/* 241 small n with tilde */
    NULL,			/* 242 small o with grave accent */
    NULL,			/* 243 small o with acute accent */
    "!8",			/* 244 small o with circumflex accent */
    NULL,			/* 245 small o with tilde */
    NULL,			/* 246 small o with diaeresis */
    "!<",			/* 247 division sign (or French oe?) */
    NULL,			/* 248 small o with oblique stroke */
    "!9",			/* 249 small u with grave accent */
    NULL,			/* 250 small u with acute accent */
    "!:",			/* 251 small u with circumflex accent */
    NULL,			/* 252 small u with diaeresis */
    NULL,			/* 253 small y with acute accent */
    NULL,			/* 254 small icelandic thorn */
    NULL,			/* 255 small y with diaeresis */
  };

static bool
init_latin1_bangbang (RECODE_STEP step,
		      RECODE_CONST_REQUEST request _GL_UNUSED,
		      RECODE_CONST_OPTION_LIST before_options,
		      RECODE_CONST_OPTION_LIST after_options)
{
  if (before_options || after_options)
    return false;

  step->step_type = RECODE_BYTE_TO_STRING;
  /* The cast is a way to silently discard the const.  */
  step->step_table = (void *) translation_table;
  return true;
}

static bool
transform_bangbang_latin1 (RECODE_SUBTASK subtask)
{
  int input_char;		/* current character */

  while (input_char = get_byte (subtask), input_char != EOF)
    {
      if (input_char >= 'A' && input_char <= 'Z')
        input_char += 'a' - 'A';
      else if (input_char == '!')
	{
	  input_char = get_byte (subtask);
	  if (input_char >= 'a' && input_char <= 'z')
	    input_char += 'A' - 'a';
	  else if (input_char < 'A' || input_char > 'Z')
	    switch (input_char)
	      {
	      case '"': input_char = '!'; break;
	      case '0': input_char = 224; break; /* a` */
	      case '1': input_char = 226; break; /* a^ */
	      case '2': input_char = 233; break; /* e' */
	      case '3': input_char = 232; break; /* e` */
	      case '4': input_char = 235; break; /* e" */
	      case '5': input_char = 234; break; /* e^ */
	      case '6': input_char = 236; break; /* e" */
	      case '7': input_char = 238; break; /* i^ */
	      case '8': input_char = 244; break; /* o^ */
	      case '9': input_char = 249; break; /* u` */
	      case ':': input_char = 251; break; /* u^ */
	      case '=': input_char = 231; break; /* c, */
	      case '>': input_char = 171; break; /* `` */
	      case '?': input_char = 187; break; /* '' */
	      case ';': input_char = 230; break; /* ae */
	      case '<':	input_char = 247; break; /* oe ??? */
	      case '@': input_char = '`'; break;
	      case '[': input_char = '{'; break;
	      case '\\': input_char = '|'; break;
	      case ']': input_char = '}'; break;
	      case '^': input_char = '~'; break;
	      case '_': input_char = 127; break; /* del */

	      case '!':
	        input_char = get_byte (subtask);
		if (input_char == 'J' || input_char == 'j')
		  RETURN_IF_NOGO (RECODE_NOT_CANONICAL, subtask);

		/* FIXME: What is canonical?  Upper case or lower case?  */
		if (input_char >= 'A' && input_char <= 'Z')
		  input_char += 1 - 'A';
	        else if (input_char >= 'a' && input_char <= 'z')
		  input_char += 1 - 'a';
	        else
		  switch (input_char)
		    {
		    case '@': input_char = 0; break;
		    case '[': input_char = 27; break;
		    case '\\': input_char = 28; break;
		    case ']': input_char = 29; break;
		    case '^': input_char = 30; break;
		    case '_': input_char = 31; break;

		    default:
		      RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
		      put_byte ('!', subtask);
		      put_byte ('!', subtask);
		      if (input_char == EOF)
		        SUBTASK_RETURN (subtask);
		    }
		break;

	      default:
		RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
	        put_byte ('!', subtask);
	        if (input_char == EOF)
		  SUBTASK_RETURN (subtask);
	      }
	}
      put_byte (input_char, subtask);
    }
  SUBTASK_RETURN (subtask);
}

bool
module_bangbang (RECODE_OUTER outer)
{
  return
    declare_single (outer, "Latin-1", "Bang-Bang",
		    outer->quality_byte_to_variable,
		    init_latin1_bangbang, transform_byte_to_variable)
    && declare_single (outer, "Bang-Bang", "Latin-1",
		       outer->quality_variable_to_byte,
		       NULL, transform_bangbang_latin1);
}

void
delmodule_bangbang (_GL_UNUSED RECODE_OUTER outer)
{
}

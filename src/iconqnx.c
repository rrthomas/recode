/* Conversion of files between different charsets and surfaces.
   Copyright © 1990, 93, 94, 97, 98, 99, 00 Free Software Foundation, Inc.
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

#define DOS_CR 13		/* carriage return */
#define DOS_LF 10		/* line feed */
#define DOS_EOF 26		/* old end of file */

#define ESCAPE 25		/* escape for diacritic application */
#define ENDLINE 30		/* end-line code for QNX */

#define TRANSLATE_AND_BREAK(c2, c3) \
  put_byte (ESCAPE, subtask);		\
  put_byte (c2, subtask);		\
  put_byte (c3, subtask);		\
  input_char = get_byte (subtask);	\
  break;

static bool
transform_ibmpc_iconqnx (RECODE_SUBTASK subtask)
{
  int input_char;

  input_char = get_byte (subtask);
  while (true)
    switch (input_char)
      {
      case DOS_EOF:
	RETURN_IF_NOGO (RECODE_NOT_CANONICAL, subtask);
	FALLTHROUGH;

      case EOF:
	SUBTASK_RETURN (subtask);

      case 133: TRANSLATE_AND_BREAK ('A', 'a');
      case 138: TRANSLATE_AND_BREAK ('A', 'e');
      case 151: TRANSLATE_AND_BREAK ('A', 'u');
      case 130: TRANSLATE_AND_BREAK ('B', 'e');
      case 144: TRANSLATE_AND_BREAK ('B', 'E');
      case 131: TRANSLATE_AND_BREAK ('C', 'a');
      case 136: TRANSLATE_AND_BREAK ('C', 'e');
      case 140: TRANSLATE_AND_BREAK ('C', 'i');
      case 147: TRANSLATE_AND_BREAK ('C', 'o');
      case 150: TRANSLATE_AND_BREAK ('C', 'u');
      case 137: TRANSLATE_AND_BREAK ('H', 'e');
      case 139: TRANSLATE_AND_BREAK ('H', 'i');
      case 129: TRANSLATE_AND_BREAK ('H', 'u');
      case 135: TRANSLATE_AND_BREAK ('K', 'c');
      case 128: TRANSLATE_AND_BREAK ('K', 'C');

      case DOS_CR:
	input_char = get_byte (subtask);
	if (input_char == DOS_LF)
	  {
	    put_byte (ENDLINE, subtask);
	    input_char = get_byte (subtask);
	  }
	else
	  put_byte (DOS_CR, subtask);
	break;

      case ENDLINE:
      case ESCAPE:
	RETURN_IF_NOGO (RECODE_AMBIGUOUS_OUTPUT, subtask);
	FALLTHROUGH;

      default:
	put_byte (input_char, subtask);
	input_char = get_byte (subtask);
      }
}

static bool
transform_iconqnx_ibmpc (RECODE_SUBTASK subtask)
{
  int input_char;		/* current character */

  input_char = get_byte (subtask);
  while (true)
    switch (input_char)
      {
      case EOF:
	SUBTASK_RETURN (subtask);

      case ENDLINE:
	put_byte (DOS_CR, subtask);
	put_byte (DOS_LF, subtask);
	input_char = get_byte (subtask);
	break;

      case DOS_CR:
	input_char = get_byte (subtask);
	if (input_char == DOS_LF)
	  RETURN_IF_NOGO (RECODE_AMBIGUOUS_OUTPUT, subtask);
	put_byte (DOS_CR, subtask);
	break;

      case ESCAPE:
	input_char = get_byte (subtask);
	switch (input_char)
	  {
	  case 'A':
	    input_char = get_byte (subtask);
	    switch (input_char)
	      {
	      case 'a': input_char = 133; break;
	      case 'e': input_char = 138; break;
	      case 'u': input_char = 151; break;

	      default:
		RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
		put_byte (ESCAPE, subtask);
		put_byte ('A', subtask);
		if (input_char == EOF)
		  SUBTASK_RETURN (subtask);
	      }
	    break;

	  case 'B':
	    input_char = get_byte (subtask);
	    switch (input_char)
	      {
	      case 'e': input_char = 130; break;
	      case 'E': input_char = 144; break;

	      default:
		RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
		put_byte (ESCAPE, subtask);
		put_byte ('B', subtask);
		if (input_char == EOF)
		  SUBTASK_RETURN (subtask);
	      }
	    break;

	  case 'C':
	    input_char = get_byte (subtask);
	    switch (input_char)
	      {
	      case 'a': input_char = 131; break;
	      case 'e': input_char = 136; break;
	      case 'i': input_char = 140; break;
	      case 'o': input_char = 147; break;
	      case 'u': input_char = 150; break;

	      default:
		RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
		put_byte (ESCAPE, subtask);
		put_byte ('C', subtask);
		if (input_char == EOF)
		  SUBTASK_RETURN (subtask);
	      }
	    break;

	  case 'H':
	    input_char = get_byte (subtask);
	    switch (input_char)
	      {
	      case 'e': input_char = 137; break;
	      case 'i': input_char = 139; break;
	      case 'u': input_char = 129; break;

	      default:
		RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
		put_byte (ESCAPE, subtask);
		put_byte ('H', subtask);
		if (input_char == EOF)
		  SUBTASK_RETURN (subtask);
	      }
	    break;

	  case 'K':
	    input_char = get_byte (subtask);
	    switch (input_char)
	      {
	      case 'c': input_char = 135; break;
	      case 'C': input_char = 128; break;

	      default:
		RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
		put_byte (ESCAPE, subtask);
		put_byte ('K', subtask);
		if (input_char == EOF)
		  SUBTASK_RETURN (subtask);
	      }
	    break;

	  default:
	    RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
	    put_byte (ESCAPE, subtask);
	    if (input_char == EOF)
	      SUBTASK_RETURN (subtask);
	  }
	FALLTHROUGH;

      default:
	put_byte (input_char, subtask);
	input_char = get_byte (subtask);
      }
}

bool
module_iconqnx (RECODE_OUTER outer)
{
  return
    declare_single (outer, "IBM-PC", "Icon-QNX",
		    outer->quality_variable_to_variable,
		    NULL, transform_ibmpc_iconqnx)
    && declare_single (outer, "Icon-QNX", "IBM-PC",
		       outer->quality_variable_to_variable,
		       NULL, transform_iconqnx_ibmpc)
    && declare_alias (outer, "QNX", "Icon-QNX");
}

void
delmodule_iconqnx (_GL_UNUSED RECODE_OUTER outer)
{
}

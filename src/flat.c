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

static bool
transform_ascii_flat (RECODE_SUBTASK subtask)
{
  int input_char;		/* current character */
  int temp_char;		/* look ahead character */

  input_char = get_byte (subtask);
  while (true)
    switch (input_char)
      {
      case EOF:
	SUBTASK_RETURN (subtask);

      case '\n':
      case '\t':
	put_byte (input_char, subtask);
	input_char = get_byte (subtask);
	break;

      case '\b':
	input_char = get_byte (subtask);
	switch (input_char)
	  {
	  case '\'':
	  case '`':
	  case '^':
	  case '"':
	  case '~':
	  case ',':
	  case '_':
	    RETURN_IF_NOGO (RECODE_AMBIGUOUS_OUTPUT, subtask);
	    input_char = get_byte (subtask);
	    break;

	  default:
	    put_byte ('\b', subtask);
	  }
	break;

      case '\'':
      case '`':
      case '^':
      case '"':
      case '~':
      case ',':
      case '_':
	temp_char = get_byte (subtask);
	if (temp_char == '\b')
	  {
	    RETURN_IF_NOGO (RECODE_AMBIGUOUS_OUTPUT, subtask);
	    input_char = get_byte (subtask);
	  }
	else
	  {
	    put_byte (input_char, subtask);
	    input_char = temp_char;
	  }
	break;

      default:
	if (!IS_ASCII (input_char))
	  {
	    RETURN_IF_NOGO (RECODE_AMBIGUOUS_OUTPUT, subtask);
	    put_byte ('M', subtask);
	    put_byte ('-', subtask);
	    input_char &= BIT_MASK (7);
	  }
	if (input_char < ' ' || input_char == BIT_MASK (7))
	  {
	    RETURN_IF_NOGO (RECODE_AMBIGUOUS_OUTPUT, subtask);
	    put_byte ('^', subtask);
	    input_char ^= (1 << 6);
	  }
	put_byte (input_char, subtask);
	input_char = get_byte (subtask);
      }
}

bool
module_flat (RECODE_OUTER outer)
{
  if (!declare_single (outer, "ASCII-BS", "flat",
		       outer->quality_variable_to_variable,
		       NULL, transform_ascii_flat))
    return false;

  return true;
}

void
delmodule_flat (_GL_UNUSED RECODE_OUTER outer)
{
}

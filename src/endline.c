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

#define CR 13			/* carriage return */
#define LF 10			/* line feed */
#define OLD_EOF 26		/* oldish end of file */

static bool
transform_data_cr (RECODE_SUBTASK subtask)
{
  bool strict = subtask->step->fallback_routine != recode_reversibility;
  int character;

  while (character = recode_get_byte (subtask), character != EOF)
    switch (character)
      {
      case '\n':
	recode_put_byte (CR, subtask);
	break;

      case CR:
	if (!strict)
	  {
	    recode_put_byte ('\n', subtask);
	    break;
	  }
	RETURN_IF_NOGO (RECODE_AMBIGUOUS_OUTPUT, subtask);
	FALLTHROUGH;

      default:
	recode_put_byte (character, subtask);
      }

  SUBTASK_RETURN (subtask);
}

static bool
transform_cr_data (RECODE_SUBTASK subtask)
{
  bool strict = subtask->step->fallback_routine != recode_reversibility;
  int character;

  while (character = recode_get_byte (subtask), character != EOF)
    switch (character)
      {
      case CR:
	recode_put_byte ('\n', subtask);
	break;

      case '\n':
	if (!strict)
	  {
	    recode_put_byte (CR, subtask);
	    break;
	  }
	RETURN_IF_NOGO (RECODE_AMBIGUOUS_OUTPUT, subtask);
	FALLTHROUGH;

      default:
	recode_put_byte (character, subtask);
      }

  SUBTASK_RETURN (subtask);
}

static bool
transform_data_crlf (RECODE_SUBTASK subtask)
{
  int character = recode_get_byte (subtask);

  while (character != EOF)
    switch (character)
      {
      case '\n':
	recode_put_byte (CR, subtask);
	recode_put_byte (LF, subtask);
	character = recode_get_byte (subtask);
	break;

      case CR:
	character = recode_get_byte (subtask);
	if (character == LF)
	  RETURN_IF_NOGO (RECODE_AMBIGUOUS_OUTPUT, subtask);
	recode_put_byte (CR, subtask);
	break;

      case OLD_EOF:
	RETURN_IF_NOGO (RECODE_AMBIGUOUS_OUTPUT, subtask);
	FALLTHROUGH;

      default:
	recode_put_byte (character, subtask);
	character = recode_get_byte (subtask);
      }

  SUBTASK_RETURN (subtask);
}

static bool
transform_crlf_data (RECODE_SUBTASK subtask)
{
  int character = recode_get_byte (subtask);

  while (character != EOF)
    switch (character)
      {
      case OLD_EOF:
	RETURN_IF_NOGO (RECODE_NOT_CANONICAL, subtask);
	SUBTASK_RETURN (subtask);

      case CR:
	character = recode_get_byte (subtask);
	if (character == LF)
	  {
	    recode_put_byte ('\n', subtask);
	    character = recode_get_byte (subtask);
	  }
	else
	  recode_put_byte (CR, subtask);
	break;

      case LF:
	RETURN_IF_NOGO (RECODE_AMBIGUOUS_OUTPUT, subtask);
	FALLTHROUGH;

      default:
	recode_put_byte (character, subtask);
	character = recode_get_byte (subtask);
      }

  SUBTASK_RETURN (subtask);
}

bool
module_endline (RECODE_OUTER outer)
{
  return
    recode_declare_single (outer, "data", "CR",
		    outer->quality_byte_to_byte,
		    NULL, transform_data_cr)
    && recode_declare_single (outer, "CR", "data",
		       outer->quality_byte_to_byte,
		       NULL, transform_cr_data)
    && recode_declare_single (outer, "data", "CR-LF",
		       outer->quality_byte_to_variable,
		       NULL, transform_data_crlf)
    && recode_declare_single (outer, "CR-LF", "data",
		       outer->quality_variable_to_byte,
		       NULL, transform_crlf_data)

    && recode_declare_alias (outer, "cl", "CR-LF");
}

void
delmodule_endline (_GL_UNUSED RECODE_OUTER outer)
{
}

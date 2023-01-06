/* Conversion of files between different charsets and surfaces.
   Copyright © 1997-2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by François Pinard <pinard@iro.umontreal.ca>, 1997.

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
permute_21 (RECODE_SUBTASK subtask)
{
  int character1;
  int character2;

  while (true)
    {
      character1 = get_byte (subtask);
      if (character1 == EOF)
	break;

      character2 = get_byte (subtask);
      if (character2 == EOF)
	{
	  put_byte (character1, subtask);
	  break;
	}

      put_byte (character2, subtask);
      put_byte (character1, subtask);
    }

  SUBTASK_RETURN (subtask);
}

static bool
permute_4321 (RECODE_SUBTASK subtask)
{
  int character1;
  int character2;
  int character3;
  int character4;

  while (true)
    {
      character1 = get_byte (subtask);
      if (character1 == EOF)
	break;

      character2 = get_byte (subtask);
      if (character2 == EOF)
	{
	  put_byte (character1, subtask);
	  break;
	}

      character3 = get_byte (subtask);
      if (character3 == EOF)
	{
	  put_byte (character2, subtask);
	  put_byte (character1, subtask);
	  break;
	}

      character4 = get_byte (subtask);
      if (character4 == EOF)
	{
	  put_byte (character3, subtask);
	  put_byte (character2, subtask);
	  put_byte (character1, subtask);
	  break;
	}

      put_byte (character4, subtask);
      put_byte (character3, subtask);
      put_byte (character2, subtask);
      put_byte (character1, subtask);
    }

  SUBTASK_RETURN (subtask);
}

bool
module_permutations (RECODE_OUTER outer)
{
  return
    declare_single (outer, "data", "21-Permutation",
		    outer->quality_variable_to_variable,
		    NULL, permute_21)
    && declare_single (outer, "21-Permutation", "data",
		       outer->quality_variable_to_variable,
		       NULL, permute_21)
    && declare_single (outer, "data", "4321-Permutation",
		       outer->quality_variable_to_variable,
		       NULL, permute_4321)
    && declare_single (outer, "4321-Permutation", "data",
		       outer->quality_variable_to_variable,
		       NULL, permute_4321)
    && declare_alias (outer, "swabytes", "21-Permutation");
}

void
delmodule_permutations (_GL_UNUSED RECODE_OUTER outer)
{
}

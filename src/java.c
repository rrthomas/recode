/* Conversion of files between different charsets and surfaces.
   Copyright © 2008 Free Software Foundation, Inc.
   Contributed by François Pinard <pinard@iro.umontreal.ca>, 2008.

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
transform_java_utf16 (RECODE_SUBTASK subtask)
{
  int character = get_byte (subtask);

  while (character != EOF)
    if (character == '\\')
      {
        char buffer[6];
        char *cursor = buffer;
        unsigned value = 0;
        bool canonical = true;

        *cursor++ = character;
        character = get_byte (subtask);
        if (character == 'u' || character == 'U')
          {
            if (character == 'U')
              canonical = false;
            *cursor++ = character;
            character = get_byte (subtask);
            while (cursor < buffer + 6)
              {
                if (character >= '0' && character <= '9')
                  value = (value << 4) | (character - '0');
                else if (character >= 'A' && character <= 'F')
                  {
                    value = (value << 4) | (character - 'A' + 10);
                    canonical = false;
                  }
                else if (character >= 'a' && character <= 'f')
                  value = (value << 4) | (character - 'a' + 10);
                else
                  break;
                *cursor++ = character;
                character = get_byte (subtask);
              }
            if (cursor == buffer + 6)
              {
                if (!canonical)
                  RETURN_IF_NOGO (RECODE_NOT_CANONICAL, subtask);
                put_ucs2 (value, subtask);
                continue;
              }
          }
        *cursor = '\0';
        for (cursor = buffer; *cursor; cursor++)
          put_ucs2 (*cursor, subtask);
      }
    else
      {
        put_ucs2 (character, subtask);
        character = get_byte (subtask);
      }

  SUBTASK_RETURN (subtask);
}

static bool
transform_utf16_java (RECODE_SUBTASK subtask)
{
  unsigned value;

  while (get_ucs2 (&value, subtask))
    if (value < 128)
      put_byte (value, subtask);
    else
      {
        char buffer[11];
        char *cursor;

        sprintf (buffer, "\\u%04x", value);
        for (cursor = buffer; *cursor; cursor++)
          put_byte (*cursor, subtask);
      }

  SUBTASK_RETURN (subtask);
}

bool
module_java (RECODE_OUTER outer)
{
  return
    declare_single (outer, "UTF-16", "Java",
		       outer->quality_ucs2_to_variable,
		       NULL, transform_utf16_java)
    && declare_single (outer, "Java", "UTF-16",
		       outer->quality_variable_to_ucs2,
		       NULL, transform_java_utf16);
}

void
delmodule_java (_GL_UNUSED RECODE_OUTER outer)
{
}

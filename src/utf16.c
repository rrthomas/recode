/* Conversion of files between different charsets and surfaces.
   Copyright © 1996, 97, 98, 99, 00 Free Software Foundation, Inc.
   Contributed by François Pinard <pinard@iro.umontreal.ca>, 1996.

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
transform_ucs4_utf16 (RECODE_SUBTASK subtask)
{
  unsigned value;

  if (get_ucs4 (&value, subtask))
    {
      if (subtask->task->byte_order_mark)
        put_ucs2 (BYTE_ORDER_MARK, subtask);

      while (true)
        {
          if (value & ~BIT_MASK (16))
            if (value < (1 << 16 | 1 << 20))
              {
                /* Double UCS-2 character.  */

                value -= 1 << 16;
                put_ucs2 (0xD800 | (BIT_MASK (10) & value >> 10), subtask);
                put_ucs2 (0xDC00 | (BIT_MASK (10) & value), subtask);
              }
            else
              {
                RETURN_IF_NOGO (RECODE_UNTRANSLATABLE, subtask);
                put_ucs2 (REPLACEMENT_CHARACTER, subtask);
              }
          else
            {
              /* Single UCS-2 character.  */

              if (value >= 0xD800 && value < 0xE000)
                RETURN_IF_NOGO (RECODE_AMBIGUOUS_OUTPUT, subtask);
              put_ucs2 (value, subtask);
            }

          if (!get_ucs4 (&value, subtask))
            break;
        }
    }

  SUBTASK_RETURN (subtask);
}

static bool
transform_utf16_ucs4 (RECODE_SUBTASK subtask)
{
  unsigned value;

  if (get_ucs2 (&value, subtask))
    {
      while (true)
        if (value >= 0xD800 && value < 0xE000)
          if (value < 0xDC00)
            {
              unsigned chunk;

              if (!get_ucs2 (&chunk, subtask))
                break;

              if (chunk >= 0xDC00 && chunk < 0xE000)
                {
                  put_ucs4 ((((1 << 16) + ((value - 0xD800) << 10))
                             | (chunk - 0xDC00)),
                            subtask);
                  if (!get_ucs2 (&value, subtask))
                    break;
                }
              else
                {
                  /* Discard the first chunk if the pair is invalid.  */

                  RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
                  value = chunk;
                }
            }
          else
            {
              /* Discard a second chunk when presented first.  */

              RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
              if (!get_ucs2 (&value, subtask))
                break;
            }
        else
          {
            put_ucs4 (value, subtask);
            if (!get_ucs2 (&value, subtask))
              break;
          }
    }

  SUBTASK_RETURN (subtask);
}

static bool
transform_ucs2_utf16 (RECODE_SUBTASK subtask)
{
  unsigned value;

  /* This function does nothing, besides checking that the number of input
     bytes is even, and that special UTF-16 values do not appear.  */

  while (get_ucs2 (&value, subtask))
    {
      if (value >= 0xD800 && value < 0xE000)
        RETURN_IF_NOGO (RECODE_AMBIGUOUS_OUTPUT, subtask);
      put_ucs2 (value, subtask);
    }

  SUBTASK_RETURN (subtask);
}

static bool
transform_utf16_ucs2 (RECODE_SUBTASK subtask)
{
  unsigned value;

  /* This function does almost nothing, besides checking that the input is
     valid UTF-16, and replacing UTF-16 extended values with the replacement
     character.  */

  if (get_ucs2 (&value, subtask))
    {
      if (subtask->task->byte_order_mark)
        put_ucs2 (BYTE_ORDER_MARK, subtask);

      while (true)
        if (value >= 0xD800 && value < 0xE000)
          if (value < 0xDC00)
            {
              unsigned chunk;

              if (!get_ucs2 (&chunk, subtask))
                break;

              if (chunk >= 0xDC00 && chunk < 0xE000)
                {
                  RETURN_IF_NOGO (RECODE_UNTRANSLATABLE, subtask);
                  put_ucs2 (REPLACEMENT_CHARACTER, subtask);
                  if (!get_ucs2 (&value, subtask))
                    break;
                }
              else
                {
                  /* Discard the first chunk if the pair is invalid.  */

                  RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
                  value = chunk;
                }
            }
          else
            {
              /* Discard a second chunk when presented first.  */

              RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
              if (!get_ucs2 (&value, subtask))
                break;
            }
        else
          {
            put_ucs2 (value, subtask);
            if (!get_ucs2 (&value, subtask))
              break;
          }
    }

  SUBTASK_RETURN (subtask);
}

bool
module_utf16 (RECODE_OUTER outer)
{
  return
    declare_single (outer, "ISO-10646-UCS-4", "UTF-16",
                    outer->quality_variable_to_variable,
                    NULL, transform_ucs4_utf16)
    && declare_single (outer, "UTF-16", "ISO-10646-UCS-4",
                       outer->quality_variable_to_variable,
                       NULL, transform_utf16_ucs4)
    && declare_single (outer, "ISO-10646-UCS-2", "UTF-16",
                       outer->quality_variable_to_variable,
                       NULL, transform_ucs2_utf16)
    && declare_single (outer, "UTF-16", "ISO-10646-UCS-2",
                       outer->quality_variable_to_variable,
                       NULL, transform_utf16_ucs2)

    && declare_alias (outer, "Unicode", "UTF-16")
    && declare_alias (outer, "TF-16", "UTF-16")
    && declare_alias (outer, "u6", "UTF-16");
}

void
delmodule_utf16 (_GL_UNUSED RECODE_OUTER outer)
{
}

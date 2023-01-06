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

static const bool bitnet_flag = false;

/* Maximum number of characters per MIME line.  */
#define MIME_LINE_LENGTH 76

/* Characters from 33 to 60 inclusive and from 62 to 126 inclusive do not
   need be quoted.  */
static const char safe_char_usual[1 << 7] =
{
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*   0- 15 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*  16- 31 */
  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*  32- 47 */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, /*  48- 63 */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*  64- 79 */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*  80- 95 */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*  96-111 */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0  /* 112-127 */
};

/* In a Bitnet environment, also quote: ! " # $ @ [ \ ] ^ ` { | } ~ */
static const char safe_char_bitnet[1 << 7] =
{
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*   0- 15 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*  16- 31 */
  0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*  32- 47 */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, /*  48- 63 */
  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*  64- 79 */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, /*  80- 95 */
  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*  96-111 */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0  /* 112-127 */
};

#define PUT_QUOTED(Character) \
  do {                                                          \
    unsigned nibble;                                                    \
                                                                \
    put_byte ('=', subtask);                                    \
    nibble = BIT_MASK (4) & (Character) >> 4;                   \
    put_byte ((nibble < 10 ? '0' : 'A' - 10) + nibble, subtask);        \
    nibble = BIT_MASK (4) & (Character);                                \
    put_byte ((nibble < 10 ? '0' : 'A' - 10) + nibble, subtask);        \
  } while (false)

static bool
transform_data_quoted_printable (RECODE_SUBTASK subtask)
{
  const char *safe_char = bitnet_flag ? safe_char_bitnet : safe_char_usual;
  unsigned available = MIME_LINE_LENGTH;
  int character = get_byte (subtask);
  int next_character;

  /* Proper maximum filling of quoted-printable lines, avoiding a line buffer
     nor much look-ahead, is a bit trickier than I initially expected.  */

  while (character != EOF)
    if (!(character & (1 << 7)) && safe_char[character])

      /* Case of a safe character.  */

      if (available > 1)
        {
          put_byte (character, subtask);
          available--;
          character = get_byte (subtask);
        }
      else
        {
          next_character = get_byte (subtask);
          if (next_character == '\n')
            {
              put_byte (character, subtask);
              put_byte ('\n', subtask);
              available = MIME_LINE_LENGTH;
              character = get_byte (subtask);
            }
          else if (next_character == EOF)
            {
              put_byte (character, subtask);
              available--;
              character = EOF;
            }
          else
            {
              put_byte ('=', subtask);
              put_byte ('\n', subtask);
              put_byte (character, subtask);
              available = MIME_LINE_LENGTH - 1;
              character = next_character;
            }
        }
    else
      switch (character)
        {
        case '\n':

          /* Case of a newline.  */

          put_byte ('\n', subtask);
          available = MIME_LINE_LENGTH;
          character = get_byte (subtask);
          break;

        case '\t':
        case ' ':

          /* Case of a space character.  */

          next_character = get_byte (subtask);
          if (next_character == '\n')
            {
              if (available < 3)
                {
                  put_byte ('=', subtask);
                  put_byte ('\n', subtask);
                }
              PUT_QUOTED (character);
              put_byte ('\n', subtask);
              available = MIME_LINE_LENGTH;
              character = get_byte (subtask);
            }
          else if (next_character == EOF)
            {
              if (available < 3)
                {
                  put_byte ('=', subtask);
                  put_byte ('\n', subtask);
                  available = MIME_LINE_LENGTH;
                }
              PUT_QUOTED (character);
              available--;
              character = EOF;
            }
          else
            {
              if (available == 1)
                {
                  put_byte ('=', subtask);
                  put_byte ('\n', subtask);
                  available = MIME_LINE_LENGTH;
                }
              put_byte (character, subtask);
              available--;
              character = next_character;
            }
          break;

        default:

          /* Case of an inconditional quotable character.  */

          if (available > 3)
            {
              PUT_QUOTED (character);
              available -= 3;
              character = get_byte (subtask);
            }
          else
            {
              next_character = get_byte (subtask);
              if (available == 3 && next_character == '\n')
                {
                  PUT_QUOTED (character);
                  put_byte ('\n', subtask);
                  available = MIME_LINE_LENGTH;
                  character = get_byte (subtask);
                }
              else if (next_character == EOF)
                {
                  PUT_QUOTED (character);
                  available--;
                  character = EOF;
                }
              else
                {
                  put_byte ('=', subtask);
                  put_byte ('\n', subtask);
                  PUT_QUOTED (character);
                  available = MIME_LINE_LENGTH - 3;
                  character = next_character;
                }
            }
        }

  if (available != MIME_LINE_LENGTH)
    {
      /* The last line was incomplete.  */

      put_byte ('=', subtask);
      put_byte ('\n', subtask);
    }

  SUBTASK_RETURN (subtask);
}

static bool
transform_quoted_printable_data (RECODE_SUBTASK subtask)
{
  const char *safe_char = bitnet_flag ? safe_char_bitnet : safe_char_usual;
  unsigned counter = 0;
  int character = get_byte (subtask);
  char buffer[MIME_LINE_LENGTH + 1];
  char *cursor;
  unsigned value;

  while (character != EOF)
    switch (character)
      {
      case '\n':
        /* Process hard line break.  */

        if (counter > MIME_LINE_LENGTH)
          RETURN_IF_NOGO (RECODE_NOT_CANONICAL, subtask);
        counter = 0;
        put_byte ('\n', subtask);
        character = get_byte (subtask);
        break;

      case ' ':
      case '\t':
        /* Process white space.  */

        cursor = buffer;
        while (character == ' ' || character == '\t')
          {
            if (cursor == buffer + MIME_LINE_LENGTH)
              {
                RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
                for (cursor = buffer;
                     cursor < buffer + MIME_LINE_LENGTH;
                     cursor++)
                  put_byte (*cursor, subtask);
              }
            counter++;
            *cursor++ = character;
            character = get_byte (subtask);
          }
        if (character == '\n' || character == EOF)
          {
            RETURN_IF_NOGO (RECODE_NOT_CANONICAL, subtask);
            counter = 0;
            break;
          }
        *cursor = '\0';
        for (cursor = buffer; *cursor; cursor++)
          put_byte (*cursor, subtask);
        break;

      case '=':
        counter++;
        character = get_byte (subtask);
        if (character == ' ' || character == '\t' || character == '\n')
          {
            /* Process soft line break.  */

            if (character == ' ' || character == '\t')
              {
                RETURN_IF_NOGO (RECODE_NOT_CANONICAL, subtask);
                while (character == ' ' || character == '\t')
                  {
                    counter++;
                    character = get_byte (subtask);
                  }
              }
            if (character != '\n')
              {
                RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
                break;
              }
            if (counter > MIME_LINE_LENGTH)
              RETURN_IF_NOGO (RECODE_NOT_CANONICAL, subtask);
            counter = 0;
            character = get_byte (subtask);
            break;
          }

        /* Process quoted value.  */

        counter++;
        if (character >= '0' && character <= '9')
          value = (character - '0') << 4;
        else if (character >= 'a' && character <= 'f')
          value = (character - 'a' + 10) << 4;
        else if (character >= 'A' && character <= 'F')
          value = (character - 'A' + 10) << 4;
        else
          {
            RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
            break;
          }
        character = get_byte (subtask);
        counter++;
        if (character >= '0' && character <= '9')
          value |= character - '0';
        else if (character >= 'a' && character <= 'f')
          value |= character - 'a' + 10;
        else if (character >= 'A' && character <= 'F')
          value |= character - 'A' + 10;
        else
          {
            RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
            break;
          }
        if (!(value & (1 << 7)) && safe_char[value])
          RETURN_IF_NOGO (RECODE_NOT_CANONICAL, subtask);

        put_byte (value, subtask);
        character = get_byte (subtask);
        break;

      default:
        /* Process safe character.  */

        counter++;
        if (character & (1 << 7) || !safe_char[character])
          RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
        put_byte (character, subtask);
        character = get_byte (subtask);
      }

  if (counter != 0)
    /* Last line is not terminated.  */
    RETURN_IF_NOGO (RECODE_NOT_CANONICAL, subtask);

  SUBTASK_RETURN (subtask);
}

bool
module_quoted_printable (RECODE_OUTER outer)
{
  return
    declare_single (outer, "data", "Quoted-Printable",
                    outer->quality_variable_to_variable,
                    NULL, transform_data_quoted_printable)
    && declare_single (outer, "Quoted-Printable", "data",
                       outer->quality_variable_to_variable,
                       NULL, transform_quoted_printable_data)
    && declare_alias (outer, "quote-printable", "Quoted-Printable")
    && declare_alias (outer, "QP", "Quoted-Printable");
}

void
delmodule_quoted_printable (_GL_UNUSED RECODE_OUTER outer)
{
}

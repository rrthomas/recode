/* Conversion of files between different charsets and surfaces.
   Copyright © 1996-2018 Free Software Foundation, Inc.
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
#include "base64.h"

/* Classification of first 128 UCS-2 characters.  */

#define D 1			/* set D -- direct character */
#define O 2			/* set O -- optional direct character */
#define W 4			/* direct white space */

#define z 0
static char classification[128] =
{
  z, z, z, z, z, z, z, z, z, W,  W, z, z, W, z, z, z, z, z, z,	/*   0- 19 */
  z, z, z, z, z, z, z, z, z, z,  z, z, W, O, O, O, O, O, O, D,	/*  20- 39 */
  D, D, O, z, D, D, D, D, D, D,  D, D, D, D, D, D, D, D, D, O,	/*  40- 59 */
  O, O, O, D, O, D, D, D, D, D,  D, D, D, D, D, D, D, D, D, D, 	/*  60- 79 */
  D, D, D, D, D, D, D, D, D, D,  D, O, z, O, O, O, O, D, D, D, 	/*  80- 99 */
  D, D, D, D, D, D, D, D, D, D,  D, D, D, D, D, D, D, D, D, D, 	/* 100-119 */
  D, D, D, O, O, O, z, z					/* 120-127 */
};
#undef z

/* Should the character be directly represented in message bodies?
   Here, & (D | O | W) is implied, since these are the only flags.  */
#define IS_BODY_DIRECT(Character) \
  (IS_ASCII (Character) && classification[Character])

/* Transformation routines.  */

/* The following diagram shows the logical steps by which three UCS-2
   characters get transformed into eight Base64 characters.  It helps
   understanding shifts and masks in the transformation functions.

	 .--------+--------.  .--------+--------.  .--------+--------.
	 |aaaaaabb|bbbbcccc|  |ccdddddd|eeeeeeff|  |ffffgggg|gghhhhhh|
	 `--------+--------'  `--------+--------'  `--------+--------'
            6   2   4   4       2   6     6   2      4   4    2   6
   .--------+--------+--------+--------+--------+--------+--------+--------.
   |00aaaaaa|00bbbbbb|00cccccc|00dddddd|00eeeeee|00ffffff|00gggggg|00hhhhhh|
   `--------+--------+--------+--------+--------+--------+--------+--------'

   .--------+--------+--------+--------+--------+--------+--------+--------.
   |AAAAAAAA|BBBBBBBB|CCCCCCCC|DDDDDDDD|EEEEEEEE|FFFFFFFF|GGGGGGGG|HHHHHHHH|
   `--------+--------+--------+--------+--------+--------+--------+--------'

   The UCS-2 characters are divided into 6 bit chunks, which are then
   encoded into Base64 characters.  */

static bool
transform_utf16_utf7 (RECODE_SUBTASK subtask)
{
  unsigned value;

  if (!recode_get_ucs2 (&value, subtask))
    SUBTASK_RETURN (subtask);

  while (true)

    if (IS_BODY_DIRECT (value))
      {
	/* Copy one direct character.  */

	recode_put_byte (value, subtask);
	if (!recode_get_ucs2 (&value, subtask))
	  SUBTASK_RETURN (subtask);
      }
    else
      {
	/* Copy a string of non-direct characters.  */

	recode_put_byte ('+', subtask);

	while (!IS_BODY_DIRECT (value))
	  {
	    unsigned split;

	    /* Process first UCS-2 value of a triplet.  */

	    recode_put_byte (base64_value_to_char[BIT_MASK (6) & value >> 10], subtask);
	    recode_put_byte (base64_value_to_char[BIT_MASK (6) & value >> 4], subtask);
	    split = (value & BIT_MASK (4)) << 2;
	    if (!recode_get_ucs2 (&value, subtask))
	      {
		recode_put_byte (base64_value_to_char[split], subtask);
		SUBTASK_RETURN (subtask);
	      }

	    /* Process second UCS-2 value of a triplet.  */

	    if (IS_BODY_DIRECT (value))
	      {
		recode_put_byte (base64_value_to_char[split], subtask);
		break;
	      }
	    recode_put_byte (base64_value_to_char[split | (BIT_MASK (2) & value >> 14)],
		      subtask);
	    recode_put_byte (base64_value_to_char[BIT_MASK (6) & value >> 8], subtask);
	    recode_put_byte (base64_value_to_char[BIT_MASK (6) & value >> 2], subtask);
	    split = (value & BIT_MASK (2)) << 4;
	    if (!recode_get_ucs2 (&value, subtask))
	      {
		recode_put_byte (base64_value_to_char[split], subtask);
		SUBTASK_RETURN (subtask);
	      }

	    /* Process third UCS-2 value of a triplet.  */

	    if (IS_BODY_DIRECT (value))
	      {
		recode_put_byte (base64_value_to_char[split], subtask);
		break;
	      }
	    recode_put_byte (base64_value_to_char[split | (BIT_MASK (4) & value >> 12)],
		      subtask);
	    recode_put_byte (base64_value_to_char[BIT_MASK (6) & value >> 6], subtask);
	    recode_put_byte (base64_value_to_char[BIT_MASK (6) & value], subtask);
	    if (!recode_get_ucs2 (&value, subtask))
	      SUBTASK_RETURN (subtask);
	  }

	if (IS_BASE64 (value))
	  recode_put_byte ('-', subtask);
      }

  SUBTASK_RETURN (subtask);
}

static bool
transform_utf7_utf16 (RECODE_SUBTASK subtask)
{
  int character;
  unsigned value;
  unsigned split;

  character = recode_get_byte (subtask);

  if (character != EOF && subtask->task->byte_order_mark)
    recode_put_ucs2 (BYTE_ORDER_MARK, subtask);

  while (character != EOF)
    if (character == '+')
      {
	character = recode_get_byte (subtask);
	while (IS_BASE64 (character))
	  {
	    /* Process first byte of first quadruplet.  */

	    value = base64_char_to_value[character] << 10;
	    character = recode_get_byte (subtask);

	    /* Process second byte of first quadruplet.  */

	    if (!IS_BASE64 (character))
	      {
		RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
		break;
	      }
	    value |= base64_char_to_value[character] << 4;
	    character = recode_get_byte (subtask);

	    /* Process third byte of first quadruplet.  */

	    if (!IS_BASE64 (character))
	      {
		RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
		break;
	      }
	    split = base64_char_to_value[character];
	    value |= split >> 2;
	    if (IS_BODY_DIRECT (value))
	      RETURN_IF_NOGO (RECODE_NOT_CANONICAL, subtask);
	    recode_put_ucs2 (value, subtask);
	    character = recode_get_byte (subtask);

	    /* Process fourth byte of first quadruplet.  */

	    if (!IS_BASE64 (character))
	      {
		if (BIT_MASK (2) & split)
		  RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
		break;
	      }
	    value = ((BIT_MASK (2) & split) << 14
		     | base64_char_to_value[character] << 8);
	    character = recode_get_byte (subtask);

	    /* Process first byte of second quadruplet.  */

	    if (!IS_BASE64 (character))
	      {
		RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
		break;
	      }
	    value |= base64_char_to_value[character] << 2;
	    character = recode_get_byte (subtask);

	    /* Process second byte of second quadruplet.  */

	    if (!IS_BASE64 (character))
	      {
		RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
		break;
	      }
	    split = base64_char_to_value[character];
	    value |= split >> 4;
	    if (IS_BODY_DIRECT (value))
	      RETURN_IF_NOGO (RECODE_NOT_CANONICAL, subtask);
	    recode_put_ucs2 (value, subtask);
	    character = recode_get_byte (subtask);

	    /* Process third byte of second quadruplet.  */

	    if (!IS_BASE64 (character))
	      {
		if (BIT_MASK (4) & split)
		  RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
		break;
	      }
	    value = ((BIT_MASK (4) & split) << 12
		     | base64_char_to_value[character] << 6);
	    character = recode_get_byte (subtask);

	    /* Process fourth byte of second quadruplet.  */

	    if (!IS_BASE64 (character))
	      {
		RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
		break;
	      }
	    value |= base64_char_to_value[character];
	    if (IS_BODY_DIRECT (value))
	      RETURN_IF_NOGO (RECODE_NOT_CANONICAL, subtask);
	    recode_put_ucs2 (value, subtask);
	    character = recode_get_byte (subtask);
	  }
	if (character == '-')
	  {
	    character = recode_get_byte (subtask);
	    if (!IS_BASE64 (character))
	      RETURN_IF_NOGO (RECODE_NOT_CANONICAL, subtask);
	  }
      }
    else
      {
	if (!IS_BODY_DIRECT (character))
	  RETURN_IF_NOGO (RECODE_NOT_CANONICAL, subtask);
	recode_put_byte (NUL, subtask);
	recode_put_byte (character, subtask);
	character = recode_get_byte (subtask);
      }

  SUBTASK_RETURN (subtask);
}

bool
module_utf7 (RECODE_OUTER outer)
{
  return
    recode_declare_single (outer, "UTF-16", "UNICODE-1-1-UTF-7",	/* RFC1642 */
		    outer->quality_variable_to_variable,
		    NULL, transform_utf16_utf7)
    && recode_declare_single (outer, "UNICODE-1-1-UTF-7", "UTF-16",
		       outer->quality_variable_to_variable,
		       NULL, transform_utf7_utf16)

    && recode_declare_alias (outer, "UTF-7", "UNICODE-1-1-UTF-7")
    && recode_declare_alias (outer, "TF-7", "UNICODE-1-1-UTF-7")
    && recode_declare_alias (outer, "u7", "UNICODE-1-1-UTF-7")

    /* Simple UCS-2 does not have to go through UTF-16.  */
    && recode_declare_single (outer, "ISO-10646-UCS-2", "UNICODE-1-1-UTF-7",
		       outer->quality_variable_to_variable,
		       NULL, transform_utf16_utf7);
}

void
delmodule_utf7 (_GL_UNUSED RECODE_OUTER outer)
{
}

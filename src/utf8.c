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

/* Read next data byte and check its value, discard an illegal sequence.
   This macro is meant to be used only within the `while' loop in
   `transform_utf8_ucs[24]'.  */
#define GET_DATA_BYTE \
  character = get_byte (subtask);						\
  if (character == EOF)							\
    {									\
      RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);		\
      break;								\
    }									\
  else if ((BIT_MASK (2) << 6 & character) != 1 << 7)			\
    {									\
      RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);		\
      continue;								\
    }									\
  else

/* Read next data byte and check its value, discard an illegal sequence.
   Merge it into `value' at POSITION.  This macro is meant to be used only
   within the `while' loop in `transform_utf8_ucs[24]'.  */
#define GET_DATA_BYTE_AT(Position) \
  GET_DATA_BYTE /* ... else */ value |= (BIT_MASK (6) & character) << Position

static bool
transform_ucs2_utf8 (RECODE_SUBTASK subtask)
{
  unsigned value;

  while (get_ucs2 (&value, subtask))
    {
      if (value & ~BIT_MASK (7))
	if (value & ~BIT_MASK (11))
	  {
	    /* 3 bytes - more than 11 bits, but not more than 16.  */
	    put_byte ((BIT_MASK (3) << 5) | (BIT_MASK (6) & value >> 12), subtask);
	    put_byte ((1 << 7) | (BIT_MASK (6) & value >> 6), subtask);
	    put_byte ((1 << 7) | (BIT_MASK (6) & value), subtask);
	  }
	else
	  {
	    /* 2 bytes - more than 7 bits, but not more than 11.  */
	    put_byte ((BIT_MASK (2) << 6) | (BIT_MASK (6) & value >> 6), subtask);
	    put_byte ((1 << 7) | (BIT_MASK (6) & value), subtask);
	  }
      else
	/* 1 byte - not more than 7 bits (that is, ASCII).  */
	put_byte (value, subtask);
    }

  SUBTASK_RETURN (subtask);
}

static bool
transform_ucs4_utf8 (RECODE_SUBTASK subtask)
{
  unsigned value;

  while (get_ucs4 (&value, subtask))
    if (value & ~BIT_MASK (16))
      if (value & ~BIT_MASK (26))
	if (value & ~BIT_MASK (31))
	  {
	    RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
	  }
  	else
	  {
	    /* 6 bytes - more than 26 bits, but not more than 31.  */
	    put_byte ((BIT_MASK (6) << 2) | (BIT_MASK (6) & value >> 30), subtask);
	    put_byte ((1 << 7) | (BIT_MASK (6) & value >> 24), subtask);
	    put_byte ((1 << 7) | (BIT_MASK (6) & value >> 18), subtask);
	    put_byte ((1 << 7) | (BIT_MASK (6) & value >> 12), subtask);
	    put_byte ((1 << 7) | (BIT_MASK (6) & value >> 6), subtask);
	    put_byte ((1 << 7) | (BIT_MASK (6) & value), subtask);
	  }
      else if (value & ~BIT_MASK (21))
	{
	  /* 5 bytes - more than 21 bits, but not more than 26.  */
	  put_byte ((BIT_MASK (5) << 3) | (BIT_MASK (6) & value >> 24), subtask);
	  put_byte ((1 << 7) | (BIT_MASK (6) & value >> 18), subtask);
	  put_byte ((1 << 7) | (BIT_MASK (6) & value >> 12), subtask);
	  put_byte ((1 << 7) | (BIT_MASK (6) & value >> 6), subtask);
	  put_byte ((1 << 7) | (BIT_MASK (6) & value), subtask);
	}
      else
	{
	  /* 4 bytes - more than 16 bits, but not more than 21.  */
	  put_byte ((BIT_MASK (4) << 4) | (BIT_MASK (6) & value >> 18), subtask);
	  put_byte ((1 << 7) | (BIT_MASK (6) & value >> 12), subtask);
	  put_byte ((1 << 7) | (BIT_MASK (6) & value >> 6), subtask);
	  put_byte ((1 << 7) | (BIT_MASK (6) & value), subtask);
	}
    else if (value & ~BIT_MASK (7))
      if (value & ~BIT_MASK (11))
	{
	  /* 3 bytes - more than 11 bits, but not more than 16.  */
	  put_byte ((BIT_MASK (3) << 5) | (BIT_MASK (6) & value >> 12), subtask);
	  put_byte ((1 << 7) | (BIT_MASK (6) & value >> 6), subtask);
	  put_byte ((1 << 7) | (BIT_MASK (6) & value), subtask);
	}
      else
	{
	  /* 2 bytes - more than 7 bits, but not more than 11.  */
	  put_byte ((BIT_MASK (2) << 6) | (BIT_MASK (6) & value >> 6), subtask);
	  put_byte ((1 << 7) | (BIT_MASK (6) & value), subtask);
	}
    else
      /* 1 byte - not more than 7 bits (that is, ASCII).  */
      put_byte (value, subtask);

  SUBTASK_RETURN (subtask);
}

/* FIXME: The UTF-8 decoding algorithms do not validate that the minimum
   length surface was indeed used.  This would be necessary for ensuring
   that the recoding is exactly reversible.  In fact, this minimum length
   surface is also a requirement of UTF-8 specification.  */

static bool
transform_utf8_ucs4 (RECODE_SUBTASK subtask)
{
  int character = get_byte (subtask);
  unsigned value;

  while (character != EOF)

    /* Process one UTF-8 value.  EOF is acceptable on first byte only.  */

    if ((character & BIT_MASK (4) << 4) == BIT_MASK (4) << 4)
      if ((character & BIT_MASK (6) << 2) == BIT_MASK (6) << 2)
	if ((character & BIT_MASK (7) << 1) == BIT_MASK (7) << 1)
	  {
	    /* 7 bytes - more than 31 bits (that is, exactly 32 :-).  */
	    RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
	    character = get_byte (subtask);
	  }
	else
	  {
	    /* 6 bytes - more than 26 bits, but not more than 31.  */
	    value = (BIT_MASK (1) & character) << 30;
	    GET_DATA_BYTE_AT (24);
	    GET_DATA_BYTE_AT (18);
	    GET_DATA_BYTE_AT (12);
	    GET_DATA_BYTE_AT (6);
	    GET_DATA_BYTE_AT (0);
	    put_ucs4 (value, subtask);
	    character = get_byte (subtask);
	  }
      else if ((character & BIT_MASK (5) << 3) == BIT_MASK (5) << 3)
	{
	  /* 5 bytes - more than 21 bits, but not more than 26.  */
	  value = (BIT_MASK (2) & character) << 24;
	  GET_DATA_BYTE_AT (18);
	  GET_DATA_BYTE_AT (12);
	  GET_DATA_BYTE_AT (6);
	  GET_DATA_BYTE_AT (0);
	  put_ucs4 (value, subtask);
	  character = get_byte (subtask);
	}
      else
	{
	  /* 4 bytes - more than 16 bits, but not more than 21.  */
	  value = (BIT_MASK (3) & character) << 18;
	  GET_DATA_BYTE_AT (12);
	  GET_DATA_BYTE_AT (6);
	  GET_DATA_BYTE_AT (0);
	  put_ucs4 (value, subtask);
	  character = get_byte (subtask);
	}
    else if ((character & BIT_MASK (2) << 6) == BIT_MASK (2) << 6)
      if ((character & BIT_MASK (3) << 5) == BIT_MASK (3) << 5)
	{
	  /* 3 bytes - more than 11 bits, but not more than 16.  */
	  value = (BIT_MASK (4) & character) << 12;
	  GET_DATA_BYTE_AT (6);
	  GET_DATA_BYTE_AT (0);
	  put_ucs4 (value, subtask);
	  character = get_byte (subtask);
	}
      else
	{
	  /* 2 bytes - more than 7 bits, but not more than 11.  */
	  value = (BIT_MASK (5) & character) << 6;
	  GET_DATA_BYTE_AT (0);
	  put_ucs4 (value, subtask);
	  character = get_byte (subtask);
	}
    else if ((character & 1 << 7) == 1 << 7)
      {
	/* Valid only as a continuation byte.  */
	RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
	character = get_byte (subtask);
      }
    else
      {
	/* 1 byte - not more than 7 bits (that is, ASCII).  */
	put_ucs4 (BIT_MASK (8) & character, subtask);
	character = get_byte (subtask);
      }

  SUBTASK_RETURN (subtask);
}

bool
module_utf8 (RECODE_OUTER outer)
{
  return
    declare_single (outer, "ISO-10646-UCS-4", "UTF-8",
		    outer->quality_variable_to_variable,
		    NULL, transform_ucs4_utf8)
    && declare_single (outer, "UTF-8", "ISO-10646-UCS-4",
		       outer->quality_variable_to_variable,
		       NULL, transform_utf8_ucs4)

    && declare_alias (outer, "UTF-2", "UTF-8")
    && declare_alias (outer, "UTF-FSS", "UTF-8")
    && declare_alias (outer, "FSS_UTF", "UTF-8")
    && declare_alias (outer, "TF-8", "UTF-8")
    && declare_alias (outer, "u8", "UTF-8")

    /* Simple UCS-2 does not have to go through UTF-16.  */
    && declare_single (outer, "ISO-10646-UCS-2", "UTF-8",
		       outer->quality_variable_to_variable,
		       NULL, transform_ucs2_utf8);
}

void
delmodule_utf8 (RECODE_OUTER outer _GL_ATTRIBUTE_MAYBE_UNUSED)
{
}

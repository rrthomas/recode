/* Conversion of files between different charsets and surfaces.
   Copyright © 1993, 94, 97, 98, 99, 00 Free Software Foundation, Inc.
   Contributed by François Pinard <pinard@iro.umontreal.ca>, 1993.

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
#include "rfc1345.h"

/* This module takes care only of short RFC 1345 forms.  Module charname.c
   takes care of the full descriptive name for characters.  */

/*----------------------------------------------------------------------.
| Return an RFC 1345 short form in a CHARSET for a given UCS2 value, or |
| NULL if this value has no such known short form.		        |
`----------------------------------------------------------------------*/

_GL_ATTRIBUTE_PURE const char *
ucs2_to_rfc1345 (recode_ucs2 code)
{
  int first = 0;
  int last = TABLE_LENGTH;

  while (first < last)
    {
      int middle = (first + last) / 2;
      const struct entry *entry = &table[middle];

      if (entry->code < code)
	first = middle + 1;
      else if (entry->code > code)
	last = middle;
      else
	return entry->rfc1345;
    }

  return NULL;
}

/*---------------------------------------------------------------------.
| Return an UCS-2 value, given an RFC 1345 short form in a CHARSET, or |
| BIT_MASK (16) if the short form is unknown.                              |
`---------------------------------------------------------------------*/

static _GL_ATTRIBUTE_PURE recode_ucs2
rfc1345_to_ucs2 (const char *string)
{
  int first = 0;
  int last = TABLE_LENGTH;

  while (first < last)
    {
      int middle = (first + last) / 2;
      const struct entry *entry = &table[inverse[middle]];
      int value = strcmp (entry->rfc1345, string);

      if (value < 0)
	first = middle + 1;
      else if (value > 0)
	last = middle;
      else
	return entry->code;
    }

  return NOT_A_CHARACTER;
}

/* Steps.  */

struct local
{
  char intro;			/* RFC 1345 intro character */
};

/*-----------------------------------------------.
| Transform a whole file from UCS-2 to RFC1345.  |
`-----------------------------------------------*/

static bool
transform_ucs2_rfc1345 (RECODE_SUBTASK subtask)
{
  struct local *local = (struct local *) subtask->step->local;
  const char intro = local->intro;
  unsigned value;

  while (get_ucs2 (&value, subtask))
    if (IS_ASCII (value))
      if (value == (unsigned)intro)
	{
	  put_byte (intro, subtask);
	  put_byte (intro, subtask);
	}
      else
	put_byte (value, subtask);
    else
      {
	const char *string = ucs2_to_rfc1345 (value);

	if (!string || !string[0])
	  RETURN_IF_NOGO (RECODE_UNTRANSLATABLE, subtask);
	else if (!string[1])
	  put_byte (string[0], subtask);
	else if (!string[2])
	  {
	    put_byte (intro, subtask);
	    put_byte (string[0], subtask);
	    put_byte (string[1], subtask);
	  }
	else
	  {
	    const char *cursor = string;

	    put_byte (intro, subtask);
	    put_byte ('_', subtask);
	    while (*cursor)
	      {
		put_byte (*cursor, subtask);
		cursor++;
	      }
	    put_byte ('_', subtask);
	  }
      }

  SUBTASK_RETURN (subtask);
}

/*-----------------------------------------------.
| Transform a whole file from RFC1345 to UCS-2.  |
`-----------------------------------------------*/

static bool
transform_rfc1345_ucs2 (RECODE_SUBTASK subtask)
{
  struct local *local = (struct local *) subtask->step->local;
  const char intro = local->intro;
  int character;

  while (character = get_byte (subtask), character != EOF)

    if (character == intro)
      {
	character = get_byte (subtask);
	if (character == EOF)
	  RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);

	if (character == intro)
	  put_ucs2 (intro, subtask);
	else if (character == '_')
	  {
	    char buffer[MAX_MNEMONIC_LENGTH + 1];
	    char *cursor = buffer;

	    character = get_byte (subtask);
	    while (true)
	      if (character == EOF)
		{
		  RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
		  break;
		}
	      else if (character == '_')
		{
		  recode_ucs2 value;

		  *cursor = NUL;
		  value = rfc1345_to_ucs2 (buffer);
		  if (value == NOT_A_CHARACTER)
		    RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
		  else
		    put_ucs2 (value, subtask);
		  break;
		}
	      else if (cursor == buffer + MAX_MNEMONIC_LENGTH)
		{
		  RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
		  break;
		}
	      else
		{
		  *cursor++ = character;
		  character = get_byte (subtask);
		}
	  }
	else
	  {
	    char buffer[3];
	    recode_ucs2 value;

	    buffer[0] = character;
	    character = get_byte (subtask);
	    if (character == EOF)
	      RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
	    buffer[1] = character;
	    buffer[2] = NUL;

	    value = rfc1345_to_ucs2 (buffer);
	    if (value == NOT_A_CHARACTER)
	      RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
	    else
	      {
		if (IS_ASCII (value))
		  RETURN_IF_NOGO (RECODE_AMBIGUOUS_OUTPUT, subtask);
		put_ucs2 (value, subtask);
	      }
	  }
      }
    else
      put_ucs2 (character, subtask);

  SUBTASK_RETURN (subtask);
}

/*-----------------------.
| Initialise the steps.  |
`-----------------------*/

static bool
term_rfc1345 (RECODE_STEP step)
{
  free (step->local);
  return true;
}

static bool
init_rfc1345 (RECODE_CONST_REQUEST request,
	      RECODE_STEP step,
	      RECODE_CONST_OPTION_LIST options _GL_ATTRIBUTE_MAYBE_UNUSED)
{
  RECODE_OUTER outer = request->outer;
  struct local *local;

  if (!ALLOC (local, 1, struct local))
    return false;

  local->intro = '&';

  step->local = local;
  step->term_routine = term_rfc1345;
  return true;
}

static bool
init_ucs2_rfc1345 (RECODE_STEP step,
		   RECODE_CONST_REQUEST request,
		   RECODE_CONST_OPTION_LIST before_options,
		   RECODE_CONST_OPTION_LIST after_options)
{
  if (before_options)
    return false;

  return init_rfc1345 (request, step, after_options);
}

static bool
init_rfc1345_ucs2 (RECODE_STEP step,
		   RECODE_CONST_REQUEST request,
		   RECODE_CONST_OPTION_LIST before_options,
		   RECODE_CONST_OPTION_LIST after_options)
{
  if (after_options)
    return false;

  return init_rfc1345 (request, step, before_options);
}

/*---------------------.
| Declare the module.  |
`---------------------*/

bool
module_rfc1345 (RECODE_OUTER outer)
{
  RECODE_ALIAS alias;

  if
    (!declare_single (outer, "ISO-10646-UCS-2", "RFC1345",
                      outer->quality_variable_to_variable,
                      init_ucs2_rfc1345, transform_ucs2_rfc1345)
     || !declare_single (outer, "RFC1345", "ISO-10646-UCS-2",
                         outer->quality_variable_to_variable,
                         init_rfc1345_ucs2, transform_rfc1345_ucs2)
     || !declare_alias (outer, "1345", "RFC1345")
     || !declare_alias (outer, "mnemonic", "RFC1345"))
    return false;

  /* Aliases for obsolete built-in encodings */
  if (alias = declare_alias (outer, "Apple-Mac", "macintosh"), !alias)
    return false;
  if (!declare_implied_surface (outer, alias, outer->cr_surface))
    return false;

  return true;
}

void
delmodule_rfc1345 (RECODE_OUTER outer _GL_ATTRIBUTE_MAYBE_UNUSED)
{
}

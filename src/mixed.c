/* Conversion of files between different charsets and surfaces.
   Copyright © 1990-2023 Free Software Foundation, Inc.
   François Pinard <pinard@iro.umontreal.ca>, 1990.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 3, or (at your option) any later
   version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
   Public License for more details.

   You should have received a copy of the GNU General Public License along
   along with this program; if not, see <https://www.gnu.org/licenses/>.
*/

#include "config.h"
#include "common.h"

/*================================\
| Mixed charset file processors.  |
\================================*/

struct mixed
{
  /* File names saved from original task.  */
  const char *input_name;
  const char *output_name;

  /* Subtask, from input directly to either output or in memory buffer.  */
  struct recode_subtask subtask;

  /* In memory buffer.  */
  struct recode_read_write_text buffer;
};

static bool
open_mixed (struct mixed *mixed, RECODE_TASK task)
{
  mixed->input_name = task->input.name;
  mixed->output_name = task->output.name;
  task->byte_order_mark = false;

  /* Open both files ourselves.  */

  if (!*(task->input.name))
    task->input.file = stdin;
  else if (task->input.file = fopen (mixed->input_name, "rb"),
	   !task->input.file)
    {
      recode_perror (NULL, "fopen (%s)", task->input.name);
      return false;
    }
  task->input.name = NULL;

  if (!*(task->output.name))
    task->output.file = stdout;
  else if (task->output.file = fopen (mixed->output_name, "wb"),
	   !task->output.file)
    {
      recode_perror (NULL, "fopen (%s)", task->output.name);
      fclose (task->input.file);
      return false;
    }
  task->output.name = NULL;

  /* Prepare for dynamic plumbing copy.  */

  memset (&mixed->subtask, 0, sizeof (struct recode_subtask));
  mixed->subtask.task = task;
  mixed->subtask.input = task->input;
  mixed->subtask.output = task->output;
  memset (&mixed->buffer, 0, sizeof (struct recode_read_write_text));

  return true;
}

static bool
close_mixed (struct mixed *mixed)
{
  /* Clean up.  */

  if (mixed->subtask.task->input.file
      && fclose (mixed->subtask.task->input.file) != 0)
    return false;
  if (mixed->subtask.task->output.file
      && fclose (mixed->subtask.task->output.file) != 0)
    return false;
  return true;
}

static void
start_accumulation (struct mixed *mixed)
{
  mixed->buffer.cursor = mixed->buffer.buffer;
  mixed->subtask.output = mixed->buffer;
}

static bool
recode_accumulated (struct mixed *mixed)
{
  RECODE_TASK task = mixed->subtask.task;
  struct recode_read_only_text saved_input = task->input;
  bool status;

  mixed->buffer = mixed->subtask.output;

  task->input.file = NULL;
  task->input.buffer = mixed->buffer.buffer;
  task->input.cursor = mixed->buffer.buffer;
  task->input.limit = mixed->buffer.cursor;

  status = recode_perform_task (task);

  task->input = saved_input;
  mixed->subtask.output = task->output;
  return status;
}

/*-----------------------------------------------------------------------.
| Transform only strings or comments in an C source, expected in ASCII.  |
`-----------------------------------------------------------------------*/

bool
recode_transform_c_source (RECODE_TASK task)
{
  struct mixed mixed;
  int character;

  if (!open_mixed (&mixed, task))
    recode_if_nogo (RECODE_SYSTEM_ERROR, &mixed.subtask);
  else
    {
      character = recode_get_byte (&mixed.subtask);
      while (character != EOF)
	switch (character)
	  {
	  case '\'':
	    /* Skip character constant, while copying it untranslated.  */

	    recode_put_byte ('\'', &mixed.subtask);
	    character = recode_get_byte (&mixed.subtask);

	    if (character == EOF)
	      {
		recode_if_nogo (RECODE_SYSTEM_ERROR, &mixed.subtask);
		break;
	      }

	    if (character == '\\')
	      {
		recode_put_byte ('\\', &mixed.subtask);
		character = recode_get_byte (&mixed.subtask);
		if (character == EOF)
		  {
		    recode_if_nogo (RECODE_SYSTEM_ERROR, &mixed.subtask);
		    break;
		  }
		recode_put_byte (character, &mixed.subtask);
		character = recode_get_byte (&mixed.subtask);
	      }

	    if (character == '\'')
	      {
		recode_put_byte ('\'', &mixed.subtask);
		character = recode_get_byte (&mixed.subtask);
	      }
	    else
	      recode_if_nogo (RECODE_SYSTEM_ERROR, &mixed.subtask);
	    break;

	  case '"':
	    /* Copy the string, translated.  */

	    recode_put_byte ('"', &mixed.subtask);
	    character = recode_get_byte (&mixed.subtask);

	    /* Read in string.  */

	    start_accumulation (&mixed);

	    while (true)
	      {
		if (character == EOF)
		  {
		    recode_accumulated (&mixed);
		    recode_if_nogo (RECODE_SYSTEM_ERROR, &mixed.subtask);
		    break;
		  }
		if (character == '"')
		  break;

		if (character == '\\')
		  {
		    recode_put_byte ('\\', &mixed.subtask);
		    character = recode_get_byte (&mixed.subtask);
		    if (character == EOF)
		      {
			recode_accumulated (&mixed);
			recode_if_nogo (RECODE_SYSTEM_ERROR, &mixed.subtask);
			break;
		      }
		  }
		recode_put_byte (character, &mixed.subtask);
		character = recode_get_byte (&mixed.subtask);
	      }
	    if (character == EOF)
	      break;

	    /* Translate string and dump it.  */

	    if (!recode_accumulated (&mixed))
	      recode_if_nogo (RECODE_SYSTEM_ERROR, &mixed.subtask);
	    recode_put_byte ('"', &mixed.subtask);
	    character = recode_get_byte (&mixed.subtask);
	    break;

	  case '/':
	    recode_put_byte ('/', &mixed.subtask);
	    character = recode_get_byte (&mixed.subtask);
	    if (character == EOF)
	      break;
	    if (character == '*')
	      {
		/* Copy the comment, translated.  */

		recode_put_byte ('*', &mixed.subtask);
		character = recode_get_byte (&mixed.subtask);

		/* Read in comment.  */

		start_accumulation (&mixed);
		while (true)
		  {
		    if (character == EOF)
		      {
			recode_accumulated (&mixed);
			recode_if_nogo (RECODE_SYSTEM_ERROR, &mixed.subtask);
			break;
		      }
		    if (character == '*')
		      {
			character = recode_get_byte (&mixed.subtask);
			if (character == EOF)
			  {
			    recode_accumulated (&mixed);
			    recode_if_nogo (RECODE_SYSTEM_ERROR, &mixed.subtask);
			    recode_put_byte ('*', &mixed.subtask);
			    break;
			  }
			if (character == '/')
			  break;
			recode_put_byte ('*', &mixed.subtask);
		      }
		    else
		      {
			recode_put_byte (character, &mixed.subtask);
			character = recode_get_byte (&mixed.subtask);
		      }
		  }

		if (character == EOF)
		  break;

		/* Translate comment and dump it.  */

		if (!recode_accumulated (&mixed))
		  recode_if_nogo (RECODE_SYSTEM_ERROR, &mixed.subtask);
		recode_put_byte ('*', &mixed.subtask);
		recode_put_byte ('/', &mixed.subtask);
		character = recode_get_byte (&mixed.subtask);
	      }
	    break;

	  default:
	    recode_put_byte (character, &mixed.subtask);
	    character = recode_get_byte (&mixed.subtask);
	    break;
	  }
    }

  if (!close_mixed (&mixed))
    recode_if_nogo (RECODE_SYSTEM_ERROR, &mixed.subtask);
  SUBTASK_RETURN (&mixed.subtask);
}

/*------------------------------------------------------------------------.
| Transform only strings or comments in an PO source, expected in ASCII.  |
`------------------------------------------------------------------------*/

/* There is a limitation to -Spo: if Recode converts some `msgstr' in a way
   that might produce quotes (or backslashes), these should then be requoted.
   Doing this would then also require to fully unquote the original `msgstr'
   string.  But it seems that such a need does not occur in most cases I can
   imagine as practical, as the ASCII subset is generally invariant under
   recoding.  My guess is that we should wait for someone to report the bug
   with a real case, to believe it is worth adding the complexity.  */

bool
recode_transform_po_source (RECODE_TASK task)
{
  struct mixed mixed;
  bool recode = false;
  bool msgstr = false;

  if (!open_mixed (&mixed, task))
    recode_if_nogo (RECODE_SYSTEM_ERROR, &mixed.subtask);
  else
    {
      int character = recode_get_byte (&mixed.subtask);
      while (character != EOF)
	switch (character)
	  {
	  case '#':
	    /* Copy a comment, recoding only those written by translators.  */

	    recode_put_byte ('#', &mixed.subtask);
	    character = recode_get_byte (&mixed.subtask);
	    if (character == EOF)
	      break;
	    recode = character == ' ' || character == '\t';

	    if (recode)
	      start_accumulation (&mixed);

	    while (character != '\n' && character != EOF)
	      {
		recode_put_byte (character, &mixed.subtask);
		character = recode_get_byte (&mixed.subtask);
	      }

	    if (recode && !recode_accumulated (&mixed))
	      recode_if_nogo (RECODE_SYSTEM_ERROR, &mixed.subtask);

	    if (character == EOF)
	      break;
	    recode_put_byte ('\n', &mixed.subtask);
	    character = recode_get_byte (&mixed.subtask);
	    break;

	  case 'm':
	    /* Attempt to recognise `msgstr'.  */

	    msgstr = false;

	    recode_put_byte ('m', &mixed.subtask);
	    character = recode_get_byte (&mixed.subtask);
	    if (character != 's')
	      break;
	    recode_put_byte ('s', &mixed.subtask);
	    character = recode_get_byte (&mixed.subtask);
	    if (character != 'g')
	      break;
	    recode_put_byte ('g', &mixed.subtask);
	    character = recode_get_byte (&mixed.subtask);
	    if (character != 's')
	      break;
	    recode_put_byte ('s', &mixed.subtask);
	    character = recode_get_byte (&mixed.subtask);
	    if (character != 't')
	      break;
	    recode_put_byte ('t', &mixed.subtask);
	    character = recode_get_byte (&mixed.subtask);
	    if (character != 'r')
	      break;
	    recode_put_byte ('r', &mixed.subtask);
	    character = recode_get_byte (&mixed.subtask);

	    msgstr = true;
	    break;

	  case '"':
	    /* Copy the string, translating only the `msgstr' ones.  */

	    recode_put_byte ('"', &mixed.subtask);
	    character = recode_get_byte (&mixed.subtask);
	    recode = msgstr;

	    if (recode)
	      start_accumulation (&mixed);

	    while (true)
	      {
		if (character == EOF)
		  {
		    if (recode)
		      {
			recode_accumulated (&mixed);
			recode_if_nogo (RECODE_SYSTEM_ERROR, &mixed.subtask);
		      }
		    break;
		  }
		if (character == '"')
		  break;

		if (character == '\\')
		  {
		    recode_put_byte ('\\', &mixed.subtask);
		    character = recode_get_byte (&mixed.subtask);
		    if (character == EOF)
		      {
			if (recode)
			  {
			    recode_accumulated (&mixed);
			    recode_if_nogo (RECODE_SYSTEM_ERROR, &mixed.subtask);
			  }
			break;
		      }
		  }
		recode_put_byte (character, &mixed.subtask);
		character = recode_get_byte (&mixed.subtask);
	      }

	    if (character == EOF)
	      break;

	    if (recode && !recode_accumulated (&mixed))
	      recode_if_nogo (RECODE_SYSTEM_ERROR, &mixed.subtask);

	    recode_put_byte ('"', &mixed.subtask);
	    character = recode_get_byte (&mixed.subtask);
	    break;

	  default:
	    recode_put_byte (character, &mixed.subtask);
	    character = recode_get_byte (&mixed.subtask);
	    break;
	  }
    }

  if (!close_mixed (&mixed))
    recode_if_nogo (RECODE_SYSTEM_ERROR, &mixed.subtask);
  SUBTASK_RETURN (&mixed.subtask);
}

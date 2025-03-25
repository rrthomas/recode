/* Conversion of files between different charsets and surfaces.
   Copyright © 1990-2023 Free Software Foundation, Inc.
   Contributed by François Pinard <pinard@iro.umontreal.ca>, 1990.

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

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "minmax.h"
#include "xbinary-io.h"

bool recode_interrupted = 0;	/* set by signal handler when some signal has been received */


/* Input and output helpers.  */

/*-------------------------------------------------------------------.
| Read one byte from the input text of TASK, or EOF if none remain.  |
`-------------------------------------------------------------------*/

int
recode_get_byte (RECODE_SUBTASK subtask)
{
  if (subtask->input.file)
    return getc (subtask->input.file);
  else if (subtask->input.cursor == subtask->input.limit)
    return EOF;
  else
    return (unsigned char) *subtask->input.cursor++;
}

/*-------------------------------------------------------------------.
| Read N bytes from the input text of TASK into DATA; return number  |
| of bytes read.                                                     |
`-------------------------------------------------------------------*/

size_t
recode_get_bytes (RECODE_SUBTASK subtask, char *data, size_t n)
{
  if (subtask->input.file)
    return fread (data, 1, n, subtask->input.file);
  else
    {
      size_t bytes_left = subtask->output.limit - subtask->output.cursor;
      size_t bytes_to_copy = MIN (n, bytes_left);
      memcpy (data, subtask->output.cursor, bytes_to_copy);
      subtask->output.cursor += bytes_to_copy;
      return bytes_to_copy;
    }
}

/*-----------------------------------------.
| Write BYTE on the output text for TASK.  |
`-----------------------------------------*/

void
recode_put_byte (char byte, RECODE_SUBTASK subtask)
{
  if (subtask->output.file)
    {
      if (putc (byte, subtask->output.file) == EOF)
        recode_if_nogo (RECODE_SYSTEM_ERROR, subtask);
    }
  else if (subtask->output.cursor == subtask->output.limit)
    recode_put_bytes (&byte, 1, subtask);
  else
    *subtask->output.cursor++ = byte;
}

/*-----------------------------------------------------.
| Write N bytes of DATA on the output text for TASK.   |
`-----------------------------------------------------*/

void
recode_put_bytes (const char *data, size_t n, RECODE_SUBTASK subtask)
{
  if (subtask->output.file)
    {
      if (fwrite (data, n, 1, subtask->output.file) != 1)
        {
          recode_perror (NULL, "fwrite ()");
          recode_if_nogo (RECODE_SYSTEM_ERROR, subtask);
        }
    }
  else {
    if (subtask->output.cursor + n > subtask->output.limit)
      {
        RECODE_OUTER outer = subtask->task->request->outer;
        size_t old_size = subtask->output.limit - subtask->output.buffer;
        size_t new_size = old_size * 3 / 2 + 40 + n;

        if (REALLOC (subtask->output.buffer, new_size, char))
          {
            subtask->output.cursor = subtask->output.buffer + old_size;
            subtask->output.limit = subtask->output.buffer + new_size;
          }
        else
          {
            recode_if_nogo (RECODE_SYSTEM_ERROR, subtask);
            return;
          }
      }
    memcpy (subtask->output.cursor, data, n);
    subtask->output.cursor += n;
  }
}

/* Error processing.  */

/*------------------------------------------------------------------------.
| Handle a given ERROR, while executing STEP within TASK.  Return true if |
| the abort level has been reached.                                       |
`------------------------------------------------------------------------*/

bool
recode_if_nogo (enum recode_error new_error, RECODE_SUBTASK subtask)
{
  RECODE_TASK task = subtask->task;

  if (new_error > task->error_so_far)
    {
      task->error_so_far = new_error;
      task->error_at_step = subtask->step;
    }
  return task->error_so_far >= task->abort_level;
}

/* Recoding execution control.  */

/*--------------.
| Copy a file.  |
`--------------*/

static void
transform_mere_copy (RECODE_SUBTASK subtask)
{
  if (subtask->input.file)
    {
      /* Reading from file.  */

      char buffer[BUFSIZ];
      size_t size;

      while (size = recode_get_bytes (subtask, buffer, BUFSIZ),
	     size == BUFSIZ)
	recode_put_bytes (buffer, BUFSIZ, subtask);
      if (size > 0)
	recode_put_bytes (buffer, size, subtask);
    }
  else
    /* Reading from buffer.  */

    if (subtask->input.cursor < subtask->input.limit)
      recode_put_bytes (subtask->input.cursor,
                 (unsigned) (subtask->input.limit - subtask->input.cursor),
                 subtask);
}

/*--------------------------------------------------.
| Recode a file using a one-to-one recoding table.  |
`--------------------------------------------------*/

bool
recode_transform_byte_to_byte (RECODE_SUBTASK subtask)
{
  unsigned const char *table
    = (unsigned const char *) subtask->step->step_table;
  int input_char;

  while (input_char = recode_get_byte (subtask), input_char != EOF)
    recode_put_byte (table[input_char], subtask);

  SUBTASK_RETURN (subtask);
}

/*---------------------------------------------------.
| Recode a file using a one-to-many recoding table.  |
`---------------------------------------------------*/

bool
recode_transform_byte_to_variable (RECODE_SUBTASK subtask)
{
  const char *const *table = (const char *const *) subtask->step->step_table;
  int input_char;
  const char *output_string;

  /* Copy the file through the one to many recoding table.  */

  while (input_char = recode_get_byte (subtask), input_char != EOF)
    if (output_string = table[input_char], output_string)
      while (*output_string)
	{
	  recode_put_byte (*output_string, subtask);
	  output_string++;
	}
   else
     RETURN_IF_NOGO (RECODE_UNTRANSLATABLE, subtask);

  SUBTASK_RETURN (subtask);
}

/*------------------------------------------------------------------------.
| Execute the conversion sequence for a recoding TASK.  If no conversions |
| are needed, merely copy the input onto the output.                      |
| Returns zero if the recoding has been found to be non-reversible.       |
| Tell what goes on if VERBOSE.                                           |
`------------------------------------------------------------------------*/

bool
recode_perform_task (RECODE_TASK task)
{
  RECODE_CONST_REQUEST request = task->request;
  struct recode_subtask subtask_block;
  RECODE_SUBTASK subtask = &subtask_block;

  struct recode_read_write_text input;
  struct recode_read_write_text output;
  memset (&input, 0, sizeof (struct recode_read_write_text));
  memset (&output, 0, sizeof (struct recode_read_write_text));

  memset (subtask, 0, sizeof (struct recode_subtask));
  subtask->task = task;
  subtask->input = task->input;

  /* Switch stdin and stdout to binary mode unless they are ttys, as this has
     nasty side-effects on several DOSish systems.  For example, the Ctrl-Z
     character is no longer interpreted as EOF, and thus the poor user cannot
     signal end of input; the INTR character also doesn't work, so they cannot
     even interrupt the program, and are stuck.  On the other hand, output to
     the screen doesn't have to follow the end-of-line format exactly, since
     it is going to be discarded anyway.  */
  if (task->input.name && !*task->input.name && !isatty (fileno (stdin)))
    xset_binary_mode (fileno (stdin), O_BINARY);
  if (task->output.name && !*task->output.name && !isatty (fileno (stdout)))
    xset_binary_mode (fileno (stdout), O_BINARY);

  /* Prepare the first input file.  */

  if (subtask->input.name)
    {
      if (!*subtask->input.name)
	subtask->input.file = stdin;
      else if (subtask->input.file = fopen (subtask->input.name, "rb"),
	       subtask->input.file == NULL)
	{
	  recode_perror (NULL, "fopen (%s)", subtask->input.name);
	  recode_if_nogo (RECODE_SYSTEM_ERROR, subtask);
	  SUBTASK_RETURN (subtask);
	}
    }

  /* Execute one pass for each step of the sequence.  */

  int child_process = -1;	/* child process number, -1 if process has not forked */
  for (unsigned sequence_index = 0;
       task->error_so_far < task->abort_level;
       sequence_index++)
    {
      child_process = -1;

      if (sequence_index > 0)
        {
          /* Select the input text for this step.  */

          subtask->input.buffer = input.buffer;
          subtask->input.cursor = input.buffer;
          subtask->input.limit = input.cursor;
          subtask->input.file = input.file;
        }

      /* Select the output text for this step.  */

      if (sequence_index + 1 < (unsigned)request->sequence_length)
	{
          subtask->output = output;
          subtask->output.cursor = subtask->output.buffer;
	}
      else
	{
	  /* Prepare the final output file.  */

	  subtask->output = task->output;
	  if (subtask->output.name)
	    {
	      if (!*subtask->output.name)
		subtask->output.file = stdout;
	      else if (subtask->output.file = fopen (subtask->output.name, "wb"),
		       subtask->output.file == NULL)
		{
		  recode_perror (NULL, "fopen (%s)", subtask->output.name);
		  recode_if_nogo (RECODE_SYSTEM_ERROR, subtask);
		  goto exit;
		}
	    }
	}

      /* Execute one recoding step.  */

      if (request->sequence_length == 0) {
	transform_mere_copy (subtask);
	break;
      }

      if (child_process <= 0)
	{
	  subtask->step = request->sequence_array + sequence_index;
	  (*subtask->step->transform_routine) (subtask);

	  /* Post-step clean up for memory sequence.  */

	  if (subtask->input.file)
	    {
	      FILE *fp = subtask->input.file;

	      subtask->input.file = NULL;
	      if (fclose (fp) != 0)
		{
		  recode_perror (NULL, "fclose (%s)", subtask->input.name);
		  recode_if_nogo (RECODE_SYSTEM_ERROR, subtask);
		  goto exit;
		}
	    }

	  /* Prepare for next step.  */

	  task->swap_input = RECODE_SWAP_UNDECIDED;

	  if (sequence_index + 1 < (unsigned)request->sequence_length)
	    {
	      output = input;
	      input = subtask->output;
	    }
	}

      if (sequence_index + 1 == (unsigned)request->sequence_length)
	break;
    }

  /* Final clean up.  */
 exit:

  if (subtask->input.file && subtask->input.file != task->input.file && fclose (subtask->input.file) != 0)
    {
      recode_perror (NULL, "fclose (%s)", subtask->input.name ? subtask->input.name : "stdin");
      recode_if_nogo (RECODE_SYSTEM_ERROR, subtask);
    }

  if (subtask->output.file && subtask->output.file != task->output.file && fclose (subtask->output.file) != 0)
    {
      recode_perror (NULL, "fclose (%s)", subtask->output.name ? subtask->output.name : "stdout");
      recode_if_nogo (RECODE_SYSTEM_ERROR, subtask);
    }

  free (input.buffer);
  free (output.buffer);

  task->output = subtask->output;
  SUBTASK_RETURN (subtask);
}

/* Library interface.  */

/* See the recode manual for a more detailed description of the library
   interface.  */

/*-----------------------.
| TASK level functions.  |
`-----------------------*/

RECODE_TASK
recode_new_task (RECODE_CONST_REQUEST request)
{
  RECODE_OUTER outer = request->outer;
  RECODE_TASK task;

  if (!ALLOC (task, 1, struct recode_task))
    return NULL;

  task->request = request;
  task->fail_level = RECODE_NOT_CANONICAL;
  task->abort_level = RECODE_USER_ERROR;
  task->error_so_far = RECODE_NO_ERROR;
  task->swap_input = RECODE_SWAP_UNDECIDED;
  task->byte_order_mark = true;

  return task;
}

bool
recode_delete_task (RECODE_TASK task)
{
  free (task);
  return true;
}

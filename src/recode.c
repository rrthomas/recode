/* Conversion of files between different charsets and surfaces.
   Copyright © 1990,92,93,94,96,97,98,99,00 Free Software Foundation, Inc.
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

/* Global declarations and definitions.  */

#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

#include "hash.h"

extern const char *program_name;

/* Error handling.  */

#include <stdarg.h>

_GL_ATTRIBUTE_FORMAT_PRINTF_SYSTEM (2, 3) void
recode_error (_GL_UNUSED RECODE_OUTER outer, const char *format, ...)
{
  va_list args;

  va_start (args, format);
  vfprintf (stderr, format, args);
  va_end (args);
  putc ('\n', stderr);
  fflush (stderr);
}

_GL_ATTRIBUTE_FORMAT_PRINTF_SYSTEM (2, 3) void
recode_perror (_GL_UNUSED RECODE_OUTER outer, const char *format, ...)
{
  int saved_errno = errno;
  va_list args;

  va_start (args, format);
  vfprintf (stderr, format, args);
  va_end (args);
  fprintf (stderr, ": %s\n", strerror (saved_errno));
  fflush (stderr);
}

_GL_ATTRIBUTE_MALLOC void *
recode_malloc (RECODE_OUTER outer, size_t size)
{
  void *result;

  result = calloc (1, size);
  if (!result)
    recode_error (outer, _("Virtual memory exhausted"));

  return result;
}

void *
recode_realloc (RECODE_OUTER outer, void *pointer, size_t size)
{
  void *result;

  result = realloc (pointer, size);
  if (!result)
    recode_error (outer, _("Virtual memory exhausted"));

  return result;
}

/* Single step handling.  */

/*------------------------------------------------------------------.
| Create a one to one table which is the inverse of the given one.  |
`------------------------------------------------------------------*/

unsigned char *
invert_table (RECODE_OUTER outer, const unsigned char *table)
{
  unsigned char flag[256];
  unsigned char *result;
  bool table_error;
  unsigned counter;

  if (!ALLOC (result, 256, unsigned char))
    return NULL;
  memset (flag, 0, 256);
  table_error = false;

  for (counter = 0; counter < 256; counter++)
    {
      if (flag[table[counter]])
	{
	  recode_error (outer, _("Codes %3d and %3u both recode to %3d"),
			result[table[counter]], counter, table[counter]);
	  table_error = true;
	}
      else
	{
	  result[table[counter]] = counter;
	  flag[table[counter]] = 1;
	}
    }
  if (table_error)
    {
      for (counter = 0; counter < 256; counter++)
	if (!flag[counter])
	  recode_error (outer, _("No character recodes to %3u"), counter);
      recode_error (outer, _("Cannot invert given one-to-one table"));
    }
  return result;
}

/*---------------------------------------------------------------------------.
| Complete a STEP descriptor by a constructed recoding array for 256 chars   |
| and the adequate recoding routine.  Use a KNOWN_PAIRS array of             |
| NUMBER_OF_PAIRS constraints.  If FIRST_HALF_IMPLIED is not zero, default   |
| the unconstrained characters of the first 128 to the identity mapping.  If |
| REVERSE is not zero, use right_table instead of left_table to complete the |
| table, yet new pairs are created only when fallback is reversibility.      |
`---------------------------------------------------------------------------*/

bool
complete_pairs (RECODE_OUTER outer, RECODE_STEP step,
		const struct recode_known_pair *known_pairs,
		unsigned number_of_pairs, bool first_half_implied, bool reverse)
{
  unsigned char left_flag[256];
  unsigned char right_flag[256];
  unsigned char left_table[256];
  unsigned char right_table[256];
  bool table_error;

  unsigned char *flag;
  unsigned char *table;
  const char **table2;
  char *cursor;
  unsigned char left;
  unsigned char right;
  unsigned char search;
  unsigned counter;
  unsigned used;

  /* Init tables with zeroes.  */

  memset (left_flag, 0, 256);
  memset (right_flag, 0, 256);
  memset (left_table, 0, 256);
  memset (right_table, 0, 256);
  table_error = false;

  /* Establish known data.  */

  for (counter = 0; counter < number_of_pairs; counter++)
    {
      left = known_pairs[counter].left;
      right = known_pairs[counter].right;

      /* Set one known correspondence.  */

      if (left_flag[left])
	{
	  if (!table_error)
	    {
	      recode_error (outer, _("Following diagnostics for `%s' to `%s'"),
			    step->before->name, step->after->name);
	      table_error = true;
	    }
	  recode_error (outer,
			_("Pair no. %u: <%3d, %3d> conflicts with <%3d, %3d>"),
			counter, left, right, left, left_table[left]);
	}
      else if (right_flag[right])
	{
	  if (!table_error)
	    {
	      recode_error (outer, _("Following diagnostics for `%s' to `%s'"),
			   step->before->name, step->after->name);
	      table_error = true;
	    }
	  recode_error (outer,
			_("Pair no. %u: <%3d, %3d> conflicts with <%3d, %3d>"),
			counter, left, right, right_table[right], right);
	}
      else
	{
	  left_flag[left] = 1;
	  left_table[left] = right;
	  right_flag[right] = 1;
	  right_table[right] = left;
	}
    }

  /* Set all the implied correspondances.  */

  if (first_half_implied)
    for (counter = 0; counter < 128; counter++)
      if (!left_flag[counter] && !right_flag[counter])
	{
	  left_flag[counter] = 1;
	  left_table[counter] = counter;
	  right_flag[counter] = 1;
	  right_table[counter] = counter;
	}

  if (step->fallback_routine == reversibility)
    {
      /* If the recoding is not strict, compute a reversible one to one
	 table.  */

      if (table_error)
	recode_error (outer,
		      _("Cannot complete table from set of known pairs"));

      /* Close the table with small permutation cycles.  */

      for (counter = 0; counter < 256; counter++)
	if (!right_flag[counter])
	  {
	    search = counter;
	    while (left_flag[search])
	      search = left_table[search];
	    left_flag[search] = 1;
	    left_table[search] = counter;
	    right_flag[counter] = 1;
	    right_table[counter] = search;
	  }

      /* Save a copy of the proper table.  */

      step->transform_routine = transform_byte_to_byte;
      if (!ALLOC (table, 256, unsigned char))
	return false;
      memcpy (table, reverse ? right_table : left_table, 256);
      step->step_type = RECODE_BYTE_TO_BYTE;
      step->step_table = table;
      step->step_table_term_routine = free;

      /* Upgrade step quality to reversible.  */

      step->quality = outer->quality_byte_reversible;
    }
  else
    {
      /* If the recoding is strict, prepare a one to many table, each
	 entry being NULL or a string of a single character.  */

      /* Select the proper table.  */

      if (reverse)
	{
	  flag = right_flag;
	  table = right_table;
	}
      else
	{
	  flag = left_flag;
	  table = left_table;
	}

      /* Allocate everything in one blow, so it will be freed likewise.  */

      used = 0;
      for (counter = 0; counter < 256; counter++)
	if (flag[counter])
	  used++;

      if (!ALLOC_SIZE (table2, 256 * sizeof (char *) + 2 * used, const char *))
	return false;
      cursor = (char *) (table2 + 256);

      /* Construct the table and the strings in parallel.  */

      for (counter = 0; counter < 256; counter++)
	if (flag[counter])
	  {
	    table2[counter] = cursor;
	    *cursor++ = table[counter];
	    *cursor++ = NUL;
	  }
	else
	  table2[counter] = NULL;

      /* Save a one to many recoding table.  */

      step->transform_routine = transform_byte_to_variable;
      step->step_type = RECODE_BYTE_TO_STRING;
      step->step_table = table2;
      step->step_table_term_routine = free;
    }

  return true;
}

/* Special handling for UCS-2 tables.  */

/*-------------------------------------------------------------------------.
| Recode a file from one byte characters to double byte UCS-2 characters.  |
`-------------------------------------------------------------------------*/

bool
transform_byte_to_ucs2 (RECODE_SUBTASK subtask)
{
  int input_char;		/* current character */
  int output_value;		/* value being output */

  if (input_char = get_byte (subtask), input_char != EOF)
    {
      if (subtask->task->byte_order_mark)
	put_ucs2 (BYTE_ORDER_MARK, subtask);

      while (input_char != EOF)
	{
	  output_value = code_to_ucs2 (subtask->step->before, input_char);
	  if (output_value < 0)
	    {
	      RETURN_IF_NOGO (RECODE_UNTRANSLATABLE, subtask);
	      put_ucs2 (REPLACEMENT_CHARACTER, subtask);
	    }
	  else
	    put_ucs2 (output_value, subtask);

	  input_char = get_byte (subtask);
	}
    }

  SUBTASK_RETURN (subtask);
}

/*-------------------------------------------------------------------------.
| Recode a file from double byte UCS-2 characters to one byte characters.  |
`-------------------------------------------------------------------------*/

struct ucs2_to_byte
  {
    recode_ucs2 code;		/* UCS-2 value */
    unsigned char byte;		/* corresponding byte */
  };

struct ucs2_to_byte_local
  {
    Hash_table *table;
    struct ucs2_to_byte *data;
  };

static size_t
ucs2_to_byte_hash (const void *void_data, size_t table_size)
{
  const struct ucs2_to_byte *data = (const struct ucs2_to_byte *) void_data;

  return data->code % table_size;
}

static bool
ucs2_to_byte_compare (const void *void_first, const void *void_second)
{
  const struct ucs2_to_byte *first = (const struct ucs2_to_byte *) void_first;
  const struct ucs2_to_byte *second = (const struct ucs2_to_byte *) void_second;

  return first->code == second->code;
}

static bool
term_ucs2_to_byte (RECODE_STEP step)
{
  hash_free (((struct ucs2_to_byte_local *) step->local)->table);
  free (((struct ucs2_to_byte_local *) step->local)->data);
  free (step->local);
  return true;
}

bool
init_ucs2_to_byte (RECODE_STEP step,
		   RECODE_CONST_REQUEST request,
		   RECODE_CONST_OPTION_LIST before_options,
		   RECODE_CONST_OPTION_LIST after_options)
{
  RECODE_OUTER outer = request->outer;
  Hash_table *table;
  struct ucs2_to_byte *data;
  unsigned counter;

  if (before_options || after_options)
    return false;

  table = hash_initialize (0, NULL,
			   ucs2_to_byte_hash, ucs2_to_byte_compare, NULL);
  if (!table)
    return false;

  if (!ALLOC (data, 256, struct ucs2_to_byte))
    {
      hash_free (table);
      return false;
    }

  for (counter = 0; counter < 256; counter++)
    {
      data[counter].code = code_to_ucs2 (step->after, counter);
      data[counter].byte = counter;
      if (!hash_insert (table, data + counter))
	{
	  hash_free (table);
	  free (data);
	  return false;
	}
    }

  if (!ALLOC (step->local, 1, struct ucs2_to_byte_local))
    {
      hash_free (table);
      free (data);
      return false;
    }
  ((struct ucs2_to_byte_local *) step->local)->table = table;
  ((struct ucs2_to_byte_local *) step->local)->data = data;

  step->term_routine = term_ucs2_to_byte;
  return true;
}

bool
transform_ucs2_to_byte (RECODE_SUBTASK subtask)
{
  Hash_table *table = ((struct ucs2_to_byte_local *) subtask->step->local)->table;
  struct ucs2_to_byte lookup;
  struct ucs2_to_byte *entry;
  unsigned input_value;		/* current UCS-2 character */

  while (get_ucs2 (&input_value, subtask))
    {
      lookup.code = input_value;
      entry = (struct ucs2_to_byte *) hash_lookup (table, &lookup);
      if (entry)
	put_byte (entry->byte, subtask);
      else
	RETURN_IF_NOGO (RECODE_UNTRANSLATABLE, subtask);
    }

  SUBTASK_RETURN (subtask);
}

/* Table editing on stdout.  */

/*------------------------------------------------------------------------.
| Produce an include file representing the recoding, on standard output.  |
`------------------------------------------------------------------------*/

bool
recode_format_table (RECODE_REQUEST request,
		     enum recode_programming_language header_language,
		     const char *header_name)
{
  RECODE_OUTER outer = request->outer;

  RECODE_CONST_STEP step; /* step being analysed */
  unsigned column;		/* column counter */
  char *name;			/* constructed name */
  char *cursor;			/* cursor in constructed name */
  const char *cursor2;		/* cursor to study strings */
  unsigned counter;		/* general purpose counter */
  bool underline;		/* previous character was underline */

  const char *start_comment;	/* string starting a comment block */
  const char *wrap_comment;	/* string separating two comment lines */
  const char *end_comment;	/* string ending a comment block */

  if (request->sequence_length == 0)
    {
      recode_error (outer, _("Identity recoding, not worth a table"));
      return false;
    }

  if (request->sequence_length > 1
      || request->sequence_array[0].step_type == RECODE_NO_STEP_TABLE)
    {
      recode_error (outer, _("Recoding is too complex for a mere table"));
      return false;
    }

  switch (header_language)
    {
    case RECODE_LANGUAGE_C:
      start_comment = "/* ";
      wrap_comment = "\n   ";
      end_comment = "  */\n";
      break;

    case RECODE_LANGUAGE_PERL:
      start_comment = "# ";
      wrap_comment = "\n# ";
      end_comment = "\n";
      break;

    default:
      /* So lint is happy!  */
      start_comment = NULL;
      wrap_comment = NULL;
      end_comment = NULL;
    }

  /* This function is called only when the recoding sequence contains a single
     step, so it is safe to use request->sequence_array[0] for the step.  */

  step = request->sequence_array;

  /* Print the header of the header file.  */

  printf (_("%sConversion table generated mechanically by %s %s"),
	  start_comment, PACKAGE, VERSION);
  printf (_("%sfor sequence %s.%s"),
	  wrap_comment, edit_sequence (request, 1), end_comment);
  printf ("\n");

  /* Construct the name of the resulting table.  */

  if (header_name)
    {
      if (!ALLOC (name, strlen (header_name) + 1, char))
	return false;
      strcpy (name, header_name);
    }
  else
    name = edit_sequence (request, 0);

  /* Ensure the table name contains only valid characters for a C identifier.
     */

  underline = false;
  cursor = name;
  for (cursor2 = name; *cursor2; cursor2++)
    if ((*cursor2 >= 'a' && *cursor2 <= 'z')
	|| (*cursor2 >= 'A' && *cursor2 <= 'Z')
	|| (*cursor2 >= '0' && *cursor2 <= '9'))
      {
	if (underline)
	  {
	    *cursor++ = '_';
	    underline = false;
	  }
	*cursor++ = *cursor2;
      }
    else if (cursor != name)
      underline = true;
  *cursor = NUL;

  /* Produce the recoding table in the correct format.  */

  if (step->step_type == RECODE_BYTE_TO_BYTE)
    {
      const unsigned char *table = (const unsigned char *) step->step_table;

      /* Produce a one to one recoding table.  */

      switch (header_language)
	{
	case RECODE_NO_LANGUAGE:
	  assert (0);

	case RECODE_LANGUAGE_C:
	  printf ("unsigned char const %s[256] =\n", name);
	  printf ("  {\n");
	  break;

	case RECODE_LANGUAGE_PERL:
	  printf ("@%s =\n", name);
	  printf ("  (\n");
	  break;

        default:
          break;
	}
      for (counter = 0; counter < 256; counter++)
	{
	  printf ("%s%3d,", counter % 8 == 0 ? "    " : " ", table[counter]);
	  if (counter % 8 == 7)
	    printf ("\t%s%3u - %3u%s",
		    start_comment, counter - 7, counter, end_comment);
	}
      switch (header_language)
	{
	case RECODE_NO_LANGUAGE:
	  assert (0);

	case RECODE_LANGUAGE_C:
	  printf ("  };\n");
	  break;

	case RECODE_LANGUAGE_PERL:
	  printf ("  );\n");
	  break;

        default:
          break;
	}
    }
  else if (step->step_type == RECODE_BYTE_TO_STRING)
    {
      const char *const *table = (const char *const *) step->step_table;

      /* Produce a one to many recoding table.  */

      switch (header_language)
	{
	case RECODE_NO_LANGUAGE:
	  assert (0);

	case RECODE_LANGUAGE_C:
	  printf ("const char *%s[256] =\n", name);
	  printf ("  {\n");
	  break;

	case RECODE_LANGUAGE_PERL:
	  printf ("@%s =\n", name);
	  printf ("  (\n");
	  break;

        default:
          break;
	}
      for (counter = 0; counter < 256; counter++)
	{
	  printf ("    ");
	  column = 4;
	  if (table[counter])
	    {
	      printf ("\"");
	      column++;
	      for (cursor2 = table[counter]; *cursor2; cursor2++)
		switch (*cursor2)
		  {
		  case ' ':
		    printf (" ");
		    column++;
		    break;

		  case '\b':
		    printf ("\\b");
		    column += 2;
		    break;

		  case '\t':
		    printf ("\\t");
		    column += 2;
		    break;

		  case '\n':
		    printf ("\\n");
		    column += 2;
		    break;

		  case '"':
		    printf ("\\\"");
		    column += 2;
		    break;

		  case '\\':
		    printf ("\\\\");
		    column += 2;
		    break;

		  case '$':
		    if (header_language == RECODE_LANGUAGE_PERL)
		      {
			printf ("\\$");
			column += 2;
			break;
		      }
                    FALLTHROUGH;

		  default:
		    if (isprint (*cursor2))
		      {
			printf ("%c", *cursor2);
			column++;
		      }
		    else
		      {
			printf ("\\%.3o", *(const unsigned char *) cursor2);
			column += 4;
		      }
		  }
	      printf ("\"");
	      column++;
	    }
	  else
	    switch (header_language)
	      {
	      case RECODE_NO_LANGUAGE:
		assert (0);

	      case RECODE_LANGUAGE_C:
		printf ("0");
		column++;
		break;

	      case RECODE_LANGUAGE_PERL:
		printf ("''");
		column += 2;
		break;

              default:
                break;
	      }
	  printf (",");
	  column++;
	  while (column < 32)
	    {
	      printf ("\t");
	      column += 8 - column % 8;
	    }
	  printf ("%s%3u%s", start_comment, counter, end_comment);
	}
      switch (header_language)
	{
	case RECODE_NO_LANGUAGE:
	  assert (0);

	case RECODE_LANGUAGE_C:
	  printf ("  };\n");
	  break;

	case RECODE_LANGUAGE_PERL:
	  printf ("  );\n");
	  break;

        default:
          break;
	}
    }
  else
    {
      recode_error (outer, _("No table to print"));
      free (name);
      return false;
    }

  free (name);
  return true;
}

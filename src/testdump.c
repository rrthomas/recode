/* Conversion of files between different charsets and surfaces.
   Copyright © 1996, 97, 98, 99, 00 Free Software Foundation, Inc.
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
#include "hash.h"

/*------------------------.
| Produce test patterns.  |
`------------------------*/

/* One should _remove_ pseudo-surfaces to _produce_ test patterns.  This
   strange-looking decision comes from the fact that test patterns are usually
   requested from the `before' position in the request.  */

static bool
test7_data (RECODE_SUBTASK subtask)
{
  unsigned counter;
  int value;

  for (counter = 0; counter < 1 << 7; counter++)
    put_byte (counter, subtask);

  /* Copy the rest verbatim.  */
  while (value = get_byte (subtask), value != EOF)
    put_byte (value, subtask);

  SUBTASK_RETURN (subtask);
}

static bool
test8_data (RECODE_SUBTASK subtask)
{
  unsigned counter;
  int value;

  for (counter = 0; counter < 1 << 8; counter++)
    put_byte (counter, subtask);

  /* Copy the rest verbatim.  */
  while (value = get_byte (subtask), value != EOF)
    put_byte (value, subtask);

  SUBTASK_RETURN (subtask);
}

static bool
test15_data (RECODE_SUBTASK subtask)
{
  unsigned counter;
  unsigned value;

  put_ucs2 (BYTE_ORDER_MARK, subtask);

  /* Before surrogate area.  */
  for (counter = 0; counter < 0xDC00; counter++)
    put_ucs2 (counter, subtask);
  /* After surrogate area.  */
  for (counter = 0xE000; counter < 1 << 16; counter++)
    switch (counter)
      {
      case BYTE_ORDER_MARK:
      case REPLACEMENT_CHARACTER:
      case BYTE_ORDER_MARK_SWAPPED:
      case NOT_A_CHARACTER:
	break;

      default:
	put_ucs2 (counter, subtask);
      }

  /* Copy the rest verbatim.  */
  while (value = get_byte (subtask), value != (unsigned)EOF)
    put_byte (value, subtask);

  SUBTASK_RETURN (subtask);
}

static bool
test16_data (RECODE_SUBTASK subtask)
{
  unsigned counter;
  unsigned value;

  for (counter = 0; counter < 1 << 16; counter++)
    put_ucs2 (counter, subtask);

  /* Copy the rest verbatim.  */
  while (value = get_byte (subtask), value != (unsigned)EOF)
    put_byte (value, subtask);

  SUBTASK_RETURN (subtask);
}

/*-----------------------------------------------.
| Produce frequency count for UCS-2 characters.  |
`-----------------------------------------------*/

struct ucs2_to_count
  {
    recode_ucs2 code;		/* UCS-2 value */
    unsigned count;		/* corresponding count */
  };

static size_t
ucs2_to_count_hash (const void *void_data, size_t table_size)
{
  const struct ucs2_to_count *data = (const struct ucs2_to_count *) void_data;

  return data->code % table_size;
}

static bool
ucs2_to_count_compare (const void *void_first, const void *void_second)
{
  const struct ucs2_to_count *first
    = (const struct ucs2_to_count *) void_first;
  const struct ucs2_to_count *second
    = (const struct ucs2_to_count *) void_second;

  return first->code == second->code;
}

static int
compare_item (const void *void_first, const void *void_second)
{
  struct ucs2_to_count *const *first
    = (struct ucs2_to_count *const *) void_first;
  struct ucs2_to_count *const *second
    = (struct ucs2_to_count *const *) void_second;

  return (*first)->code - (*second)->code;
}

static void
put_string (const char *string, RECODE_SUBTASK subtask)
{
  const char *cursor;

  for (cursor = string; *cursor; cursor++)
    put_byte (*cursor, subtask);
}

static bool
produce_count (RECODE_SUBTASK subtask)
{
  RECODE_OUTER outer = subtask->task->request->outer;
  Hash_table *table;		/* hash table for UCS-2 characters */
  size_t size;			/* number of different characters */
  struct ucs2_to_count **array;	/* array into hash table items */

  table = hash_initialize (0, NULL,
			   ucs2_to_count_hash, ucs2_to_count_compare, free);
  if (!table)
    return false;

  /* Count characters.  */

  {
    unsigned character;		/* current character being counted */

    while (get_ucs2 (&character, subtask))
      {
	struct ucs2_to_count lookup;
	struct ucs2_to_count *entry;

	lookup.code = character;
	entry = (struct ucs2_to_count *) hash_lookup (table, &lookup);
	if (entry)
	  entry->count++;
	else
	  {
	    if (!ALLOC (entry, 1, struct ucs2_to_count))
	      {
		hash_free (table);
		return false;
	      }
	    entry->code = character;
	    entry->count = 1;
	    if (!hash_insert (table, entry))
	      {
		hash_free (table);
		free (entry);
		return false;
	      }
	  }
      }
  }

  /* Sort results.  */

  size = hash_get_n_entries (table);

  if (!ALLOC (array, size, struct ucs2_to_count *))
    {
      hash_free (table);
      return false;
    }
  hash_get_entries (table, (void **) array, size);

  qsort (array, size, sizeof (struct ucs2_to_count *), compare_item);

  /* Produce the report.  */

  /* FIXME: Produce it column-wise.  (See transp.c).  */

  {
    const unsigned non_count_width = 12;
    struct ucs2_to_count **cursor;
    char *buffer;
    unsigned count_width;
    unsigned maximum_count = 0;
    unsigned column = 0;
    unsigned delayed = 0;

    for (cursor = array; cursor < array + size; cursor++)
      if ((*cursor)->count > maximum_count)
	maximum_count = (*cursor)->count;
    if (asprintf (&buffer, "%u", maximum_count) == -1)
      return false;
    count_width = strlen (buffer);
    free (buffer);

    for (cursor = array; cursor < array + size; cursor++)
      {
	unsigned character = (*cursor)->code;
	const char *mnemonic = ucs2_to_rfc1345 (character);

	if (column + count_width + non_count_width > 80)
	  {
	    put_byte ('\n', subtask);
	    delayed = 0;
	    column = 0;
	  }
	else
	  while (delayed)
	    {
	      put_byte (' ', subtask);
	      delayed--;
	    }

	if (asprintf (&buffer, "%*u  %.4X", (int)count_width, (*cursor)->count, character) == -1)
          return false;
        put_string (buffer, subtask);
        free (buffer);
	if (mnemonic)
	  {
	    put_byte (' ', subtask);
	    put_string (mnemonic, subtask);
	    delayed = 6 - 1 - strlen (mnemonic);
	  }
	else
	  delayed = 6;

	column += count_width + non_count_width;
      }

    if (column)
      put_byte ('\n', subtask);
  }

  /* Clean-up.  */

  free (array);
  hash_free (table);

  SUBTASK_RETURN (subtask);
}

/*---------------------------.
| Fully dump an UCS-2 file.  |
`---------------------------*/

static bool
produce_full_dump (RECODE_SUBTASK subtask)
{
  unsigned character;		/* character to dump */

  /* Dump all characters.  */

  if (get_ucs2 (&character, subtask))
    {
      bool french = should_prefer_french();
      const char *charname;	/* charname for code */
      char buffer[50];

      put_string (_("UCS2   Mne   Description\n\n"), subtask);

      while (1)
	{
	  const char *mnemonic = ucs2_to_rfc1345 (character);

	  sprintf (buffer, "%.4X", character);
          put_string (buffer, subtask);
	  if (mnemonic)
            {
              sprintf (buffer, "   %-3s", mnemonic);
              put_string (buffer, subtask);
            }
	  else
	    put_string ("      ", subtask);

	  if (french)
	    {
	      charname = ucs2_to_french_charname (character);
	      if (!charname)
		charname = ucs2_to_charname (character);
	    }
	  else
	    {
	      charname = ucs2_to_charname (character);
	      if (!charname)
		charname = ucs2_to_french_charname (character);
	    }

	  if (charname)
	    {
	      put_string ("   ", subtask);
	      put_string (charname, subtask);
	    }
	  put_byte ('\n', subtask);

	  if (!get_ucs2 (&character, subtask))
	    break;
	}
    }

  SUBTASK_RETURN (subtask);
}

/*-----------------------------------------.
| Declare charsets, surfaces and aliases.  |
`-----------------------------------------*/

bool
module_testdump (RECODE_OUTER outer)
{
  /* Test surfaces.  */

  if (!declare_single (outer, "test7", "data",
		       outer->quality_variable_to_byte,
		       NULL, test7_data))
    return false;
  if (!declare_single (outer, "test8", "data",
		       outer->quality_variable_to_byte,
		       NULL, test8_data))
    return false;
  if (!declare_single (outer, "test15", "data",
		       outer->quality_variable_to_ucs2,
		       NULL, test15_data))
    return false;
  if (!declare_single (outer, "test16", "data",
		       outer->quality_variable_to_ucs2,
		       NULL, test16_data))
    return false;

  /* Analysis charsets.  */

  if (!declare_single (outer, "ISO-10646-UCS-2", "count-characters",
		       outer->quality_ucs2_to_variable,
		       NULL, produce_count))
    return false;
  if (!declare_single (outer, "ISO-10646-UCS-2", "dump-with-names",
		       outer->quality_ucs2_to_variable,
		       NULL, produce_full_dump))
    return false;

  return true;
}

void
delmodule_testdump (RECODE_OUTER outer _GL_ATTRIBUTE_MAYBE_UNUSED)
{
}

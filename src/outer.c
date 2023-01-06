/* Conversion of files between different charsets and surfaces.
   Copyright © 1990-2022 Free Software Foundation, Inc.
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
#include "hash.h"

/*-----------------------------------------------------------------------.
| This dummy fallback routine is used to flag the intent of a reversible |
| coding as a fallback, which is the traditional Recode behaviour.       |
`-----------------------------------------------------------------------*/

_GL_ATTRIBUTE_CONST bool
reversibility (_GL_UNUSED RECODE_SUBTASK subtask, _GL_UNUSED unsigned code)
{
  return false;
}

/*-------------------------------------------------------------------------.
| Allocate and initialize a new single step, save for the before and after |
| charsets and quality.							   |
`-------------------------------------------------------------------------*/

static RECODE_SINGLE
new_single_step (RECODE_OUTER outer)
{
  RECODE_SINGLE single;

  if (!ALLOC (single, 1, struct recode_single))
    return NULL;
  single->next = outer->single_list;
  outer->single_list = single;
  outer->number_of_singles++;

  single->initial_step_table = NULL;
  single->init_routine = NULL;
  single->transform_routine = NULL;
  single->fallback_routine = reversibility;

  return single;
}

/*-------------------------------------------------------------------------.
| Create and initialize a new single step for recoding between BEFORE_NAME |
| and AFTER_NAME.  Give it a recoding QUALITY, also saving an INIT_ROUTINE |
| and a TRANSFORM_ROUTINE functions.                                       |
`-------------------------------------------------------------------------*/

RECODE_SINGLE
declare_single (RECODE_OUTER outer,
		const char *before_name, const char *after_name,
		struct recode_quality quality,
		Recode_init init_routine, Recode_transform transform_routine)
{
  RECODE_SINGLE single = new_single_step (outer);
  RECODE_ALIAS before = NULL, after = NULL;

  if (!single)
    return NULL;

  if (strcmp (before_name, "data") == 0)
    {
      single->before = outer->data_symbol;
      after = find_alias (outer, after_name, SYMBOL_CREATE_DATA_SURFACE);
      single->after = after->symbol;
    }
  else if (strcmp(after_name, "data") == 0)
    {
      before = find_alias (outer, before_name, SYMBOL_CREATE_DATA_SURFACE);
      single->before = before->symbol;
      single->after = outer->data_symbol;
    }
  else
    {
      before = find_alias (outer, before_name, SYMBOL_CREATE_CHARSET);
      single->before = before->symbol;
      after = find_alias (outer, after_name, SYMBOL_CREATE_CHARSET);
      single->after = after->symbol;
    }

  if (!single->before || !single->after)
    {
      if (before)
        delete_alias (before);
      if (after)
        delete_alias (after);
      outer->single_list = single->next;
      free (single);
      return NULL;
    }

  single->quality = quality;
  single->init_routine = init_routine;
  single->transform_routine = transform_routine;

  if (single->before == outer->data_symbol)
    {
      if (single->after->resurfacer)
	recode_error (outer, _("Resurfacer set more than once for `%s'"),
		      after_name);
      single->after->resurfacer = single;
    }
  else if (single->after == outer->data_symbol)
    {
      if (single->before->unsurfacer)
	recode_error (outer, _("Unsurfacer set more than once for `%s'"),
		      before_name);
      single->before->unsurfacer = single;
    }

  return single;
}

/*---------------------------------------------------------------.
| Declare a charset available through `iconv', given the NAME of |
| this charset (which might already exist as an alias), and the  |
| ICONV_NAME to use when calling `iconv'.  Make two single steps |
| in and out of it.                                              |
`---------------------------------------------------------------*/

static bool
internal_iconv (RECODE_SUBTASK subtask)
{
  recode_if_nogo (RECODE_USER_ERROR, subtask);
  SUBTASK_RETURN (subtask);
}

bool
declare_iconv (RECODE_OUTER outer, const char *name, const char *iconv_name)
{
  RECODE_ALIAS alias;
  RECODE_SINGLE single;

  if (alias = find_alias (outer, name, ALIAS_FIND_AS_EITHER),
      !alias)
    if (alias = find_alias (outer, name, SYMBOL_CREATE_CHARSET),
	!alias)
      return false;
  assert(alias->symbol->type == RECODE_CHARSET);

  if (!alias->symbol->iconv_name)
    alias->symbol->iconv_name = iconv_name;

  if (single = new_single_step (outer), !single)
    return false;
  single->before = alias->symbol;
  single->after = outer->iconv_pivot;
  single->quality = outer->quality_variable_to_variable;
  single->init_routine = NULL;
  single->transform_routine = internal_iconv;

  if (single = new_single_step (outer), !single)
    return false;
  single->before = outer->iconv_pivot;
  single->after = alias->symbol;
  single->quality = outer->quality_variable_to_variable;
  single->init_routine = NULL;
  single->transform_routine = internal_iconv;

  return true;
}

/*--------------------------------------------------------------------------.
| Associate an explode format DATA structure with charset NAME_COMBINED, an |
| 8-bit charset.  A NULL value for NAME_EXPLODED implies UCS-2.  Otherwise, |
| NAME_EXPLODED should be the name of a 8-bit based charset.                |
`--------------------------------------------------------------------------*/

bool
declare_explode_data (RECODE_OUTER outer, const unsigned short *data,
		      const char *name_combined, const char *name_exploded)
{
  RECODE_ALIAS alias;
  RECODE_SYMBOL charset_combined;
  RECODE_SYMBOL charset_exploded;
  RECODE_SINGLE single;

  if (alias = find_alias (outer, name_combined, SYMBOL_CREATE_CHARSET),
      !alias)
    return false;

  charset_combined = alias->symbol;
  assert(charset_combined->type == RECODE_CHARSET);

  if (name_exploded)
    {
      if (alias = find_alias (outer, name_exploded, SYMBOL_CREATE_CHARSET),
	  !alias)
	return false;

      charset_exploded = alias->symbol;
      assert(charset_exploded->type == RECODE_CHARSET);
    }
  else
    {
      charset_combined->data_type = RECODE_EXPLODE_DATA;
      charset_combined->data = (void *) data;
      charset_exploded = outer->ucs2_charset;
    }

  single = new_single_step (outer);
  if (!single)
    return false;

  single->before = charset_combined;
  single->after = charset_exploded;
  single->quality = outer->quality_byte_to_variable;
  single->initial_step_table = (void *) data;
  single->init_routine = init_explode;
  single->transform_routine
    = name_exploded ? explode_byte_byte : explode_byte_ucs2;

  single = new_single_step (outer);
  if (!single)
    return false;

  single->before = charset_exploded;
  single->after = charset_combined;
  single->quality = outer->quality_variable_to_byte;
  single->initial_step_table = (void *) data;
  single->init_routine = init_combine;
  single->transform_routine
    = name_exploded ? combine_byte_byte : combine_ucs2_byte;

  return true;
}

/*-------------------------------------------------------------------.
| Associate an UCS-2 strip format DATA structure with charset NAME.  |
`-------------------------------------------------------------------*/

bool
declare_strip_data (RECODE_OUTER outer, struct strip_data *data,
		    const char *name)
{
  RECODE_ALIAS alias;
  RECODE_SYMBOL charset;
  RECODE_SINGLE single;

  if (alias = find_alias (outer, name, SYMBOL_CREATE_CHARSET), !alias)
    return false;

  charset = alias->symbol;
  assert(charset->type == RECODE_CHARSET);
  charset->data_type = RECODE_STRIP_DATA;
  charset->data = data;

  single = new_single_step (outer);
  if (!single)
    return false;

  single->before = charset;
  single->after = outer->ucs2_charset;
  single->quality = outer->quality_byte_to_ucs2;
  single->transform_routine = transform_byte_to_ucs2;

  single = new_single_step (outer);
  if (!single)
    return false;

  single->before = outer->ucs2_charset;
  single->after = charset;
  single->quality = outer->quality_ucs2_to_byte;
  single->init_routine = init_ucs2_to_byte;
  single->transform_routine = transform_ucs2_to_byte;

  return true;
}

/*---------------------------------------------------------------.
| For a given SINGLE step, roughly establish a conversion cost.  |
`---------------------------------------------------------------*/

static void
estimate_single_cost (_GL_UNUSED RECODE_OUTER outer, RECODE_SINGLE single)
{
  int cost;

  /* Ensure a small average cost for each single step, yet much trying to
     avoid single steps prone to loosing information.  */

  cost = single->quality.reversible ? 10 : 200;

  /* Use a few heuristics based on the byte size of both charsets.  */

  switch (single->quality.in_size)
    {
    case RECODE_1:
      /* The fastest is to get one character per read byte.  */
      cost += 15;
      break;

    case RECODE_2:
      /* Reading two requires a routine call and swapping considerations.  */
      cost += 25;
      break;

    case RECODE_4:
      /* Reading four is more work than reading two.  */
      cost += 30;
      break;

    case RECODE_N:
      /* Analysing varysizes is surely much harder than producing them.  */
      cost += 60;

    default:
      break;
    }

  switch (single->quality.out_size)
    {
    case RECODE_1:
      /* Information might be more often lost when not going through UCS.  */
      cost += 20;
      break;

    case RECODE_2:
      /* This is our best bet while writing.  */
      cost += 10;
      break;

    case RECODE_4:
      /* Writing four is more work than writing two.  */
      cost += 15;
      break;

    case RECODE_N:
      /* Writing varysizes requires loops and such.  */
      cost += 35;
      break;

    default:
      break;
    }

  /* Consider speed for fine tuning the cost.  */

  if (single->quality.slower)
    cost += 3;
  else if (single->quality.faster)
    cost -= 2;

  /* Write the price on the ticket.  */

  single->conversion_cost = cost;
  return;
}

/*----------------------------------------.
| Initialize all collected single steps.  |
`----------------------------------------*/

#include "decsteps.h"
bool module_iconv (struct recode_outer *);
void delmodule_iconv (struct recode_outer *);

static bool
register_all_modules (RECODE_OUTER outer)
{
  RECODE_ALIAS alias;
  RECODE_SINGLE single;
  unsigned counter;
  unsigned char *table;

  if (!ALLOC (table, 256, unsigned char))
    return false;
  for (counter = 0; counter < 256; counter++)
    table[counter] = counter;
  outer->one_to_same = table;

  prepare_for_aliases (outer);
  outer->single_list = NULL;
  outer->number_of_singles = 0;

  if (alias = find_alias (outer, "data", SYMBOL_CREATE_CHARSET), !alias)
    return false;
  outer->data_symbol = alias->symbol;

  if (alias = find_alias (outer, "ISO-10646-UCS-2", SYMBOL_CREATE_CHARSET),
      !alias)
    return false;
  assert(alias->symbol->type == RECODE_CHARSET);
  outer->ucs2_charset = alias->symbol;

  if (alias = find_alias (outer, ":iconv:", SYMBOL_CREATE_CHARSET),
      !alias)
    return false;
  assert(alias->symbol->type == RECODE_CHARSET);
  outer->iconv_pivot = alias->symbol;
  if (!declare_alias (outer, ":", ":iconv:"))
    return false;
  /* Needed for compatibility with Recode 3.6.  */
  if (!declare_alias (outer, ":libiconv:", ":iconv:"))
    return false;

  if (alias = find_alias (outer, "CR-LF", SYMBOL_CREATE_CHARSET), !alias)
    return false;
  alias->symbol->type = RECODE_DATA_SURFACE;
  outer->crlf_surface = alias->symbol;

  if (alias = find_alias (outer, "CR", SYMBOL_CREATE_CHARSET), !alias)
    return false;
  alias->symbol->type = RECODE_DATA_SURFACE;
  outer->cr_surface = alias->symbol;

  if (!declare_alias (outer, "ASCII", "ANSI_X3.4-1968"))
    return false;
  if (!declare_alias (outer, "BS", "ASCII-BS"))
    return false;
  if (!declare_alias (outer, "Latin-1", "ISO-8859-1"))
    return false;

#include "inisteps.h"

  /* Force this one last: it does not segregate between charsets and aliases,
     confusing some other initialisations that would come after it.  */
  if (!make_argmatch_arrays (outer))
    return false;
  if (outer->use_iconv)
    if (!module_iconv (outer))
      return false;

  for (single = outer->single_list; single; single = single->next)
    estimate_single_cost (outer, single);

  return true;
}

static void
unregister_all_modules (RECODE_OUTER outer)
{
#include "tersteps.h"
  if (outer->use_iconv)
    delmodule_iconv(outer);
}

/* Library interface.  */

/* See the recode manual for a more detailed description of the library
   interface.  */

/*-------------------------.
| GLOBAL level functions.  |
`-------------------------*/

RECODE_OUTER
recode_new_outer (unsigned flags)
{
  RECODE_OUTER outer = (RECODE_OUTER) calloc (1, sizeof (struct recode_outer));

  if (!outer)
    {
      recode_error (NULL, _("Virtual memory exhausted"));
      if (flags & RECODE_AUTO_ABORT_FLAG)
	exit (1);
      return NULL;
    }

  outer->auto_abort = (flags & RECODE_AUTO_ABORT_FLAG) != 0;
  outer->use_iconv = (flags & RECODE_NO_ICONV_FLAG) == 0;
  outer->strict_mapping = (flags & RECODE_STRICT_MAPPING_FLAG) != 0;
  outer->force = (flags & RECODE_FORCE_FLAG) != 0;

  if (!register_all_modules (outer) || !make_argmatch_arrays (outer))
    {
      recode_delete_outer (outer);
      return NULL;
    }

  outer->quality_byte_reversible.in_size = RECODE_1;
  outer->quality_byte_reversible.out_size = RECODE_1;
  outer->quality_byte_reversible.reversible = true;
  outer->quality_byte_reversible.faster = true;

  outer->quality_byte_to_byte.in_size = RECODE_1;
  outer->quality_byte_to_byte.out_size = RECODE_1;
  outer->quality_byte_to_byte.faster = true;

  outer->quality_byte_to_ucs2.in_size = RECODE_1;
  outer->quality_byte_to_ucs2.out_size = RECODE_2;

  outer->quality_byte_to_variable.in_size = RECODE_1;
  outer->quality_byte_to_variable.out_size = RECODE_N;

  outer->quality_ucs2_to_byte.in_size = RECODE_2;
  outer->quality_ucs2_to_byte.out_size = RECODE_1;

  outer->quality_ucs2_to_variable.in_size = RECODE_2;
  outer->quality_ucs2_to_variable.out_size = RECODE_N;

  outer->quality_variable_to_byte.in_size = RECODE_N;
  outer->quality_variable_to_byte.out_size = RECODE_1;
  outer->quality_variable_to_byte.slower = true;

  outer->quality_variable_to_ucs2.in_size = RECODE_N;
  outer->quality_variable_to_ucs2.out_size = RECODE_2;
  outer->quality_variable_to_ucs2.slower = true;

  outer->quality_variable_to_variable.in_size = RECODE_N;
  outer->quality_variable_to_variable.out_size = RECODE_N;
  outer->quality_variable_to_variable.slower = true;

  return outer;
}

bool
recode_delete_outer (RECODE_OUTER outer)
{
  unregister_all_modules (outer);
  while (outer->number_of_symbols > 0)
    {
      RECODE_SYMBOL symbol = outer->symbol_list;

      outer->symbol_list = symbol->next;
      outer->number_of_symbols--;
      free (symbol);
    }
  while (outer->number_of_singles > 0)
    {
      RECODE_SINGLE single = outer->single_list;

      outer->single_list = single->next;
      outer->number_of_singles--;
      free (single);
    }
  free (outer->pair_restriction);
  if (outer->alias_table)
    hash_free ((Hash_table *) outer->alias_table);
  if (outer->argmatch_charset_array)
    {
      const char **cursor;

      for (cursor = outer->argmatch_charset_array; *cursor; cursor++)
        free ((char *) *cursor);
      for (cursor = outer->argmatch_surface_array; *cursor; cursor++)
        free ((char *) *cursor);
      free (outer->argmatch_charset_array);
    }
  free ((void *) outer->one_to_same);
  free (outer);
  return true;
}

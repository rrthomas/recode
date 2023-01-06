/* Conversion of files between different charsets and surfaces.
   Copyright © 1990-2001 Free Software Foundation, Inc.
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

/* Quality handling.  */

/*---------------------------------------.
| Return a string describing a quality.	 |
`---------------------------------------*/

static const char *
quality_to_string (struct recode_quality quality)
{
  static char buffer[100];

  if (quality.reversible)
    return _("reversible");

  sprintf (buffer, _("%s to %s"),
	   (quality.in_size == RECODE_1 ? _("byte")
	    : quality.in_size == RECODE_2 ? _("ucs2") : _("variable")),
	   (quality.out_size == RECODE_1 ? _("byte")
	    : quality.out_size == RECODE_2 ? _("ucs2") : _("variable")));
  return buffer;
}

/*-----------------------------------------------------------------------.
| Adjust the FIRST quality of a step for the result of merging this step |
| with another succeeding one, given its SECOND quality.                 |
`-----------------------------------------------------------------------*/

static void
merge_qualities (struct recode_quality *first,
		 const struct recode_quality second)
{
  first->out_size = second.out_size;
  first->reversible = first->reversible && second.reversible;
  first->slower = first->slower || second.slower;
  first->faster = first->faster && second.faster;
}

/* Editing out sequences.  */

/*---------------------------------------------.
| Add CHARACTER to the generated work string.  |
`---------------------------------------------*/

static void
add_work_character (RECODE_REQUEST request, int character)
{
  if (request->work_string_length + 1 >= request->work_string_allocated)
    {
      char *new_work_string;

      request->work_string_allocated += 100;
      new_work_string = (char *)
	realloc (request->work_string, request->work_string_allocated);
      if (new_work_string)
        request->work_string = new_work_string;
      else
        return; /* the diagnostic gets truncated, no need to fuss about it.  */
    }
  request->work_string[request->work_string_length++] = character;
}

/*----------------------------------.
| Add a STRING to the work string.  |
`----------------------------------*/

static void
add_work_string (RECODE_REQUEST request, const char *string)
{
  while (*string)
    add_work_character (request, *string++);
}

/*----------------------------------------------------------------------.
| Generate a string describing the current sequence and return it.      |
| Include a description of recoding quality only if EDIT_QUALITY is not |
| zero.								        |
`----------------------------------------------------------------------*/

char *
edit_sequence (RECODE_REQUEST request, bool edit_quality)
{
  RECODE_OUTER outer = request->outer;

  request->work_string_length = 0;

  if (request->sequence_length < 0)
    add_work_string (request, _("*Unachievable*"));
  else if (request->sequence_length == 0)
    add_work_string (request, _("*mere copy*"));
  else
    {
      RECODE_SYMBOL last_charset_printed = NULL;
      RECODE_STEP step = request->sequence_array;

      while (step < request->sequence_array + request->sequence_length)
	{
	  RECODE_STEP unsurfacer_start = step;
	  RECODE_STEP unsurfacer_end;

	  /* Find unsurfacers.  */

	  while (step < request->sequence_array + request->sequence_length
		 && step->after == outer->data_symbol)
	    step++;
	  unsurfacer_end = step;

	  /* Print BEFORE, sparing it if syntax permits.  */

	  if (step != unsurfacer_start
	      || step == request->sequence_array + request->sequence_length
	      || step->before != last_charset_printed)
	    {
	      if (unsurfacer_start != request->sequence_array)
		add_work_character (request, ',');
	      if (step < request->sequence_array + request->sequence_length)
		{
		  last_charset_printed = step->before;
		  add_work_string (request, last_charset_printed->name);
		}
	    }

	  /* Print unsurfacers.  */

	  for (step = unsurfacer_end - 1; step >= unsurfacer_start; step--)
	    {
	      add_work_character (request, '/');
	      add_work_string (request, step->before->name);
	    }
	  step = unsurfacer_end;

	  /* Print AFTER.  */

	  add_work_string (request, "..");
	  if (step < request->sequence_array + request->sequence_length
	      && step->before != outer->data_symbol)
	    {
	      last_charset_printed = step->after;
	      add_work_string (request, last_charset_printed->name);
	      step++;
	    }
	  else
	    {
	      last_charset_printed = outer->data_symbol;
	      add_work_string (request, last_charset_printed->name);
	    }

	  /* Print resurfacers.  */

	  while (step < request->sequence_array + request->sequence_length
		 && step->before == outer->data_symbol)
	    {
	      add_work_character (request, '/');
	      last_charset_printed = NULL;
	      add_work_string (request, step->after->name);
	      step++;
	    }
	}

      if (edit_quality)
	{
	  struct recode_quality quality = outer->quality_byte_reversible;
	  RECODE_CONST_STEP step2;

	  for (step2 = request->sequence_array;
	       step2 < request->sequence_array + request->sequence_length;
	       step2++)
	    merge_qualities (&quality, step2->quality);
	  add_work_character (request, ' ');
	  add_work_character (request, '(');
	  add_work_string (request, quality_to_string (quality));
	  add_work_character (request, ')');
	}
    }

  add_work_character (request, NUL);
  return request->work_string;
}

/* Sequence construction and optimisation.  */

/*-------------------------------------------------------------------------.
| To the end of a given SEQUENCE, add a new recoding step initialised from |
| SINGLE.                                                                  |
`-------------------------------------------------------------------------*/

static bool
add_to_sequence (RECODE_REQUEST request, RECODE_SINGLE single,
		 RECODE_CONST_OPTION_LIST before_options,
		 RECODE_CONST_OPTION_LIST after_options)
{
  RECODE_OUTER outer = request->outer;
  RECODE_STEP step;

  if ((unsigned)request->sequence_length == request->sequence_allocated)
    {
      unsigned old_allocated = request->sequence_allocated;

      if (request->sequence_allocated == 0)
	request->sequence_allocated = 16;
      else
	request->sequence_allocated *= 2;

      if (!REALLOC (request->sequence_array, request->sequence_allocated,
		    struct recode_step))
	{
	  recode_error (outer, _("Virtual memory exhausted!"));
	  return false;
	}

      memset (request->sequence_array + old_allocated, 0,
              (request->sequence_allocated - old_allocated) * sizeof (struct recode_step));
    }

  step = request->sequence_array + request->sequence_length++;
  step->before = single->before;
  step->after = single->after;
  step->step_table = single->initial_step_table;
  step->step_table_term_routine = NULL;
  step->step_type
    = step->step_table ? RECODE_COMBINE_EXPLODE : RECODE_NO_STEP_TABLE;
  step->transform_routine = single->transform_routine;
  step->fallback_routine = single->fallback_routine;
  step->term_routine = NULL;

  if (single->init_routine)
    {
      if (!(*single->init_routine) (step, request,
				    before_options, after_options))
	{
	  recode_error (outer, _("Step initialisation failed"));
	  return false;
	}
    }
  else if (before_options || after_options)
    {
      recode_error (outer,
		    _("Step initialisation failed (unprocessed options)"));
      return false;
    }

  return true;
}

/*----------------------------------------------------------------------.
| Find a SEQUENCE of single steps to achieve a conversion from charset  |
| BEFORE to charset AFTER.  Return false only if no sequence could been |
| found.  Explain what was selected if VERBOSE.                         |
`----------------------------------------------------------------------*/

/* Cost corresponding to an impossible conversion.  */
#define UNREACHABLE	30000

static bool
find_sequence (RECODE_REQUEST request,
	       RECODE_CONST_SYMBOL before,
	       RECODE_CONST_OPTION_LIST before_options,
	       RECODE_CONST_SYMBOL after,
	       RECODE_CONST_OPTION_LIST after_options)
{
  RECODE_OUTER outer = request->outer;
  struct search
    {
      RECODE_SINGLE single; /* single step aiming towards after */
      int cost;			/* cost from here through after */
    };
  struct search *search_array;	/* critical path search tree */
  struct search *search;	/* item in search_array for charset */
  RECODE_SINGLE single;	/* cursor in possible single_singles */
  int cost;			/* cost under consideration */
  bool modified;		/* if modified since last iteration */
  RECODE_CONST_SYMBOL charset;	/* charset while reconstructing */

  if (!ALLOC (search_array, outer->number_of_symbols, struct search))
    return false;

  /* Search for an economical route, looking our way backward from the after
     towards the before.  */

  for (search = search_array;
       search < search_array + outer->number_of_symbols;
       search++)
    {
      search->single = NULL;
      search->cost = UNREACHABLE;
    }
  search_array[after->ordinal].cost = 0;

  modified = true;
  while (modified)
    {
      modified = false;
      for (single = outer->single_list; single; single = single->next)
	if (!single->before->ignore)
	  {
	    cost = search_array[single->after->ordinal].cost;
	    if (cost != UNREACHABLE)
	      {
		cost += single->conversion_cost;
		search = search_array + single->before->ordinal;
		if (cost < search->cost)
		  {
		    search->single = single;
		    search->cost = cost;
		    modified = true;
		  }
	      }
	  }
    }

  if (search_array[before->ordinal].cost == UNREACHABLE)
    {
      /* No path has been found.  */

      free (search_array);
      return false;
    }

  /* Save the retained best path in the sequence array.  */

  for (charset = before; charset != after; charset = single->after)
    {
      single = search_array[charset->ordinal].single;
      if (!add_to_sequence (request, single,
			    charset == before ? before_options : NULL,
			    single->after == after ? after_options : NULL))
	break;
    }

  free (search_array);
  return charset == after;
}

/*---------------------------------------------------------------------------.
| If the step table is usable for step merging, return its type.  Otherwise, |
| return RECODE_NO_STEP_TABLE.  Any non-trivial algorithm for the step       |
| prevents step merging.                                                     |
`---------------------------------------------------------------------------*/

static enum recode_step_type
table_type (RECODE_CONST_REQUEST request,
	    RECODE_CONST_STEP step)
{
  /* When producing source headers, we do not care about algorithms.  */

  if (!request->make_header_flag)

    switch (step->step_type)
      {
      case RECODE_BYTE_TO_BYTE:
	if (step->transform_routine != transform_byte_to_byte)
	  return RECODE_NO_STEP_TABLE;
	break;

      case RECODE_BYTE_TO_STRING:
	if (step->transform_routine != transform_byte_to_variable)
	  return RECODE_NO_STEP_TABLE;
	break;

      default:
	return RECODE_NO_STEP_TABLE;
      }

  return step->step_type;
}

/*---------------------------------------------------------------.
| Order two struct item's lexicographically of their key value.	 |
`---------------------------------------------------------------*/

struct item
  {
    unsigned short code;	/* UCS-2 value */
    unsigned char byte;		/* charset code [0..255] */
  };

static int
compare_struct_item (const void *void_first, const void *void_second)
{
  return (((const struct item *) void_first)->code
	  - ((const struct item *) void_second)->code);
}

/*------------------------------------------------------------------------.
| Complete the initialisation of a double step which just has been merged |
| into a single STEP.  Establish known pairings by comparing UCS-2 values |
| between the before and after charsets.  Create new pairs only when      |
| fallback is reversibility.                                              |
`------------------------------------------------------------------------*/

static bool
complete_double_ucs2_step (RECODE_OUTER outer, RECODE_STEP step)
{
  struct side
    {
      RECODE_SYMBOL charset;	/* charset */
      struct item item[256];	/* array of binding items */
      size_t number_of_items;	/* number of binding items in array */
    };

  const struct strip_data *data; /* UCS-2 data table */
  struct side side_array[2];	/* information for each side */
  struct side *side;		/* cursor into side_array */
  bool reversed;		/* if both sides reversed */
  const recode_ucs2 *pool;	/* pool for fetching UCS-2 characters */
  unsigned offset;		/* cursor in double table strings */
  unsigned byte;		/* character code */
  unsigned row_counter;		/* double table row counter */
  unsigned position_counter;	/* double table column counter */
  struct item *item_cursor;	/* cursor in arrays of binding items */
  struct item *left;		/* left binding items cursor */
  struct item *left_limit;	/* limit value for left */
  struct item *right;		/* right binding items cursor */
  struct item *right_limit;	/* limit value for right */
  struct recode_known_pair pair_array[256]; /* obtained pairings */
  struct recode_known_pair *pair_cursor; /* cursor in array of pairings */

  /* For ensuring reversibility, known pairs should be computed the same
     way regardless of the direction of recoding.  This canonalisation is
     ensured through the charset values, which are increasing along the
     initialisation order.  This should also reflect the charset order in
     rfc1345.txt.  */

  if (step->before < step->after)
    {
      side_array[0].charset = step->before;
      side_array[1].charset = step->after;
      reversed = false;
    }
  else
    {
      side_array[0].charset = step->after;
      side_array[1].charset = step->before;
      reversed = true;
    }

  for (side = side_array; side < side_array + 2; side++)
    {
      /* Construct the array of binding items for the charset.  */

      data = (const struct strip_data *) side->charset->data;
      pool = data->pool;
      item_cursor = side->item;
      byte = 0;

      for (row_counter = 0;
	   row_counter < (256 / STRIP_SIZE);
	   row_counter++)
	if (offset = data->offset[row_counter], offset)
	  for (position_counter = 0;
	       position_counter < STRIP_SIZE;
	       position_counter++)
	    {
	      unsigned code = pool[offset + position_counter];

	      if (code != BIT_MASK (16))
		{
		  /* Establish a new binding item.  */
		  item_cursor->byte = byte;
		  item_cursor->code = code;
		  item_cursor++;
		}
	      byte++;
	    }
	else
	  byte += STRIP_SIZE;

      side->number_of_items = item_cursor - side->item;
    }

  /* Sort both arrays of binding items into lexicographical order.  The
     taken time, which is O(n.log(n)), is gained back when the further
     pairing is completed in a time which is linear instead of quadratic.  */

  qsort (side_array[0].item, side_array[0].number_of_items,
	 sizeof (struct item), compare_struct_item);
  qsort (side_array[1].item, side_array[1].number_of_items,
	 sizeof (struct item), compare_struct_item);

  /* Scan both arrays of binding items simultaneously, saving as pairs
     those codes having the same UCS-2 value.  */

  left = side_array[0].item;
  left_limit = left + side_array[0].number_of_items;
  right = side_array[1].item;
  right_limit = right + side_array[1].number_of_items;
  pair_cursor = pair_array;

  while (left < left_limit && right < right_limit)
    {
      int value = left->code - right->code;

      if (value < 0)
	left++;
      else if (value > 0)
	right++;
      else
	{
	  pair_cursor->left = (left++)->byte;
	  pair_cursor->right = (right++)->byte;
	  pair_cursor++;
	}
    }

  /* Complete the recoding table out of this.  */

  return
    complete_pairs (outer, step,
		    pair_array, pair_cursor - pair_array, false, reversed);
}

static bool
delete_compressed_one_to_many (RECODE_STEP step)
{
  free (step->local);
  return true;
}

static void
delete_step (RECODE_STEP step)
{
  /* Run step destructor, and delete step_table if allocated.  */
  if (step->term_routine)
    (*step->term_routine) (step);

  if (step->step_table_term_routine != NULL)
    (*step->step_table_term_routine) (step->step_table);
}

/*---------------------------------------------------------------------.
| Optimize a SEQUENCE of single steps by creating new single steps, if |
| this can be done by merging adjacent steps which are simple enough.  |
`---------------------------------------------------------------------*/

static bool
simplify_sequence (RECODE_REQUEST request)
{
  RECODE_OUTER outer = request->outer;

  unsigned saved_steps;		/* number of saved steps */
  RECODE_STEP in;               /* next studied sequence step */
  RECODE_STEP out;              /* next rewritten sequence step */
  RECODE_STEP limit;            /* last value for IN */
  unsigned char *accum;		/* byte_to_byte accumulated recoding */
  const char **string;		/* byte_to_variable recoding */
  unsigned char temp[256];	/* temporary value for accum array */
  unsigned counter;		/* all purpose counter */

  /* Tell users what is the goal.  */

  if (request->verbose_flag)
    fprintf (stderr, _("Request: %s\n"), edit_sequence (request, 0));

  saved_steps = 0;

  /* See if there are some double steps to merge.  */

  in = request->sequence_array;
  out = request->sequence_array;
  limit = in + request->sequence_length;

  while (in < limit)
    if (in < limit - 1
	&& in[0].before->data_type == RECODE_STRIP_DATA
	&& in[0].after == outer->ucs2_charset
	&& in[1].before == outer->ucs2_charset
	&& in[1].after->data_type == RECODE_STRIP_DATA)
      {
        /* Free old steps before overwriting anything.  */
        delete_step (in);
        delete_step (in + 1);

	/* This is a double UCS-2 step.  */
	out->before = in[0].before;
	out->after = in[1].after;
	out->quality = in[0].quality;
	merge_qualities (&out->quality, in[1].quality);
	out->transform_routine = transform_byte_to_byte;

	/* Initialize the new single step, so it can be later merged with
	   others.  */
	if (!complete_double_ucs2_step (outer, out))
	  return false;

	in += 2;
	saved_steps++;
	out++;
      }
    else if (in < limit - 1
	     && in[0].after == outer->iconv_pivot
	     && in[1].before == outer->iconv_pivot)
      {
        /* Free old steps before overwriting anything.  */
        delete_step (in);
        delete_step (in + 1);

	/* This is a double `iconv' step.  */
	out->before = in[0].before;
	out->after = in[1].after;
	out->quality = in[0].quality;
	merge_qualities (&out->quality, in[1].quality);
	out->transform_routine = transform_with_iconv;

	in += 2;
	saved_steps++;
	out++;
      }
    else if (out != in)
      *out++ = *in++;
    else
      out++, in++;

  request->sequence_length = out - request->sequence_array;

  /* Recopy the sequence array over itself, while merging subsequences of
     one or more consecutive one-to-one recodings, including an optional
     final one-to-many recoding.  */

  in = request->sequence_array;
  out = request->sequence_array;
  limit = in + request->sequence_length;

  while (in < limit)
    if (in < limit - 1
	&& table_type (request, in) == RECODE_BYTE_TO_BYTE
	&& table_type (request, in + 1) != RECODE_NO_STEP_TABLE

	/* Initialise a cumulative one-to-one recoding with the identity
	   permutation.  Just avoid doing it if not enough memory.  */

	&& ALLOC (accum, 256, unsigned char))
      {
	memcpy (accum, in->step_table, 256);
	out->before = in->before;
	out->after = in->after;
	out->quality = in->quality;
	delete_step (in++);

	/* Merge in all consecutive one-to-one recodings.  */

	while (in < limit
	       && (table_type (request, in) == RECODE_BYTE_TO_BYTE))
	  {
	    const unsigned char *table = (const unsigned char *) in->step_table;

	    for (counter = 0; counter < 256; counter++)
	      temp[counter] = table[accum[counter]];
	    memcpy (accum, temp, 256);

	    out->after = in->after;
	    merge_qualities (&out->quality, in->quality);
	    delete_step (in++);
	    saved_steps++;
	  }

	/* Check for *one* possible one-to-many recoding.  */

	if (in < limit && (table_type (request, in) == RECODE_BYTE_TO_STRING)

	    /* Merge in the one-to-many recoding.  Just avoid doing it if not
	       enough memory.  */

	    && (ALLOC (string, 256, const char *)))
	  {
	    const char *const *table = (const char *const *) in->step_table;

	    for (counter = 0; counter < 256; counter++)
	      string[counter] = table[accum[counter]];
	    free (accum);
	    out->step_type = RECODE_BYTE_TO_STRING;
	    out->step_table = string;
            if (in->step_table_term_routine)
              {
                out->local = (void *) table;   /* Save reference to old table for destructor.  */
                out->term_routine = delete_compressed_one_to_many;
              }
            out->step_table_term_routine = free;
	    out->transform_routine = transform_byte_to_variable;
	    out->after = in->after;
	    merge_qualities (&out->quality, in->quality);
	    in++;
	    saved_steps++;
	  }
	else
	  {
	    /* Make the new single step be a one-to-one recoding.  */

	    out->step_type = RECODE_BYTE_TO_BYTE;
	    out->step_table = accum;
	    out->transform_routine = transform_byte_to_byte;
	  }

	out++;
      }
    else if (out != in)
      *out++ = *in++;
    else
      out++, in++;

  request->sequence_length = out - request->sequence_array;

  /* Delete a single remaining step, if it happens to be the identity
     one-to-one recoding.  */

  in = request->sequence_array;

  if (request->sequence_length == 1
      && table_type (request, in) == RECODE_BYTE_TO_BYTE
      && memcmp (in->step_table, outer->one_to_same, 256) == 0)
    {
      request->sequence_length = 0;
      saved_steps++;
      delete_step (in);
    }

  /* Tell the user if something changed.  */

  if (saved_steps > 0 && request->verbose_flag)
    fprintf (stderr, _("Shrunk to: %s\n"), edit_sequence (request, 0));
  return true;
}

/* Decoding step sequence parameters.  */

/*-------------------------------------.
| Scan next name into SCANNED_STRING.  |
`-------------------------------------*/

static bool
scan_identifier (RECODE_REQUEST request)
{
  char *cursor = request->scanned_string;

  while (*request->scan_cursor && *request->scan_cursor != ','
	 && (request->scan_cursor[0] != '.' || request->scan_cursor[1] != '.')
	 && *request->scan_cursor != '/' && *request->scan_cursor != '+')
    *cursor++ = *request->scan_cursor++;
  *cursor = NUL;

  return true;
}

/*----------------------------------------------------------------.
| Scan all options and return a list of them.                     |
| FIXME: Need to be able to differentiate empty list from error.  |
`----------------------------------------------------------------*/

static RECODE_OPTION_LIST
scan_options (RECODE_REQUEST request)
{
  RECODE_OUTER outer = request->outer;
  RECODE_OPTION_LIST list = NULL;
  RECODE_OPTION_LIST last = NULL;

  while (*request->scan_cursor == '+')
    {
      RECODE_OPTION_LIST new_
	= ALLOC (new_, 1, struct recode_option_list);
      char *copy;

      if (!new_)
	break;			/* FIXME: should interrupt decoding */

      request->scan_cursor++;
      scan_identifier (request);
      ALLOC (copy, strlen (request->scanned_string) + 1, char);
      if (!copy)
	{
	  free (new_);
	  break;		/* FIXME: should interrupt decoding */
	}
      strcpy (copy, request->scanned_string);

      new_->option = copy;
      if (!list)
	list = new_;
      new_->next = last;
      last = new_;
    }
  return list;
}

/*--------------------------------------------------------------------------.
| See if there is another charset after the current one in the subrequest.  |
| In such case, skip and ignore current surfaces, and return false.         |
`--------------------------------------------------------------------------*/

static bool
scan_check_if_last_charset (RECODE_REQUEST request)
{
  const char *cursor = request->scan_cursor;

  while (*cursor && *cursor != ',' && !(cursor[0] == '.' && cursor[1] != '.'))
    cursor++;

  if (*cursor && *cursor != ',')
    {
      request->scan_cursor = cursor;
      return false;
    }

    return true;
}

/*------------------------------------------------------------------------.
| Program, in the SEQUENCE under construction, the removal of surfaces in |
| reverse order of their appearance, using recursion.                     |
`------------------------------------------------------------------------*/

static bool
scan_unsurfacers (RECODE_REQUEST request)
{
  RECODE_OUTER outer = request->outer;
  RECODE_SYMBOL surface = NULL;
  RECODE_OPTION_LIST surface_options = NULL;

  request->scan_cursor++;
  scan_identifier (request);
  if (*request->scanned_string)
    {
      RECODE_ALIAS alias = find_alias (outer, request->scanned_string,
					  ALIAS_FIND_AS_SURFACE);

      if (!alias)
	{
	  recode_error (outer, _("Unrecognised surface name `%s'"),
			request->scanned_string);
	  return false;
	}
      surface = alias->symbol;
      /* FIXME: Should check that it does not itself have implied surfaces?  */
    }
  if (*request->scan_cursor == '+')
    surface_options = scan_options (request);

  if (*request->scan_cursor == '/')
    if (!scan_unsurfacers (request))
      return false;

  if (surface && surface->unsurfacer)
    return
      add_to_sequence (request, surface->unsurfacer, surface_options, NULL);

  return true;
}

/*------------------------------------------------------------------------.
| Program, in the SEQUENCE under construction, the removal of surfaces in |
| reverse order of the given list, using recursion.                       |
`------------------------------------------------------------------------*/

static bool
add_unsurfacers_to_sequence (RECODE_REQUEST request,
			     struct recode_surface_list *list)
{
  if (list->next)
    if (!add_unsurfacers_to_sequence (request, list->next))
      return false;

  if (list->surface->unsurfacer)
    return
      add_to_sequence (request, list->surface->unsurfacer, NULL, NULL);

  return true;
}

/*---------------------------------------------------------------------------.
| Scan next surfaced charset of the form CHARSET[/SURFACE]*, and return this |
| charset after having added it to the SEQUENCE under construction.  If      |
| BEFORE is not NULL (BEFORE_OPTIONS then holds associated options), this    |
| charset is in another position then the first, and then, a sequence of     |
| steps will be generated.  If BEFORE is NULL, then this charset is in a     |
| before position (seen options will be returned at the position indicated   |
| by OPTIONS_POINTER), surfaces should be removed from right to left instead |
| of applied from left to right.  If neither in an initial position nor in a |
| final position, surfaces are ignored, that is optimised out.  Tell what is |
| going on if VERBOSE.                                                       |
`---------------------------------------------------------------------------*/

static RECODE_SYMBOL
scan_charset (RECODE_REQUEST request,
	      RECODE_CONST_SYMBOL before,
	      RECODE_CONST_OPTION_LIST before_options,
	      RECODE_OPTION_LIST *options_pointer)
{
  RECODE_OUTER outer = request->outer;
  RECODE_ALIAS alias;
  RECODE_SYMBOL charset;
  RECODE_OPTION_LIST charset_options = NULL;

  scan_identifier (request);
  alias = find_alias (outer, request->scanned_string, ALIAS_FIND_AS_EITHER);
  if (*request->scan_cursor == '+')
    charset_options = scan_options (request);
  if (!alias)
    return NULL;
  charset = alias->symbol;

  if (before)
    {
      /* We are scanning in an AFTER position.  */

      if (!find_sequence (request, before, before_options,
			  charset, charset_options))
	{
	  recode_error (outer, _("No way to recode from `%s' to `%s'"),
			before->name, charset->name);
	  return NULL;
	}

      /* Ignore everything about surfaces, except in last position of a
	 subrequest.  This optimises out the application of surfaces, when
	 these would be immediately followed by their removal.  */

      if (scan_check_if_last_charset (request))
	{
	  if (*request->scan_cursor == '/')
	    {
	      while (*request->scan_cursor == '/')
		{
		  RECODE_SYMBOL surface = NULL;
		  RECODE_OPTION_LIST surface_options = NULL;

		  request->scan_cursor++;
		  scan_identifier (request);
		  if (*request->scanned_string)
		    {
		      RECODE_ALIAS alias2
			= find_alias (outer, request->scanned_string,
				      ALIAS_FIND_AS_SURFACE);

		      if (!alias2)
			{
			  recode_error (outer,
					_("Unrecognised surface name `%s'"),
					request->scanned_string);
			  return NULL;
			}
		      surface = alias2->symbol;
		      /* FIXME: Should check that it does not itself have
			 implied surfaces?  */
		    }
		  if (*request->scan_cursor == '+')
		    surface_options = scan_options (request);

		  if (surface && surface->resurfacer)
		    if (!add_to_sequence (request, surface->resurfacer,
					  NULL, surface_options))
		      return NULL;
		}
	    }
	  else if (alias->implied_surfaces && !request->make_header_flag)
	    {
	      struct recode_surface_list *list;

	      for (list = alias->implied_surfaces; list; list = list->next)
		if (list->surface->resurfacer)
		  if (!add_to_sequence (request, list->surface->resurfacer,
					NULL, NULL))
		    return NULL;
	    }
	}
    }
  else
    {
      /* We are scanning in a BEFORE position.  */

      *options_pointer = charset_options;

      if (*request->scan_cursor == '/')
	{
	  if (!scan_unsurfacers (request))
	    return NULL;
	}
      else if (alias->implied_surfaces && !request->make_header_flag)
	{
	  if (!add_unsurfacers_to_sequence (request, alias->implied_surfaces))
	    return NULL;
	}
    }

  return charset;
}

/*-------------------------------------------------------------------.
| Into SEQUENCE, scan next request of the form CHARSET[..CHARSET]*.  |
`-------------------------------------------------------------------*/

static bool
scan_request (RECODE_REQUEST request)
{
  RECODE_OUTER outer = request->outer;
  RECODE_OPTION_LIST options;
  RECODE_SYMBOL charset = scan_charset (request, NULL, NULL, &options);

  if (!charset)
    return false;

  if (request->scan_cursor[0] == '.' && request->scan_cursor[1] == '.')
    while (request->scan_cursor[0] == '.' && request->scan_cursor[1] == '.')
      {
	request->scan_cursor += 2;
	charset = scan_charset (request, charset, options, NULL);
	if (!charset)
	  return false;
      }
  else if (*request->scan_cursor == NUL)
    {
      /* No `..' at all implies a conversion to the default charset.  */
      charset = scan_charset (request, charset, options, NULL);
      if (!charset)
	return false;
    }
  else
    {
      recode_error (outer, _("Expecting `..' in request"));
      return false;
    }

  return true;
}

/*-------------------------------------------------------------------------.
| Establish a SEQUENCE of recoding steps described by the STRING argument, |
| which should have the form: REQUEST[,REQUEST]*.                          |
`-------------------------------------------------------------------------*/

static bool
decode_request (RECODE_REQUEST request, const char *string)
{
  RECODE_OUTER outer = request->outer;

  request->scan_cursor = string;
  if (!ALLOC (request->scanned_string, strlen (string) + 1, char))
    return false;
  request->sequence_length = 0;

  if (*request->scan_cursor)
    {
      if (!scan_request (request))
	{
	  free (request->scanned_string);
	  return false;
	}
      while (*request->scan_cursor == ',')
	{
	  request->scan_cursor++;
	  if (!scan_request (request))
	    {
	      free (request->scanned_string);
	      return false;
	    }
	}
    }

  free (request->scanned_string);
  return true;
}

/* Library interface.  */

/* See the recode manual for a more detailed description of the library
   interface.  */

/*--------------------------.
| REQUEST level functions.  |
`--------------------------*/

/* Guarantee four NULs at the end of the output memory buffer for TASK, yet
   not counting them as data.  (Four is the maximum, needed for UCS-4.)  */

static bool
guarantee_nul_terminator (RECODE_TASK task)
{
  if (task->output.cursor + 4 >= task->output.limit)
    {
      RECODE_OUTER outer = task->request->outer;
      size_t size = task->output.cursor - task->output.buffer;

      if (REALLOC (task->output.buffer, size + 4, char))
	{
	  task->output.cursor = task->output.buffer + size;
	  task->output.limit = task->output.buffer + size + 4;
	}
      else
        return false;
    }
  task->output.cursor[0] = NUL;
  task->output.cursor[1] = NUL;
  task->output.cursor[2] = NUL;
  task->output.cursor[3] = NUL;

  return true;
}

RECODE_REQUEST
recode_new_request (RECODE_OUTER outer)
{
  RECODE_REQUEST request;

  if (!ALLOC (request, 1, struct recode_request))
    return NULL;

  request->outer = outer;
  request->diaeresis_char = '"';

  request->work_string_allocated = 0;
  return request;
}

bool
recode_delete_request (RECODE_REQUEST request)
{
  for (RECODE_STEP step = request->sequence_array;
       step < request->sequence_array + request->sequence_length;
       step++)
    delete_step (step);
  free (request->sequence_array);
  free (request->work_string);
  free (request);
  return true;
}

bool
recode_scan_request (RECODE_REQUEST request, const char *string)
{
  return
    decode_request (request, string)
    && simplify_sequence (request);
}

char *
recode_string (RECODE_CONST_REQUEST request, const char *input_string)
{
  char *output_buffer = NULL;
  size_t output_length = 0;
  size_t output_allocated = 0;

  recode_buffer_to_buffer (request, input_string, strlen (input_string),
			   &output_buffer, &output_length, &output_allocated);
  return output_buffer;
}

bool
recode_string_to_buffer (RECODE_CONST_REQUEST request,
			 const char *input_string,
			 char **output_buffer_pointer,
			 size_t *output_length_pointer,
			 size_t *output_allocated_pointer)
{
  return
    recode_buffer_to_buffer (request, input_string, strlen (input_string),
			     output_buffer_pointer, output_length_pointer,
			     output_allocated_pointer);
}

bool
recode_string_to_file (RECODE_CONST_REQUEST request,
		       const char *input_string,
		       FILE *output_file)
{
  return
    recode_buffer_to_file (request, input_string, strlen (input_string),
			   output_file);
}

bool
recode_buffer_to_buffer (RECODE_CONST_REQUEST request,
			 const char *input_buffer,
			 size_t input_length,
			 char **output_buffer_pointer,
			 size_t *output_length_pointer,
			 size_t *output_allocated_pointer)
{
  RECODE_TASK task = recode_new_task (request);
  bool success;

  if (!task)
    return false;

  task->input.buffer = input_buffer;
  task->input.cursor = input_buffer;
  task->input.limit = input_buffer + input_length;
  task->output.buffer = *output_buffer_pointer;
  task->output.cursor = *output_buffer_pointer;
  task->output.limit = *output_buffer_pointer + *output_allocated_pointer;

  success = recode_perform_task (task) && guarantee_nul_terminator (task);
  *output_buffer_pointer = task->output.buffer;
  *output_length_pointer = task->output.cursor - task->output.buffer;
  *output_allocated_pointer = task->output.limit - task->output.buffer;

  recode_delete_task (task);
  return success;
}

bool
recode_buffer_to_file (RECODE_CONST_REQUEST request,
		       const char *input_buffer,
		       size_t input_length,
		       FILE *output_file)
{
  RECODE_TASK task = recode_new_task (request);
  bool success;

  if (!task)
    return false;

  task->input.buffer = input_buffer;
  task->input.cursor = input_buffer;
  task->input.limit = input_buffer + input_length;
  task->output.file = output_file;

  success = recode_perform_task (task);

  recode_delete_task (task);
  return success;
}

bool
recode_file_to_buffer (RECODE_CONST_REQUEST request,
		       FILE *input_file,
		       char **output_buffer_pointer,
		       size_t *output_length_pointer,
		       size_t *output_allocated_pointer)
{
  RECODE_TASK task = recode_new_task (request);
  bool success;

  if (!task)
    return false;

  task->input.file = input_file;
  task->output.buffer = *output_buffer_pointer;
  task->output.cursor = *output_buffer_pointer;
  task->output.limit = *output_buffer_pointer + *output_allocated_pointer;

  success = recode_perform_task (task) && guarantee_nul_terminator (task);
  *output_buffer_pointer = task->output.buffer;
  *output_length_pointer = task->output.cursor - task->output.buffer;
  *output_allocated_pointer = task->output.limit - task->output.buffer;

  recode_delete_task (task);
  return success;
}

bool
recode_file_to_file (RECODE_CONST_REQUEST request,
		     FILE *input_file,
		     FILE *output_file)
{
  RECODE_TASK task = recode_new_task (request);
  bool success;

  if (!task)
    return false;

  task->input.file = input_file;
  task->output.file = output_file;

  success = recode_perform_task (task);

  recode_delete_task (task);
  return success;
}

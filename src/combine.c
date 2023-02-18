/* Conversion of files between different charsets and surfaces.
   Copyright © 1990-2018 Free Software Foundation, Inc.
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

/* FIXME: Softly report errors.  */

/* The satisfactory aspects are that Recode is now able to combine a set of
   sequence of UCS-2 characters into single codes, or explode those single
   codes into the original sequence.  It may happen that many sequences reduce
   to the same code, one of them is arbitrarily taken as canonical.  Any
   combining sequence of an equivalent set produces the equivalent code, but
   at explode time, codes are always turned into their canonical sequence.  I
   did not limit the length of a combining sequence, yet it is usually small.
   Also, I did not put any limit on the number of possibly equivalent
   sequences.  For combining, for each possible partial match in any sequence,
   there is a state.  States corresponding to the scan of a single code are
   found by hash coding, while other states are linearly searched on a linked
   list of possible shifts, rooted on the state corresponding to the partial
   sequence, but without its last code.  I expect this combination of methods
   to execute reasonably fast.  Exploding is much simpler, and I merely use
   hash coding.

   The less satisfactory aspects are that the user interface is still very
   crude.  For the time being, I merely added a "combined" charset: combining
   is done by `u2..co', exploding is done by `co..u2'.  We surely could do
   much better, and build on the concept of options (introduced by `+' in the
   request syntax), Options are probably going to be used for other
   transliteration matters.  Even if built on UCS-2, combining matters would
   have natural extensions in other _smaller_ charsets, and it would be nice
   being able to derive the functionality automatically.  One difficulty to
   foresee is that, contrarily to surfaces, options like transliteration or
   combining _require_ specific charsets (like UCS-2) as a vehicle, and if
   UCS-2 is not otherwise mandated in the request, we would have to create
   such an UCS-2 intermediate.  On the other hand, if UCS-2 is sandwiched
   somewhere in a complex series of steps, we should implicitly propagate
   options towards the UCS-2 step.  Even if nothing looks utterly difficult, I
   guess that for doing it right, a lot of attention and care is needed.
   Also, I'm pretty sure that scrutinizing the documents (from w3.org or
   Unicode) that were suggested to us will raise new specific concerns.  */

/* Exploding.  */

/*---------------------------.
| Initialize for exploding.  |
`---------------------------*/

static size_t
combined_hash (const void *void_data, size_t table_size)
{
  const unsigned short *data = (const unsigned short *) void_data;

  return *data % table_size;
}

static bool
combined_compare (const void *void_first, const void *void_second)
{
  const unsigned short *first = (const unsigned short *) void_first;
  const unsigned short *second = (const unsigned short *) void_second;

  return *first == *second;
}

bool
recode_init_explode (RECODE_STEP step,
	      RECODE_CONST_REQUEST request _GL_UNUSED,
	      RECODE_CONST_OPTION_LIST before_options,
	      RECODE_CONST_OPTION_LIST after_options)
{
  const unsigned short *data = (const unsigned short *) step->step_table;
  Hash_table *table;

  if (before_options || after_options)
    return false;

  table = hash_initialize (0, NULL, combined_hash, combined_compare, NULL);

  if (!table)
    return false;
  step->step_type = RECODE_EXPLODE_STEP;
  step->step_table = table;
  step->step_table_term_routine = (void (*) (void *)) hash_free;

  if (!data)
    return true;

  while (*data != DONE)
    {
      if (!hash_insert (table, data))
	return false;

      while (*data != DONE)
	data++;
      data++;
    }

  return true;
}

/*------------------------------------.
| Execute exploding transformations.  |
`------------------------------------*/

bool
recode_explode_byte_byte (RECODE_SUBTASK subtask)
{
  Hash_table *table = (Hash_table *) subtask->step->step_table;
  unsigned value;

  while (value = recode_get_byte (subtask), value != (unsigned)EOF)
    {
      unsigned short lookup = value;
      unsigned short *result = (unsigned short *) hash_lookup (table, &lookup);

      if (result)
	{
	  result++;
	  while (*result != DONE && *result != ELSE)
	    {
	      recode_put_byte (*result, subtask);
	      result++;
	    }
	}
      else
	recode_put_byte (value, subtask);
    }

  SUBTASK_RETURN (subtask);
}

bool
recode_explode_ucs2_byte (RECODE_SUBTASK subtask)
{
  Hash_table *table = (Hash_table *) subtask->step->step_table;
  unsigned value;

  while (recode_get_ucs2 (&value, subtask))
    {
      unsigned short lookup = value;
      unsigned short *result = (unsigned short *) hash_lookup (table, &lookup);

      if (result)
	{
	  result++;
	  while (*result != DONE && *result != ELSE)
	    {
	      recode_put_byte (*result, subtask);
	      result++;
	    }
	}
      else
	recode_put_byte (value, subtask);
    }

  SUBTASK_RETURN (subtask);
}

bool
recode_explode_byte_ucs2 (RECODE_SUBTASK subtask)
{
  Hash_table *table = (Hash_table *) subtask->step->step_table;
  unsigned value;

  if (value = recode_get_byte (subtask), value != (unsigned)EOF)
    {
      if (subtask->task->byte_order_mark)
	recode_put_ucs2 (BYTE_ORDER_MARK, subtask);

      while (true)
	{
	  unsigned short lookup = value;
	  unsigned short *result
            = (unsigned short *) hash_lookup (table, &lookup);

	  if (result)
	    {
	      result++;
	      while (*result != DONE && *result != ELSE)
		recode_put_ucs2 (*result++, subtask);
	    }
	  else
	    recode_put_ucs2 (value, subtask);

	  if (value = recode_get_byte (subtask), value == (unsigned)EOF)
	    break;
	}
    }

  SUBTASK_RETURN (subtask);
}

bool
recode_explode_ucs2_ucs2 (RECODE_SUBTASK subtask)
{
  Hash_table *table = (Hash_table *) subtask->step->step_table;
  unsigned value;

  if (recode_get_ucs2 (&value, subtask))
    {
      if (subtask->task->byte_order_mark)
	recode_put_ucs2 (BYTE_ORDER_MARK, subtask);

      while (true)
	{
	  unsigned short lookup = value;
	  unsigned short *result
            = (unsigned short *) hash_lookup (table, &lookup);

	  if (result)
	    {
	      result++;
	      while (*result != DONE && *result != ELSE)
		recode_put_ucs2 (*result++, subtask);
	    }
	  else
	    recode_put_ucs2 (value, subtask);

	  if (!recode_get_ucs2 (&value, subtask))
	    break;
	}
    }

  SUBTASK_RETURN (subtask);
}

/* Combining.  */

/* A combining state represents the history of reading one or more characters
   while forming a combining sequence.

   Hash coding is used to find an initial state from the first character in a
   sequence.  If that state is not found, the character surely does not start
   any sequence.  For subsequent characters, possible shifted states from the
   current state are kept on that state, as a linked chain.  This combination
   of methods is expected to behave fast enough in practice.  */

struct state
{
  unsigned short character;	/* last character seen to trigger this state */
  unsigned short result;	/* character equivalent to the combination */
  struct state *shift;		/* list of states for one more character */
  struct state *unshift;	/* state for one less character (back link) */
  struct state *next;		/* next state in a linked chain of states */
};

/*---------------------------.
| Initialize for combining.  |
`---------------------------*/

static size_t
state_hash (const void *void_data, size_t table_size)
{
  const struct state *data = (const struct state *) void_data;

  return data->character % table_size;
}

static bool
state_compare (const void *void_first, const void *void_second)
{
  const struct state *first = (const struct state *) void_first;
  const struct state *second = (const struct state *) void_second;

  return first->character == second->character;
}

static void
state_free (void *void_state)
{
  struct state *state = (struct state *) void_state;
  struct state *shift = state->shift;

  while (shift != NULL)
    {
      struct state *next_shift = shift->next;
      state_free (shift);
      shift = next_shift;
    }
  free (state);
}

static struct state *
prepare_shifted_state (struct state *state, unsigned character,
		       RECODE_CONST_STEP step)
{
  if (state)
    {
      struct state *shift = state->shift;

      while (shift)
	if (shift->character == character)
	  return shift;
	else
	  shift = shift->next;

      if (shift = (struct state *) malloc (sizeof (struct state)), !shift)
	return NULL;

      shift->character = character;
      shift->result = NOT_A_CHARACTER;
      shift->shift = NULL;
      shift->unshift = state;
      shift->next = state->shift;
      state->shift = shift;
      return shift;
    }
  else
    {
      Hash_table *table = (Hash_table *) step->step_table;
      struct state lookup;

      lookup.character = character;
      state = (struct state *) hash_lookup (table, &lookup);
      if (!state)
	{
	  if (state = (struct state *) malloc (sizeof (struct state)), !state)
	    return NULL;

	  state->character = character;
	  state->result = character;
	  state->shift = NULL;
	  state->unshift = NULL;
	  state->next = NULL;

	  if (!hash_insert (table, state))
	    return NULL;
	}
      return state;
    }
}

static struct state *
find_shifted_state (struct state *state, unsigned character,
		    RECODE_CONST_STEP step)
{
  if (state)
    {
      struct state *shift = state->shift;

      while (shift)
	if (shift->character == character)
	  return shift;
	else
	  shift = shift->next;

      return NULL;
    }
  else
    {
      Hash_table *table = (Hash_table *) step->step_table;
      struct state lookup;

      lookup.character = character;
      return (struct state *) hash_lookup (table, &lookup);
    }
}

bool
recode_init_combine (RECODE_STEP step,
	      RECODE_CONST_REQUEST request _GL_UNUSED,
	      RECODE_CONST_OPTION_LIST before_options,
	      RECODE_CONST_OPTION_LIST after_options)
{
  const unsigned short *data = (const unsigned short *) step->step_table;
  Hash_table *table;

  if (before_options || after_options)
    return false;

  table = hash_initialize (0, NULL, state_hash, state_compare, state_free);

  if (!table)
    return false;
  step->step_type = RECODE_COMBINE_STEP;
  step->step_table = table;
  step->step_table_term_routine = (void (*) (void *)) hash_free;

  if (!data)
    return true;

  while (*data != DONE)
    {
      unsigned short result = *data++;
      struct state *state = NULL;

      while (*data != DONE)
	if (*data == ELSE)
	  {
	    if (state)
	      {
		if (state->result != NOT_A_CHARACTER)
		  abort ();
		state->result = result;
		state = NULL;
	      }
	    data++;
	  }
	else
	  {
	    state = prepare_shifted_state (state, *data++, step);
	    if (!state)
	      return false;
	  }

      if (state)
	{
	  if (state->result != NOT_A_CHARACTER
	      && state->result != state->character)
	    abort ();
	  state->result = result;
	}
      data++;
    }

  return true;
}

/*------------------.
| Handle bactrack.  |
`------------------*/

/* When we cannot shift into a state allowing a terminal resulting character,
   we ought to backtrack until such a terminal character if found, then output
   this resulting character for representing the partial sequence which ends
   with that state.  Then, we merely copy characters seen after that state.

   This approach does not properly scan for combinings which might exist in
   the copied characters, presuming that this case does not occur in practice.
   If we later find that it does, backtracing will have to be revisited.  */

static void
backtrack_byte (struct state *state, RECODE_SUBTASK subtask)
{
  if (state->result == NOT_A_CHARACTER)
    {
      backtrack_byte (state->unshift, subtask);
      recode_put_byte (state->character, subtask);
    }
  else
    recode_put_byte (state->result, subtask);
}

static void
backtrack_ucs2 (struct state *state,  RECODE_SUBTASK subtask)
{
  if (state->result == NOT_A_CHARACTER)
    {
      backtrack_ucs2 (state->unshift, subtask);
      recode_put_ucs2 (state->character, subtask);
    }
  else
    recode_put_ucs2 (state->result, subtask);
}

/*------------------------------------.
| Execute combining transformations.  |
`------------------------------------*/

bool
recode_combine_byte_byte (RECODE_SUBTASK subtask)
{
  struct state *state = NULL;
  unsigned value;

  if (value = recode_get_byte (subtask), value != (unsigned)EOF)
    {
      while (true)
	{
	  struct state *shift
	    = find_shifted_state (state, value, subtask->step);

	  if (shift)
	    {
	      state = shift;
	      if (value = recode_get_byte (subtask), value == (unsigned)EOF)
		break;
	    }
	  else if (state)
	    {
	      if (state->result == NOT_A_CHARACTER)
		backtrack_byte (state, subtask);
	      else
		recode_put_byte (state->result, subtask);
	      state = NULL;
	    }
	  else
	    {
	      recode_put_byte (value, subtask);
	      if (value = recode_get_byte (subtask), value == (unsigned)EOF)
		break;
	    }
	}

      if (state)
	{
	  if (state->result == NOT_A_CHARACTER)
	    backtrack_byte (state, subtask);
	  else
	    recode_put_byte (state->result, subtask);
	}
    }

  SUBTASK_RETURN (subtask);
}

bool
recode_combine_ucs2_byte (RECODE_SUBTASK subtask)
{
  struct state *state = NULL;
  unsigned value;

  if (recode_get_ucs2 (&value, subtask))
    {
      while (true)
	{
	  struct state *shift
	    = find_shifted_state (state, value, subtask->step);

	  if (shift)
	    {
	      state = shift;
	      if (!recode_get_ucs2 (&value, subtask))
		break;
	    }
	  else if (state)
	    {
	      if (state->result == NOT_A_CHARACTER)
		backtrack_byte (state, subtask);
	      else
		recode_put_byte (state->result, subtask);
	      state = NULL;
	    }
	  else
	    {
	      recode_put_byte (value, subtask);
	      if (!recode_get_ucs2 (&value, subtask))
		break;
	    }
	}

      if (state)
	{
	  if (state->result == NOT_A_CHARACTER)
	    backtrack_byte (state, subtask);
	  else
	    recode_put_byte (state->result, subtask);
	}
    }

  SUBTASK_RETURN (subtask);
}

bool
recode_combine_byte_ucs2 (RECODE_SUBTASK subtask)
{
  unsigned value;

  if (value = recode_get_byte (subtask), value != (unsigned)EOF)
    {
      struct state *state = NULL;

      if (subtask->task->byte_order_mark)
	recode_put_ucs2 (BYTE_ORDER_MARK, subtask);

      while (true)
	{
	  struct state *shift
	    = find_shifted_state (state, value, subtask->step);

	  if (shift)
	    {
	      state = shift;
	      if (value = recode_get_byte (subtask), value == (unsigned)EOF)
		break;
	    }
	  else if (state)
	    {
	      if (state->result == NOT_A_CHARACTER)
		backtrack_ucs2 (state, subtask);
	      else
		recode_put_ucs2 (state->result, subtask);
	      state = NULL;
	    }
	  else
	    {
	      recode_put_ucs2 (value, subtask);
	      if (value = recode_get_byte (subtask), value == (unsigned)EOF)
		break;
	    }
	}

      if (state)
	{
	  if (state->result == NOT_A_CHARACTER)
	    backtrack_ucs2 (state, subtask);
	  else
	    recode_put_ucs2 (state->result, subtask);
	}
    }

  SUBTASK_RETURN (subtask);
}

bool
recode_combine_ucs2_ucs2 (RECODE_SUBTASK subtask)
{
  unsigned value;

  if (recode_get_ucs2 (&value, subtask))
    {
      struct state *state = NULL;

      if (subtask->task->byte_order_mark)
	recode_put_ucs2 (BYTE_ORDER_MARK, subtask);

      while (true)
	{
	  struct state *shift
	    = find_shifted_state (state, value, subtask->step);

	  if (shift)
	    {
	      state = shift;
	      if (!recode_get_ucs2 (&value, subtask))
		break;
	    }
	  else if (state)
	    {
	      if (state->result == NOT_A_CHARACTER)
		backtrack_ucs2 (state, subtask);
	      else
		recode_put_ucs2 (state->result, subtask);
	      state = NULL;
	    }
	  else
	    {
	      recode_put_ucs2 (value, subtask);
	      if (!recode_get_ucs2 (&value, subtask))
		break;
	    }
	}

      if (state)
	{
	  if (state->result == NOT_A_CHARACTER)
	    backtrack_ucs2 (state, subtask);
	  else
	    recode_put_ucs2 (state->result, subtask);
	}
    }

  SUBTASK_RETURN (subtask);
}

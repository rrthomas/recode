/* Conversion of files between different charsets and surfaces.
   Copyright © 1993-2018 Free Software Foundation, Inc.
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

#include <locale.h>

#include "hash.h"

/*-------------------------------------------------------------------------.
| Return the corresponding UCS-2 value in a CHARSET for a given CODE, or a |
| negative number if this symbol is not defined.			   |
`-------------------------------------------------------------------------*/

_GL_ATTRIBUTE_PURE int
recode_code_to_ucs2 (RECODE_CONST_SYMBOL charset, unsigned code)
{
  /* FIXME: if no specific UCS-2 translation, assume an identity map.  */
  if (charset->data_type != RECODE_STRIP_DATA)
    return code;

  const struct strip_data *data = (const struct strip_data *) charset->data;
  const recode_ucs2 *pool = data->pool;
  unsigned offset = data->offset[code / STRIP_SIZE];
  unsigned value = pool[offset + code % STRIP_SIZE];

  return value == BIT_MASK (16) ? -1 : (int) value;
}

/*---------------------------------------------------------.
| Return true if BEFORE to AFTER is currently restricted.  |
`---------------------------------------------------------*/

static _GL_ATTRIBUTE_PURE bool
check_restricted (RECODE_CONST_OUTER outer,
		  RECODE_CONST_SYMBOL before,
		  RECODE_CONST_SYMBOL after)
{
  struct recode_known_pair *pair;
  int left;
  int right;

  for (pair = outer->pair_restriction;
       pair < outer->pair_restriction + outer->pair_restrictions;
       pair++)
    {
      /* Reject the charset if the characters in the pair do not exist of
	 if their respective definition do not match.  */

      left = recode_code_to_ucs2 (before, pair->left);
      if (left < 0)
	return true;
      right = recode_code_to_ucs2 (after, pair->right);
      if (right < 0)
	return true;
      if (left != right)
	return true;
    }

  /* No restriction found.  */

  return false;
}

/* Charset names.  */

/*-------------------------------------.
| Prepare aliases for initialisation.  |
`-------------------------------------*/

static size_t
alias_hasher (const void *void_alias, size_t limit)
{
  RECODE_CONST_ALIAS alias = (RECODE_CONST_ALIAS) void_alias;

  return hash_string (alias->name, limit);
}

static bool
alias_comparator (const void *void_first, const void *void_second)
{
  RECODE_CONST_ALIAS first = (RECODE_CONST_ALIAS) void_first;
  RECODE_CONST_ALIAS second = (RECODE_CONST_ALIAS) void_second;

  return strcmp (first->name, second->name) == 0;
}

static void
alias_free (void *void_alias)
{
  RECODE_ALIAS alias = (RECODE_ALIAS) void_alias;
  struct recode_surface_list *list, *next;

  for (list = alias->implied_surfaces; list; list = next)
    {
      next = list->next;
      free (list);
    }
  free (alias);
}

bool
recode_prepare_for_aliases (RECODE_OUTER outer)
{
  outer->symbol_list = NULL;
  outer->number_of_symbols = 0;

  outer->alias_table
    = hash_initialize (800, NULL, alias_hasher, alias_comparator, alias_free);
  if (!outer->alias_table)
    return false;

  return true;
}

/*---------------------------------------------------------------------------.
| Return a newly allocated copy of symbol NAME, with upper case letters      |
| turned into lower case, and all non alphanumeric discarded, or NULL if any |
| problem.                                                                   |
`---------------------------------------------------------------------------*/

static char *
name_for_argmatch (RECODE_OUTER outer, const char *name)
{
  char *result;
  char *out;
  const char *in;
  int character;

  if (!ALLOC (result, strlen (name) + 1, char))
    return NULL;
  out = result;
  for (in = name; *in; in++)
    {
      character = *(const unsigned char *) in;
      if ((character >= 'a' && character <= 'z')
	  || (character >= '0' && character <= '9'))
	*out++ = character;
      else if (character >= 'A' && character <= 'Z')
	*out++ = character - 'A' + 'a';
    }
  *out = NUL;

  return result;
}

/*---------------------------------------------------------------------------.
| Given an abbreviated NAME of a charset or surface, return its full name,   |
| properly capitalized and punctuated, or NULL if this cannot be done        |
| successfully.  FIND_TYPE may restrict the interpretation.  A NULL or empty |
| string means the default charset, if this default charset is defined.      |
`---------------------------------------------------------------------------*/

static const char *
disambiguate_name (RECODE_OUTER outer,
		   const char *name, enum alias_find_type find_type)
{
  char *hashname;
  int ordinal;
  const char *result;

  result = NULL;		/* for lint */

  /* Look for a match.  */

  if (!name || !*name)
    switch (find_type)
      {
      case ALIAS_FIND_AS_CHARSET:
      case ALIAS_FIND_AS_EITHER:
	name = getenv ("DEFAULT_CHARSET");
	if (!name || !*name)
          name = locale_charset();
	if (!name || !*name)
          return NULL;
	break;

      default:
	return NULL;
      }

  hashname = name_for_argmatch (outer, name);
  if (!hashname)
    return NULL;

  switch (find_type)
    {
    case SYMBOL_CREATE_CHARSET:
    case SYMBOL_CREATE_DATA_SURFACE:
      abort ();

    case ALIAS_FIND_AS_CHARSET:
      ordinal = argmatch (hashname, outer->argmatch_charset_array, NULL, 0);
      result = ordinal < 0 ? NULL : outer->realname_charset_array[ordinal];
      break;

    case ALIAS_FIND_AS_SURFACE:
      ordinal = argmatch (hashname, outer->argmatch_surface_array, NULL, 0);
      result = ordinal < 0 ? NULL : outer->realname_surface_array[ordinal];
      break;

    case ALIAS_FIND_AS_EITHER:
      ordinal = argmatch (hashname, outer->argmatch_charset_array, NULL, 0);
      if (ordinal >= 0)
	result = outer->realname_charset_array[ordinal];
      else
	{
	  ordinal = argmatch (hashname, outer->argmatch_surface_array, NULL, 0);
	  result = ordinal < 0 ? NULL : outer->realname_surface_array[ordinal];
	}
      break;

    default:
      break;
    }

  free (hashname);
  return result;
}

/*----------------------------------------------------------------------------.
| Delete the given alias.                                                     |
`----------------------------------------------------------------------------*/

void
recode_delete_alias (RECODE_ALIAS alias)
{
  free (alias->symbol);
  free (alias);
}

/*----------------------------------------------------------------------------.
| Return the alias from its given NAME, possibly abbreviated.  If FIND_TYPE   |
| is any of SYMBOL_CREATE_*, NAME is not abbreviated, create a new symbol if  |
| it does not exist, or return NULL if any problem happens at creation time.  |
| For other TYPE values, never create a new charset, merely return NULL if    |
| NAME cannot be correctly disambiguated.                                     |
`----------------------------------------------------------------------------*/

RECODE_ALIAS
recode_find_alias (RECODE_OUTER outer, const char *name,
	    enum alias_find_type find_type)
{
  struct recode_alias lookup;
  RECODE_ALIAS alias;
  RECODE_SYMBOL symbol;
  enum recode_symbol_type type = RECODE_NO_SYMBOL_TYPE;

  switch (find_type)
    {
    case SYMBOL_CREATE_CHARSET:
      type = RECODE_CHARSET;
      break;

    case SYMBOL_CREATE_DATA_SURFACE:
      type = RECODE_DATA_SURFACE;
      break;

    default:
      /* Clean and disambiguate first as requested.  */

      name = disambiguate_name (outer, name, find_type);
      if (!name)
	return NULL;
    }

  /* Search the whole hash bucket and return any match.  */

  if (!name)
    return NULL;
  lookup.name = name;
  if (alias = (RECODE_ALIAS) hash_lookup
      ((const Hash_table *) outer->alias_table, &lookup), alias)
    return alias;

  /* If we reach this point, find_type is necessarily one of SYMBOL_CREATE_*.
     For any other value of find_type, the alias would have already been found
     and returned.  So, here, a new symbol does have to be created.  */

  if (!ALLOC (symbol, 1, struct recode_symbol))
    return NULL;
  symbol->ordinal = outer->number_of_symbols++;
  symbol->name = name;
  symbol->iconv_name = NULL;
  symbol->type = type;
  symbol->ignore = false;
  symbol->data_type = RECODE_NO_CHARSET_DATA;
  symbol->data = NULL;
  symbol->resurfacer = NULL;
  symbol->unsurfacer = NULL;

  if (!ALLOC (alias, 1, struct recode_alias))
    {
      free (symbol);
      return NULL;
    }
  alias->name = name;
  alias->symbol = symbol;
  alias->implied_surfaces = NULL;
  if (!hash_insert ((Hash_table *) outer->alias_table, alias))
    {
      recode_delete_alias (alias);
      return NULL;
    }

  symbol->next = outer->symbol_list;
  outer->symbol_list = symbol;

  return alias;
}

/*-----------------------------------------------------------------------.
| Have NAME to be an alternate name for OLD_NAME.  Create OLD_NAME if it |
| does not exist already.                                                |
`-----------------------------------------------------------------------*/

RECODE_ALIAS
recode_declare_alias (RECODE_OUTER outer, const char *name, const char *old_name)
{
  struct recode_alias lookup;
  RECODE_ALIAS alias;
  RECODE_SYMBOL symbol;

  /* Find the symbol.  */

  if (alias = recode_find_alias (outer, old_name, SYMBOL_CREATE_CHARSET), !alias)
    return NULL;
  symbol = alias->symbol;

  lookup.name = name;
  if (alias = (RECODE_ALIAS) hash_lookup
      ((Hash_table *) outer->alias_table, &lookup), alias)
    {
      if (alias->symbol == symbol)
	return alias;
      recode_error (outer, _("Charset %s already exists and is not %s"),
		    name, old_name);
      return NULL;
    }

  /* Make the alias.  */

  if (!ALLOC (alias, 1, struct recode_alias))
    return NULL;
  alias->name = name;
  alias->symbol = symbol;
  alias->implied_surfaces = NULL;
  if (!hash_insert ((Hash_table *) outer->alias_table, alias))
    {
      free (alias);
      return NULL;
    }

  return alias;
}

/*-------------------------------------------------------------------------.
| To the end of implied surfaces for CHARSET_NAME, add the one represented |
| by SURFACE_NAME.  Both names are created as necessary.                   |
`-------------------------------------------------------------------------*/

bool
recode_declare_implied_surface (RECODE_OUTER outer, RECODE_ALIAS alias,
			 RECODE_CONST_SYMBOL surface)
{
  struct recode_surface_list *list;
  struct recode_surface_list *hook;

  if (!ALLOC (hook, 1, struct recode_surface_list))
    return false;

  hook->surface = surface;
  hook->next = NULL;

  if (alias->implied_surfaces)
    {
      list = alias->implied_surfaces;
      while (list->next)
	list = list->next;
      list->next = hook;
    }
  else
    alias->implied_surfaces = hook;

  return true;
}

/*-------------------------------------------.
| Construct the string arrays for argmatch.  |
`-------------------------------------------*/

struct make_argmatch_walk
  {
    RECODE_OUTER outer;
    unsigned charset_counter;	/* number of acceptable charset names */
    unsigned surface_counter;	/* number of acceptable surface names */
  };

static bool
make_argmatch_walker_1 (void *void_alias, void *void_walk)
{
  RECODE_ALIAS alias = (RECODE_ALIAS) void_alias;
  struct make_argmatch_walk *walk = (struct make_argmatch_walk *) void_walk;

  if (alias->symbol->type == RECODE_CHARSET)
    walk->charset_counter++;
  else
    walk->surface_counter++;

  return true;
}

static bool
make_argmatch_walker_2 (void *void_alias, void *void_walk)
{
  RECODE_ALIAS alias = (RECODE_ALIAS) void_alias;
  struct make_argmatch_walk *walk = (struct make_argmatch_walk *) void_walk;
  RECODE_OUTER outer = walk->outer;

  if (alias->symbol->type == RECODE_CHARSET)
    {
      char *string = name_for_argmatch (outer, alias->name);

      if (!string)
	abort ();
      outer->argmatch_charset_array[walk->charset_counter] = string;
      outer->realname_charset_array[walk->charset_counter] = alias->name;
      walk->charset_counter++;
    }
  else
    {
      char *string = name_for_argmatch (outer, alias->name);

      if (!string)
	abort ();
      outer->argmatch_surface_array[walk->surface_counter] = string;
      outer->realname_surface_array[walk->surface_counter] = alias->name;
      walk->surface_counter++;
    }

  return true;
}

bool
recode_make_argmatch_arrays (RECODE_OUTER outer)
{
  struct make_argmatch_walk walk; /* wanderer's data */

  /* It may happen that new modules are added only once all initialisation
     completed.  To handle that case, free previous arrays if any.  */

  if (outer->argmatch_charset_array)
    {
      const char **cursor;

      for (cursor = outer->argmatch_charset_array; *cursor; cursor++)
	free ((char *) *cursor);
      for (cursor = outer->argmatch_surface_array; *cursor; cursor++)
	free ((char *) *cursor);
      free (outer->argmatch_charset_array);
    }

  /* Count how many strings we need.  */

  walk.outer = outer;
  walk.charset_counter = 0;
  walk.surface_counter = 0;
  hash_do_for_each ((Hash_table *) outer->alias_table,
	 	    make_argmatch_walker_1, &walk);

  /* Allocate the argmatch and realname arrays, each with a NULL sentinel.  */

  {
    const char **cursor;

    if (!ALLOC (cursor, 2*walk.charset_counter + 2*walk.surface_counter + 4,
		const char *))
      return false;

    outer->argmatch_charset_array = cursor;
    cursor += walk.charset_counter;
    *cursor++ = NULL;

    outer->argmatch_surface_array = cursor;
    cursor += walk.surface_counter;
    *cursor++ = NULL;

    outer->realname_charset_array = (const char **) cursor;
    cursor += walk.charset_counter;
    *cursor++ = NULL;

    outer->realname_surface_array = (const char **) cursor;
    cursor += walk.surface_counter;
    *cursor = NULL;
  }

  /* Fill in the arrays.  */

  walk.charset_counter = 0;
  walk.surface_counter = 0;
  hash_do_for_each ((Hash_table *) outer->alias_table,
	 	    make_argmatch_walker_2, &walk);

  return true;
}

/*-------------------------------------------------------------------------.
| Order two strings lexicographically, ignoring case and comparing numeric |
| values for run of decimal digits at first.                               |
`-------------------------------------------------------------------------*/

static _GL_ATTRIBUTE_PURE int
compare_strings (const char *stringA, const char *stringB)
{
  int delayed = 0;

  while (*stringA && *stringB)
    if (*stringA >= '0' && *stringA <= '9')
      if (*stringB >= '0' && *stringB <= '9')
	{
	  unsigned valueA = 0;
	  unsigned valueB = 0;

	  while (*stringA >= '0' && *stringA <= '9'
		 && *stringB >= '0' && *stringB <= '9')
	    {
	      valueA = 10 * valueA + *stringA - '0';
	      valueB = 10 * valueB + *stringB - '0';
	      if (delayed == 0)
		delayed = *stringA - *stringB;
	      stringA++;
	      stringB++;
	    }
	  while (*stringA >= '0' && *stringA <= '9')
	    {
	      valueA = 10 * valueA + *stringA - '0';
	      if (delayed == 0)
		delayed = 1;
	      stringA++;
	    }
	  while (*stringB >= '0' && *stringB <= '9')
	    {
	      valueB = 10 * valueB + *stringB - '0';
	      if (delayed == 0)
		delayed = -1;
	      stringB++;
	    }
	  if (valueA - valueB != 0)
	    return valueA - valueB;
	}
      else
	return -1;
    else
      if (*stringB >= '0' && *stringB <= '9')
	return 1;
      else
	{
	  char charA = *stringA;
	  char charB = *stringB;

	  if (charA >= 'a' && charA <= 'z')
	    charA += 'A' - 'a';
	  if (charB >= 'a' && charB <= 'z')
	    charB += 'A' - 'a';
	  if (charA - charB != 0)
	    return charA - charB;
	  if (delayed == 0)
	    delayed = *stringA - *stringB;
	  stringA++;
	  stringB++;
	}

  return *stringA ? 1 : *stringB ? -1 : delayed;
}

/*--------------------------------------------------------------------------.
| Order two struct recode_alias, using the first key to group all surfaces  |
| together, the second key to group charsets or surfaces having the same    |
| unaliased name, the third key to list unaliases names first, and the last |
| key to order aliased names.                                               |
`--------------------------------------------------------------------------*/

static int
compare_struct_alias (const void *void_first, const void *void_second)
{
  RECODE_CONST_ALIAS first = (RECODE_CONST_ALIAS) void_first;
  RECODE_CONST_ALIAS second = (RECODE_CONST_ALIAS) void_second;
  int value;

  if (first->symbol->type == RECODE_CHARSET
      && second->symbol->type != RECODE_CHARSET)
    return 1;
  if (first->symbol->type != RECODE_CHARSET
      && second->symbol->type == RECODE_CHARSET)
    return -1;

  if (value = compare_strings (first->symbol->name, second->symbol->name),
      value != 0)
    return value;

  if (first->name == first->symbol->name
      && second->name != second->symbol->name)
    return -1;
  if (second->name == second->symbol->name
      && first->name != first->symbol->name)
    return 1;

  return compare_strings (first->name, second->name);
}

/*-------------------------------------------------------------------------.
| List all available symbols, obeying restrictions for an AFTER charset if |
| any.                                                                     |
`-------------------------------------------------------------------------*/

struct list_symbols_walk
  {
    RECODE_ALIAS array;
    unsigned number;
  };

static bool
list_symbols_walker_1 (void *void_alias, void *void_walk)
{
  RECODE_ALIAS alias = (RECODE_ALIAS) void_alias;
  struct list_symbols_walk *walk = (struct list_symbols_walk *) void_walk;

  if (!alias->symbol->ignore)
    walk->number++;

  return true;
}

static bool
list_symbols_walker_2 (void *void_alias, void *void_walk)
{
  RECODE_ALIAS alias = (RECODE_ALIAS) void_alias;
  struct list_symbols_walk *walk = (struct list_symbols_walk *) void_walk;

  if (!alias->symbol->ignore)
    walk->array[walk->number++] = *alias;

  return true;
}

bool
recode_list_all_symbols (RECODE_OUTER outer, RECODE_CONST_SYMBOL after)
{
  struct list_symbols_walk walk; /* wanderer's data */
  RECODE_ALIAS alias;		/* cursor into sorted array */
  bool list_flag;		/* if the current alias should be listed */

  /* Count how many symbols we have.  */

  walk.number = 0;
  hash_do_for_each ((Hash_table *) outer->alias_table,
	 	    list_symbols_walker_1, &walk);

  /* Allocate a structure to hold them.  */

  if (!ALLOC (walk.array, walk.number, struct recode_alias))
    return false;

  /* Copy all symbols in it.  */

  walk.number = 0;
  hash_do_for_each ((Hash_table *) outer->alias_table,
	 	    list_symbols_walker_2, &walk);

  /* Sort it.  */

  qsort (walk.array, (size_t) walk.number, sizeof (struct recode_alias),
	 compare_struct_alias);

  /* Print it, one line per symbol, giving the true symbol name first,
     followed by all its alias in lexicographic order.  */

  list_flag = false;
  for (alias = walk.array; alias < walk.array + walk.number; alias++)
    {
      /* Begin a new line with the true symbol name when it changes.  */

      if (alias == walk.array
	  || alias->symbol->name != (alias - 1)->symbol->name)
	{
	  if (list_flag && alias != walk.array)
	    putchar ('\n');

	  list_flag
	    = !after || !check_restricted (outer, alias->symbol, after);

	  if (list_flag && alias->symbol->type != RECODE_CHARSET)
	    putchar ('/');
	}
      else if (list_flag)
	putchar (' ');

      /* Print a name and its usual surfaces.  */

      if (list_flag)
	{
	  struct recode_surface_list *cursor;

	  fputs (alias->name, stdout);
	  for (cursor = alias->implied_surfaces; cursor; cursor = cursor->next)
	    {
	      putchar ('/');
	      fputs (cursor->surface->name, stdout);
	    }
	}
    }
  if (list_flag)
    putchar ('\n');

  /* Release the work array.  */

  free (walk.array);
  return true;
}

/* Charset contents.  */

/*-----------------------------------------------------------------.
| Decode a known PAIRS argument, given in STRING, constructing the |
| pair_restriction array out of it.                                |
`-----------------------------------------------------------------*/

bool
recode_decode_known_pairs (RECODE_OUTER outer, const char *string)
{
  const char *cursor;
  char *after;
  int left_value;
  int right_value;
  int *pointer;

  if (!ALLOC (outer->pair_restriction, 16, struct recode_known_pair))
    return false;

  left_value = -1;
  right_value = -1;
  pointer = &left_value;

  cursor = string;
  while (*cursor)
    switch (*cursor)
      {
      default:
	return false;

      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
	*pointer = strtoul (cursor, &after, 0);
	cursor = after;
	if (*pointer > 255)
	  return false;
	break;

      case ':':
	cursor++;
	if (left_value < 0 || pointer != &left_value)
	  return false;
	pointer = &right_value;
	break;

      case ',':
	cursor++;
	if (left_value < 0 || right_value < 0)
	  return false;

	outer->pair_restriction[outer->pair_restrictions].left
	  = (unsigned char) left_value;
	outer->pair_restriction[outer->pair_restrictions].right
	  = (unsigned char) right_value;
	outer->pair_restrictions++;

	if (outer->pair_restrictions % 16 == 0)
	  if (!REALLOC (outer->pair_restriction,
			outer->pair_restrictions + 16,
			struct recode_known_pair))
	    return false;

	left_value = -1;
	right_value = -1;
	pointer = &left_value;
	break;
      }

  if (left_value < 0 || right_value < 0)
    return false;

  outer->pair_restriction[outer->pair_restrictions].left
    = (unsigned char) left_value;
  outer->pair_restriction[outer->pair_restrictions].right
    = (unsigned char) right_value;
  outer->pair_restrictions++;

  return true;
}

/*-----------------------------------------------------------------.
| Print a concise, tabular CHARSET description on standard output, |
| presented according to LIST_FORMAT.                              |
`-----------------------------------------------------------------*/

bool
recode_list_concise_charset (RECODE_OUTER outer,
                             RECODE_CONST_SYMBOL charset,
                             const enum recode_list_format list_format)
{
  unsigned half;		/* half 0, half 1 of the table */
  const char *format;		/* format string */
  const char *blanks;		/* white space to replace format string */
  unsigned counter;		/* code counter */
  unsigned counter2;		/* code counter */
  unsigned code;		/* code value */

  /* Ensure we have a strip table to play with.  */

  if (charset->data_type != RECODE_STRIP_DATA)
    {
      recode_error (outer,
		    _("Cannot list `%s', no names available for this charset"),
		    charset->name);
      return false;
    }

  printf ("%s\n", charset->name);

  /* Select format for numbers.  */

  switch (list_format)
    {
    default:
      return false;			/* cannot happen */

    case RECODE_NO_FORMAT:
    case RECODE_DECIMAL_FORMAT:
      format = "%3d";
      blanks = "   ";
      break;

    case RECODE_OCTAL_FORMAT:
      format = "%0.3o";
      blanks = "   ";
      break;

    case RECODE_HEXADECIMAL_FORMAT:
      format = "%0.2x";
      blanks = "  ";
      break;
    }

  /* Print both halves of the table.  */

  for (half = 0; half < 2; half++)
    {
      /* Skip printing this half if it is empty.  */

      for (code = 128 * half; code < 128 * (half + 1); code++)
	if (recode_code_to_ucs2 (charset, code) >= 0)
	  break;
      if (code == 128 * (half + 1))
	continue;

      /* Print this half.  */

      printf ("\n");
      for (counter = 128 * half; counter < 128 * half + 16; counter++)
	for (counter2 = 0; counter2 < 128; counter2 += 16)
	  {
	    int ucs2;
	    const char *mnemonic;

	    if (counter2 > 0)
	      printf ("  ");

	    code = counter + counter2;
	    ucs2 = recode_code_to_ucs2 (charset, code);
	    mnemonic = ucs2 >= 0 ? recode_ucs2_to_rfc1345 (ucs2) : NULL;

	    /* FIXME: Trailing space elimination is not always effective.  */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
	    if (ucs2 >= 0)
	      printf (format, code);
	    else if (mnemonic || counter2 != 112)
	      fputs (blanks,  stdout);
#pragma GCC diagnostic pop

	    if (mnemonic)
	      printf (counter2 == 112 ? " %s\n" : " %-3s", mnemonic);
	    else
	      printf (counter2 == 112 ? "\n" : "    ");
	  }
    }

  return true;
}

/*------------------------------------------------------.
| Print a full CHARSET description on standard output.  |
`------------------------------------------------------*/

static void
list_full_charset_line (int code, recode_ucs2 ucs2, bool french)
{
  const char *mnemonic = recode_ucs2_to_rfc1345 (ucs2);
  const char *charname;

  if (code >= 0)
    printf ("%3d  %.3o  %.2x", code, (unsigned)code, (unsigned)code);
  else
    fputs (" +    +   + ", stdout);

  printf ("   %.4X", ucs2);

  if (mnemonic)
    printf ("  %-3s", mnemonic);
  else
    fputs ("     ", stdout);

  if (french)
    {
      charname = recode_ucs2_to_french_charname (ucs2);
      if (!charname)
	charname = recode_ucs2_to_charname (ucs2);
    }
  else
    {
      charname = recode_ucs2_to_charname (ucs2);
      if (!charname)
	charname = recode_ucs2_to_french_charname (ucs2);
    }

  if (charname)
    {
      fputs ("  ", stdout);
      fputs (charname, stdout);
    }
  putchar ('\n');
}

/*-----------------------------------------------.
| Decide if we prefer French or English output.  |
`-----------------------------------------------*/

bool
recode_should_prefer_french (void)
{
  const char *string = setlocale(LC_MESSAGES, NULL);

  return string && string[0] == 'f' && string[1] == 'r';
}

bool
recode_list_full_charset (RECODE_OUTER outer, RECODE_CONST_SYMBOL charset)
{
  bool french = recode_should_prefer_french();

  /* See which data is available.  */

  switch (charset->data_type)
    {
    case RECODE_EXPLODE_DATA:
      {
	const unsigned short *data = (const unsigned short *) charset->data;
	unsigned code;		/* code counter */
	unsigned expected;	/* expected value for code counter */
	bool insert_white;	/* insert a while line before printing */

	/* Print the long table according to explode data.  */

	printf (_("Dec  Oct Hex   UCS2  Mne  %s\n"), charset->name);
	insert_white = true;
	expected = 0;

	while (*data != DONE)
	  {
	    code = *data++;
	    while (expected < code)
	      {
		if (insert_white)
		  {
		    putchar ('\n');
		    insert_white = false;
		  }
		list_full_charset_line (expected, expected, french);
		expected++;
	      }
	    if (*data == ELSE || *data == DONE)
	      insert_white = true;
	    else
	      {
		if (insert_white)
		  {
		    putchar ('\n');
		    insert_white = false;
		  }
		list_full_charset_line (code, *data++, french);
		while (*data != ELSE && *data != DONE)
		  list_full_charset_line (-1, *data++, french);
	      }
	    while (*data != DONE)
	      data++;
	    expected = code + 1;
	    data++;
	  }
      }
      return true;

    case RECODE_STRIP_DATA:
      {
	unsigned code;		/* code counter */
	int ucs2;		/* UCS-2 translation */
	bool insert_white;	/* insert a while line before printing */

	/* Print the long table according to strip data.  */

	printf (_("Dec  Oct Hex   UCS2  Mne  %s\n"), charset->name);
	insert_white = true;

	for (code = 0; code < 256; code++)
	  if ((ucs2 = recode_code_to_ucs2 (charset, code)), ucs2 >= 0)
	    {
	      if (insert_white)
		{
		  putchar ('\n');
		  insert_white = false;
		}
	      list_full_charset_line (code, ucs2, french);
	    }
	  else
	    insert_white = true;
      }
      return true;

    default:
      recode_error (outer, _("Sorry, no names available for `%s'"),
		    charset->name);
      return false;
    }
}

/*----------------------------------------------------------------------------.
| This is a diagnostic tool.  Report all charsets which are a subset of       |
| another, or are identical.  Return true only if there are no such subsets.  |
`----------------------------------------------------------------------------*/

bool
recode_find_and_report_subsets (RECODE_OUTER outer)
{
  bool success = true;
  RECODE_SYMBOL charset1;

  for (charset1 = outer->symbol_list;
       charset1;
       charset1 = charset1->next)
    {
      const struct strip_data *table1
	= (const struct strip_data *) charset1->data;
      RECODE_SYMBOL charset2;

      if (charset1->ignore || charset1->data_type != RECODE_STRIP_DATA)
	continue;

      for (charset2 = outer->symbol_list;
	   charset2;
	   charset2 = charset2->next)
	{
	  const struct strip_data *table2
	    = (const struct strip_data *) charset2->data;

	  if (charset2->ignore || charset2->data_type != RECODE_STRIP_DATA
	      || charset2 == charset1)
	    continue;

	  {
	    bool subset = true;
	    unsigned distance = 0;
	    unsigned counter;
	    unsigned slider;

	    for (counter = 0; counter < 256/STRIP_SIZE; counter++)
	      {
		const recode_ucs2 *pool1 = table1->pool;
		const recode_ucs2 *pool2 = table2->pool;
		const short offset1 = table1->offset[counter];
		const short offset2 = table2->offset[counter];

		if (pool1 != pool2 || offset1 != offset2)
		  for (slider = 0; slider < STRIP_SIZE; slider++)
		    {
		      recode_ucs2 value1 = pool1[offset1 + slider];
		      recode_ucs2 value2 = pool2[offset2 + slider];

		      if (value1 != value2)
			{
			  if (value1 == BIT_MASK (16))
			    distance++;
			  else
			    {
			      subset = false;
			      break;
			    }
			}
		    }
		if (!subset)
		  break;
	      }

	    if (subset)
	      {
		if (distance == 0)
		  printf ("[  0] %s == %s\n",
			  charset1->name, charset2->name);
		else
		  printf ("[%3u] %s < %s\n", distance,
			  charset1->name, charset2->name);

		success = false;
	      }
	  }
	}
    }

  return success;
}

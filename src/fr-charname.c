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
#include "fr-charname.h"

/*--------------------------------------------------------------------.
| Return a statically allocated full charname associated with a given |
| SYMBOL, or NULL if not found.					      |
`--------------------------------------------------------------------*/

const char *
ucs2_to_french_charname (int ucs2)
{
  static char result[MAX_CHARNAME_LENGTH + 1];
  int first;
  int last;
  int middle;
  int value;
  const char *in;
  char *out;
  const char *cursor;

  /* Find the symbol by binary searching the charname table.  */

  first = 0;
  last = NUMBER_OF_CHARNAMES;
  while (first < last)
    {
      middle = (first + last) / 2;
      if (charname[middle].code < ucs2)
	first = middle + 1;
      else if (charname[middle].code > ucs2)
	last = middle;
      else
	break;
    }

  /* If the UCS value has not been found, return the NULL string.  */

  if (first >= last)
    return NULL;

  /* Else, construct the resulting charname.  */

  out = NULL;
  for (in = charname[middle].crypted; *in; in++)
    {

      /* Decrypt the next word.  */

      value = *(const unsigned char *) in - 1;
      if (value >= NUMBER_OF_SINGLES)
	value = (NUMBER_OF_SINGLES + 255 * (value - NUMBER_OF_SINGLES)
		 + *(const unsigned char *) ++in - 1);

      /* Copy it.  */

      if (out)
	*out++ = ' ';
      else
	out = result;

      for (cursor = word[value]; *cursor; cursor++)
	*out++ = *cursor;
    }

  /* Return the result.  */

  if (out)
    *out = NUL;
  return result;
}

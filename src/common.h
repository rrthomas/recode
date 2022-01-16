/* Conversion of files between different charsets and surfaces.
   Copyright © 1990, 93, 94, 96, 97, 98, 99, 00 Free Software Foundation, Inc.
   Contributed by François Pinard <pinard@iro.umontreal.ca>, 1988.

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

#include "cleaner.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#include "xalloc.h"
#include "argmatch.h"
#include "localcharset.h"
#include "error.h"
#include "gettext.h"
#include "unused-parameter.h"

#if ENABLE_NLS
# define _(Text) gettext (Text)
#else
# define _(Text) Text
#endif
#define N_(Text) Text

#ifndef FALLTHROUGH
# if __GNUC__ < 7
#  define FALLTHROUGH ((void) 0)
# else
#  define FALLTHROUGH __attribute__ ((__fallthrough__))
# endif
#endif

/* Generate a mask of LENGTH one-bits, right justified in a word.  */
#define BIT_MASK(Length) ((1U << (Length)) - 1)

/* Indicate if CHARACTER holds into 7 bits.  */
#define IS_ASCII(Character) \
  (!((Character) & ~BIT_MASK (7)))

#include "recodext.h"

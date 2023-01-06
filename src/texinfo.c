/* Conversion of files between different charsets and surfaces.
   Copyright © 1990-2018 Free Software Foundation, Inc.
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

#include "config.h"
#include "common.h"
#include "decsteps.h"

static const unsigned short texinfo_data[] =
  {
    160, '@', ' ', DONE,
    161, '@', 'e', 'x', 'c', 'l', 'a', 'm', 'd', 'o', 'w', 'n', '{', '}',
    DONE,
    171, '`', '`', DONE,
    187, '\'', '\'', DONE,
    191, '@', 'q', 'u', 'e', 's', 't', 'i', 'o', 'n', 'd', 'o', 'w', 'n',
    '{', '}',
    DONE,
    192, '@', '`', 'A', DONE,
    194, '@', '^', 'A', DONE,
    196, '@', '"', 'A', DONE,
    197, '@', 'A', 'A', '{', '}', DONE,
    199, '@', ',', '{', 'C', '}', DONE,
    200, '@', '`', 'E', DONE,
    201, '@', '\'', 'E', DONE,
    202, '@', '^', 'E', DONE,
    203, '@', '"', 'E', DONE,
    206, '@', '^', 'I', DONE,
    207, '@', '"', 'I', DONE,
    210, '@', '`', 'O', DONE,
    212, '@', '^', 'O', DONE,
    214, '@', '"', 'O', DONE,
    216, '@', 'O', '{', '}', DONE,
    217, '@', '`', 'U', DONE,
    219, '@', '^', 'U', DONE,
    220, '@', '"', 'U', DONE,
    223, '@', 's', 's', '{', '}', DONE,
    224, '@', '`', 'a', DONE,
    226, '@', '^', 'a', DONE,
    228, '@', '"', 'a', DONE,
    229, '@', 'a', 'a', '{', '}', DONE,
    231, '@', ',', '{', 'c', '}', DONE,
    232, '@', '`', 'e', DONE,
    233, '@', '\'', 'e', DONE,
    234, '@', '^', 'e', DONE,
    235, '@', '"', 'e', DONE,
    236, '@', '`', 'i', DONE,
    237, '@', '\'', 'i', DONE,
    238, '@', '^', '{', '@', 'd', 'o', 't', 'l', 'e', 's', 's', '{', 'i',
    '}', '}',
    DONE,
    239, '@', '"', '{', '@', 'd', 'o', 't', 'l', 'e', 's', 's', '{', 'i',
    '}', '}',
    DONE,
    242, '@', '`', 'o', DONE,
    244, '@', '^', 'o', DONE,
    246, '@', '"', 'o', DONE,
    249, '@', '`', 'u', DONE,
    251, '@', '^', 'u', DONE,
    252, '@', '"', 'u', DONE,
    DONE
  };

bool
module_texinfo (RECODE_OUTER outer)
{
  return
    declare_explode_data (outer, texinfo_data, "Latin-1", "Texinfo")
    && declare_alias (outer, "texi", "Texinfo")
    && declare_alias (outer, "ti", "Texinfo");
}

void
delmodule_texinfo (_GL_UNUSED RECODE_OUTER outer)
{
}

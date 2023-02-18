/* Conversion of files between different charsets and surfaces.
   Copyright © 1990-2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by François Pinard <pinard@iro.umontreal.ca>, 1988.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License
   as published by the Free Software Foundation; either version 3 of the
   License, or (at your option) any later version.

   This library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Recode Library; see the file `COPYING.LIB'.
   If not, see <https://www.gnu.org/licenses/>.
*/

/* Step name: ascii_latin1.  */

%%

\<\b\"|\"\b\<			{ recode_put_byte (171, subtask); }
\>\b\"|\"\b\>			{ recode_put_byte (187, subtask); }
A\b`|`\bA			{ recode_put_byte (192, subtask); }
A\b'|'\bA			{ recode_put_byte (193, subtask); }
A\b^|^\bA			{ recode_put_byte (194, subtask); }
A\b~|~\bA			{ recode_put_byte (195, subtask); }
A\b\"|\"\bA			{ recode_put_byte (196, subtask); }
C\b,|,\bC			{ recode_put_byte (199, subtask); }
E\b`|`\bE			{ recode_put_byte (200, subtask); }
E\b'|'\bE			{ recode_put_byte (201, subtask); }
E\b^|^\bE			{ recode_put_byte (202, subtask); }
E\b\"|\"\bE			{ recode_put_byte (203, subtask); }
I\b`|`\bI			{ recode_put_byte (204, subtask); }
I\b'|'\bI			{ recode_put_byte (205, subtask); }
I\b^|^\bI			{ recode_put_byte (206, subtask); }
I\b\"|\"\bI			{ recode_put_byte (207, subtask); }
N\b~|~\bN			{ recode_put_byte (209, subtask); }
O\b`|`\bO			{ recode_put_byte (210, subtask); }
O\b'|'\bO			{ recode_put_byte (211, subtask); }
O\b^|^\bO			{ recode_put_byte (212, subtask); }
O\b~|~\bO			{ recode_put_byte (213, subtask); }
O\b\"|\"\bO			{ recode_put_byte (214, subtask); }
O\b\/|\/\bO			{ recode_put_byte (216, subtask); }
U\b`|`\bU			{ recode_put_byte (217, subtask); }
U\b'|'\bU			{ recode_put_byte (218, subtask); }
U\b^|^\bU			{ recode_put_byte (219, subtask); }
U\b\"|\"\bU			{ recode_put_byte (220, subtask); }
Y\b'|'\bY			{ recode_put_byte (221, subtask); }
s\b\"|\"\bs			{ recode_put_byte (223, subtask); }
a\b`|`\ba			{ recode_put_byte (224, subtask); }
a\b'|'\ba			{ recode_put_byte (225, subtask); }
a\b^|^\ba			{ recode_put_byte (226, subtask); }
a\b~|~\ba			{ recode_put_byte (227, subtask); }
a\b\"|\"\ba			{ recode_put_byte (228, subtask); }
c\b,|,\bc			{ recode_put_byte (231, subtask); }
e\b`|`\be			{ recode_put_byte (232, subtask); }
e\b'|'\be			{ recode_put_byte (233, subtask); }
e\b^|^\be			{ recode_put_byte (234, subtask); }
e\b\"|\"\be			{ recode_put_byte (235, subtask); }
i\b`|`\bi			{ recode_put_byte (236, subtask); }
i\b'|'\bi			{ recode_put_byte (237, subtask); }
i\b^|^\bi			{ recode_put_byte (238, subtask); }
i\b\"|\"\bi			{ recode_put_byte (239, subtask); }
n\b~|~\bn			{ recode_put_byte (241, subtask); }
o\b`|`\bo			{ recode_put_byte (242, subtask); }
o\b'|'\bo			{ recode_put_byte (243, subtask); }
o\b^|^\bo			{ recode_put_byte (244, subtask); }
o\b~|~\bo			{ recode_put_byte (245, subtask); }
o\b\"|\"\bo			{ recode_put_byte (246, subtask); }
o\b\/|\/\bo			{ recode_put_byte (248, subtask); }
u\b`|`\bu			{ recode_put_byte (249, subtask); }
u\b'|'\bu			{ recode_put_byte (250, subtask); }
u\b^|^\bu			{ recode_put_byte (251, subtask); }
u\b\"|\"\bu			{ recode_put_byte (252, subtask); }
y\b'|'\by			{ recode_put_byte (253, subtask); }
y\b\"|\"\by			{ recode_put_byte (255, subtask); }

%%

bool
module_ascii_latin1 (RECODE_OUTER outer)
{
  if (!recode_declare_single (outer, "ASCII-BS", "Latin-1",
		       outer->quality_variable_to_byte,
		       NULL, transform_ascii_latin1))
    return false;

  return true;
}

void
delmodule_ascii_latin1 (RECODE_OUTER outer)
{
}

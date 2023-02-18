/* Conversion of files between different charsets and surfaces.
   Copyright © 1990-2000 Free Software Foundation, Inc.
   Contributed by François Pinard <pinard@iro.umontreal.ca>, 1989.

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

/* Step name: texte_latin1.  */

%{

void texte_latin1_diaeresis (void);

%}

s			(^|[^a-zA-Z])
d			[:"]
l			[a-zA-Z]
%%

``			{ recode_put_byte (171, subtask); }
''			{ recode_put_byte (187, subtask); }

A`			{ recode_put_byte (192, subtask); }
A^			{ recode_put_byte (194, subtask); }
A{d}/{l}		{ if (yytext[1] == request->diaeresis_char)
			    recode_put_byte (196, subtask);
			  else
			    ECHO;
			}
a`			{ recode_put_byte (224, subtask); }
a^			{ recode_put_byte (226, subtask); }
a{d}/{l}		{ if (yytext[1] == request->diaeresis_char)
			    recode_put_byte (228, subtask);
			  else
			    ECHO;
			}

C\,/[aAoOuU]		{ recode_put_byte (199, subtask); }
c\,/[aAoOuU]		{ recode_put_byte (231, subtask); }

E`			{ recode_put_byte (200, subtask); }
E'''			{ recode_put_byte (201, subtask); recode_put_byte (187, subtask); }
E''			{ recode_put_byte ('E', subtask); recode_put_byte (187, subtask); }
E'			{ recode_put_byte (201, subtask); }
E^			{ recode_put_byte (202, subtask); }
E{d}/{l}		{ if (yytext[1] == request->diaeresis_char)
			    recode_put_byte (203, subtask);
			  else
			    ECHO;
			}
e`			{ recode_put_byte (232, subtask); }
e'''			{ recode_put_byte (233, subtask); recode_put_byte (187, subtask); }
e''			{ recode_put_byte ('e', subtask); recode_put_byte (187, subtask); }
e'			{ recode_put_byte (233, subtask); }
e^			{ recode_put_byte (234, subtask); }
e{d}/{l}		{ if (yytext[1] == request->diaeresis_char)
			    recode_put_byte (235, subtask);
			  else
			    ECHO;
			}

I`			{ recode_put_byte (204, subtask); }
I^			{ recode_put_byte (206, subtask); }
I{d}/{l}		{ if (yytext[1] == request->diaeresis_char)
			    recode_put_byte (207, subtask);
			  else
			    ECHO;
			}
i`			{ recode_put_byte (236, subtask); }
i^			{ recode_put_byte (238, subtask); }
i{d}/{l}		{ if (yytext[1] == request->diaeresis_char)
			    recode_put_byte (239, subtask);
			  else
			    ECHO;
			}

O`			{ recode_put_byte (210, subtask); }
O^			{ recode_put_byte (212, subtask); }
O{d}/{l}		{ if (yytext[1] == request->diaeresis_char)
			    recode_put_byte (214, subtask);
			  else
			    ECHO;
			}
o`			{ recode_put_byte (242, subtask); }
o^			{ recode_put_byte (244, subtask); }
o{d}/{l}		{ if (yytext[1] == request->diaeresis_char)
			    recode_put_byte (246, subtask);
			  else
			    ECHO;
			}

U`			{ recode_put_byte (217, subtask); }
U^			{ recode_put_byte (219, subtask); }
U{d}/{l}		{ if (yytext[1] == request->diaeresis_char)
			    recode_put_byte (220, subtask);
			  else
			    ECHO;
			}
u`			{ recode_put_byte (249, subtask); }
u^			{ recode_put_byte (251, subtask); }
u{d}/{l}		{ if (yytext[1] == request->diaeresis_char)
			    recode_put_byte (252, subtask);
			  else
			    ECHO;
			}

{s}[Bb]esaigue{d}	{ texte_latin1_diaeresis (); }
{s}[Cc]igue{d}		{ texte_latin1_diaeresis (); }
{s}[Aa]igue{d}		{ texte_latin1_diaeresis (); }
{s}[Aa]mbigue{d}	{ texte_latin1_diaeresis (); }
{s}[Cc]ontigue{d}	{ texte_latin1_diaeresis (); }
{s}[Ee]xigue{d}		{ texte_latin1_diaeresis (); }
{s}[Ss]ubaigue{d}	{ texte_latin1_diaeresis (); }
{s}[Ss]uraigue{d}	{ texte_latin1_diaeresis (); }
{s}[Aa]i{d}		{ texte_latin1_diaeresis (); }
{s}[Cc]ongai{d}		{ texte_latin1_diaeresis (); }
{s}[Gg]oi{d}		{ texte_latin1_diaeresis (); }
{s}[Hh]ai{d}kai{d}	{ if (yytext[4] == request->diaeresis_char)
			    texte_latin1_diaeresis ();
			  else
			    ECHO;
			}
{s}[Ii]noui{d}		{ texte_latin1_diaeresis (); }
[JjTtLl]'[Aa][Ii]{d}	{ ECHO; }
{s}[Ss]ai{d}		{ texte_latin1_diaeresis (); }
{s}[Ss]amurai{d}	{ texte_latin1_diaeresis (); }
{s}[Tt]hai{d}		{ texte_latin1_diaeresis (); }
{s}[Tt]okai{d}		{ texte_latin1_diaeresis (); }
{s}[Cc]anoe{d}		{ texte_latin1_diaeresis (); }
{s}Esau{d}		{ texte_latin1_diaeresis (); }
%%

void
texte_latin1_diaeresis (void)
{
  unsigned counter;

  for (counter = 0; counter < yyleng; counter++)
    if (yytext[counter+1] == request->diaeresis_char)
      {
	switch (yytext[counter])
	  {
	    /* The next "case 'A'" line once triggered a `NULL in input'
	       diagnostic in flex.  This astonishing bug has been hard to
	       isolate, so I'll leave this comment around for a while.  */

	  case 'A': recode_put_byte (196, subtask); break;
	  case 'E': recode_put_byte (203, subtask); break;
	  case 'I': recode_put_byte (207, subtask); break;
	  case 'O': recode_put_byte (214, subtask); break;
	  case 'U': recode_put_byte (220, subtask); break;
	  case 'a': recode_put_byte (228, subtask); break;
	  case 'e': recode_put_byte (235, subtask); break;
	  case 'i': recode_put_byte (239, subtask); break;
	  case 'o': recode_put_byte (246, subtask); break;
	  case 'u': recode_put_byte (252, subtask); break;
	  case 'y': recode_put_byte (255, subtask); break;
	  default:  recode_put_byte (yytext[counter], subtask);
	  }
	counter++;
      }
    else
      recode_put_byte (yytext[counter], subtask);
}

bool
module_texte_latin1 (RECODE_OUTER outer)
{
  return
    recode_declare_single (outer, "Texte", "Latin-1",
		    outer->quality_variable_to_byte, NULL,
		    transform_texte_latin1)
    && recode_declare_alias (outer, "txte", "Texte");
}

void
delmodule_texte_latin1 (RECODE_OUTER outer)
{
}

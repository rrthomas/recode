/* Conversion of files between different charsets and surfaces.
   Copyright © 1990, 93, 94, 97, 98, 99, 00 Free Software Foundation, Inc.
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

/* Previously, the CR-LF surface was built-in.  It is now handled through a
   separate, optional step.  An optimisation might use a flag to trigger old
   built-in code.  For now, depend on a variable preset to false.  */
static const bool auto_crlf = false;

#define DOS_EOF 26		/* oldish end of file */
#define DOS_CR 13		/* carriage return */
#define DOS_LF 10		/* line feed */

/* Correspondance for IBM PC ruler graphics characters into ASCII graphics
   approximations.  The current principles are:

   - Single horizontal rulers are made up of dashes.
   - Double horizontal rulers are made up of equal signs.
   - Both single and double vertical are made up with `|'.
   - Both upper corners are rounded down with periods.
   - Lower corners are rounded up with grave/acute accent on left/right.
   - Other crossing rulers are approximated with plus signs, with exceptions
     for double horizontal ruler crossings but not at corners: they are
     equal signs inside a table, and `|' at left or right margin.
*/

unsigned char convert_rulers[48] =
  {
    '#',			/* 176 */
    '#',			/* 177 */
    '#',			/* 178 */
    '|',			/* 179 */
    '+',			/* 180 */
    '|',			/* 181 */
    '+',			/* 182 */
    '.',			/* 183 */
    '.',			/* 184 */
    '|',			/* 185 */
    '|',			/* 186 */
    '.',			/* 187 */
    '\'',			/* 188 */
    '\'',			/* 189 */
    '\'',			/* 190 */
    '.',			/* 191 */
    '`',			/* 192 */
    '+',			/* 193 */
    '+',			/* 194 */
    '+',			/* 195 */
    '-',			/* 196 */
    '+',			/* 197 */
    '|',			/* 198 */
    '+',			/* 199 */
    '`',			/* 200 */
    '.',			/* 201 */
    '=',			/* 202 */
    '=',			/* 203 */
    '|',			/* 204 */
    '=',			/* 205 */
    '=',			/* 206 */
    '=',			/* 207 */
    '+',			/* 208 */
    '=',			/* 209 */
    '+',			/* 210 */
    '`',			/* 211 */
    '`',			/* 212 */
    '.',			/* 213 */
    '.',			/* 214 */
    '+',			/* 215 */
    '=',			/* 216 */
    '\'',			/* 217 */
    '.',			/* 218 */
    '#',			/* 219 */
    '#',			/* 220 */
    '#',			/* 221 */
    '#',			/* 222 */
    '#',			/* 223 */
  };

/* Data for IBM PC to ISO Latin-1 code conversions.  */

static struct recode_known_pair known_pairs[] =
  {
    { 20, 182},			/* pilcrow sign */
    { 21, 167},			/* section sign */

    {128, 199},			/* capital letter C with cedilla */
    {129, 252},			/* small letter u with diaeresis */
    {130, 233},			/* small letter e with acute accent */
    {131, 226},			/* small letter a with circumflex accent */
    {132, 228},			/* small letter a with diaeresis */
    {133, 224},			/* small letter a with grave accent */
    {134, 229},			/* small letter a with ring above */
    {135, 231},			/* small letter c with cedilla */
    {136, 234},			/* small letter e with circumflex accent */
    {137, 235},			/* small letter e with diaeresis */
    {138, 232},			/* small letter e with grave accent */
    {139, 239},			/* small letter i with diaeresis */
    {140, 238},			/* small letter i with circumflex accent */
    {141, 236},			/* small letter i with grave accent */
    {142, 196},			/* capital letter A with diaeresis */
    {143, 197},			/* capital letter A with ring above */
    {144, 201},			/* capital letter E with acute accent */
    {145, 230},			/* small ligature a with e */
    {146, 198},			/* capital ligature A with E */
    {147, 244},			/* small letter o with circumblex accent */
    {148, 246},			/* small letter o with diaeresis */
    {149, 242},			/* small letter o with grave accent */
    {150, 251},			/* small letter u with circumflex accent */
    {151, 249},			/* small letter u with grave accent */
    {152, 255},			/* small letter y with diaeresis */
    {153, 214},			/* capital letter O with diaeresis */
    {154, 220},			/* capital letter U with diaeresis */
    {155, 162},			/* cent sign */
    {156, 163},			/* pound sign */
    {157, 165},			/* yen sign */

    {160, 225},			/* small letter a with acute accent */
    {161, 237},			/* small letter i with acute accent */
    {162, 243},			/* small letter o with acute accent */
    {163, 250},			/* small letter u with acute accent */
    {164, 241},			/* small letter n with tilde */
    {165, 209},			/* capital letter N with tilde */
    {166, 170},			/* feminine ordinal indicator */
    {167, 186},			/* masculine ordinal indicator */
    {168, 191},			/* inverted question mark */

    {170, 172},			/* not sign */
    {171, 189},			/* vulgar fraction one half */
    {172, 188},			/* vulgar fraction one quarter */
    {173, 161},			/* inverted exclamation mark */
    {174, 171},			/* left angle quotation mark */
    {175, 187},			/* right angle quotation mark */

    {225, 223},			/* small german letter sharp s */

    {230, 181},			/* small Greek letter mu micro sign */

    {241, 177},			/* plus-minus sign */

    {246, 247},			/* division sign */

    {248, 176},			/* degree sign */

    {250, 183},			/* middle dot */

    {253, 178},			/* superscript two */

    {255, 160},			/* no-break space */
  };
#define NUMBER_OF_PAIRS \
  (sizeof (known_pairs) / sizeof (struct recode_known_pair))

static bool
transform_latin1_ibmpc (RECODE_SUBTASK subtask)
{
  if (subtask->step->fallback_routine == reversibility)
    {
      const unsigned char *table
	= (const unsigned char *) subtask->step->step_table;
      int input_char;

      while (input_char = get_byte (subtask), input_char != EOF)
	if (input_char == '\n')
	  {
	    put_byte (DOS_CR, subtask);
	    put_byte (DOS_LF, subtask);
	  }
	else
	  put_byte (table[input_char], subtask);
    }
  else
    {
      const char *const *table
	= (const char *const *) subtask->step->step_table;
      int input_char;

      while (input_char = get_byte (subtask), input_char != EOF)
	if (input_char == '\n')
	  {
	    put_byte (DOS_CR, subtask);
	    put_byte (DOS_LF, subtask);
	  }
	else if (table[input_char])
	  put_byte (*table[input_char], subtask);
	else
	  RETURN_IF_NOGO (RECODE_UNTRANSLATABLE, subtask);
    }
  SUBTASK_RETURN (subtask);
}

static bool
transform_ibmpc_latin1 (RECODE_SUBTASK subtask)
{
  if (subtask->step->fallback_routine == reversibility)
    {
      const unsigned char *table
	= (const unsigned char *) subtask->step->step_table;
      int input_char = get_byte (subtask);

      while (input_char != EOF)
	switch (input_char)
	  {
	  case DOS_EOF:
	    RETURN_IF_NOGO (RECODE_NOT_CANONICAL, subtask);
	    input_char = EOF;
	    break;

	  case DOS_CR:
	    input_char = get_byte (subtask);
	    if (input_char == DOS_LF)
	      {
		put_byte ('\n', subtask);
		input_char = get_byte (subtask);
	      }
	    else
	      put_byte (table[DOS_CR], subtask);
	    break;

	  case DOS_LF:
	    RETURN_IF_NOGO (RECODE_AMBIGUOUS_OUTPUT, subtask);
	    FALLTHROUGH;

	  default:
	    put_byte (table[input_char], subtask);
	    input_char = get_byte (subtask);
	  }
    }
  else
    {
      const char *const *table
	= (const char *const *) subtask->step->step_table;
      int input_char = get_byte (subtask);

      while (input_char != EOF)
	switch (input_char)
	  {
	  case DOS_EOF:
	    RETURN_IF_NOGO (RECODE_NOT_CANONICAL, subtask);
	    input_char = EOF;
	    break;

	  case DOS_CR:
	    input_char = get_byte (subtask);
	    if (input_char == DOS_LF)
	      {
		put_byte ('\n', subtask);
		input_char = get_byte (subtask);
	      }
	    else if (table[DOS_CR])
	      put_byte (*table[DOS_CR], subtask);
	    else
	      RETURN_IF_NOGO (RECODE_UNTRANSLATABLE, subtask);
	    break;

	  case DOS_LF:
	    RETURN_IF_NOGO (RECODE_AMBIGUOUS_OUTPUT, subtask);
	    FALLTHROUGH;

	  default:
	    if (table[input_char])
	      put_byte (*table[input_char], subtask);
	    else
	      RETURN_IF_NOGO (RECODE_UNTRANSLATABLE, subtask);
	    input_char = get_byte (subtask);
	  }
    }

  SUBTASK_RETURN (subtask);
}

static bool
init_latin1_ibmpc (RECODE_STEP step,
		   RECODE_CONST_REQUEST request,
		   RECODE_CONST_OPTION_LIST before_options,
		   RECODE_CONST_OPTION_LIST after_options)
{
  if (before_options || after_options)
    return false;

  if (!complete_pairs (request->outer, step,
		       known_pairs, NUMBER_OF_PAIRS, true, true))
    return false;

  if (auto_crlf)
    step->transform_routine = transform_latin1_ibmpc;
  else if (step->fallback_routine == reversibility)
    step->transform_routine = transform_byte_to_byte;
  else
    step->transform_routine = transform_byte_to_variable;

  return true;
}

static bool
init_ibmpc_latin1 (RECODE_STEP step,
		   RECODE_CONST_REQUEST request,
		   RECODE_CONST_OPTION_LIST before_options,
		   RECODE_CONST_OPTION_LIST after_options)
{
  RECODE_OUTER outer = request->outer;
  unsigned char *table;

  if (before_options || after_options)
    return false;

  if (!complete_pairs (outer, step,
		       known_pairs, NUMBER_OF_PAIRS, true, false))
    return false;

  if (auto_crlf)
    step->transform_routine = transform_ibmpc_latin1;
  else if (step->fallback_routine == reversibility)
    step->transform_routine = transform_byte_to_byte;
  else
    step->transform_routine = transform_byte_to_variable;

  /* FIXME: Allow ascii_graphics even with strict mapping.  Reported by
     David E. A. Wilson <david@osiris.cs.uow.edu.au>.  */

  if (request->ascii_graphics)
    {
      if (!ALLOC (table, 256, unsigned char))
	return false;
      memcpy (table, step->step_table, 256);
      memcpy (table + 176, convert_rulers, 48);
      free (step->step_table);
      step->step_table = table;
    }

  return true;
}

bool
module_ibmpc (RECODE_OUTER outer)
{
  RECODE_ALIAS alias;

  if (!declare_single (outer, "Latin-1", "IBM-PC",
		       outer->quality_byte_to_variable,
		       init_latin1_ibmpc, transform_latin1_ibmpc))
    return false;
  if (!declare_single (outer, "IBM-PC", "Latin-1",
		       outer->quality_variable_to_variable,
		       init_ibmpc_latin1, transform_ibmpc_latin1))
    return false;

  if (alias = declare_alias (outer, "IBM-PC", "IBM-PC"), !alias)
    return false;
  if (!declare_implied_surface (outer, alias, outer->crlf_surface))
    return false;

  if (alias = declare_alias (outer, "dos", "IBM-PC"), !alias)
    return false;
  if (!declare_implied_surface (outer, alias, outer->crlf_surface))
    return false;

  if (alias = declare_alias (outer, "MSDOS", "IBM-PC"), !alias)
    return false;
  if (!declare_implied_surface (outer, alias, outer->crlf_surface))
    return false;

  if (alias = declare_alias (outer, "pc", "IBM-PC"), !alias)
    return false;
  if (!declare_implied_surface (outer, alias, outer->crlf_surface))
    return false;

  return true;
}

void
delmodule_ibmpc (RECODE_OUTER outer _GL_UNUSED_PARAMETER)
{
}

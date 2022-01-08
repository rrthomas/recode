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

static const char *const translation_table[128] =
  {
    NULL,			/* 128 */
    NULL,			/* 129 */
    NULL,			/* 130 */
    NULL,			/* 131 */
    NULL,			/* 132 */
    NULL,			/* 133 */
    NULL,			/* 134 */
    NULL,			/* 135 */
    NULL,			/* 136 */
    NULL,			/* 137 */
    NULL,			/* 138 */
    NULL,			/* 139 */
    NULL,			/* 140 */
    NULL,			/* 141 */
    NULL,			/* 142 */
    NULL,			/* 143 */
    NULL,			/* 144 */
    NULL,			/* 145 */
    NULL,			/* 146 */
    NULL,			/* 147 */
    NULL,			/* 148 */
    NULL,			/* 149 */
    NULL,			/* 150 */
    NULL,			/* 151 */
    NULL,			/* 152 */
    NULL,			/* 153 */
    NULL,			/* 154 */
    NULL,			/* 155 */
    NULL,			/* 156 */
    NULL,			/* 157 */
    NULL,			/* 158 */
    NULL,			/* 159 */

    " ",			/* 160 no-break space */
    NULL,			/* 161 inverted exclamation mark */
    NULL,			/* 162 cent sign */
    NULL,			/* 163 pound sign */
    NULL,			/* 164 currency sign */
    NULL,			/* 165 yen sign */
    NULL,			/* 166 broken bar */
    NULL,			/* 167 paragraph sign, section sign */
    NULL,			/* 168 diaeresis */
    NULL,			/* 169 copyright sign */
    NULL,			/* 170 feminine ordinal indicator */
    "<\b\"",			/* 171 left angle quotation mark */
    NULL,			/* 172 not sign */
    NULL,			/* 173 soft hyphen */
    NULL,			/* 174 registered trade mark sign */
    NULL,			/* 175 macron */
    NULL,			/* 176 degree sign */
    NULL,			/* 177 plus-minus sign */
    NULL,			/* 178 superscript two */
    NULL,			/* 179 superscript three */
    NULL,			/* 180 acute accent */
    NULL,			/* 181 small greek mu, micro sign */
    NULL,			/* 182 pilcrow sign */
    NULL,			/* 183 middle dot */
    NULL,			/* 184 cedilla */
    NULL,			/* 185 superscript one */
    NULL,			/* 186 masculine ordinal indicator */
    ">\b\"",			/* 187 right angle quotation mark */
    NULL,			/* 188 vulgar fraction one quarter */
    NULL,			/* 189 vulgar fraction one half */
    NULL,			/* 190 vulgar fraction three quarters */
    NULL,			/* 191 inverted question mark */
    "`\bA",			/* 192 capital A with grave accent */
    "'\bA",			/* 193 capital A with acute accent */
    "^\bA",			/* 194 capital A with circumflex accent */
    "~\bA",			/* 195 capital A with tilde */
    "\"\bA",			/* 196 capital A diaeresis */
    NULL,			/* 197 capital A with ring above */
    NULL,			/* 198 capital diphthong A with E */
    ",\bC",			/* 199 capital C with cedilla */
    "`\bE",			/* 200 capital E with grave accent */
    "'\bE",			/* 201 capital E with acute accent */
    "^\bE",			/* 202 capital E with circumflex accent */
    "\"\bE",			/* 203 capital E with diaeresis */
    "`\bI",			/* 204 capital I with grave accent */
    "'\bI",			/* 205 capital I with acute accent */
    "^\bI",			/* 206 capital I with circumflex accent */
    "\"\bI",			/* 207 capital I with diaeresis */
    NULL,			/* 208 capital icelandic ETH */
    "~\bN",			/* 209 capital N with tilde */
    "`\bO",			/* 210 capital O with grave accent */
    "'\bO",			/* 211 capital O with acute accent */
    "^\bO",			/* 212 capital O with circumflex accent */
    "~\bO",			/* 213 capital O with tilde */
    "\"\bO",			/* 214 capital O with diaeresis */
    NULL,			/* 215 multiplication sign */
    "/\bO",			/* 216 capital O with oblique stroke */
    "`\bU",			/* 217 capital U with grave accent */
    "'\bU",			/* 218 capital U with acute accent */
    "^\bU",			/* 219 capital U with circumflex accent */
    "\"\bU",			/* 220 capital U with diaeresis */
    "'\bY",			/* 221 capital Y with acute accent */
    NULL,			/* 222 capital icelandic THORN */
    "\"\bs",			/* 223 small german sharp s */
    "`\ba",			/* 224 small a with grave accent */
    "'\ba",			/* 225 small a with acute accent */
    "^\ba",			/* 226 small a with circumflex accent */
    "~\ba",			/* 227 small a with tilde */
    "\"\ba",			/* 228 small a with diaeresis */
    NULL,			/* 229 small a with ring above */
    NULL,			/* 230 small diphthong a with e */
    ",\bc",			/* 231 small c with cedilla */
    "`\be",			/* 232 small e with grave accent */
    "'\be",			/* 233 small e with acute accent */
    "^\be",			/* 234 small e with circumflex accent */
    "\"\be",			/* 235 small e with diaeresis */
    "`\bi",			/* 236 small i with grave accent */
    "'\bi",			/* 237 small i with acute accent */
    "^\bi",			/* 238 small i with circumflex accent */
    "\"\bi",			/* 239 small i with diaeresis */
    NULL,			/* 240 small icelandic eth */
    "~\bn",			/* 241 small n with tilde */
    "`\bo",			/* 242 small o with grave accent */
    "'\bo",			/* 243 small o with acute accent */
    "^\bo",			/* 244 small o with circumflex accent */
    "~\bo",			/* 245 small o with tilde */
    "\"\bo",			/* 246 small o with diaeresis */
    NULL,			/* 247 division sign */
    "/\bo",			/* 248 small o with oblique stroke */
    "`\bu",			/* 249 small u with grave accent */
    "'\bu",			/* 250 small u with acute accent */
    "^\bu",			/* 251 small u with circumflex accent */
    "\"\bu",			/* 252 small u with diaeresis */
    "'\by",			/* 253 small y with acute accent */
    NULL,			/* 254 small icelandic thorn */
    "\"\by",			/* 255 small y with diaeresis */
  };

static bool
init_latin1_ascii (RECODE_STEP step,
		   RECODE_CONST_REQUEST request,
		   RECODE_CONST_OPTION_LIST before_options,
		   RECODE_CONST_OPTION_LIST after_options)
{
  RECODE_OUTER outer = request->outer;

  const char **table;
  char *pool;
  unsigned counter;

  if (before_options || after_options)
    return false;

  if (!ALLOC_SIZE (table, 256 * sizeof (char *) + 256, const char *))
    return false;
  pool = (char *) (table + 256);

  for (counter = 0; counter < 128; counter++)
    {
      table[counter] = pool;
      *pool++ = counter;
      *pool++ = NUL;
    }
  for (; counter < 256; counter++)
    table[counter] = translation_table[counter - 128];

  step->step_type = RECODE_BYTE_TO_STRING;
  step->step_table = table;
  step->step_table_term_routine = free;
  return true;
}

bool
module_latin1_ascii (RECODE_OUTER outer)
{
  if (!declare_single (outer, "Latin-1", "ASCII-BS",
		       outer->quality_byte_to_variable,
		       init_latin1_ascii, transform_byte_to_variable))
    return false;

  return true;
}

void
delmodule_latin1_ascii (RECODE_OUTER outer _GL_UNUSED_PARAMETER)
{
}

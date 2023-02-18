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

#define ASCII "ASCII"
/* Previously: #define ASCII "ASCII-BS" */

#include "config.h"
#include "common.h"
#include "decsteps.h"

static unsigned char const ascii_to_ebcdic[256] =
  {
      0,   1,   2,   3,  55,  45,  46,  47,     /*   0 -   7 */
     22,   5,  37,  11,  12,  13,  14,  15,     /*   8 -  15 */
     16,  17,  18,  19,  60,  61,  50,  38,     /*  16 -  23 */
     24,  25,  63,  39,  28,  29,  30,  31,     /*  24 -  31 */
     64,  79, 127, 123,  91, 108,  80, 125,     /*  32 -  39 */
     77,  93,  92,  78, 107,  96,  75,  97,     /*  40 -  47 */
    240, 241, 242, 243, 244, 245, 246, 247,     /*  48 -  55 */
    248, 249, 122,  94,  76, 126, 110, 111,     /*  56 -  63 */
    124, 193, 194, 195, 196, 197, 198, 199,     /*  64 -  71 */
    200, 201, 209, 210, 211, 212, 213, 214,     /*  72 -  79 */
    215, 216, 217, 226, 227, 228, 229, 230,     /*  80 -  87 */
    231, 232, 233,  74, 224,  90,  95, 109,     /*  88 -  95 */
    121, 129, 130, 131, 132, 133, 134, 135,     /*  96 - 103 */
    136, 137, 145, 146, 147, 148, 149, 150,     /* 104 - 111 */
    151, 152, 153, 162, 163, 164, 165, 166,     /* 112 - 119 */
    167, 168, 169, 192, 106, 208, 161,   7,     /* 120 - 127 */
     32,  33,  34,  35,  36,  21,   6,  23,     /* 128 - 135 */
     40,  41,  42,  43,  44,   9,  10,  27,     /* 136 - 143 */
     48,  49,  26,  51,  52,  53,  54,   8,     /* 144 - 151 */
     56,  57,  58,  59,   4,  20,  62, 225,     /* 152 - 159 */
     65,  66,  67,  68,  69,  70,  71,  72,     /* 160 - 167 */
     73,  81,  82,  83,  84,  85,  86,  87,     /* 168 - 175 */
     88,  89,  98,  99, 100, 101, 102, 103,     /* 176 - 183 */
    104, 105, 112, 113, 114, 115, 116, 117,     /* 184 - 191 */
    118, 119, 120, 128, 138, 139, 140, 141,     /* 192 - 199 */
    142, 143, 144, 154, 155, 156, 157, 158,     /* 200 - 207 */
    159, 160, 170, 171, 172, 173, 174, 175,     /* 208 - 215 */
    176, 177, 178, 179, 180, 181, 182, 183,     /* 216 - 223 */
    184, 185, 186, 187, 188, 189, 190, 191,     /* 224 - 231 */
    202, 203, 204, 205, 206, 207, 218, 219,     /* 232 - 239 */
    220, 221, 222, 223, 234, 235, 236, 237,     /* 240 - 247 */
    238, 239, 250, 251, 252, 253, 254, 255,     /* 248 - 255 */
  };

static unsigned char const ascii_to_ebcdic_ccc[256] =
  {
      0,   1,   2,   3,  55,  45,  46,  47,     /*   0 -   7 */
     22,   5,  37,  11,  12,  13,  14,  15,     /*   8 -  15 */
     16,  17,  18,  19,  60,  61,  50,  38,     /*  16 -  23 */
     24,  25,  63,  39,  28,  29,  30,  31,     /*  24 -  31 */
     64,  90, 127, 123,  91, 108,  80, 125,     /*  32 -  39 */
     77,  93,  92,  78, 107,  96,  75,  97,     /*  40 -  47 */
    240, 241, 242, 243, 244, 245, 246, 247,     /*  48 -  55 */
    248, 249, 122,  94,  76, 126, 110, 111,     /*  56 -  63 */
    124, 193, 194, 195, 196, 197, 198, 199,     /*  64 -  71 */
    200, 201, 209, 210, 211, 212, 213, 214,     /*  72 -  79 */
    215, 216, 217, 226, 227, 228, 229, 230,     /*  80 -  87 */
    231, 232, 233,  74, 224,  79,  95, 109,     /*  88 -  95 */
    121, 129, 130, 131, 132, 133, 134, 135,     /*  96 - 103 */
    136, 137, 145, 146, 147, 148, 149, 150,     /* 104 - 111 */
    151, 152, 153, 162, 163, 164, 165, 166,     /* 112 - 119 */
    167, 168, 169, 192, 106, 208, 161,   7,     /* 120 - 127 */
      4,   6,   8,   9,  10,  20,  21,  23,     /* 128 - 135 */
     26,  27, 138, 139, 140, 141, 142, 143,     /* 136 - 143 */
     32,  33,  34,  35,  36,  40,  41,  42,     /* 144 - 151 */
     43,  44, 154, 155, 156, 157, 158, 159,     /* 152 - 159 */
     48,  49,  51,  52,  53,  54,  56,  57,     /* 160 - 167 */
     58,  59, 170, 171, 172, 173, 174, 175,     /* 168 - 175 */
    176, 177, 178, 179, 180, 181, 182, 183,     /* 176 - 183 */
    184, 185, 186, 187, 188, 189, 190, 191,     /* 184 - 191 */
    128,  65,  66,  67,  68,  69,  70,  71,     /* 192 - 199 */
     72,  73, 202, 203, 204, 205, 206, 207,     /* 200 - 207 */
    144,  81,  82,  83,  84,  85,  86,  87,     /* 208 - 215 */
     88,  89, 218, 219, 220, 221, 222, 223,     /* 216 - 223 */
    160, 225,  98,  99, 100, 101, 102, 103,     /* 224 - 231 */
    104, 105, 234, 235, 236, 237, 238, 239,     /* 232 - 239 */
    112, 113, 114, 115, 116, 117, 118, 119,     /* 240 - 247 */
    120,  62, 250, 251, 252, 253, 254, 255,     /* 248 - 255 */
  };

/* This is almost identical to GNU dd's "ibm" table.  In dd's "ibm"
   table, both 91 and 213 recode to 173, both 93 and 229 recode to 189,
   and no character recodes to 74 or 106.  I suspect two errors in there.
   So, I arbitrarily choose to recode 213 by 74 and 229 by 106.  */

static unsigned char const ascii_to_ebcdic_ibm[256] =
  {
      0,   1,   2,   3,  55,  45,  46,  47,     /*   0 -   7 */
     22,   5,  37,  11,  12,  13,  14,  15,     /*   8 -  15 */
     16,  17,  18,  19,  60,  61,  50,  38,     /*  16 -  23 */
     24,  25,  63,  39,  28,  29,  30,  31,     /*  24 -  31 */
     64,  90, 127, 123,  91, 108,  80, 125,     /*  32 -  39 */
     77,  93,  92,  78, 107,  96,  75,  97,     /*  40 -  47 */
    240, 241, 242, 243, 244, 245, 246, 247,     /*  48 -  55 */
    248, 249, 122,  94,  76, 126, 110, 111,     /*  56 -  63 */
    124, 193, 194, 195, 196, 197, 198, 199,     /*  64 -  71 */
    200, 201, 209, 210, 211, 212, 213, 214,     /*  72 -  79 */
    215, 216, 217, 226, 227, 228, 229, 230,     /*  80 -  87 */
    231, 232, 233, 173, 224, 189,  95, 109,     /*  88 -  95 */
    121, 129, 130, 131, 132, 133, 134, 135,     /*  96 - 103 */
    136, 137, 145, 146, 147, 148, 149, 150,     /* 104 - 111 */
    151, 152, 153, 162, 163, 164, 165, 166,     /* 112 - 119 */
    167, 168, 169, 192,  79, 208, 161,   7,     /* 120 - 127 */
     32,  33,  34,  35,  36,  21,   6,  23,     /* 128 - 135 */
     40,  41,  42,  43,  44,   9,  10,  27,     /* 136 - 143 */
     48,  49,  26,  51,  52,  53,  54,   8,     /* 144 - 151 */
     56,  57,  58,  59,   4,  20,  62, 225,     /* 152 - 159 */
     65,  66,  67,  68,  69,  70,  71,  72,     /* 160 - 167 */
     73,  81,  82,  83,  84,  85,  86,  87,     /* 168 - 175 */
     88,  89,  98,  99, 100, 101, 102, 103,     /* 176 - 183 */
    104, 105, 112, 113, 114, 115, 116, 117,     /* 184 - 191 */
    118, 119, 120, 128, 138, 139, 140, 141,     /* 192 - 199 */
    142, 143, 144, 154, 155, 156, 157, 158,     /* 200 - 207 */
    159, 160, 170, 171, 172,  74, 174, 175,     /* 208 - 215 */
    176, 177, 178, 179, 180, 181, 182, 183,     /* 216 - 223 */
    184, 185, 186, 187, 188, 106, 190, 191,     /* 224 - 231 */
    202, 203, 204, 205, 206, 207, 218, 219,     /* 232 - 239 */
    220, 221, 222, 223, 234, 235, 236, 237,     /* 240 - 247 */
    238, 239, 250, 251, 252, 253, 254, 255,     /* 248 - 255 */
  };

static bool
init_ascii_ebcdic (RECODE_STEP step,
		   RECODE_CONST_REQUEST request _GL_UNUSED,
		   RECODE_CONST_OPTION_LIST before_options,
		   RECODE_CONST_OPTION_LIST after_options)
{
  if (before_options || after_options)
    return false;

  step->step_type = RECODE_BYTE_TO_BYTE;
  /* The cast is a way to silently discard the const.  */
  step->step_table = (void *) ascii_to_ebcdic;
  return true;
}

static bool
init_ebcdic_ascii (RECODE_STEP step,
		   RECODE_CONST_REQUEST request,
		   RECODE_CONST_OPTION_LIST before_options,
		   RECODE_CONST_OPTION_LIST after_options)
{
  if (before_options || after_options)
    return false;

  if (step->step_table = recode_invert_table (request->outer, ascii_to_ebcdic),
      step->step_table)
    {
      step->step_type = RECODE_BYTE_TO_BYTE;
      step->step_table_term_routine = free;
      return true;
    }
  else
    return false;
}

static bool
init_ascii_ebcdic_ccc (RECODE_STEP step,
		       RECODE_CONST_REQUEST request _GL_UNUSED,
		       RECODE_CONST_OPTION_LIST before_options,
		       RECODE_CONST_OPTION_LIST after_options)
{
  if (before_options || after_options)
    return false;

  step->step_type = RECODE_BYTE_TO_BYTE;
  /* The cast is a way to silently discard the const.  */
  step->step_table = (void *) ascii_to_ebcdic_ccc;
  return true;
}

static bool
init_ebcdic_ccc_ascii (RECODE_STEP step,
		       RECODE_CONST_REQUEST request,
		       RECODE_CONST_OPTION_LIST before_options,
		       RECODE_CONST_OPTION_LIST after_options)

{
  if (before_options || after_options)
    return false;

  if (step->step_table = recode_invert_table (request->outer, ascii_to_ebcdic_ccc),
      step->step_table)
    {
      step->step_type = RECODE_BYTE_TO_BYTE;
      step->step_table_term_routine = free;
      return true;
    }
  else
    return false;
}

static bool
init_ascii_ebcdic_ibm (RECODE_STEP step,
		       RECODE_CONST_REQUEST request _GL_UNUSED,
		       RECODE_CONST_OPTION_LIST before_options,
		       RECODE_CONST_OPTION_LIST after_options)
{
  if (before_options || after_options)
    return false;

  step->step_type = RECODE_BYTE_TO_BYTE;
  /* The cast is a way to silently discard the const.  */
  step->step_table = (void *) ascii_to_ebcdic_ibm;
  return true;
}

static bool
init_ebcdic_ibm_ascii (RECODE_STEP step,
		       RECODE_CONST_REQUEST request,
		       RECODE_CONST_OPTION_LIST before_options,
		       RECODE_CONST_OPTION_LIST after_options)
{
  if (before_options || after_options)
    return false;

  if (step->step_table = recode_invert_table (request->outer, ascii_to_ebcdic_ibm),
      step->step_table)
    {
      step->step_type = RECODE_BYTE_TO_BYTE;
      step->step_table_term_routine = free;
      return true;
    }
  else
    return false;
}

bool
module_ebcdic (RECODE_OUTER outer)
{
  return
    recode_declare_single (outer, ASCII, "EBCDIC",
		    outer->quality_byte_reversible,
		    init_ascii_ebcdic, recode_transform_byte_to_byte)
    && recode_declare_single (outer, "EBCDIC", ASCII,
		       outer->quality_byte_reversible,
		       init_ebcdic_ascii, recode_transform_byte_to_byte)
    && recode_declare_single (outer, ASCII, "EBCDIC-CCC",
		       outer->quality_byte_reversible,
		       init_ascii_ebcdic_ccc, recode_transform_byte_to_byte)
    && recode_declare_single (outer, "EBCDIC-CCC", ASCII,
		       outer->quality_byte_reversible,
		       init_ebcdic_ccc_ascii, recode_transform_byte_to_byte)
    && recode_declare_single (outer, ASCII, "EBCDIC-IBM",
		       outer->quality_byte_reversible,
		       init_ascii_ebcdic_ibm, recode_transform_byte_to_byte)
    && recode_declare_single (outer, "EBCDIC-IBM", ASCII,
		       outer->quality_byte_reversible,
		       init_ebcdic_ibm_ascii, recode_transform_byte_to_byte);
}

void
delmodule_ebcdic (_GL_UNUSED RECODE_OUTER outer)
{
}

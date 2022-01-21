/* Conversion of files between different charsets and surfaces.
   Copyright © 1993, 94, 96, 97, 98, 99, 00 Free Software Foundation, Inc.
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
#include "decsteps.h"

/* RFC 1345 style description for AtariST (non official).

   NUSHSXEXETEQAKBLBSHTLFVTFFCRSOSIDLD1D2D3D4NKSYEBCNEMSBECFSGSRSUS
   SP! " NbDO% & ' ( ) * + , - . / 0 1 2 3 4 5 6 7 8 9 : ; < = > ?
   AtA B C D E F G H I J K L M N O P Q R S T U V W X Y Z <(//)>'>_
   '!a b c d e f g h i j k l m n o p q r s t u v w x y z (!!!!)'?DT
   C,u:e'a>a:a!aac,e>e:e!i:i>i!A:AAE'aeAEo>o:o!u>u!y:O:U:CtPdYessf2
   a'i'o'u'n?N?-a-o?ININO1214!I<<>>a?o?O/o/oeOEA!A?O?':''/-PICoRgTM
   ijIJA+B+G+D+H+W+Z+X+TjJ+K+L+M+N+S+E+P+ZJQ+R+ShT+N%K%M%P%ZjSECa00
   a*b*G*p*S*s*Myt*F*h*Omd*Iof*(-*P=3+->==<IuIl-:?2DGSb.MRTnS2S3S'm */

/* Andreas Schwab writes:

   There are some characters which I'm not sure about.  When compared with
   IBM865 (which I think is the original source for the Atari charset) the
   characters "bullet operator" and "middle dot" (dec 249/250) are swapped.
   This may be intentional or a bug in the table for IBM865.  Also character
   dec 238 looks more like "element of" than "greek small letter epsilon",
   actually it is a bit too large for both interpretations.  I suppose that
   the other changes were made for copyright reasons.  */

/* Andreas later adds:

   This is a fix for the AtariST encoding table.  It was derived from the
   Omega unicode translation tables, which itself claims to be based on
   ftp://plan9.att.com/plan9/unixsrc/tcs.shar.Z.  Unfortunately i wasn't able
   to access this yet.  */

static const recode_ucs2 pool[256] =
  {
    /*   0 */ 0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
    /*   8 */ 0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F,
    /*  16 */ 0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017,
    /*  24 */ 0x0018, 0x0019, 0x001A, 0x001B, 0x001C, 0x001D, 0x001E, 0x001F,
    /*  32 */ 0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
    /*  40 */ 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
    /*  48 */ 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
    /*  56 */ 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
    /*  64 */ 0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
    /*  72 */ 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
    /*  80 */ 0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
    /*  88 */ 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
    /*  96 */ 0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
    /* 104 */ 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
    /* 112 */ 0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
    /* 120 */ 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
    /* 128 */ 0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x00E0, 0x00E5, 0x00E7,
    /* 136 */ 0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x00EC, 0x00C4, 0x00C5,
    /* 144 */ 0x00C9, 0x00E6, 0x00C6, 0x00F4, 0x00F6, 0x00F2, 0x00FB, 0x00F9,
    /* 152 */ 0x00FF, 0x00D6, 0x00DC, 0x00A2, 0x00A3, 0x00A5, 0x00DF, 0x0192,
    /* 160 */ 0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x00F1, 0x00D1, 0x00AA, 0x00BA,
    /* 168 */ 0x00BF, 0x2310, 0x00AC, 0x00BD, 0x00BC, 0x00A1, 0x00AB, 0x00BB,
    /* 176 */ 0x00E3, 0x00F5, 0x00D8, 0x00F8, 0x0153, 0x0152, 0x00C0, 0x00C3,
    /* 184 */ 0x00D5, 0x00A8, 0x00B4, 0x2020, 0x00B6, 0x00A9, 0x00AE, 0x2122,
    /* 192 */ 0x0133, 0x0132, 0x05D0, 0x05D1, 0x05D2, 0x05D3, 0x05D4, 0x05D5,
    /* 200 */ 0x05D6, 0x05D7, 0x05D8, 0x05D9, 0x05DB, 0x05DC, 0x05DE, 0x05E0,
    /* 208 */ 0x05E1, 0x05E2, 0x05E4, 0x05E6, 0x05E7, 0x05E8, 0x05E9, 0x05EA,
    /* 216 */ 0x05DF, 0x05DA, 0x05DD, 0x05E3, 0x05E5, 0x00A7, 0x2038, 0x221E,
    /* 224 */ 0x03B1, 0x03B2, 0x0393, 0x03C0, 0x03A3, 0x03C3, 0x00B5, 0x03C4,
    /* 232 */ 0x03A6, 0x03B8, 0x2126, 0x03B4, 0x222E, 0x03C6, 0x2208, 0x220F,
    /* 240 */ 0x2261, 0x00B1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00F7, 0x2248,
    /* 248 */ 0x00B0, 0x2022, 0x00B7, 0x221A, 0x207F, 0x00B2, 0x00B3, 0x00AF
  };

static struct strip_data table =
  {
    pool,
    {
      0,    8,   16,   24,   32,   40,   48,   56,   64,   72,   80,   88,
      96,  104,  112,  120,  128,  136,  144,  152,  160,  168,  176,  184,
      192, 200,  208,  216,  224,  232,  240,  248
    }
};

bool
module_atarist (RECODE_OUTER outer)
{
  return declare_strip_data (outer, &table, "AtariST");
}

void
delmodule_atarist (RECODE_OUTER outer _GL_ATTRIBUTE_MAYBE_UNUSED)
{
}

/* Conversion of files between different charsets and surfaces.
   Copyright © 1996, 97, 98, 99, 00 Free Software Foundation, Inc.
   Contributed by François Pinard <pinard@iro.umontreal.ca>, 1996.

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

/* Description of some UCS-2 combinings.  */

#define DONE NOT_A_CHARACTER
#define ELSE BYTE_ORDER_MARK_SWAPPED

static const unsigned short combining_data [] =
  {
    /* Diacriticized letters.  */

    0x00C0, 0x0041, 0x0300, DONE,
    0x00C1, 0x0041, 0x0301, DONE,
    0x00C2, 0x0041, 0x0302, DONE,
    0x00C3, 0x0041, 0x0303, DONE,
    0x00C4, 0x0041, 0x0308, DONE,
    0x00C5, 0x0041, 0x030A, DONE,
    0x00C7, 0x0043, 0x0327, DONE,
    0x00C8, 0x0045, 0x0300, DONE,
    0x00C9, 0x0045, 0x0301, DONE,
    0x00CA, 0x0045, 0x0302, DONE,
    0x00CB, 0x0045, 0x0308, DONE,
    0x00CC, 0x0049, 0x0300, DONE,
    0x00CD, 0x0049, 0x0301, DONE,
    0x00CE, 0x0049, 0x0302, DONE,
    0x00CF, 0x0049, 0x0308, DONE,
    0x00D1, 0x004E, 0x0303, DONE,
    0x00D2, 0x004F, 0x0300, DONE,
    0x00D3, 0x004F, 0x0301, DONE,
    0x00D4, 0x004F, 0x0302, DONE,
    0x00D5, 0x004F, 0x0303, DONE,
    0x00D6, 0x004F, 0x0308, DONE,
    0x00D8, 0x004F, 0x0338, DONE,
    0x00D9, 0x0055, 0x0300, DONE,
    0x00DA, 0x0055, 0x0301, DONE,
    0x00DB, 0x0055, 0x0302, DONE,
    0x00DC, 0x0055, 0x0308, DONE,
    0x00DD, 0x0059, 0x0301, DONE,
    0x00E0, 0x0061, 0x0300, DONE,
    0x00E1, 0x0061, 0x0301, DONE,
    0x00E2, 0x0061, 0x0302, DONE,
    0x00E3, 0x0061, 0x0303, DONE,
    0x00E4, 0x0061, 0x0308, DONE,
    0x00E5, 0x0061, 0x030A, DONE,
    0x00E7, 0x0063, 0x0327, DONE,
    0x00E8, 0x0065, 0x0300, DONE,
    0x00E9, 0x0065, 0x0301, DONE,
    0x00EA, 0x0065, 0x0302, DONE,
    0x00EB, 0x0065, 0x0308, DONE,
    0x00EC, 0x0069, 0x0300, DONE,
    0x00ED, 0x0069, 0x0301, DONE,
    0x00EE, 0x0069, 0x0302, DONE,
    0x00EF, 0x0069, 0x0308, DONE,
    0x00F1, 0x006E, 0x0303, DONE,
    0x00F2, 0x006F, 0x0300, DONE,
    0x00F3, 0x006F, 0x0301, DONE,
    0x00F4, 0x006F, 0x0302, DONE,
    0x00F5, 0x006F, 0x0303, DONE,
    0x00F6, 0x006F, 0x0308, DONE,
    0x00F8, 0x006F, 0x0338, DONE,
    0x00F9, 0x0075, 0x0300, DONE,
    0x00FA, 0x0075, 0x0301, DONE,
    0x00FB, 0x0075, 0x0302, DONE,
    0x00FC, 0x0075, 0x0308, DONE,
    0x00FD, 0x0079, 0x0301, DONE,
    0x00FF, 0x0079, 0x0308, DONE,
    0x0100, 0x0041, 0x0304, DONE,
    0x0101, 0x0061, 0x0304, DONE,
    0x0102, 0x0041, 0x0306, DONE,
    0x0103, 0x0061, 0x0306, DONE,
    0x0104, 0x0041, 0x0328, DONE,
    0x0105, 0x0061, 0x0328, DONE,
    0x0106, 0x0043, 0x0301, DONE,
    0x0107, 0x0063, 0x0301, DONE,
    0x0108, 0x0043, 0x0302, DONE,
    0x0109, 0x0063, 0x0302, DONE,
    0x010A, 0x0043, 0x0307, DONE,
    0x010B, 0x0063, 0x0307, DONE,
    0x010C, 0x0043, 0x030C, DONE,
    0x010D, 0x0063, 0x030C, DONE,
    0x010E, 0x0044, 0x030C, DONE,
    0x010F, 0x0064, 0x030C, DONE,
    0x0110, 0x0044, 0x0335, DONE,
    0x0111, 0x0064, 0x0335, DONE,
    0x0112, 0x0045, 0x0304, DONE,
    0x0113, 0x0065, 0x0304, DONE,
    0x0114, 0x0045, 0x0306, DONE,
    0x0115, 0x0065, 0x0306, DONE,
    0x0116, 0x0045, 0x0307, DONE,
    0x0117, 0x0065, 0x0307, DONE,
    0x0118, 0x0045, 0x0328, DONE,
    0x0119, 0x0065, 0x0328, DONE,
    0x011A, 0x0045, 0x030C, DONE,
    0x011B, 0x0065, 0x030C, DONE,
    0x011C, 0x0047, 0x0302, DONE,
    0x011D, 0x0067, 0x0302, DONE,
    0x011E, 0x0047, 0x0306, DONE,
    0x011F, 0x0067, 0x0306, DONE,
    0x0120, 0x0047, 0x0307, DONE,
    0x0121, 0x0067, 0x0307, DONE,
    0x0122, 0x0047, 0x0327, DONE,
    0x0123, 0x0067, 0x0327, DONE,
    0x0124, 0x0048, 0x0302, DONE,
    0x0125, 0x0068, 0x0302, DONE,
    0x0126, 0x0048, 0x0335, DONE,
    0x0127, 0x0068, 0x0335, DONE,
    0x0128, 0x0049, 0x0303, DONE,
    0x0129, 0x0069, 0x0303, DONE,
    0x012A, 0x0049, 0x0304, DONE,
    0x012B, 0x0069, 0x0304, DONE,
    0x012C, 0x0049, 0x0306, DONE,
    0x012D, 0x0069, 0x0306, DONE,
    0x012E, 0x0049, 0x0328, DONE,
    0x012F, 0x0069, 0x0328, DONE,
    0x0130, 0x0049, 0x0307, DONE,
    0x0134, 0x004A, 0x0302, DONE,
    0x0135, 0x006A, 0x0302, DONE,
    0x0136, 0x004B, 0x0327, DONE,
    0x0137, 0x006B, 0x0327, DONE,
    0x0139, 0x004C, 0x0301, DONE,
    0x013A, 0x006C, 0x0301, DONE,
    0x013B, 0x004C, 0x0327, DONE,
    0x013C, 0x006C, 0x0327, DONE,
    0x013D, 0x004C, 0x030C, DONE,
    0x013E, 0x006C, 0x030C, DONE,
    0x0141, 0x004C, 0x0337, DONE,
    0x0142, 0x006C, 0x0337, DONE,
    0x0143, 0x004E, 0x0301, DONE,
    0x0144, 0x006E, 0x0301, DONE,
    0x0145, 0x004E, 0x0327, DONE,
    0x0146, 0x006E, 0x0327, DONE,
    0x0147, 0x004E, 0x030C, DONE,
    0x0148, 0x006E, 0x030C, DONE,
    0x014C, 0x004F, 0x0304, DONE,
    0x014D, 0x006F, 0x0304, DONE,
    0x014E, 0x004F, 0x0306, DONE,
    0x014F, 0x006F, 0x0306, DONE,
    0x0150, 0x004F, 0x030B, DONE,
    0x0151, 0x006F, 0x030B, DONE,
    0x0154, 0x0052, 0x0301, DONE,
    0x0155, 0x0072, 0x0301, DONE,
    0x0156, 0x0052, 0x0327, DONE,
    0x0157, 0x0072, 0x0327, DONE,
    0x0158, 0x0052, 0x030C, DONE,
    0x0159, 0x0072, 0x030C, DONE,
    0x015A, 0x0053, 0x0301, DONE,
    0x015B, 0x0073, 0x0301, DONE,
    0x015C, 0x0053, 0x0302, DONE,
    0x015D, 0x0073, 0x0302, DONE,
    0x015E, 0x0053, 0x0327, DONE,
    0x015F, 0x0073, 0x0327, DONE,
    0x0160, 0x0053, 0x030C, DONE,
    0x0161, 0x0073, 0x030C, DONE,
    0x0162, 0x0054, 0x0327, DONE,
    0x0163, 0x0074, 0x0327, DONE,
    0x0164, 0x0054, 0x030C, DONE,
    0x0165, 0x0074, 0x030C, DONE,
    0x0166, 0x0054, 0x0335, DONE,
    0x0167, 0x0074, 0x0335, DONE,
    0x0168, 0x0055, 0x0303, DONE,
    0x0169, 0x0075, 0x0303, DONE,
    0x016A, 0x0055, 0x0304, DONE,
    0x016B, 0x0075, 0x0304, DONE,
    0x016C, 0x0055, 0x0306, DONE,
    0x016D, 0x0075, 0x0306, DONE,
    0x016E, 0x0055, 0x030A, DONE,
    0x016F, 0x0075, 0x030A, DONE,
    0x0170, 0x0055, 0x030B, DONE,
    0x0171, 0x0075, 0x030B, DONE,
    0x0172, 0x0055, 0x0328, DONE,
    0x0173, 0x0075, 0x0328, DONE,
    0x0174, 0x0057, 0x0302, DONE,
    0x0175, 0x0077, 0x0302, DONE,
    0x0176, 0x0059, 0x0302, DONE,
    0x0177, 0x0079, 0x0302, DONE,
    0x0178, 0x0059, 0x0308, DONE,
    0x0179, 0x005A, 0x0301, DONE,
    0x017A, 0x007A, 0x0301, DONE,
    0x017B, 0x005A, 0x0307, DONE,
    0x017C, 0x007A, 0x0307, DONE,
    0x017D, 0x005A, 0x030C, DONE,
    0x017E, 0x007A, 0x030C, DONE,
    0x0180, 0x0062, 0x0335, DONE,
    0x0197, 0x0049, 0x0335, DONE,
    0x019A, 0x006C, 0x0335, DONE,
    0x019B, 0x03BB, 0x0335, DONE,
    0x019F, 0x004F, 0x0335, DONE,
    0x01A0, 0x004F, 0x031B, DONE,
    0x01A1, 0x006F, 0x031B, DONE,
    0x01AB, 0x0074, 0x0321, DONE,
    0x01AE, 0x0054, 0x0322, DONE,
    0x01AF, 0x0055, 0x031B, DONE,
    0x01B0, 0x0075, 0x031B, DONE,
    0x01CD, 0x0041, 0x030C, DONE,
    0x01CE, 0x0061, 0x030C, DONE,
    0x01CF, 0x0049, 0x030C, DONE,
    0x01D0, 0x0069, 0x030C, DONE,
    0x01D1, 0x004F, 0x030C, DONE,
    0x01D2, 0x006F, 0x030C, DONE,
    0x01D3, 0x0055, 0x030C, DONE,
    0x01D4, 0x0075, 0x030C, DONE,
    0x01D5, 0x0055, 0x0308, 0x0304, ELSE, 0x00DC, 0x0304, DONE,
    0x01D6, 0x0075, 0x0308, 0x0304, ELSE, 0x00FC, 0x0304, DONE,
    0x01D7, 0x0055, 0x0308, 0x0301, ELSE, 0x00DC, 0x0301, DONE,
    0x01D8, 0x0075, 0x0308, 0x0301, ELSE, 0x00FC, 0x0301, DONE,
    0x01D9, 0x0055, 0x0308, 0x030C, ELSE, 0x00DC, 0x030C, DONE,
    0x01DA, 0x0075, 0x0308, 0x030C, ELSE, 0x00FC, 0x030C, DONE,
    0x01DB, 0x0055, 0x0308, 0x0300, ELSE, 0x00DC, 0x0300, DONE,
    0x01DC, 0x0075, 0x0308, 0x0300, ELSE, 0x00FC, 0x0300, DONE,
    0x01DE, 0x0041, 0x0308, 0x0304, ELSE, 0x00C4, 0x0304, DONE,
    0x01DF, 0x0061, 0x0308, 0x0304, ELSE, 0x00E4, 0x0304, DONE,
    0x01E0, 0x0041, 0x0307, 0x0304, DONE,
    0x01E1, 0x0061, 0x0307, 0x0304, DONE,
    0x01E2, 0x00C6, 0x0304, DONE,
    0x01E3, 0x00E6, 0x0304, DONE,
    0x01E4, 0x0047, 0x0335, DONE,
    0x01E5, 0x0067, 0x0335, DONE,
    0x01E6, 0x0047, 0x030C, DONE,
    0x01E7, 0x0067, 0x030C, DONE,
    0x01E8, 0x004B, 0x030C, DONE,
    0x01E9, 0x006B, 0x030C, DONE,
    0x01EA, 0x004F, 0x0328, DONE,
    0x01EB, 0x006F, 0x0328, DONE,
    0x01EC, 0x004F, 0x0328, 0x0304, ELSE, 0x01EA, 0x0304, DONE,
    0x01ED, 0x006F, 0x0328, 0x0304, ELSE, 0x01EB, 0x0304, DONE,
    0x01EE, 0x01B7, 0x030C, DONE,
    0x01EF, 0x0292, 0x030C, DONE,
    0x01F0, 0x006A, 0x030C, DONE,
    0x0386, 0x0391, 0x0384, DONE,
    0x0388, 0x0395, 0x0384, DONE,
    0x0389, 0x0397, 0x0384, DONE,
    0x038A, 0x0399, 0x0384, DONE,
    0x038C, 0x039F, 0x0384, DONE,
    0x038E, 0x03A5, 0x0384, DONE,
    0x038F, 0x03A9, 0x0384, DONE,
    0x0390, 0x03B9, 0x0385, DONE,
    0x03AA, 0x0399, 0x0308, DONE,
    0x03AB, 0x03A5, 0x0308, DONE,
    0x03AC, 0x03B1, 0x0384, DONE,
    0x03AD, 0x03B5, 0x0384, DONE,
    0x03AE, 0x03B7, 0x0384, DONE,
    0x03AF, 0x03B9, 0x0384, DONE,
    0x03B0, 0x03C5, 0x0385, DONE,
    0x03CA, 0x03B9, 0x0308, DONE,
    0x03CB, 0x03C5, 0x0308, DONE,
    0x03CC, 0x03BF, 0x0384, DONE,
    0x03CD, 0x03C5, 0x0384, DONE,
    0x03CE, 0x03C9, 0x0384, DONE,
    0x03D3, 0x03D2, 0x0384, DONE,
    0x03D4, 0x03D2, 0x0308, DONE,
    0x0401, 0x0415, 0x0308, DONE,
    0x0403, 0x0413, 0x0301, DONE,
    0x0407, 0x0406, 0x0308, DONE,
    0x040C, 0x041A, 0x0301, DONE,
    0x040E, 0x0423, 0x0306, DONE,
    0x0419, 0x0418, 0x0306, DONE,
    0x0439, 0x0438, 0x0306, DONE,
    0x0451, 0x0435, 0x0308, DONE,
    0x0453, 0x0433, 0x0301, DONE,
    0x0457, 0x0456, 0x0308, DONE,
    0x045C, 0x043A, 0x0301, DONE,
    0x045E, 0x0443, 0x0306, DONE,
    0x0476, 0x0474, 0x030F, DONE,
    0x0477, 0x0475, 0x030F, DONE,
    0x0492, 0x0413, 0x0335, DONE,
    0x0493, 0x0433, 0x0335, DONE,
    0x0498, 0x0417, 0x0327, DONE,
    0x0499, 0x0437, 0x0327, DONE,
    0x04AA, 0x0421, 0x0327, DONE,
    0x04AB, 0x0441, 0x0327, DONE,
    0x04B0, 0x04AE, 0x0335, DONE,
    0x04B1, 0x04AF, 0x0335, DONE,
    0x04BE, 0x04BC, 0x0328, DONE,
    0x04BF, 0x04BD, 0x0328, DONE,
    0x04C1, 0x0416, 0x0306, DONE,
    0x04C2, 0x0436, 0x0306, DONE,
    0x04C5, 0x041A, 0x0328, DONE,
    0x04C6, 0x043A, 0x0328, DONE,
    0x04C9, 0x0425, 0x0328, DONE,
    0x04CA, 0x0445, 0x0328, DONE,
    0x0958, 0x0915, 0x093C, DONE,
    0x0959, 0x0916, 0x093C, DONE,
    0x095A, 0x0917, 0x093C, DONE,
    0x095B, 0x091C, 0x093C, DONE,
    0x095C, 0x0921, 0x093C, DONE,
    0x095D, 0x0922, 0x093C, DONE,
    0x095E, 0x092B, 0x093C, DONE,
    0x095F, 0x092F, 0x093C, DONE,
    0x09DC, 0x09A1, 0x09BC, DONE,
    0x09DD, 0x09A2, 0x09BC, DONE,
    0x09DF, 0x09AF, 0x09BC, DONE,
    0x0A59, 0x0A16, 0x0A3C, DONE,
    0x0A5A, 0x0A17, 0x0A3C, DONE,
    0x0A5B, 0x0A1C, 0x0A3C, DONE,
    0x0A5C, 0x0A21, 0x0A3C, DONE,
    0x0A5E, 0x0A2B, 0x0A3C, DONE,
    0x0B5C, 0x0B21, 0x0B3C, DONE,
    0x0B5D, 0x0B22, 0x0B3C, DONE,
    0x0B5F, 0x0B2F, 0x0B3C, DONE,
    0x1014, 0x1004, 0x104C, DONE,
    0x1015, 0x1005, 0x104C, DONE,
    0x1016, 0x1006, 0x104C, DONE,

    /* Ligatures, digraphs.  */

    0x00C6, 0x0041, 0x0045, DONE,
    0x00E6, 0x0061, 0x0065, DONE,
    0x0132, 0x0049, 0x004A, DONE,
    0x0133, 0x0069, 0x006A, DONE,
    0x013F, 0x004C, 0x00B7, DONE,
    0x0140, 0x006C, 0x00B7, DONE,
    0x0149, 0x0027, 0x006E, DONE,
    0x0152, 0x004F, 0x0045, DONE,
    0x0153, 0x006F, 0x0065, DONE,
    0x01C4, 0x0044, 0x005A, 0x030C, ELSE, 0x0044, 0x017D, DONE,
    0x01C5, 0x0044, 0x007A, 0x030C, ELSE, 0x0044, 0x017E, DONE,
    0x01C6, 0x0064, 0x007A, 0x030C, ELSE, 0x0064, 0x017E, DONE,
    0x01C7, 0x004C, 0x004A, DONE,
    0x01C8, 0x004C, 0x006A, DONE,
    0x01C9, 0x006C, 0x006A, DONE,
    0x01CA, 0x004E, 0x004A, DONE,
    0x01CB, 0x004E, 0x006A, DONE,
    0x01CC, 0x006E, 0x006A, DONE,
    0x02A3, 0x0064, 0x007A, DONE,
    0x02A4, 0x0064, 0x0292, DONE,
    0x02A5, 0x0064, 0x0291, DONE,
    0x02A6, 0x0074, 0x0073, DONE,
    0x02A7, 0x0074, 0x0283, DONE,
    0x02A8, 0x0074, 0x0255, DONE,
    0x0409, 0x041B, 0x042C, DONE,
    0x040A, 0x041D, 0x042C, DONE,
    0x0459, 0x043B, 0x044C, DONE,
    0x045A, 0x043D, 0x044C, DONE,
    0x0EDC, 0x0EAB, 0x0E99, DONE,
    0x0EDD, 0x0EAB, 0x0EA1, DONE,
    0x203C, 0x0021, 0x0021, DONE,
    0x203D, 0x003F, 0x0021, DONE,
    DONE
  };

/* UCS-2 input and output.  */

/*-------------------------------------------------------------------------.
| Get one UCS-2 VALUE for TASK, maybe swapping pair of bytes as we go.     |
| Whenever a byte order mark is seen, either straight or swapped, always   |
| use it to decide whether itself and subsequent UCS-2 values should be    |
| swapped, or not.  At the very beginning of the text stream, a byte order |
| mark is merely swallowed and never returned.  Everywhere else, it is     |
| transmitted as a zero-width non-breaking space.                          |
`-------------------------------------------------------------------------*/

/* An UCS-2 file canonically has a byte order mark at its very beginning.
   So, concatenating many UCS-2 files might produce some spurious, but valid
   zero-width non-breaking spaces.  Those are produced for each file, after
   the first, starting with a byte order mark, regardless of byte order.  */

bool
get_ucs2 (unsigned *value, RECODE_SUBTASK subtask)
{
  while (true)
    {
      int character1;
      int character2;
      unsigned chunk;

      character1 = get_byte (subtask);
      if (character1 == EOF)
        return false;
      character2 = get_byte (subtask);
      if (character2 == EOF)
        {
          recode_if_nogo (RECODE_INVALID_INPUT, subtask);
          return false;
        }

      switch (subtask->task->swap_input)
        {
        case RECODE_SWAP_UNDECIDED:
          chunk = ((BIT_MASK (8) & character1) << 8) | (BIT_MASK (8) & character2);
          switch (chunk)
            {
            case BYTE_ORDER_MARK:
              subtask->task->swap_input = RECODE_SWAP_NO;
              break;

            case BYTE_ORDER_MARK_SWAPPED:
              subtask->task->swap_input = RECODE_SWAP_YES;
              break;

            default:
              *value = chunk;
              subtask->task->swap_input = RECODE_SWAP_NO;
              if (subtask->task->byte_order_mark)
                RETURN_IF_NOGO (RECODE_NOT_CANONICAL, subtask);
              return true;
            }
          break;

        case RECODE_SWAP_NO:
          chunk = ((BIT_MASK (8) & character1) << 8) | (BIT_MASK (8) & character2);
          switch (chunk)
            {
            case BYTE_ORDER_MARK:
              RETURN_IF_NOGO (RECODE_NOT_CANONICAL, subtask);
              break;

            case BYTE_ORDER_MARK_SWAPPED:
              subtask->task->swap_input = RECODE_SWAP_YES;
              RETURN_IF_NOGO (RECODE_NOT_CANONICAL, subtask);
              break;

            default:
              *value = chunk;
              return true;
            }
          break;

        case RECODE_SWAP_YES:
          chunk = ((BIT_MASK (8) & character2) << 8) | (BIT_MASK (8) & character1);
          switch (chunk)
            {
            case BYTE_ORDER_MARK:
              RETURN_IF_NOGO (RECODE_NOT_CANONICAL, subtask);
              break;

            case BYTE_ORDER_MARK_SWAPPED:
              subtask->task->swap_input = RECODE_SWAP_NO;
              RETURN_IF_NOGO (RECODE_NOT_CANONICAL, subtask);
              break;

            default:
              *value = chunk;
              return true;
            }
          break;

        default:
          break;
        }
    }
}

/*-------------------------------.
| Put one UCS-2 VALUE for TASK.  |
`-------------------------------*/

bool
put_ucs2 (unsigned value, RECODE_SUBTASK subtask)
{
  put_byte (BIT_MASK (8) & value >> 8, subtask);
  put_byte (BIT_MASK (8) & value, subtask);
  return true;
}

/* UCS-4 input and output.  */

/*-------------------------------.
| Get one UCS-4 VALUE for TASK.  |
`-------------------------------*/

bool
get_ucs4 (unsigned *value, RECODE_SUBTASK subtask)
{
  int character;
  unsigned chunk;

  character = get_byte (subtask);
  if (character == EOF)
    return false;
  chunk = (BIT_MASK (8) & character) << 24;

  character = get_byte (subtask);
  if (character == EOF)
    {
      recode_if_nogo (RECODE_INVALID_INPUT, subtask);
      return false;
    }
  chunk |= (BIT_MASK (8) & character) << 16;

  character = get_byte (subtask);
  if (character == EOF)
    {
      recode_if_nogo (RECODE_INVALID_INPUT, subtask);
      return false;
    }
  chunk |= (BIT_MASK (8) & character) << 8;

  character = get_byte (subtask);
  if (character == EOF)
    {
      recode_if_nogo (RECODE_INVALID_INPUT, subtask);
      return false;
    }
  chunk |= BIT_MASK (8) & character;

  *value = chunk;
  return true;
}

/*-------------------------------.
| Put one UCS-4 VALUE for TASK.  |
`-------------------------------*/

bool
put_ucs4 (unsigned value, RECODE_SUBTASK subtask)
{
  put_byte (BIT_MASK (8) & value >> 24, subtask);
  put_byte (BIT_MASK (8) & value >> 16, subtask);
  put_byte (BIT_MASK (8) & value >> 8, subtask);
  put_byte (BIT_MASK (8) & value, subtask);
  return true;
}

/* Provided steps.  */

/*-----------.
| Combined.  |
`-----------*/

static bool
init_combined_ucs2 (RECODE_STEP step,
                    RECODE_CONST_REQUEST request,
                    RECODE_CONST_OPTION_LIST before_options,
                    RECODE_CONST_OPTION_LIST after_options)
{
  step->before->data_type = RECODE_EXPLODE_DATA;
  step->before->data = (void *) combining_data;
  return init_explode (step, request, before_options, after_options);
}

static bool
init_ucs2_combined (RECODE_STEP step,
                    RECODE_CONST_REQUEST request,
                    RECODE_CONST_OPTION_LIST before_options,
                    RECODE_CONST_OPTION_LIST after_options)
{
  step->after->data_type = RECODE_EXPLODE_DATA;
  step->after->data = (void *) combining_data;
  return init_combine (step, request, before_options, after_options);
}

/*-----------------------------.
| Transform Latin-1 to UCS-4.  |
`-----------------------------*/

static bool
transform_latin1_ucs4 (RECODE_SUBTASK subtask)
{
  int character;

  while (character = get_byte (subtask), character != EOF)
    put_ucs4 (BIT_MASK (8) & character, subtask);

  SUBTASK_RETURN (subtask);
}

/*---------------------------.
| Transform UCS-2 to UCS-4.  |
`---------------------------*/

static bool
transform_ucs2_ucs4 (RECODE_SUBTASK subtask)
{
  unsigned value;

  while (get_ucs2 (&value, subtask))
    put_ucs4 (value, subtask);

  SUBTASK_RETURN (subtask);
}

/*-----------------------------------------------------------------.
| Declare the basic UCS-2 and UCS-4 charsets and transformations.  |
`-----------------------------------------------------------------*/

bool
module_ucs (RECODE_OUTER outer)
{
  return
    declare_single (outer, "combined-UCS-2", "ISO-10646-UCS-2",
                    outer->quality_ucs2_to_variable,
                    init_combined_ucs2, explode_ucs2_ucs2)
    && declare_single (outer, "ISO-10646-UCS-2", "combined-UCS-2",
                       outer->quality_variable_to_ucs2,
                       init_ucs2_combined, combine_ucs2_ucs2)
    && declare_single (outer, "latin1", "ISO-10646-UCS-4",
                       outer->quality_byte_to_variable,
                       NULL, transform_latin1_ucs4)
    && declare_single (outer, "ISO-10646-UCS-2", "ISO-10646-UCS-4",
                       outer->quality_variable_to_variable,
                       NULL, transform_ucs2_ucs4)

    && declare_alias (outer, "UCS", "ISO-10646-UCS-4")
    && declare_alias (outer, "UCS-4", "ISO-10646-UCS-4")
    && declare_alias (outer, "ISO_10646", "ISO-10646-UCS-4")
    && declare_alias (outer, "10646", "ISO-10646-UCS-4")
    && declare_alias (outer, "u4", "ISO-10646-UCS-4")

    && declare_alias (outer, "UCS-2", "ISO-10646-UCS-2")
    && declare_alias (outer, "UNICODE-1-1", "ISO-10646-UCS-2") /* RFC1641 */
    && declare_alias (outer, "BMP", "ISO-10646-UCS-2")
    && declare_alias (outer, "u2", "ISO-10646-UCS-2")
    && declare_alias (outer, "rune", "ISO-10646-UCS-2")

    && declare_alias (outer, "co", "combined-UCS-2");
}

void
delmodule_ucs (_GL_UNUSED RECODE_OUTER outer)
{
}

/* Vietnamese charset processing.
   Copyright © 1999-2018 Progiciels Bourbeau-Pinard, Inc.
   Contributed by François Pinard <pinard@iro.umontreal.ca>, 1999.

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

static const recode_ucs2 tcvn_pool[256] =
  {
    /*   0 */ 0x0000, 0x00DA, 0x1EE4, 0x0003, 0x1EEA, 0x1EEC, 0x1EEE, 0x0007,
    /*   8 */ 0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F,
    /*  16 */ 0x0010, 0x1EE8, 0x1EF0, 0x1EF2, 0x1EF6, 0x1EF8, 0x00DD, 0x1EF4,
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
    /* 128 */ 0x00C0, 0x1EA2, 0x00C3, 0x00C1, 0x1EA0, 0x1EB6, 0x1EAC, 0x00C8,
    /* 136 */ 0x1EBA, 0x1EBC, 0x00C9, 0x1EB8, 0x1EC6, 0x00CC, 0x1EC8, 0x0128,
    /* 144 */ 0x00CD, 0x1ECA, 0x00D2, 0x1ECE, 0x00D5, 0x00D3, 0x1ECC, 0x1ED8,
    /* 152 */ 0x1EDC, 0x1EDE, 0x1EE0, 0x1EDA, 0x1EE2, 0x00D9, 0x1EE6, 0x0168,
    /* 160 */ 0xFFFF, 0x0102, 0x00C2, 0x00CA, 0x00D4, 0x01A0, 0x01AF, 0x0110,
    /* 168 */ 0x0103, 0x00E2, 0x00EA, 0x00F4, 0x01A1, 0x01B0, 0x0111, 0x1EB0,
    /* 176 */ 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x00E0, 0x1EA3, 0x00E3,
    /* 184 */ 0x00E1, 0x1EA1, 0x1EB2, 0x1EB1, 0x1EB3, 0x1EB5, 0x1EAF, 0x1EB4,
    /* 192 */ 0x1EAE, 0x1EA6, 0x1EA8, 0x1EAA, 0x1EA4, 0x1EC0, 0x1EB7, 0x1EA7,
    /* 200 */ 0x1EA9, 0x1EAB, 0x1EA5, 0x1EAD, 0x00E8, 0x1EC2, 0x1EBB, 0x1EBD,
    /* 208 */ 0x00E9, 0x1EB9, 0x1EC1, 0x1EC3, 0x1EC5, 0x1EBF, 0x1EC7, 0x00EC,
    /* 216 */ 0x1EC9, 0x1EC4, 0x1EBE, 0x1ED2, 0x0129, 0x00ED, 0x1ECB, 0x00F2,
    /* 224 */ 0x1ED4, 0x1ECF, 0x00F5, 0x00F3, 0x1ECD, 0x1ED3, 0x1ED5, 0x1ED7,
    /* 232 */ 0x1ED1, 0x1ED9, 0x1EDD, 0x1EDF, 0x1EE1, 0x1EDB, 0x1EE3, 0x00F9,
    /* 240 */ 0x1ED6, 0x1EE7, 0x0169, 0x00FA, 0x1EE5, 0x1EEB, 0x1EED, 0x1EEF,
    /* 248 */ 0x1EE9, 0x1EF1, 0x1EF3, 0x1EF7, 0x1EF9, 0x00FD, 0x1EF5, 0x1ED0,
  };

static struct strip_data tcvn_strip_data =
  {
    tcvn_pool,
    {
      0,    8,   16,   24,   32,   40,   48,   56,   64,   72,   80,   88,
      96,  104,  112,  120,  128,  136,  144,  152,  160,  168,  176,  184,
      192, 200,  208,  216,  224,  232,  240,  248
    }
  };

static const recode_ucs2 viscii_pool[256] =
  {
    /*   0 */ 0x0000, 0x0001, 0x1EB2, 0x0003, 0x0004, 0x1EB4, 0x1EAA, 0x0007,
    /*   8 */ 0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F,
    /*  16 */ 0x0010, 0x0011, 0x0012, 0x0013, 0x1EF6, 0x0015, 0x0016, 0x0017,
    /*  24 */ 0x0018, 0x1EF8, 0x001A, 0x001B, 0x001C, 0x001D, 0x1EF4, 0x001F,
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
    /* 128 */ 0x1EA0, 0x1EAE, 0x1EB0, 0x1EB6, 0x1EA4, 0x1EA6, 0x1EA8, 0x1EAC,
    /* 136 */ 0x1EBC, 0x1EB8, 0x1EBE, 0x1EC0, 0x1EC2, 0x1EC4, 0x1EC6, 0x1ED0,
    /* 144 */ 0x1ED2, 0x1ED4, 0x1ED6, 0x1ED8, 0x1EE2, 0x1EDA, 0x1EDC, 0x1EDE,
    /* 152 */ 0x1ECA, 0x1ECE, 0x1ECC, 0x1EC8, 0x1EE6, 0x0168, 0x1EE4, 0x1EF2,
    /* 160 */ 0x00D5, 0x1EAF, 0x1EB1, 0x1EB7, 0x1EA5, 0x1EA7, 0x1EA9, 0x1EAD,
    /* 168 */ 0x1EBD, 0x1EB9, 0x1EBF, 0x1EC1, 0x1EC3, 0x1EC5, 0x1EC7, 0x1ED1,
    /* 176 */ 0x1ED3, 0x1ED5, 0x1ED7, 0x1EE0, 0x01A0, 0x1ED9, 0x1EDD, 0x1EDF,
    /* 184 */ 0x1ECB, 0x1EF0, 0x1EE8, 0x1EEA, 0x1EEC, 0x01A1, 0x1EDB, 0x01AF,
    /* 192 */ 0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x1EA2, 0x0102, 0x1EB3, 0x1EB5,
    /* 200 */ 0x00C8, 0x00C9, 0x00CA, 0x1EBA, 0x00CC, 0x00CD, 0x0128, 0x1EF3,
    /* 208 */ 0x0110, 0x1EE9, 0x00D2, 0x00D3, 0x00D4, 0x1EA1, 0x1EF7, 0x1EEB,
    /* 216 */ 0x1EED, 0x00D9, 0x00DA, 0x1EF9, 0x1EF5, 0x00DD, 0x1EE1, 0x01B0,
    /* 224 */ 0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x1EA3, 0x0103, 0x1EEF, 0x1EAB,
    /* 232 */ 0x00E8, 0x00E9, 0x00EA, 0x1EBB, 0x00EC, 0x00ED, 0x0129, 0x1EC9,
    /* 240 */ 0x0111, 0x1EF1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x1ECF, 0x1ECD,
    /* 248 */ 0x1EE5, 0x00F9, 0x00FA, 0x0169, 0x1EE7, 0x00FD, 0x1EE3, 0x1EEE,
  };

static struct strip_data viscii_strip_data =
  {
    viscii_pool,
    {
      0,    8,   16,   24,   32,   40,   48,   56,   64,   72,   80,   88,
      96,  104,  112,  120,  128,  136,  144,  152,  160,  168,  176,  184,
      192, 200,  208,  216,  224,  232,  240,  248
    }
  };

static const recode_ucs2 vps_pool[256] =
  {
    /*   0 */ 0x0000, 0x0001, 0x1EA0, 0x1EAC, 0x1EB6, 0x1EB8, 0x1EC6, 0x0007,
    /*   8 */ 0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F,
    /*  16 */ 0x1ECA, 0x1ECC, 0x1ED8, 0x1EE2, 0x1EE4, 0x1EF0, 0x0016, 0x0017,
    /*  24 */ 0x0018, 0x1EF4, 0x001A, 0x001B, 0x1EAA, 0x1EEE, 0x001E, 0x001F,
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
    /* 128 */ 0x00C0, 0x1EA2, 0x00C3, 0x1EA4, 0x1EA6, 0x1EA8, 0x1ECD, 0x1ED7,
    /* 136 */ 0x0102, 0x1EBF, 0x1EC1, 0x1EC3, 0x1EC7, 0x1EAE, 0x1EB0, 0x1EB2,
    /* 144 */ 0x1EBE, 0xFFFF, 0xFFFF, 0x1EC0, 0x1EC2, 0x1EC4, 0x1ED0, 0x1ED2,
    /* 152 */ 0x1ED4, 0xFFFF, 0x00FD, 0x1EF7, 0x1EF5, 0x1EDA, 0x1EDC, 0x1EDE,
    /* 160 */ 0xFFFF, 0x1EAF, 0x1EB1, 0x1EB3, 0x1EB5, 0x1EB7, 0x1EE0, 0x1EDB,
    /* 168 */ 0x00D9, 0x1EDD, 0x1EDF, 0x1EE1, 0x0168, 0x1EE8, 0x1EE3, 0x1EEA,
    /* 176 */ 0x1ED5, 0x1EEC, 0x1EF2, 0x1EF8, 0x00CD, 0x00CC, 0x1ED9, 0x1EC8,
    /* 184 */ 0x0128, 0x00D3, 0x1EED, 0x1EEF, 0x00D2, 0x1ECE, 0x00D5, 0x1EF1,
    /* 192 */ 0x1EA7, 0x00C1, 0x00C2, 0x1EA5, 0x1EA9, 0x1EAB, 0x1EAD, 0x0111,
    /* 200 */ 0x1EBB, 0x00C9, 0x00CA, 0x1EB9, 0x1EC9, 0x1EC5, 0x1ECB, 0x1EF9,
    /* 208 */ 0x01AF, 0x1EE6, 0x1ED3, 0x1ED1, 0x00D4, 0x1ECF, 0x01A1, 0x00C8,
    /* 216 */ 0x1EEB, 0x1EE9, 0x00DA, 0x0169, 0x01B0, 0x00DD, 0x1EBA, 0xFFFF,
    /* 224 */ 0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x1EA3, 0x1EA1, 0x0103, 0xFFFF,
    /* 232 */ 0x00E8, 0x00E9, 0x00EA, 0x1EBD, 0x00EC, 0x00ED, 0xFFFF, 0x0129,
    /* 240 */ 0x1EB4, 0x0110, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0xFFFF, 0x01A0,
    /* 248 */ 0x1EE5, 0x00F9, 0x00FA, 0x1EE7, 0xFFFF, 0x1EF6, 0x1EBC, 0x1EF3,
  };

static struct strip_data vps_strip_data =
  {
    vps_pool,
    {
      0,    8,   16,   24,   32,   40,   48,   56,   64,   72,   80,   88,
      96,  104,  112,  120,  128,  136,  144,  152,  160,  168,  176,  184,
      192, 200,  208,  216,  224,  232,  240,  248
    }
  };

static const unsigned short viscii_viqr_data[] =
  {
    2, 'A', '(', '?', DONE,
    5, 'A', '(', '~', DONE,
    6, 'A', '^', '~', DONE,
    20, 'Y', '?', DONE,
    25, 'Y', '~', DONE,
    30, 'Y', '.', DONE,
    128, 'A', '.', DONE,
    129, 'A', '(', '\'', DONE,
    130, 'A', '(', '`', DONE,
    131, 'A', '(', '.', DONE,
    132, 'A', '^', '\'', DONE,
    133, 'A', '^', '`', DONE,
    134, 'A', '^', '?', DONE,
    135, 'A', '^', '.', DONE,
    136, 'E', '~', DONE,
    137, 'E', '.', DONE,
    138, 'E', '^', '\'', DONE,
    139, 'E', '^', '`', DONE,
    140, 'E', '^', '?', DONE,
    141, 'E', '^', '~', DONE,
    142, 'E', '^', '.', DONE,
    143, 'O', '^', '\'', DONE,
    144, 'O', '^', '`', DONE,
    145, 'O', '^', '?', DONE,
    146, 'O', '^', '~', DONE,
    147, 'O', '^', '.', DONE,
    148, 'O', '+', '.', DONE,
    149, 'O', '+', '\'', DONE,
    150, 'O', '+', '`', DONE,
    151, 'O', '+', '?', DONE,
    152, 'I', '.', DONE,
    153, 'O', '?', DONE,
    154, 'O', '.', DONE,
    155, 'I', '?', DONE,
    156, 'U', '?', DONE,
    157, 'U', '~', DONE,
    158, 'U', '.', DONE,
    159, 'Y', '`', DONE,
    160, 'O', '~', DONE,
    161, 'a', '(', '\'', DONE,
    162, 'a', '(', '`', DONE,
    163, 'a', '(', '.', DONE,
    164, 'a', '^', '\'', DONE,
    165, 'a', '^', '`', DONE,
    166, 'a', '^', '?', DONE,
    167, 'a', '^', '.', DONE,
    168, 'e', '~', DONE,
    169, 'e', '.', DONE,
    170, 'e', '^', '\'', DONE,
    171, 'e', '^', '`', DONE,
    172, 'e', '^', '?', DONE,
    173, 'e', '^', '~', DONE,
    174, 'e', '^', '.', DONE,
    175, 'o', '^', '\'', DONE,
    176, 'o', '^', '`', DONE,
    177, 'o', '^', '?', DONE,
    178, 'o', '^', '~', DONE,
    179, 'O', '+', '~', DONE,
    180, 'O', '+', DONE,
    181, 'o', '^', '.', DONE,
    182, 'o', '+', '`', DONE,
    183, 'o', '+', '?', DONE,
    184, 'i', '.', DONE,
    185, 'U', '+', '.', DONE,
    186, 'U', '+', '\'', DONE,
    187, 'U', '+', '`', DONE,
    188, 'U', '+', '?', DONE,
    189, 'o', '+', DONE,
    190, 'o', '+', '\'', DONE,
    191, 'U', '+', DONE,
    192, 'A', '`', DONE,
    193, 'A', '\'', DONE,
    194, 'A', '^', DONE,
    195, 'A', '~', DONE,
    196, 'A', '?', DONE,
    197, 'A', '(', DONE,
    198, 'a', '(', '?', DONE,
    199, 'a', '(', '~', DONE,
    200, 'E', '`', DONE,
    201, 'E', '\'', DONE,
    202, 'E', '^', DONE,
    203, 'E', '?', DONE,
    204, 'I', '`', DONE,
    205, 'I', '\'', DONE,
    206, 'I', '~', DONE,
    207, 'y', '`', DONE,
    208, 'D', 'D', ELSE, 'D', 'd', ELSE, 'd', 'D', DONE,
    209, 'u', '+', '\'', DONE,
    210, 'O', '`', DONE,
    211, 'O', '\'', DONE,
    212, 'O', '^', DONE,
    213, 'a', '.', DONE,
    214, 'y', '?', DONE,
    215, 'u', '+', '`', DONE,
    216, 'u', '+', '?', DONE,
    217, 'U', '`', DONE,
    218, 'U', '\'', DONE,
    219, 'y', '~', DONE,
    220, 'y', '.', DONE,
    221, 'Y', '\'', DONE,
    222, 'o', '+', '~', DONE,
    223, 'u', '+', DONE,
    224, 'a', '`', DONE,
    225, 'a', '\'', DONE,
    226, 'a', '^', DONE,
    227, 'a', '~', DONE,
    228, 'a', '?', DONE,
    229, 'a', '(', DONE,
    230, 'u', '+', '~', DONE,
    231, 'a', '^', '~', DONE,
    232, 'e', '`', DONE,
    233, 'e', '\'', DONE,
    234, 'e', '^', DONE,
    235, 'e', '?', DONE,
    236, 'i', '`', DONE,
    237, 'i', '\'', DONE,
    238, 'i', '~', DONE,
    239, 'i', '?', DONE,
    240, 'd', 'd', DONE,
    241, 'u', '+', '.', DONE,
    242, 'o', '`', DONE,
    243, 'o', '\'', DONE,
    244, 'o', '^', DONE,
    245, 'o', '~', DONE,
    246, 'o', '?', DONE,
    247, 'o', '.', DONE,
    248, 'u', '.', DONE,
    249, 'u', '`', DONE,
    250, 'u', '\'', DONE,
    251, 'u', '~', DONE,
    252, 'u', '?', DONE,
    253, 'y', '\'', DONE,
    254, 'o', '+', '.', DONE,
    255, 'U', '+', '~', DONE,
    DONE
  };

static const unsigned short viscii_vni_data[] =
  {
    2, 'A', 0xDA, DONE,
    5, 'A', 0xDC, DONE,
    6, 'A', 0xC3, DONE,
    20, 'Y', 0xDB, DONE,
    25, 'Y', 0xD5, DONE,
    30, 0xCE, DONE,
    128, 'A', 0xCF, DONE,
    129, 'A', 0xC8, DONE,
    130, 'A', 0xC9, DONE,
    131, 'A', 0xCB, DONE,
    132, 'A', 0xC1, DONE,
    133, 'A', 0xC0, DONE,
    134, 'A', 0xC5, DONE,
    135, 'A', 0xC4, DONE,
    136, 'E', 0xD5, DONE,
    137, 'E', 0xCF, DONE,
    138, 'E', 0xC1, DONE,
    139, 'E', 0xC0, DONE,
    140, 'E', 0xC5, DONE,
    141, 'E', 0xC3, DONE,
    142, 'E', 0xC4, DONE,
    143, 'O', 0xC1, DONE,
    144, 'O', 0xC0, DONE,
    145, 'O', 0xC5, DONE,
    146, 'O', 0xC3, DONE,
    147, 'O', 0xC4, DONE,
    148, 0xD4, 0xCF, DONE,
    149, 0xD4, 0xD9, DONE,
    150, 0xD4, 0xD8, DONE,
    151, 0xD4, 0xDB, DONE,
    152, 0xD2, DONE,
    153, 'O', 0xDB, DONE,
    154, 'O', 0xCF, DONE,
    155, 0xC6, DONE,
    156, 'U', 0xDB, DONE,
    157, 'U', 0xD5, DONE,
    158, 'U', 0xCF, DONE,
    159, 'Y', 0xD8, DONE,
    160, 'O', 0xD5, DONE,
    161, 'a', 0xE9, DONE,
    162, 'a', 0xE8, DONE,
    163, 'a', 0xEB, DONE,
    164, 'a', 0xE1, DONE,
    165, 'a', 0xE0, DONE,
    166, 'a', 0xE5, DONE,
    167, 'a', 0xE4, DONE,
    168, 'e', 0xF5, DONE,
    169, 'e', 0xEF, DONE,
    170, 'e', 0xE1, DONE,
    171, 'e', 0xE0, DONE,
    172, 'e', 0xE5, DONE,
    173, 'e', 0xE3, DONE,
    174, 'e', 0xE4, DONE,
    175, 'o', 0xE1, DONE,
    176, 'o', 0xE0, DONE,
    177, 'o', 0xE5, DONE,
    178, 'o', 0xE3, DONE,
    179, 0xD4, 0xD5, DONE,
    180, 0xD4, DONE,
    181, 'o', 0xE4, DONE,
    182, 0xF4, 0xF8, DONE,
    183, 0xF4, 0xFB, DONE,
    184, 0xF2, DONE,
    185, 0xD6, 0xCF, DONE,
    186, 0xD6, 0xD9, DONE,
    187, 0xD6, 0xD8, DONE,
    188, 0xD6, 0xDB, DONE,
    189, 0xF4, DONE,
    190, 0xF4, 0xF9, DONE,
    191, 0xD6, DONE,
    192, 'A', 0xD8, DONE,
    193, 'A', 0xD9, DONE,
    194, 'A', 0xC2, DONE,
    195, 'A', 0xD5, DONE,
    196, 'A', 0xDB, DONE,
    197, 'A', 0xCA, DONE,
    198, 'a', 0xFA, DONE,
    199, 'a', 0xFC, DONE,
    200, 'E', 0xD8, DONE,
    201, 'E', 0xD9, DONE,
    202, 'E', 0xC2, DONE,
    203, 'E', 0xDB, DONE,
    204, 0xCC, DONE,
    205, 0xCD, DONE,
    206, 0xD3, DONE,
    207, 'y', 0xF8, DONE,
    208, 0xD1, DONE,
    209, 0xF6, 0xF9, DONE,
    210, 'O', 0xD8, DONE,
    211, 'O', 0xD9, DONE,
    212, 'O', 0xC2, DONE,
    213, 'a', 0xEF, DONE,
    214, 'y', 0xFB, DONE,
    215, 0xF6, 0xF8, DONE,
    216, 0xF6, 0xFB, DONE,
    217, 'U', 0xD8, DONE,
    218, 'U', 0xD9, DONE,
    219, 'y', 0xF5, DONE,
    220, 0xEE, DONE,
    221, 'Y', 0xD9, DONE,
    222, 0xF4, 0xF5, DONE,
    223, 0xF6, DONE,
    224, 'a', 0xF8, DONE,
    225, 'a', 0xF9, DONE,
    226, 'a', 0xE2, DONE,
    227, 'a', 0xF5, DONE,
    228, 'a', 0xFB, DONE,
    229, 'a', 0xEA, DONE,
    230, 0xF6, 0xF5, DONE,
    231, 'a', 0xE3, DONE,
    232, 'e', 0xF8, DONE,
    233, 'e', 0xF9, DONE,
    234, 'e', 0xE2, DONE,
    235, 'e', 0xFB, DONE,
    236, 0xEC, DONE,
    237, 0xED, DONE,
    238, 0xF3, DONE,
    239, 0xE6, DONE,
    240, 0xF1, DONE,
    241, 0xF6, 0xEF, DONE,
    242, 'o', 0xF8, DONE,
    243, 'o', 0xF9, DONE,
    244, 'o', 0xE2, DONE,
    245, 'o', 0xF5, DONE,
    246, 'o', 0xFB, DONE,
    247, 'o', 0xEF, DONE,
    248, 'u', 0xEF, DONE,
    249, 'u', 0xF8, DONE,
    250, 'u', 0xF9, DONE,
    251, 'u', 0xF5, DONE,
    252, 'u', 0xFB, DONE,
    253, 'y', 0xF9, DONE,
    254, 0xF4, 0xEF, DONE,
    255, 0xD6, 0xD5, DONE,
    DONE
  };

bool
module_vietnamese (RECODE_OUTER outer)
{
  if (!recode_declare_strip_data (outer, &tcvn_strip_data, "TCVN"))
    return false;
  if (!recode_declare_strip_data (outer, &viscii_strip_data, "VISCII"))
    return false;
  if (!recode_declare_strip_data (outer, &vps_strip_data, "VPS"))
    return false;
  if (!recode_declare_explode_data (outer, viscii_viqr_data, "VISCII", "VIQR"))
    return false;
  if (!recode_declare_explode_data (outer, viscii_vni_data, "VISCII", "VNI"))
    return false;
  return true;
}

void
delmodule_vietnamese (_GL_UNUSED RECODE_OUTER outer)
{
}

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
#include "hash.h"

/* FIXME: An @code{HTML} text which has spurious semi-colons to end entities
   (in strict mode) or does not always have them (in non-strict mode) is
   not canonical.  */

/* Tables.  */

/* The following entities, said to be from Emacs-w3, are ignored for the
   time being, as recode is not too fond of graphical approximations:

	&ensp;          \
	&emsp;          \ \
	&ndash;         -
	&mdash;         --
	&lsquo;         `
	&rsquo;         '
	&ldquo;         ``
	&rdquo;         ''
	&frac18;        1/8
	&frac38;        3/8
	&frac58;        5/8
	&frac78;        7/8
	&hellip;        . . .
	&larr;          <--
	&rarr;          -->
	&trade;         (TM)
*/

/* XML with stand-alone=yes */
#define V00		(1 << 0)
/* Old Emacs-W3, HTML 1.1 ? */
#define V11		(1 << 1)
/* RFC1866, HTML 2.0 */
#define V20		(1 << 2)
/* RFC2070, HTML-i18n */
#define V27		(1 << 3)
/* HTML 3.2 */
#define V32		(1 << 4)
/* HTML 4.0 */
#define V40		(1 << 5)

#define ENTRY(Code, String, Flags) \
  { Code, Flags, String }

static struct ucs2_to_string translations [] =
  {
    ENTRY (33, "excl",      0       				 ),
    ENTRY (34, "quot",      0 | V00 	  | V20 | V27 | V32 | V40),
    ENTRY (35, "num",       0       				 ),
    ENTRY (36, "dollar",    0       				 ),
    ENTRY (37, "percnt",    0       				 ),
    ENTRY (38, "amp",       0 | V00 | V11 | V20 | V27 | V32 | V40),
    ENTRY (39, "apos",      0 | V00 				 ),
    ENTRY (40, "lpar",      0       				 ),
    ENTRY (41, "rpar",      0       				 ),
    ENTRY (42, "ast",       0       				 ),
    ENTRY (43, "plus",      0       				 ),
    ENTRY (44, "comma",     0       				 ),
    ENTRY (45, "horbar",    0       				 ),
    ENTRY (46, "period",    0       				 ),
    ENTRY (58, "colon",     0       				 ),
    ENTRY (59, "semi",      0       				 ),
    ENTRY (60, "lt",        0 | V00 | V11 | V20 | V27 | V32 | V40),
    ENTRY (61, "equals",    0       				 ),
    ENTRY (62, "gt",        0 | V00 | V11 | V20 | V27 | V32 | V40),
    ENTRY (63, "quest",     0       				 ),
    ENTRY (64, "commat",    0       				 ),
    ENTRY (91, "lsqb",      0       				 ),
    ENTRY (93, "rsqb",      0       				 ),
    ENTRY (94, "uarr",      0       				 ),
    ENTRY (95, "lowbar",    0       				 ),
    ENTRY (96, "grave",     0       				 ),
    ENTRY (123, "lcub",     0       				 ),
    ENTRY (124, "verbar",   0       				 ),
    ENTRY (125, "rcub",     0       				 ),
    ENTRY (126, "tilde",    0       				 ),
    ENTRY (160, "nbsp",     0       		| V27 | V32 | V40),
    ENTRY (161, "iexcl",    0       		| V27 | V32 | V40),
    ENTRY (162, "cent",     0       		| V27 | V32 | V40),
    ENTRY (163, "pound",    0       		| V27 | V32 | V40),
    ENTRY (164, "curren",   0       		| V27 | V32 | V40),
    ENTRY (165, "yen",      0       		| V27 | V32 | V40),
    ENTRY (166, "brkbar",   0       | V11                        ),
    ENTRY (166, "brvbar",   0       		| V27 | V32 | V40),
    ENTRY (167, "sect",     0       		| V27 | V32 | V40),
    ENTRY (168, "die",      0       | V11                        ),
    ENTRY (168, "uml",      0       		| V27 | V32 | V40),
    ENTRY (169, "copy",     0       		| V27 | V32 | V40),
    ENTRY (170, "ordf",     0       		| V27 | V32 | V40),
    ENTRY (171, "laquo",    0       		| V27 | V32 | V40),
    ENTRY (172, "not",      0       		| V27 | V32 | V40),
    ENTRY (173, "hyphen",   0       | V11                        ),
    ENTRY (173, "shy",      0       		| V27 | V32 | V40),
    ENTRY (174, "reg",      0       		| V27 | V32 | V40),
    ENTRY (175, "hibar",    0       | V11                        ),
    ENTRY (175, "macr",     0       		| V27 | V32 | V40),
    ENTRY (176, "deg",      0       		| V27 | V32 | V40),
    ENTRY (177, "plusmn",   0       		| V27 | V32 | V40),
    ENTRY (178, "sup2",     0       		| V27 | V32 | V40),
    ENTRY (179, "sup3",     0       		| V27 | V32 | V40),
    ENTRY (180, "acute",    0       		| V27 | V32 | V40),
    ENTRY (181, "micro",    0       		| V27 | V32 | V40),
    ENTRY (182, "para",     0       		| V27 | V32 | V40),
    ENTRY (183, "middot",   0       		| V27 | V32 | V40),
    ENTRY (184, "cedil",    0       		| V27 | V32 | V40),
    ENTRY (185, "sup1",     0       		| V27 | V32 | V40),
    ENTRY (186, "ordm",     0       		| V27 | V32 | V40),
    ENTRY (187, "raquo",    0       		| V27 | V32 | V40),
    ENTRY (188, "frac14",   0       		| V27 | V32 | V40),
    ENTRY (189, "half",     0       | V11                        ),
    ENTRY (189, "frac12",   0       		| V27 | V32 | V40),
    ENTRY (190, "frac34",   0       		| V27 | V32 | V40),
    ENTRY (191, "iquest",   0       		| V27 | V32 | V40),
    ENTRY (192, "Agrave",   0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (193, "Aacute",   0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (194, "Acircu",   0       | V11                        ),
    ENTRY (194, "Acirc",    0       	  | V20 | V27 | V32 | V40),
    ENTRY (195, "Atilde",   0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (196, "Adiaer",   0       | V11                        ),
    ENTRY (196, "Auml",     0       	  | V20 | V27 | V32 | V40),
    ENTRY (197, "Aring",    0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (198, "AE",       0       | V11                        ),
    ENTRY (198, "AElig",    0       	  | V20 | V27 | V32 | V40),
    ENTRY (199, "Ccedil",   0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (200, "Egrave",   0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (201, "Eacute",   0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (202, "Ecircu",   0       | V11                        ),
    ENTRY (202, "Ecirc",    0       	  | V20 | V27 | V32 | V40),
    ENTRY (203, "Ediaer",   0       | V11                        ),
    ENTRY (203, "Euml",     0       	  | V20 | V27 | V32 | V40),
    ENTRY (204, "Igrave",   0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (205, "Iacute",   0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (206, "Icircu",   0       | V11                        ),
    ENTRY (206, "Icirc",    0       	  | V20 | V27 | V32 | V40),
    ENTRY (207, "Idiaer",   0       | V11                        ),
    ENTRY (207, "Iuml",     0       	  | V20 | V27 | V32 | V40),
    ENTRY (208, "ETH",      0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (209, "Ntilde",   0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (210, "Ograve",   0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (211, "Oacute",   0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (212, "Ocircu",   0       | V11                        ),
    ENTRY (212, "Ocirc",    0       	  | V20 | V27 | V32 | V40),
    ENTRY (213, "Otilde",   0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (214, "Odiaer",   0       | V11                        ),
    ENTRY (214, "Ouml",     0       	  | V20 | V27 | V32 | V40),
    ENTRY (215, "MULT",     0       | V11                        ),
    ENTRY (215, "times",    0       		| V27 | V32 | V40),
    ENTRY (216, "Ostroke",  0       | V11                        ),
    ENTRY (216, "Oslash",   0       	  | V20 | V27 | V32 | V40),
    ENTRY (217, "Ugrave",   0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (218, "Uacute",   0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (219, "Ucircu",   0       | V11                        ),
    ENTRY (219, "Ucirc",    0       	  | V20 | V27 | V32 | V40),
    ENTRY (220, "Udiaer",   0       | V11                        ),
    ENTRY (220, "Uuml",     0       	  | V20 | V27 | V32 | V40),
    ENTRY (221, "Yacute",   0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (222, "THORN",    0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (223, "ssharp",   0       | V11                        ),
    ENTRY (223, "szlig",    0       	  | V20 | V27 | V32 | V40),
    ENTRY (224, "agrave",   0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (225, "aacute",   0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (226, "acircu",   0       | V11                        ),
    ENTRY (226, "acirc",    0       	  | V20 | V27 | V32 | V40),
    ENTRY (227, "atilde",   0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (228, "adiaer",   0       | V11                        ),
    ENTRY (228, "auml",     0       	  | V20 | V27 | V32 | V40),
    ENTRY (229, "aring",    0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (230, "ae",       0       | V11                        ),
    ENTRY (230, "aelig",    0       	  | V20 | V27 | V32 | V40),
    ENTRY (231, "ccedil",   0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (232, "egrave",   0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (233, "eacute",   0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (234, "ecircu",   0       | V11                        ),
    ENTRY (234, "ecirc",    0       	  | V20 | V27 | V32 | V40),
    ENTRY (235, "ediaer",   0       | V11                        ),
    ENTRY (235, "euml",     0       	  | V20 | V27 | V32 | V40),
    ENTRY (236, "igrave",   0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (237, "iacute",   0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (238, "icircu",   0       | V11                        ),
    ENTRY (238, "icirc",    0       	  | V20 | V27 | V32 | V40),
    ENTRY (239, "idiaer",   0       | V11                        ),
    ENTRY (239, "iuml",     0       	  | V20 | V27 | V32 | V40),
    ENTRY (240, "eth",      0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (241, "ntilde",   0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (242, "ograve",   0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (243, "oacute",   0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (244, "ocircu",   0       | V11                        ),
    ENTRY (244, "ocirc",    0       	  | V20 | V27 | V32 | V40),
    ENTRY (245, "otilde",   0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (246, "odiaer",   0       | V11                        ),
    ENTRY (246, "ouml",     0       	  | V20 | V27 | V32 | V40),
    ENTRY (247, "DIVIS",    0       | V11                        ),
    ENTRY (247, "divide",   0       		| V27 | V32 | V40),
    ENTRY (248, "ostroke",  0       | V11                        ),
    ENTRY (248, "oslash",   0       	  | V20 | V27 | V32 | V40),
    ENTRY (249, "ugrave",   0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (250, "uacute",   0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (251, "ucircu",   0       | V11                        ),
    ENTRY (251, "ucirc",    0       	  | V20 | V27 | V32 | V40),
    ENTRY (252, "udiaer",   0       | V11                        ),
    ENTRY (252, "uuml",     0       	  | V20 | V27 | V32 | V40),
    ENTRY (253, "yacute",   0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (254, "thorn",    0       | V11 | V20 | V27 | V32 | V40),
    ENTRY (255, "ydiaer",   0       | V11                        ),
    ENTRY (255, "yuml",     0       	  | V20 | V27 | V32 | V40),
    ENTRY (338, "OElig",    0       			    | V40),
    ENTRY (339, "oelig",    0       			    | V40),
    ENTRY (352, "Scaron",   0       			    | V40),
    ENTRY (353, "scaron",   0       			    | V40),
    ENTRY (376, "Yuml",     0       			    | V40),
    ENTRY (402, "fnof",     0       			    | V40),
    ENTRY (710, "circ",     0       			    | V40),
    ENTRY (732, "tilde",    0       			    | V40),
    ENTRY (913, "Alpha",    0       			    | V40),
    ENTRY (914, "Beta",     0       			    | V40),
    ENTRY (915, "Gamma",    0       			    | V40),
    ENTRY (916, "Delta",    0       			    | V40),
    ENTRY (917, "Epsilon",  0       			    | V40),
    ENTRY (918, "Zeta",     0       			    | V40),
    ENTRY (919, "Eta",      0       			    | V40),
    ENTRY (920, "Theta",    0       			    | V40),
    ENTRY (921, "Iota",     0       			    | V40),
    ENTRY (922, "Kappa",    0       			    | V40),
    ENTRY (923, "Lambda",   0       			    | V40),
    ENTRY (924, "Mu",       0       			    | V40),
    ENTRY (925, "Nu",       0       			    | V40),
    ENTRY (926, "Xi",       0       			    | V40),
    ENTRY (927, "Omicron",  0       			    | V40),
    ENTRY (928, "Pi",       0       			    | V40),
    ENTRY (929, "Rho",      0       			    | V40),
    ENTRY (931, "Sigma",    0       			    | V40),
    ENTRY (932, "Tau",      0       			    | V40),
    ENTRY (933, "Upsilon",  0       			    | V40),
    ENTRY (934, "Phi",      0       			    | V40),
    ENTRY (935, "Chi",      0       			    | V40),
    ENTRY (936, "Psi",      0       			    | V40),
    ENTRY (937, "Omega",    0       			    | V40),
    ENTRY (945, "alpha",    0       			    | V40),
    ENTRY (946, "beta",     0       			    | V40),
    ENTRY (947, "gamma",    0       			    | V40),
    ENTRY (948, "delta",    0       			    | V40),
    ENTRY (949, "epsilon",  0       			    | V40),
    ENTRY (950, "zeta",     0       			    | V40),
    ENTRY (951, "eta",      0       			    | V40),
    ENTRY (952, "theta",    0       			    | V40),
    ENTRY (953, "iota",     0       			    | V40),
    ENTRY (954, "kappa",    0       			    | V40),
    ENTRY (955, "lambda",   0       			    | V40),
    ENTRY (956, "mu",       0       			    | V40),
    ENTRY (957, "nu",       0       			    | V40),
    ENTRY (958, "xi",       0       			    | V40),
    ENTRY (959, "omicron",  0       			    | V40),
    ENTRY (960, "pi",       0       			    | V40),
    ENTRY (961, "rho",      0       			    | V40),
    ENTRY (962, "sigmaf",   0       			    | V40),
    ENTRY (963, "sigma",    0       			    | V40),
    ENTRY (964, "tau",      0       			    | V40),
    ENTRY (965, "upsilon",  0       			    | V40),
    ENTRY (966, "phi",      0       			    | V40),
    ENTRY (967, "chi",      0       			    | V40),
    ENTRY (968, "psi",      0       			    | V40),
    ENTRY (969, "omega",    0       			    | V40),
    ENTRY (977, "thetasym", 0       			    | V40),
    ENTRY (978, "upsih",    0       			    | V40),
    ENTRY (982, "piv",      0       			    | V40),
    ENTRY (8194, "ensp",    0       			    | V40),
    ENTRY (8195, "emsp",    0       			    | V40),
    ENTRY (8201, "thinsp",  0       			    | V40),
    ENTRY (8204, "zwnj",    0       		| V27       | V40),
    ENTRY (8205, "zwj",     0       		| V27       | V40),
    ENTRY (8206, "lrm",     0       		| V27       | V40),
    ENTRY (8207, "rlm",     0       		| V27       | V40),
    ENTRY (8211, "ndash",   0       			    | V40),
    ENTRY (8212, "mdash",   0       			    | V40),
    ENTRY (8216, "lsquo",   0       			    | V40),
    ENTRY (8217, "rsquo",   0       			    | V40),
    ENTRY (8218, "sbquo",   0       			    | V40),
    ENTRY (8220, "ldquo",   0       			    | V40),
    ENTRY (8221, "rdquo",   0       			    | V40),
    ENTRY (8222, "bdquo",   0       			    | V40),
    ENTRY (8224, "dagger",  0       			    | V40),
    ENTRY (8225, "Dagger",  0       			    | V40),
    ENTRY (8226, "bull",    0       			    | V40),
    ENTRY (8230, "hellip",  0       			    | V40),
    ENTRY (8240, "permil",  0       			    | V40),
    ENTRY (8242, "prime",   0       			    | V40),
    ENTRY (8243, "Prime",   0       			    | V40),
    ENTRY (8249, "lsaquo",  0       			    | V40),
    ENTRY (8250, "rsaquo",  0       			    | V40),
    ENTRY (8254, "oline",   0       			    | V40),
    ENTRY (8260, "frasl",   0       			    | V40),
    ENTRY (8364, "euro",    0       			    | V40),
    ENTRY (8465, "image",   0       			    | V40),
    ENTRY (8472, "weierp",  0       			    | V40),
    ENTRY (8476, "real",    0       			    | V40),
    ENTRY (8482, "trade",   0       			    | V40),
    ENTRY (8501, "alefsym", 0       			    | V40),
    ENTRY (8592, "larr",    0       			    | V40),
    ENTRY (8593, "uarr",    0       			    | V40),
    ENTRY (8594, "rarr",    0       			    | V40),
    ENTRY (8595, "darr",    0       			    | V40),
    ENTRY (8596, "harr",    0       			    | V40),
    ENTRY (8629, "crarr",   0       			    | V40),
    ENTRY (8656, "lArr",    0       			    | V40),
    ENTRY (8657, "uArr",    0       			    | V40),
    ENTRY (8658, "rArr",    0       			    | V40),
    ENTRY (8659, "dArr",    0       			    | V40),
    ENTRY (8660, "hArr",    0       			    | V40),
    ENTRY (8704, "forall",  0       			    | V40),
    ENTRY (8706, "part",    0       			    | V40),
    ENTRY (8707, "exist",   0       			    | V40),
    ENTRY (8709, "empty",   0       			    | V40),
    ENTRY (8711, "nabla",   0       			    | V40),
    ENTRY (8712, "isin",    0       			    | V40),
    ENTRY (8713, "notin",   0       			    | V40),
    ENTRY (8715, "ni",      0       			    | V40),
    ENTRY (8719, "prod",    0       			    | V40),
    ENTRY (8721, "sum",     0       			    | V40),
    ENTRY (8722, "minus",   0       			    | V40),
    ENTRY (8727, "lowast",  0       			    | V40),
    ENTRY (8730, "radic",   0       			    | V40),
    ENTRY (8733, "prop",    0       			    | V40),
    ENTRY (8734, "infin",   0       			    | V40),
    ENTRY (8736, "ang",     0       			    | V40),
    ENTRY (8743, "and",     0       			    | V40),
    ENTRY (8744, "or",      0       			    | V40),
    ENTRY (8745, "cap",     0       			    | V40),
    ENTRY (8746, "cup",     0       			    | V40),
    ENTRY (8747, "int",     0       			    | V40),
    ENTRY (8756, "there4",  0       			    | V40),
    ENTRY (8764, "sim",     0       			    | V40),
    ENTRY (8773, "cong",    0       			    | V40),
    ENTRY (8776, "asymp",   0       			    | V40),
    ENTRY (8800, "ne",      0       			    | V40),
    ENTRY (8801, "equiv",   0       			    | V40),
    ENTRY (8804, "le",      0       			    | V40),
    ENTRY (8805, "ge",      0       			    | V40),
    ENTRY (8834, "sub",     0       			    | V40),
    ENTRY (8835, "sup",     0       			    | V40),
    ENTRY (8836, "nsub",    0       			    | V40),
    ENTRY (8838, "sube",    0       			    | V40),
    ENTRY (8839, "supe",    0       			    | V40),
    ENTRY (8853, "oplus",   0       			    | V40),
    ENTRY (8855, "otimes",  0       			    | V40),
    ENTRY (8869, "perp",    0       			    | V40),
    ENTRY (8901, "sdot",    0       			    | V40),
    ENTRY (8968, "lceil",   0       			    | V40),
    ENTRY (8969, "rceil",   0       			    | V40),
    ENTRY (8970, "lfloor",  0       			    | V40),
    ENTRY (8971, "rfloor",  0       			    | V40),
    ENTRY (9001, "lang",    0       			    | V40),
    ENTRY (9002, "rang",    0       			    | V40),
    ENTRY (9674, "loz",     0       			    | V40),
    ENTRY (9824, "spades",  0       			    | V40),
    ENTRY (9827, "clubs",   0       			    | V40),
    ENTRY (9829, "hearts",  0       			    | V40),
    ENTRY (9830, "diams",   0       			    | V40),
    ENTRY (0,    NULL,      0       				 )
  };

#undef ENTRY

/* UCS-2 to HTML.  */

/*-------------------------------------.
| Return hash value given TABLE_SIZE.  |
`-------------------------------------*/

static size_t
code_hash (const void *void_data, size_t table_size)
{
  struct ucs2_to_string const *data = (struct ucs2_to_string const *) void_data;

  return data->code % table_size;
}

/*----------------------------------------.
| Say if two translations are identical.  |
`----------------------------------------*/

static bool
code_compare (const void *void_first, const void *void_second)
{
  struct ucs2_to_string const *first
    = (struct ucs2_to_string const *) void_first;
  struct ucs2_to_string const *second
    = (struct ucs2_to_string const *) void_second;

  return first->code == second->code;
}

/*-----------------.
| Initialisation.  |
`-----------------*/

static bool
init_ucs2_html (RECODE_STEP step,
		RECODE_CONST_REQUEST request,
		RECODE_CONST_OPTION_LIST before_options,
		RECODE_CONST_OPTION_LIST after_options,
		unsigned mask)
{
  Hash_table *table;
  struct ucs2_to_string const *cursor;

  if (before_options || after_options)
    return false;

  table = hash_initialize (0, NULL, code_hash, code_compare, NULL);
  if (!table)
    return false;

  for (cursor = translations; cursor->code; cursor++)
    if (cursor->flags & mask
	&& (!request->diacritics_only || cursor->code > 128))
      if (!hash_insert (table, cursor))
	{
	  hash_free (table);
	  return false;
	}

  step->step_type = RECODE_UCS2_TO_STRING;
  step->step_table = table;
  step->step_table_term_routine = (void (*) (void *)) hash_free;
  return true;
}

static bool
init_ucs2_html_v00 (RECODE_STEP step,
		    RECODE_CONST_REQUEST request,
		    RECODE_CONST_OPTION_LIST before_options,
		    RECODE_CONST_OPTION_LIST after_options)
{
  return
    init_ucs2_html (step, request, before_options, after_options, V00);
}

static bool
init_ucs2_html_v11 (RECODE_STEP step,
		    RECODE_CONST_REQUEST request,
		    RECODE_CONST_OPTION_LIST before_options,
		    RECODE_CONST_OPTION_LIST after_options)
{
  return
    init_ucs2_html (step, request, before_options, after_options, V11);
}

static bool
init_ucs2_html_v20 (RECODE_STEP step,
		    RECODE_CONST_REQUEST request,
		    RECODE_CONST_OPTION_LIST before_options,
		    RECODE_CONST_OPTION_LIST after_options)
{
  return
    init_ucs2_html (step, request, before_options, after_options, V20);
}

static bool
init_ucs2_html_v27 (RECODE_STEP step,
		    RECODE_CONST_REQUEST request,
		    RECODE_CONST_OPTION_LIST before_options,
		    RECODE_CONST_OPTION_LIST after_options)
{
  return
    init_ucs2_html (step, request, before_options, after_options, V27);
}

static bool
init_ucs2_html_v32 (RECODE_STEP step,
		    RECODE_CONST_REQUEST request,
		    RECODE_CONST_OPTION_LIST before_options,
		    RECODE_CONST_OPTION_LIST after_options)
{
  return
    init_ucs2_html (step, request, before_options, after_options, V32);
}

static bool
init_ucs2_html_v40 (RECODE_STEP step,
		    RECODE_CONST_REQUEST request,
		    RECODE_CONST_OPTION_LIST before_options,
		    RECODE_CONST_OPTION_LIST after_options)
{
  return
    init_ucs2_html (step, request, before_options, after_options, V40);
}

/*-----------------.
| Transformation.  |
`-----------------*/

static bool
transform_ucs2_html (RECODE_SUBTASK subtask)
{
  Hash_table *table = (Hash_table *) subtask->step->step_table;
  unsigned value;

  while (recode_get_ucs2 (&value, subtask))
    {
      struct ucs2_to_string lookup;
      struct ucs2_to_string *entry;

      lookup.code = value;
      entry = (struct ucs2_to_string *) hash_lookup (table, &lookup);
      if (entry)
	{
	  const char *cursor = entry->string;

	  recode_put_byte ('&', subtask);
	  while (*cursor)
	    {
	      recode_put_byte (*cursor, subtask);
	      cursor++;
	    }
	  recode_put_byte (';', subtask);
	}
      else if ((value < 32 && value != '\n' && value != '\t') || value >= 127)
	{
	  unsigned divider = 10000;

	  recode_put_byte ('&', subtask);
	  recode_put_byte ('#', subtask);
	  while (divider > value)
	    divider /= 10;
	  while (divider > 1)
	    {
	      recode_put_byte ('0' + value / divider, subtask);
	      value %= divider;
	      divider /= 10;
	    }
	  recode_put_byte ('0' + value, subtask);
	  recode_put_byte (';', subtask);
	}
      else
	recode_put_byte(value, subtask);
    }

  SUBTASK_RETURN (subtask);
}

/* HTML to UCS-2.  */

#define ENTITY_BUFFER_LENGTH 20

/*
&quot;	{ if (request->diacritics_only) ECHO; else recode_put_ucs2 (34, subtask); }
&amp;	{ if (request->diacritics_only) ECHO; else recode_put_ucs2 (38, subtask); }
&lt;	{ if (request->diacritics_only) ECHO; else recode_put_ucs2 (60, subtask); }
&gt;	{ if (request->diacritics_only) ECHO; else recode_put_ucs2 (62, subtask); }
*/

/*-------------------------------------.
| Return hash value given TABLE_SIZE.  |
`-------------------------------------*/

static size_t
string_hash (const void *void_data, size_t table_size)
{
  struct ucs2_to_string const *data = (struct ucs2_to_string const *) void_data;

  return hash_string (data->string, table_size);
}

/*----------------------------------------.
| Say if two translations are identical.  |
`----------------------------------------*/

static bool
string_compare (const void *void_first, const void *void_second)
{
  struct ucs2_to_string const *first = (struct ucs2_to_string const *) void_first;
  struct ucs2_to_string const *second = (struct ucs2_to_string const *) void_second;

  return strcmp (first->string, second->string) == 0;
}

/*-----------------.
| Initialisation.  |
`-----------------*/

static bool
init_html_ucs2 (RECODE_STEP step,
		RECODE_CONST_REQUEST request,
		RECODE_CONST_OPTION_LIST before_options,
		RECODE_CONST_OPTION_LIST after_options,
		unsigned mask)
{
  Hash_table *table;
  struct ucs2_to_string const *cursor;

  if (before_options || after_options)
    return false;

  table = hash_initialize (0, NULL, string_hash, string_compare, NULL);
  if (!table)
    return false;

  for (cursor = translations; cursor->code; cursor++)
    if (cursor->flags & mask
	&& (!request->diacritics_only || cursor->code > 128))
      if (!hash_insert (table, cursor))
	{
	  hash_free (table);
	  return false;
	}

  step->step_type = RECODE_STRING_TO_UCS2;
  step->step_table = table;
  step->step_table_term_routine = (void (*) (void *)) hash_free;
  return true;
}

static bool
init_html_v00_ucs2 (RECODE_STEP step,
		    RECODE_CONST_REQUEST request,
		    RECODE_CONST_OPTION_LIST before_options,
		    RECODE_CONST_OPTION_LIST after_options)
{
  return
    init_html_ucs2 (step, request, before_options, after_options, V00);
}

static bool
init_html_v11_ucs2 (RECODE_STEP step,
		    RECODE_CONST_REQUEST request,
		    RECODE_CONST_OPTION_LIST before_options,
		    RECODE_CONST_OPTION_LIST after_options)
{
  return
    init_html_ucs2 (step, request, before_options, after_options, V11);
}

static bool
init_html_v20_ucs2 (RECODE_STEP step,
		    RECODE_CONST_REQUEST request,
		    RECODE_CONST_OPTION_LIST before_options,
		    RECODE_CONST_OPTION_LIST after_options)
{
  return
    init_html_ucs2 (step, request, before_options, after_options, V20);
}

static bool
init_html_v27_ucs2 (RECODE_STEP step,
		    RECODE_CONST_REQUEST request,
		    RECODE_CONST_OPTION_LIST before_options,
		    RECODE_CONST_OPTION_LIST after_options)
{
  return
    init_html_ucs2 (step, request, before_options, after_options, V27);
}

static bool
init_html_v32_ucs2 (RECODE_STEP step,
		    RECODE_CONST_REQUEST request,
		    RECODE_CONST_OPTION_LIST before_options,
		    RECODE_CONST_OPTION_LIST after_options)
{
  return
    init_html_ucs2 (step, request, before_options, after_options, V32);
}

static bool
init_html_v40_ucs2 (RECODE_STEP step,
		    RECODE_CONST_REQUEST request,
		    RECODE_CONST_OPTION_LIST before_options,
		    RECODE_CONST_OPTION_LIST after_options)
{
  return
    init_html_ucs2 (step, request, before_options, after_options, V40);
}

/*-----------------.
| Transformation.  |
`-----------------*/

static bool
transform_html_ucs2 (RECODE_SUBTASK subtask)
{
  RECODE_CONST_REQUEST request = subtask->task->request;
  int input_char;

  input_char = recode_get_byte (subtask);
  if (input_char != EOF)
    recode_put_ucs2 (BYTE_ORDER_MARK, subtask);	/* FIXME: experimental */

  /* According to RFC 2718 and the Unicode Standard, if you declare the
     character encoding of your page using HTTP as either "UTF-16LE" or
     "UTF-16BE" then you should not use a byte-order mark at the beginning
     of the page. Only if the page is labelled in HTTP using IANA charset
     name "UTF-16" is a byte-order mark appropriate.  */

  while (input_char != EOF)

    if (input_char == '&')
      {
	char buffer[ENTITY_BUFFER_LENGTH];
	char *cursor = buffer;
	bool valid = true;
	bool echo = false;

	input_char = recode_get_byte (subtask);
	if (input_char == '#')
	  {
	    input_char = recode_get_byte (subtask);
	    if (input_char == 'x' || input_char == 'X')
	      {
		unsigned value = 0;

		/* Scan &#[xX][0-9a-fA-F]+; notation.  */

		*cursor++ = '#';
		*cursor++ = input_char;
		input_char = recode_get_byte (subtask);

		while (valid)
		  {
		    if (input_char >= '0' && input_char <= '9')
		      value = 16 * value + input_char - '0';
		    else if (input_char >= 'A' && input_char <= 'F')
		      value = 16 * value + input_char - 'A' + 10;
		    else if (input_char >= 'a' && input_char <= 'f')
		      value = 16 * value + input_char - 'a' + 10;
		    else
		      break;

		    if (value >= 65535)
		      valid = false;
		    else if (cursor == buffer + ENTITY_BUFFER_LENGTH - 2)
		      valid = false;
		    else
		      {
			*cursor++ = input_char;
			input_char = recode_get_byte (subtask);
		      }
		  }

		/* No digit found at all?  */
		if ((cursor - buffer) == 2)
		  valid = false;

		if (valid)
		  if (request->diacritics_only)
		    {
		      echo = true;
		      *cursor = '\0';
		    }
		  else
		    {
		      recode_put_ucs2 (value, subtask);
		      if (input_char == ';')
			input_char = recode_get_byte (subtask);
		    }
		else
		  *cursor = '\0';
	      }
	    else
	      {
		unsigned value = 0;

		/* Scan &#[0-9]+; notation.  */

		*cursor++ = '#';

		while (valid)
		  {
		    if (input_char >= '0' && input_char <= '9')
		      value = 10 * value + input_char - '0';
		    else
		      break;

		    if (value >= 65535)
		      valid = false;
		    else if (cursor == buffer + ENTITY_BUFFER_LENGTH - 2)
		      valid = false;
		    else
		      {
			*cursor++ = input_char;
			input_char = recode_get_byte (subtask);
		      }
		  }

		/* No digit found at all?  */
		if ((cursor - buffer) == 1)
		  valid = false;

		if (valid)
		  if (request->diacritics_only)
		    {
		      echo = true;
		      *cursor = '\0';
		    }
		  else
		    {
		      recode_put_ucs2 (value, subtask);
		      if (input_char == ';')
			input_char = recode_get_byte (subtask);
		    }
		else
		  *cursor = '\0';
	      }
	  }
	else if ((input_char >= 'A' && input_char <= 'Z')
		 || (input_char >= 'a' && input_char <= 'z'))
	  {
	    /* Scan &[A-Za-z][A-Za-z0-9]*; notation.  */

	    *cursor++ = input_char;
	    input_char = recode_get_byte (subtask);

	    while (valid
		   && input_char != EOF
		   && ((input_char >= 'A' && input_char <= 'Z')
		       || (input_char >= 'a' && input_char <= 'z')
		       || (input_char >= '0' && input_char <= '9')))
	      if (cursor == buffer + ENTITY_BUFFER_LENGTH - 2)
		valid = false;
	      else
		{
		  *cursor++ = input_char;
		  input_char = recode_get_byte (subtask);
		}
	    *cursor = '\0';

	    if (valid)
	      {
		struct ucs2_to_string lookup;
		struct ucs2_to_string *entry;

		lookup.string = buffer;
		entry = (struct ucs2_to_string *) hash_lookup
		  ((const Hash_table *) subtask->step->step_table, &lookup);
		if (entry)
		  {
		    recode_put_ucs2 (entry->code, subtask);
		    if (input_char == ';')
		      input_char = recode_get_byte (subtask);
		  }
		else
		  valid = false;
	      }
	  }
	else /* Not '#' or A-z, so not a valid entity.  */
	  {
	    valid = false;
	    /* Push back '&', but do not output current input_char yet.  */
	    *cursor = '\0';
	  }

	if (echo || !valid)
	  {
	    recode_put_ucs2 ('&', subtask);
	    for (cursor = buffer; *cursor; cursor++)
	      recode_put_ucs2 (*cursor, subtask);
	    /* input_char is processed again in the next iteration.  */
	  }
      }
    else
      {
	recode_put_ucs2 (input_char, subtask);
	input_char = recode_get_byte (subtask);
      }

  SUBTASK_RETURN (subtask);
}

/* Module declaration.  */
bool
module_html (RECODE_OUTER outer)
{
  return
    recode_declare_single (outer, "ISO-10646-UCS-2", "XML-standalone",
		    outer->quality_byte_to_variable,
		    init_ucs2_html_v00, transform_ucs2_html)
    && recode_declare_single (outer, "XML-standalone", "ISO-10646-UCS-2",
		       outer->quality_variable_to_byte,
		       init_html_v00_ucs2, transform_html_ucs2)
    && recode_declare_single (outer, "ISO-10646-UCS-2", "HTML_1.1",
		    outer->quality_byte_to_variable,
		    init_ucs2_html_v11, transform_ucs2_html)
    && recode_declare_single (outer, "HTML_1.1", "ISO-10646-UCS-2",
		       outer->quality_variable_to_byte,
		       init_html_v11_ucs2, transform_html_ucs2)
    && recode_declare_single (outer, "ISO-10646-UCS-2", "HTML_2.0",
		       outer->quality_byte_to_variable,
		       init_ucs2_html_v20, transform_ucs2_html)
    && recode_declare_single (outer, "HTML_2.0", "ISO-10646-UCS-2",
		       outer->quality_variable_to_byte,
		       init_html_v20_ucs2, transform_html_ucs2)
    && recode_declare_single (outer, "ISO-10646-UCS-2", "HTML-i18n",
		       outer->quality_byte_to_variable,
		       init_ucs2_html_v27, transform_ucs2_html)
    && recode_declare_single (outer, "HTML-i18n", "ISO-10646-UCS-2",
		       outer->quality_variable_to_byte,
		       init_html_v27_ucs2, transform_html_ucs2)
    && recode_declare_single (outer, "ISO-10646-UCS-2", "HTML_3.2",
		       outer->quality_byte_to_variable,
		       init_ucs2_html_v32, transform_ucs2_html)
    && recode_declare_single (outer, "HTML_3.2", "ISO-10646-UCS-2",
		       outer->quality_variable_to_byte,
		       init_html_v32_ucs2, transform_html_ucs2)
    && recode_declare_single (outer, "ISO-10646-UCS-2", "HTML_4.0",
		       outer->quality_byte_to_variable,
		       init_ucs2_html_v40, transform_ucs2_html)
    && recode_declare_single (outer, "HTML_4.0", "ISO-10646-UCS-2",
		       outer->quality_variable_to_byte,
		       init_html_v40_ucs2, transform_html_ucs2)

    && recode_declare_alias (outer, "h0", "XML-standalone")
    && recode_declare_alias (outer, "h1", "HTML_1.1")
    && recode_declare_alias (outer, "RFC1866", "HTML_2.0")
    && recode_declare_alias (outer, "1866", "HTML_2.0")
    && recode_declare_alias (outer, "h2", "HTML_2.0")
    && recode_declare_alias (outer, "RFC2070", "HTML-i18n")
    && recode_declare_alias (outer, "2070", "HTML-i18n")
    && recode_declare_alias (outer, "h3", "HTML_3.2")
    && recode_declare_alias (outer, "h4", "HTML_4.0")
    /* HTML defaults to the highest level available.  */
    && recode_declare_alias (outer, "HTML", "HTML_4.0")
    && recode_declare_alias (outer, "h", "HTML_4.0");
}

void
delmodule_html (_GL_UNUSED RECODE_OUTER outer)
{
}

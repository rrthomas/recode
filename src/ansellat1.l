/*
 * Copyright (c) 1998, Wolfram Schneider <wosch@freebsd.org>
 *                     Konrad Zuse Zentrum für Informationstechnik Berlin.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *  $Id: ansellat1.l,v 1.1 1998/03/25 11:34:53 wolfram Exp wolfram $
 *
 *
 * 8-Bit ANSEL to ISO8859-1 conversion. This file support only a subset
 * of Z39.47-1993 (ANSEL). It is incomplete due the lack of free
 * available documentation ;-(
 *
 * http://www.niso.org/pccodes.htm
 * Z39.47-1993 Extended Latin Alphabet Coded Character Set for
 * Bibliographic Use (ANSEL)
 * 
 * Character sets are a basic building block of automated information
 * systems. Z39.47 provides a table of coded values for the
 * representation of characters of the extended Latin alphabet in
 * machine-readable form for thirty-five languages written in the Latin
 * alphabet and for fifty-one romanized languages.
 * 
 * 27 pp. ISBN: 1-880124-02-5 Price: $40.00 
 * 
 *
 * http://wwnet.com/~krugman1/gentext/gedstand.txt
 * 8-Bit ANSEL
 * 
 * The 8-Bit ANSEL (American National Standard for Extended Latin
 * Alphabet Coded Character Set for Bibliographic Use, Z39.47, 1985
 * copyright) is the default character set for GEDCOM. [...]
 * The ANSEL standard
 * specifies an extended 8-bit configuration (above 128) to represent the
 * spacing and non-spacing graphic characters that make up most of the
 * Latin based languages.  ANSEL is a super-set of ASCII.  The standard
 * ASCII characters including the control characters are preserved.
 * 
 * ANSEL is known by two other names: (1) ANSI Z39.47-1985) and (2) the
 * American Library Association character set, used in library systems
 * worldwide, including the MARC (MAchine- Readable Catalog) format.
 * 
 * A description of the codes for the ANSEL character set has been
 * reproduced with permission and is included with the printed version of
 * The GEDCOM Standard. The description of ANSEL codes is not included in
 * the electronic version.  This description may be purchased from the
 * American National Standards Institute at 1430 Broadway, New York,
 * N.Y. 10018.  The description of the ANSEL character set standard
 * includes the following:
 * 
 *     *       An 8-Bit Code Table showing the ASCII and extended ANSEL codes
 *     *       An explanation or legend of these codes
 *     *       A chart that identifies the ANSEL Non-spacing Graphic Characters
 *     *       A chart that identifies the ASCII Control Characters
 *     *       A chart that identifies the ASCII Graphic Characters
 * 
 * Character-set codes 0 through 127 are the same for 8-Bit ANSEL and
 * 8-Bit ASCII (USA version-- ANSI 8-Bit). Character-set codes 128
 * through 255 are unique to the ANSEL character set.
 * 
 * Changes
 * ========
 * Fix ansel characters 
 * 
 *      Capital C with cedilla
 *      Lower case C with cedilla
 *      Capital ETH 
 *      Lower case eth
 *      Capital O with slash
 *      Capital Y with acute accent
 *      Lower case Icelandic thorn     
 *      capital OE ligature
 * 
 * Thanks to Larry E. Dixson <ldix@loc.gov>
 * 
 */


/* Step name: ansel_latin1.  */

%%

\241 { put_byte (76, subtask); }
\242 { put_byte (216, subtask); }
\243  { put_byte (208, subtask); }
\244 { put_byte (222, subtask); }
\245 { put_byte (198, subtask); }

\253 { put_byte (177, subtask); }

\261 { put_byte (108, subtask); }
\262 { put_byte (248, subtask); }
\263 { put_byte (240, subtask); }
\264 { put_byte (254, subtask); }
\265 { put_byte (230, subtask); }
\266 { put_byte (111, subtask); }

\271 { put_byte (163, subtask); }

\303 { put_byte (169, subtask); }
\340 { put_byte (191, subtask); }
\341A { put_byte (192, subtask); }
\341E { put_byte (200, subtask); }
\341I { put_byte (204, subtask); }
\341O { put_byte (210, subtask); }
\341U { put_byte (217, subtask); }
\341a { put_byte (224, subtask); }
\341e { put_byte (232, subtask); }
\341i { put_byte (236, subtask); }
\341o { put_byte (242, subtask); }
\341u { put_byte (249, subtask); }

\342A { put_byte (193, subtask); }
\342E { put_byte (201, subtask); }
\342I { put_byte (205, subtask); }
\342O { put_byte (211, subtask); }
\342U { put_byte (218, subtask); }
\342Y { put_byte (221, subtask); }

\342a { put_byte (225, subtask); }
\342e { put_byte (233, subtask); }
\342i { put_byte (237, subtask); }
\342o { put_byte (243, subtask); }
\342u { put_byte (250, subtask); }
\342y { put_byte (253, subtask); }

\343A { put_byte (194, subtask); }
\343E { put_byte (202, subtask); }
\343I { put_byte (206, subtask); }
\343O { put_byte (212, subtask); }
\343U { put_byte (219, subtask); }

\343a { put_byte (226, subtask); }
\343e { put_byte (234, subtask); }
\343i { put_byte (238, subtask); }
\343o { put_byte (244, subtask); }
\343u { put_byte (251, subtask); }

\344A { put_byte (195, subtask); }
\344O { put_byte (213, subtask); }
\344N { put_byte (209, subtask); }
\344a { put_byte (227, subtask); }
\344o { put_byte (245, subtask); }
\344n { put_byte (241, subtask); }

\345  { put_byte (175, subtask); }

\350A { put_byte (196, subtask); }
\350E { put_byte (203, subtask); }
\350I { put_byte (207, subtask); }
\350O { put_byte (214, subtask); }
\350U { put_byte (220, subtask); }

\350a { put_byte (228, subtask); }
\350e { put_byte (235, subtask); }
\350i { put_byte (239, subtask); }
\350o { put_byte (246, subtask); }
\350u { put_byte (252, subtask); }
\350y { put_byte (255, subtask); }

\352A { put_byte (197, subtask); }
\352a { put_byte (229, subtask); }

\360C { put_byte (199, subtask); }
\360c { put_byte (231, subtask); }
%%

bool
module_ansel_latin1 (RECODE_OUTER outer)
{
  return declare_single (outer, "Z39.47:1993", "Latin-1",
                         outer->quality_variable_to_byte, NULL,
                         transform_ansel_latin1)
    && declare_alias (outer, "8bitANSEL", "Z39.47:1993")
    && declare_alias (outer, "Z39.47", "Z39.47:1993")
    && declare_alias (outer, "ANSEL", "Z39.47:1993")
    && declare_alias (outer, "USMARC", "Z39.47:1993")
    && declare_alias (outer, "MARC", "Z39.47:1993")
    && declare_alias (outer, "LOC", "Z39.47:1993"); /* Library of Congress */
}

void
delmodule_ansel_latin1 (RECODE_OUTER outer)
{
}

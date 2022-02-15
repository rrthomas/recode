#!/usr/bin/python3
# -*- coding: utf-8 -*-
# Automatically derive Recode table files from various sources.
# Copyright © 1993, 1994, 1997, 1998, 1999, 2000 Free Software Foundation, Inc.
# François Pinard <pinard@iro.umontreal.ca>, 1993.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, see <https://www.gnu.org/licenses/>.

"""\
`tables.py' derives Recode table files from various sources.

Usage: python tables.py [OPTION]... DATA-FILE...

Output selection:
  -e   Produce C source file for explode data (explode.c)
  -i   Produce C source file for iconv charsets (iconvdecl.h)
  -m   Produce C inclusion file for short RFC 1345 mnemonics (rfc1345.h)
  -n   Produce C inclusion file for character names (charname.h)
  -p   Produce C source files for strip data (strip-pool.c and strip-data.c)
  -t   Produce Texinfo inclusion file for RFC 1345 (rfc1345.texi)

Modality options:
  -C DIRECTORY   Change to DIRECTORY prior to processing
  -F             Produce French versions for -n, -s or -t
  -v             Increase verbosity

DATA-FILEs may be rfc1345.txt, mnemonic[.,]ds, Unicode maps, or .def files
from Keld's chset* packages.  The digesting order is usually important.
When `-F' and `-n' are used, process Alain's tables.
"""

import re, sys, io

# Character constants.
REPLACEMENT_CHARACTER = 0xFFFD
NOT_A_CHARACTER = 0xFFFF

# Main driver.

class Main:
    directory = None
    charnames = None
    explodes = None
    iconv = None
    mnemonics = None
    strips = None
    verbose = False

    def main(self, *arguments):
        if not arguments:
            sys.stdout.write(__doc__)
            return
        import getopt
        French_option = False
        options, arguments = getopt.getopt(arguments, 'C:Feimnptv')
        for option, value in options:
            if option == '-C':
                self.directory = value
            elif option == '-F':
                French_option = True
            elif option == '-e':
                if not self.explodes:
                    self.explodes = Explodes()
                self.explodes.do_sources = True
            elif option == '-i':
                if not self.iconv:
                    self.iconv = Iconv()
                self.iconv.do_sources = True
            elif option == '-m':
                if not self.mnemonics:
                    self.mnemonics = Mnemonics()
                self.mnemonics.do_sources = True
            elif option == '-n':
                if not self.charnames:
                    self.charnames = Charnames()
                self.charnames.do_sources = True
            elif option == '-p':
                if not self.strips:
                    self.strips = Strips()
                self.strips.do_sources = True
            elif option == '-t':
                if not self.strips:
                    self.strips = Strips()
                self.strips.do_texinfo = True
            elif option == '-v':
                self.verbose = True

        # Read all data tables.
        if self.directory:
            import os
            os.chdir(self.directory)
        if self.iconv:
            self.iconv.digest()
        for name in arguments:
            input = Input(name)
            while True:
                line = input.readline()
                if not line:
                    break
                if line[0] == '\n':
                    continue
                if line[:2] == '/*':
                    while line.find('*/') < 0:
                        line = input.readline()
                    continue
                if input.begins('#    Name:'):
                    if not self.strips:
                        self.strips = Strips()
                    self.strips.digest_unimap(input)
                    break
                if line[0] == '#':
                    continue
                if input.begins('escape_char'):
                    if not self.mnemonics:
                        self.mnemonics = Mnemonics()
                    self.mnemonics.digest_mnemonics_ds(input)
                    break
                if input.match('Network Working Group +K\. Simonsen$'):
                    if (self.charnames
                            and self.charnames.do_sources
                            and not French_option):
                        while not input.begins(
                            '   3rd field is the long descriptive'):
                            line = input.readline()
                        if not self.mnemonics:
                            self.mnemonics = Mnemonics()
                        self.mnemonics.digest_rfc1345(input)
                    if self.explodes or self.strips:
                        while line != '5.  CHARSET TABLES\n':
                            line = input.readline()
                        if not self.strips:
                            self.strips = Strips()
                        self.strips.digest_rfc1345(input)
                    break
                if input.begins('@@\t'):
                    if self.charnames.do_sources and French_option:
                        self.charnames.digest_french(input)
                    break
                if line == '&referenceset\n':
                    while line != '\n':
                        line = input.readline()
                    if not self.strips:
                        self.strips = Strips()
                    if not self.mnemonics:
                        self.mnemonics = Mnemonics()
                    self.strips.digest_rfc1345(input)
                    break
                if line in ('   Repertoire according to ISO/IEC 10646-1:1993\n',
                            '   Control characters\n',
                            '   Private use\n'):
                    while line not in ('   Plane 000\n',
                                       '   plane 000\n'):
                        line = input.readline()
                    if not self.mnemonics:
                        self.mnemonics = Mnemonics()
                    self.mnemonics.digest_iso10646_def(input)
                    break
                input.die("Data file with unknown contents")
        for instance in (self.explodes,
                         self.strips,
                         self.charnames,
                         self.iconv,
                         self.mnemonics):
            if instance:
                instance.complete(French_option)

run = Main()
main = run.main

class Options:

    def __init__(self):
        self.do_sources = False
        self.do_texinfo = False

# Charnames.

class Charnames(Options):
    SOURCES = 'charname.h'

    # Name of character, given its numerical value.
    charname_map = {}

    # Maximum printable length of a character name.
    max_length = 0

    # Frequency of each word, then its crypt code.
    code_map = {}

    def digest_french(self, input):
        self.preset_french()
        fold_table = list(range(256))
        for before, after in zip(
                u'ABCDEFGHIJKLMNOPQRSTUVWXYZÀÂÇÈÉÊÎÏÑÔÖÛ'.encode('ISO-8859-1'),
                u'abcdefghijklmnopqrstuvwxyzàâçèéêîïñôöû'.encode('ISO-8859-1')):
            fold_table[before] = after
        folding = ''.join(map(chr, fold_table))
        ignorables = (
                u'<commande>'.encode('ISO-8859-1'),
                u'<réservé>'.encode('ISO-8859-1'),
                u'<pas un caractère>'.encode('ISO-8859-1'))
        while True:
            line = input.readline()
            if not line:
                break
            if input.begins('@@\t'):
                continue
            # Pour éliminer la fin de ligne.
            line = line.rstrip()
            input.line = line
            match = input.match('([0-9A-F]{4})\t([^(]+)( \\(.*\\))?( \\*)?$')
            if match:
                ucs = int(match.group(1), 16)
                text = match.group(2).translate(folding)
                if text in ignorables:
                    continue
                self.declare(ucs, re.sub(r' +\*$', '', text, 1))
            else:
                input.warn("Unrecognised line")

    def preset_french(self):
        self.max_length = 0
        ucs = 0x0000
        for text in (
            u"nul (nul)",                                        # 0000
            u"début d'en-tête (soh)",                            # 0001
            u"début de texte (stx)",                             # 0002
            u"fin de texte (etx)",                               # 0003
            u"fin de transmission (eot)",                        # 0004
            u"demande (enq)",                                    # 0005
            u"accusé de réception positif (ack)",                # 0006
            u"sonnerie (bel)",                                   # 0007
            u"espace arrière (bs)",                              # 0008
            u"tabulation horizontale (ht)",                      # 0009
            u"interligne (lf)",                                  # 000A
            u"tabulation verticale (vt)",                        # 000B
            u"page suivante (ff)",                               # 000C
            u"retour de chariot (cr)",                           # 000D
            u"hors code (so)",                                   # 000E
            u"en code (si)",                                     # 000F
            u"échappement transmission (dle)",                   # 0010
            u"commande d'appareil un (dc1)",                     # 0011
            u"commande d'appareil deux (dc2)",                   # 0012
            u"commande d'appareil trois (dc3)",                  # 0013
            u"commande d'appareil quatre (dc4)",                 # 0014
            u"accusé de réception négatif (nak)",                # 0015
            u"synchronisation (syn)",                            # 0016
            u"fin de transmission de bloc (etb)",                # 0017
            u"annulation (can)",                                 # 0018
            u"fin de support (em)",                              # 0019
            u"caractère de substitution (sub)",                  # 001A
            u"échappement (esc)",                                # 001B
            u"séparateur de fichier (fs)",                       # 001C
            u"séparateur de groupe (gs)",                        # 001D
            u"séparateur d'article (rs)",                        # 001E
            u"séparateur de sous-article (us)",                  # 001F
            ):
            self.declare(ucs, text)
            ucs += 1
        ucs = 0x007F
        for text in (
            u"suppression (del)",                                # 007F
            u"caractère de bourre (pad)",                        # 0080
            u"octet supérieur prédéfini (hop)",                  # 0081
            u"arrêt permis ici (bph)",                           # 0082
            u"aucun arrêt ici (nbh)",                            # 0083
            u"index (ind)",                                      # 0084
            u"à la ligne (nel)",                                 # 0085
            u"début de zone sélectionnée (ssa)",                 # 0086
            u"fin de zone sélectionnée (esa)",                   # 0087
            u"arrêt de tabulateur horizontal (hts)",             # 0088
            u"tabulateur horizontal avec justification (htj)",   # 0089
            u"arrêt de tabulateur vertical (vts)",               # 008A
            u"interligne partiel vers <= bas (pld)",             # 008B
            u"interligne partiel vers <= haut (plu)",            # 008C
            u"index inversé (ri)",                               # 008D
            u"remplacement unique deux (ss2)",                   # 008E
            u"remplacement unique trois (ss3)",                  # 008F
            u"chaîne de commande d'appareil (dcs)",              # 0090
            u"usage privé un (pu1)",                             # 0091
            u"usage privé deux (pu2)",                           # 0092
            u"mise en mode transmission (sts)",                  # 0093
            u"annulation du caractère précédent (cch)",          # 0094
            u"message en attente (mw)",                          # 0095
            u"début de zone protégée (sga)",                     # 0096
            u"fin de zone protégée (ega)",                       # 0097
            u"début de chaîne (sos)",                            # 0098
            u"introducteur de caractère graphique unique (sgci)",# 0099
            u"introducteur de caractère unique (sci)",           # 009A
            u"introducteur de séquence de commande (csi)",       # 009B
            u"fin de chaîne (st)",                               # 009C
            u"commande de système d'exploitation (osc)",         # 009D
            u"message privé (pm)",                               # 009E
            u"commande de progiciel (apc)",                      # 009F
            ):
            self.declare(ucs, text)
            ucs += 1

    def declare(self, ucs, text):
        self.charname_map[ucs] = text
        if len(text) > self.max_length:
            self.max_length = len(text)
        for word in text.split():
            self.code_map[word] = self.code_map.get(word, 0) + 1

    def presort_word(self, word):
        return -self.code_map[word], word

    # Write a compressed list of character names.
    def complete(self, french):
        if not self.do_sources:
            return
        if french:
            write = Output('fr-%s' % self.SOURCES).write
        else:
            write = Output(self.SOURCES).write
        # Establish a mild compression scheme.  Words word[:singles]
        # will be represented by a single byte running from 1 to
        # singles.  All remaining words will be represented by two
        # bytes, the first one running slowly from singles+1 to 255,
        # the second cycling faster from 1 to 255.
        if run.verbose:
            sys.stdout.write('  sorting words...')
        pairs = list(map(self.presort_word, self.code_map.keys()))
        pairs.sort()
        words = list(map(lambda pair: pair[1], pairs))
        pairs = None
        if run.verbose:
            sys.stdout.write(' %d of them\n' % len(words))
        count = len(words)
        singles = (255 * 255 - count) // 254
        # Transmit a few values for further usage by the C code.
        if run.verbose:
            sys.stdout.write('  sorting names...')
        ucs2_table = list(self.charname_map.keys())
        ucs2_table.sort()
        if run.verbose:
            sys.stdout.write(' %d of them\n' % len(ucs2_table))
        write('\n'
              '#define NUMBER_OF_SINGLES %d\n'
              '#define MAX_CHARNAME_LENGTH %d\n'
              '#define NUMBER_OF_CHARNAMES %d\n'
              % (singles, self.max_length, len(ucs2_table)))
        # Establish a mild compression scheme (one or two bytes per word).
        sys.stdout.write("  writing words\n")
        write('\n'
              'static const char *const word[%d] =\n'
              '  {\n'
              % count)
        char1 = 1
        char2 = 1
        for counter in range(singles):
            word = words[counter]
            write('    %-28s/* \\%0.3o */\n'
                  % ('"%s",' % re.sub('"', r'\"', word), char1))
            self.code_map[words[counter]] = char1
            char1 += 1
        for counter in range(singles, count):
            word = words[counter]
            write('    %-28s/* \\%0.3o\\%0.3o */\n'
                  % ('"%s",' % re.sub('"', r'\"', word, 1), char1, char2))
            self.code_map[words[counter]] = 256 * char1 + char2
            if char2 == 255:
                char1 += 1
                char2 = 1
            else:
                char2 += 1
        write('  };\n')
        sys.stdout.write("  writing names\n")
        write('\n'
              'struct charname\n'
              '  {\n'
              '    recode_ucs2 code;\n'
              '    const char *crypted;\n'
              '  };\n'
              '\n'
              'static const struct charname charname[NUMBER_OF_CHARNAMES] =\n'
              '  {\n')
        for ucs2 in ucs2_table:
            write('    {0x%04X, "' % ucs2)
            for word in self.charname_map[ucs2].split():
                if word in self.code_map:
                    code = self.code_map[word]
                    if code < 256:
                        write('\\%0.3o' % code)
                    else:
                        write('\\%0.3o\\%0.3o' % (code // 256, code % 256))
                else:
                    sys.stdout.write('??? %s\n' % word)
            write('"},\n')
        write('  };\n')

# Explodes.

class Explodes(Options):
    SOURCES = 'explode.c'

    def __init__(self):
        Options.__init__(self)
        # Table fragments will be produced while reading data tables.
        self.write = Output(self.SOURCES).write
        write = self.write
        write('\n'
              '#include "config.h"\n'
              '#include "common.h"\n')

    def complete(self, french):
        if not self.do_sources:
            return
        # Print the collectable initialization function.
        sys.stdout.write("Completing %s\n" % self.SOURCES)
        write = self.write
        write('\n'
              'bool\n'
              'module_explodes (struct recode_outer *outer)\n'
              '{\n')
        count = 0
        while self.declare_charset:
            write('  if (!declare_explode_data (outer, &data_%d, "%s"))\n'
                  '    return false;\n'
                  % (count, self.declare_charset[0]))
            del self.declare_charset[0]
            count += 1
        write('\n')
        while declare_alias:
            write('  if (!declare_alias (outer, "%s", "%s"))\n'
                  '    return false;\n'
                  % declare_alias[0])
            del declare_alias[0]
        write('\n'
              '  return true;\n'
              '}\n'
              '\n'
              'void\n'
              'delmodule_explodes (_GL_UNUSED struct recode_outer *outer)\n'
              '{\n'
              '}\n')

# Iconv.

class Iconv(Options):
    SOURCES = 'iconvdecl.h'

    data = []

    def digest(self):
        canonical = {}
        for charset in ('Georgian-Academy', 'Georgian-PS', 'MuleLao-1',
                        'Macintosh', 'MacArabic', 'MacCentralEurope',
                        'MacCroatian', 'MacCyrillic', 'MacGreek', 'MacHebrew',
                        'MacIceland', 'MacRoman', 'MacRomania', 'MacThai',
                        'MacTurkish', 'MacUkraine'):
            canonical[charset.upper()] = charset

        # Read in the encodings.def file.
        sys.stdout.write("Reading from `iconv -l'\n")
        libc = None
        import os
        names = []
        for line in os.popen('iconv -l'):
            if libc is None:
                libc = len(line.split('/')) == 3
            if libc:
                first, second, empty = line.split('/')
                assert empty == '\n', repr(line)
                name = second or first
                if name not in names:
                    names.append(name)
                    self.data.append((name, ()))
            else:
                aliases = []
                for alias in line.split():
                    if alias in canonical:
                        alias = canonical[alias]
                    aliases.append(alias)
                self.data.append((aliases[0], aliases[1:]))

    def complete(self, french):
        def write_charset(format, charset):
            write(format % charset)
            write(format % (charset + "-translit"))
        if not self.do_sources:
            return
        write = Output(self.SOURCES).write
        count = 1
        for charset, aliases in self.data:
            versions = 2 # Normal, //TRANSLIT
            count = count + (versions + len(aliases)) * versions
        write('\n'
              "/* This is derived from Bruno Haible's `libiconv' package.  */"
              '\n'
              'static const char *iconv_name_list[%d] =\n'
              '  {\n'
              % count)
        for charset, aliases in self.data:
            if aliases:
                write_charset('    "%s",\n', charset)
                for alias in aliases[:-1]:
                    write_charset('\t"%s",\n', alias)
                write_charset('\t"%s", NULL,\n', aliases[-1])
            else:
                write_charset('    "%s", NULL,\n', charset)
        write('    NULL\n'
              '  };\n')

# Mnemonics.

class Mnemonics(Options):
    SOURCES = 'rfc1345.h'

    # Ignore any mnemonic whose length is greater than MAX_MNEMONIC_LENGTH.
    MAX_MNEMONIC_LENGTH = 3

    # Numeric value of a character, given its mnemonic.
    ucs2_map = {}

    table_length = 0
    mnemonic_map = {}

    # Read in a mnemonics file.
    def digest_mnemonics_ds(self, input):
        while input.readline():
            match = input.match('<([^ \t\n]+)>\t<U(....)>')
            if match:
                mnemonic = re.sub('/(.)', r'\1', match.group(1))
                ucs2 = int(match.group(2), 16)
                self.declare(mnemonic, ucs2, input.warn)

    # Read in Keld's list of 10646 characters.
    def digest_iso10646_def(self, input):
        while True:
            line = input.readline()
            if not line:
                break
            if line == '\n':
                continue
            if len(line) == 3:
                continue
            if input.begins('   \.\.\.'):
                continue
            if line == '   Presentation forms\n':
                continue
            if input.begins('   naming: first vertical '):
                continue
            match = input.match('   row ([0-9][0-9][0-9])$')
            if match and int(match.group(1)) < 256:
                row = int(match.group(1))
                cell = 0
                continue
            if line == '   cell 00\n':
                cell = 0
                continue
            match = input.match('   cell ([0-9][0-9][0-9])$')
            if match and int(match.group(1)) < 256:
                cell = int(match.group(1))
                continue
            if input.match('   [^ ]+'):
                if not input.match('   [A-Z][A-Z][A-Z]'):
                    continue
            if input.match('   [^ ].*'):
                if cell == 256:
                    input.warn("Over 256 cells in row %d", row)
                cell += 1
                continue
            match = (input.match('([^ ])  [^ ].*')
                     or input.match('([^ ][^ ]+) [^ ].*'))
            if match:
                if cell == 256:
                    input.warn("Over 256 cells in row %d", row)
                self.declare(match.group(1), 256*row + cell, input.warn)
                cell += 1
                continue
            input.warn("Unrecognised line")

    # Read the text of RFC 1345, saving all character names it declares.
    def digest_rfc1345(self, input):
        def read_line(input=input):
            skip = False
            while True:
                line = input.readline()
                if not line:
                    break
                if input.begins('Simonsen'):
                    skip = True
                    continue
                if skip:
                    if input.begins('RFC 1345'):
                        skip = False
                    continue
                if input.begins('4.  CHARSETS'):
                    break
                if line == '\n':
                    continue
                if line[0] == ' ':
                    return line[:-1].lstrip()
            return None
        self.max_length = 0
        # Read the character descriptions.  Count words in charnames.
        line = read_line()
        while line:
            # Look ahead one line and merge it if it should.
            next = read_line()
            while next:
                match = re.match('             *( .*)', next)
                if not match:
                    break
                line += match.group(1)
                next = read_line()
            # Separate fields and save needed information.
            match = re.search('([^ ]+) +[0-9a-f]+ +(.*)', line)
            if match:
                mnemo = match.group(1)
                text = match.group(2).lower()
                if mnemo in self.ucs2_map:
                    run.charnames.declare(self.ucs2_map[mnemo], text)
                elif len(mnemo) <= self.MAX_MNEMONIC_LENGTH:
                    input.warn("No known UCS-2 code for `%s'", mnemo)
            elif not re.search(' +e000', line):
                input.warn("Unrecognised line")
            line = next

    # Declare a correspondence between a mnemonic and an UCS-2 value.
    def declare(self, mnemonic, ucs2, warn):
        if len(mnemonic) > self.MAX_MNEMONIC_LENGTH:
            return
        if self.do_sources:
            if ucs2 in self.mnemonic_map:
                if self.mnemonic_map[ucs2] != mnemonic:
                    warn("U+%04X `%s' known as `%s'",
                               ucs2, mnemonic, self.mnemonic_map[ucs2])
                    if len(mnemonic) < len(self.mnemonic_map[ucs2]):
                        self.mnemonic_map[ucs2] = mnemonic
            else:
                self.mnemonic_map[ucs2] = mnemonic
                self.table_length += 1
        if mnemonic in self.ucs2_map:
            if self.ucs2_map[mnemonic] != ucs2:
                warn("`%s' U+%04X known as U+%04X",
                     mnemonic, ucs2, self.ucs2_map[mnemonic])
                #FIXME: ??? cell = self.ucs2_map[mnemonic] - 256*row
        else:
            self.ucs2_map[mnemonic] = ucs2

    def complete(self, french):
        if self.do_sources:
            self.complete_sources()

    # Write an UCS-2 to RFC 1345 mnemonic table.
    def complete_sources(self):
        inverse_map = {}
        write = Output(self.SOURCES).write
        write('\n'
              '#define TABLE_LENGTH %d\n'
              '#define MAX_MNEMONIC_LENGTH %d\n'
              % (self.table_length, self.MAX_MNEMONIC_LENGTH))
        write('\n'
              'struct entry\n'
              '  {\n'
              '    recode_ucs2 code;\n'
              '    const char *rfc1345;\n'
              '  };\n'
              '\n'
              'static const struct entry table[TABLE_LENGTH] =\n'
              '  {\n')
        count = 0
        indices = list(self.mnemonic_map.keys())
        indices.sort()
        for ucs2 in indices:
            text = self.mnemonic_map[ucs2]
            inverse_map[text] = count
            write('    /* %4d */ {0x%04X, "%s"},\n'
                  % (count, ucs2, re.sub(r'([\"])', r'\\\1', text)))
            count += 1
        write('  };\n')

        write('\n'
              'static const unsigned short inverse[TABLE_LENGTH] =\n'
              '  {')
        count = 0
        keys = list(inverse_map.keys())
        keys.sort()
        for text in keys:
            if count % 10 == 0:
                if count != 0:
                    write(',')
                write('\n    /* %4d */ ' % count)
            else:
                write(', ')
            write('%4d' % inverse_map[text])
            count += 1
        write('\n'
              '  };\n')

# Global table of strips.

class Strips(Options):
    POOL = 'strip-pool.c'
    DATA = 'strip-data.c'
    TEXINFO = 'rfc1345.texi'

    # Change STRIP_SIZE in `src/recode.h' if you change the value here.
    # See the accompanying documentation there, as needed.
    STRIP_SIZE = 8

    # Prepare the production of tables.
    pool_size = 0
    pool_refs = 0
    strip_map = {}
    strips = []

    # While digesting files.
    used_map = {}
    table = []
    declare_alias = []
    implied_surface = {}

    def __init__(self):
        Options.__init__(self)
        self.write_data = None
        self.aliases_map = {}
        self.remark_map = {}
        self.declare_charset = []
        # Prepare to read various tables.
        self.charset_ordinal = 0
        self.discard_charset = False
        self.alias_count = 0
        self.comment = ''

    def init_write_data(self):
        if self.do_sources and not self.write_data:
            # Table fragments will be produced while reading data tables.
            self.write_data = Output(self.DATA).write
            write = self.write_data
            write('\n'
                  '#include "config.h"\n'
                  '#include "common.h"\n'
                  '#include "decsteps.h"\n')

    # Read the text of RFC 1345, saving all charsets it declares.
    # UCS-2 mnemonics files should have been read in already.
    def digest_rfc1345(self, input):
        self.init_write_data()
        # Informal canonical order of presentation.
        CHARSET, REM, ALIAS, ESC, BITS, CODE = range(6)
        charset = None
        skip = False
        while True:
            line = input.readline()
            if not line:
                break
            if input.begins('Simonsen'):
                skip = True
                continue
            if skip:
                if input.begins('RFC 1345'):
                    skip = False
                continue
            if line == '\n':
                continue
            if line == 'ACKNOWLEDGEMENTS\n':
                break
            line, count = re.subn('^  ?', '', line)
            if not count:
                continue
            input.line = line
            # Recognize `&charset'.
            match = input.match('&charset (.*)')
            if match:
                # Before beginning a new charset, process the previous one.
                if charset:
                    self.charset_done(charset, remark, aliases)
                charset = match.group(1)
                # Prepare for processing a new charset: save the charset
                # name for further declaration; announce this charset in
                # the array initialization section; and initialize its
                # processing.
                if run.verbose:
                    sys.stdout.write("  %d) %s\n"
                                     % (self.charset_ordinal + 1, charset))
                status = CHARSET
                self.comment = '\n/* %s\n' % charset
                hashname = re.sub('[^a-z0-9]', '', charset.lower())
                if hashname in self.used_map:
                    input.warn("Duplicate of %s (discarded)",
                               self.used_map[hashname])
                    self.discard_charset = True
                    continue
                self.used_map[hashname] = charset
                self.alias_count = 0
                self.table = [NOT_A_CHARACTER] * 256
                codedim = 0
                code = 0
                aliases = []
                remark = []
                #FIXME:match = re.match('(CP|IBM|windows-)([0-9]+)$', charset)
                match = re.match('(CP|IBM)([0-9]+)$', charset)
                if match:
                    self.implied_surface[match.group(2)] = 'crlf'
                    self.implied_surface['CP' + match.group(2)] = 'crlf'
                    self.implied_surface['IBM' + match.group(2)] = 'crlf'
                    self.declare_alias.append((charset, charset))
                    self.alias_count += 1
                    continue
                if charset in ('macintosh', 'macintosh_ce'):
                    self.implied_surface[charset] = 'cr'
                    self.declare_alias.append((charset, charset))
                    self.alias_count += 1
                    continue
                continue
            # Recognize other `&' directives.
            match = input.match('&rem (.*)')
            if match and not input.begins('&rem &alias'):
                # Keld now prefers `&rem' to be allowed everywhere.
                #if status > REM:
                #    input.warn("`&rem' out of sequence")
                #status = REM;
                if self.do_texinfo:
                    # Save remarks for Texinfo.
                    text = match.group(1)
                    remark.append(text)
                continue
            match = input.match('(&rem )?&alias (.*)')
            if match:
                if status > ALIAS:
                    input.warn("`&alias' out of sequence")
                status = ALIAS
                # Save synonymous charset names for later declarations.
                alias = match.group(2)
                if alias[-1] == ' ':
                    input.warn("Spurious trailing whitespace")
                    alias = alias.rstrip()
                self.comment = self.comment + '   %s\n' % alias
                hashname = re.sub('[^a-z0-9]', '', alias.lower())
                if hashname in self.used_map:
                    if self.used_map[hashname] != charset:
                        input.warn("Duplicate of %s", self.used_map[hashname])
                        continue
                else:
                    self.used_map[hashname] = charset
                aliases.append(alias)
                match = re.match('(CP|IBM)([0-9]+)$', alias)
                if match:
                    self.implied_surface[match.group(2)] = 'crlf'
                    self.implied_surface['CP' + match.group(2)] = 'crlf'
                    self.implied_surface['IBM' + match.group(2)] = 'crlf'
                elif alias in ('mac', 'macce'):
                    self.implied_surface[alias] = 'cr'
                self.declare_alias.append((alias, charset))
                self.alias_count += 1
                continue
            if input.match('&g[0-4]esc'):
                if status > ESC:
                    input.warn("`&esc' out of sequence")
                status = ESC
                continue
            match = input.match('&bits ([0-9]+)$')
            if match:
                if status > BITS:
                    input.warn("`&bits' out of sequence")
                status = BITS
                if int(match.group(1)) > 8:
                    input.warn("`&bits %s' not accepted (charset discarded)",
                               match.group(1))
                    self.discard_charset = True
                continue
            match = input.match('&code (.*)')
            if match:
                if status > CODE:
                    input.warn("`&code' out of sequence")
                status = CODE
                # Save the code position.
                code = int(match.group(1))
                continue
            # Other lines cause the charset to be discarded.
            match = input.match('&([^ ]+)')
            if match:
                if not self.discard_charset:
                    input.warn("`&%s' not accepted (charset discarded)",
                               match.group(1))
                    self.discard_charset = True
            if self.discard_charset:
                continue
            # Save all other tokens into the double table.
            for token in line.split():
                if token == '??':
                    self.table[code] = NOT_A_CHARACTER
                elif token == '__':
                    self.table[code] = REPLACEMENT_CHARACTER
                elif token in run.mnemonics.ucs2_map:
                    self.table[code] = run.mnemonics.ucs2_map[token]
                    if len(token) > codedim:
                        codedim = len(token)
                else:
                    input.warn("Unknown mnemonic for code: %s", token)
                    self.table[code] = REPLACEMENT_CHARACTER
                code += 1
        # Push the last charset out.
        self.charset_done(charset, remark, aliases)

    # Read a Unicode map, as found in ftp://ftp.unicode.com/MAPPINGS.
    def digest_unimap(self, input):
        self.init_write_data()
        line = input.line
        match = input.match('# +Name: +([^ ]+) to Unicode table$')
        if match:
            # Set comment.
            name = match.group(1).split()
            charset = name[0]
            del name[0]
            self.comment = '\n/* %s\n' % charset
            # Set charset.
            hashname = re.sub('[^a-z0-9]', '', charset.lower())
            if self.used_map[hashname]:
                input.warn("`%s' duplicates `%s' (charset discarded)",
                           hashname, self.used_map[hashname])
                self.discard_charset = True
                return
            self.used_map[hashname] = charset
            # Prepare for read.
            self.alias_count = 0
            self.table = [NOT_A_CHARACTER] * 256
            codedim = 0
            code = 0
            aliases = []
            remark = []
        if self.discard_charset:
            return
        # Process aliases.
        for alias in name:
            self.comment = self.comment + '   %s\n' % alias

            hashname = re.sub('[^a-z0-9]', '', alias.lower())
            if self.used_map[hashname] and self.used_map[hashname] != charset:
                input.warn("`%s' duplicates `%s'", hashname,
                           self.used_map[hashname])
                continue
            self.used_map[hashname] = charset

            aliases.append(alias)
            self.declare_alias.append((alias, charset))
            self.alias_count += 1
        # Read table contents.
        while True:
            line = input.readline()
            if not line:
                break
            if line == '\n':
                continue
            if line[0] == '#':
                continue
            if input.match('0x([0-9A-F]+)\t\t#UNDEFINED$'):
                continue
            if input.search('\032'):
                # Old MS-DOS C-z !!
                break
            match = input.match('0x([0-9A-F]+)\t0x([0-9A-F]+)\t\#')
            if match:
                self.table[int(match.group(1), 16)] = int(match.group(2), 16)
            else:
                input.warn("Unrecognised input line")
        # Complete processing.
        self.charset_done(charset, remark, aliases)

    # Print all accumulated information for the charset.  If the
    # charset should be discarded, adjust tables.
    def charset_done(self, charset, remark, aliases):
        if self.discard_charset:
            while self.alias_count > 0:
                del self.declare_alias[-1]
                self.alias_count -= 1
            self.discard_charset = False
            self.comment = ''
        if not self.comment:
            return
        if self.do_texinfo:
            # Save the documentation.
            aliases.sort()
            self.aliases_map[charset] = aliases
            self.remark_map[charset] = remark
        if run.explodes:
            write = run.explodes.write
            # Make introductory C comments.
            write(self.comment)
            write('*/\n')
            # Make the table for this charset.
            write('\n'
                  'static const unsigned short data_%d[] =\n'
                  '  {\n'
                  % self.charset_ordinal)
            for code in range(256):
                if code != self.table[code]:
                    write('    %3d, 0x%.4X, DONE,\n'
                          % (code, self.table[code]))
            write('    DONE\n'
                  '  };\n')
            # Register the table.
            self.declare_charset.append(charset)
        if self.do_sources:
            write = self.write_data
            # Make introductory C comments.
            write(self.comment)
            write('*/\n')
            # Make the table for this charset.
            write('\n'
                  'static struct strip_data data_%d =\n'
                  '  {\n'
                  '    ucs2_data_pool,\n'
                  '    {\n'
                  % self.charset_ordinal)
            count = 0
            for code in range(0, 256, self.STRIP_SIZE):
                if count % 12 == 0:
                    if count != 0:
                        write(',\n')
                    write('      ')
                else:
                    write(', ')
                strip = self.table[code:code+self.STRIP_SIZE]
                write('%4d' % self.pool_index(strip))
                count += 1
            write('\n'
                  '    }\n'
                  '  };\n')
            # Register the table.
            self.declare_charset.append(charset)
        self.charset_ordinal += 1
        self.comment = ''

    # Return the pool index for strip.  Add to the pool as required.
    def pool_index(self, strip):

        def format(item):
            return '%04X' % item

        self.pool_refs += 1
        text = ''.join(map(format, strip))
        if text not in self.strip_map:
            self.strip_map[text] = self.pool_size
            self.pool_size = self.pool_size + self.STRIP_SIZE
            self.strips.append(text)
        return self.strip_map[text]

    def complete(self, french):
        if self.do_sources:
            self.complete_sources()
        if self.do_texinfo:
            self.complete_texinfo(french)

    def complete_sources(self):
        # Give memory statistics.
        sys.stdout.write('Table memory = %d bytes (pool %d, refs %d)\n'
                         % (self.pool_size * 2 + self.pool_refs * 2,
                            self.pool_size * 2,
                            self.pool_refs * 2))

        # Print the collectable initialization function.
        sys.stdout.write("Completing %s\n" % self.DATA)
        write = self.write_data
        write('\n'
              '_GL_ATTRIBUTE_CONST bool\n'
              'module_strips (_GL_UNUSED struct recode_outer *outer)\n'
              '{\n'
              '  RECODE_ALIAS alias;\n'
              '\n')
        count = 0
        while self.declare_charset:
            write('  if (!declare_strip_data (outer, &data_%d, "%s"))\n'
                  '    return false;\n'
                  % (count, self.declare_charset[0]))
            del self.declare_charset[0]
            count += 1
        write('\n')
        while self.declare_alias:
            alias, charset = self.declare_alias[0]
            if alias in self.implied_surface:
                write('  if (alias = declare_alias (outer, "%s", "%s"),'
                      ' !alias)\n'
                      '    return false;\n'
                      % self.declare_alias[0])
                write('  if (!declare_implied_surface (outer, alias,'
                      ' outer->%s_surface))\n'
                      '    return false;\n'
                      % self.implied_surface[alias])
            else:
                write('  if (!declare_alias (outer, "%s", "%s"))\n'
                      '    return false;\n'
                      % self.declare_alias[0])
            del self.declare_alias[0]
        write('\n'
              '  return true;\n'
              '}\n'
              '\n'
              'void\n'
              'delmodule_strips (_GL_UNUSED struct recode_outer *outer)\n'
              '{\n'
              '}\n')

        # Write the pool file.
        write = Output(self.POOL).write
        write('\n'
              '#include "config.h"\n'
              '#include "common.h"\n'
              '\n'
              'const recode_ucs2 ucs2_data_pool[%d] =\n'
              '  {'
              % self.pool_size)
        count = 0
        for strip in self.strips:
            for pos in range(0, self.STRIP_SIZE * 4, 4):
                if count % 8 == 0:
                    if count != 0:
                        write(',')
                    write('\n    /* %4d */ ' % count)
                else:
                    write(', ')
                write('0x' + strip[pos:pos+4])
                count += 1
        write('\n'
              '  };\n')

    def complete_texinfo(self, french):
        if french:
            write = Output('fr-%s' % self.TEXINFO, noheader=True).write
        else:
            write = Output(self.TEXINFO, noheader=True).write
        charsets = list(self.remark_map.keys())
        charsets.sort()
        for charset in charsets:
            write('\n'
                  '@item %s\n'
                  '@tindex %s@r{, aliases and source}\n'
                  % (charset, re.sub(':([0-9]+)', r'(\1)', charset)))
            aliases = self.aliases_map[charset]
            if aliases:
                if len(aliases) == 1:
                    if aliases[0]:      # FIXME: why empty sometimes?
                        write('@tindex %s\n'
                              '@code{%s} is an alias for this charset.\n'
                              % (re.sub(':([0-9]+)', r'(\1)', aliases[0]),
                                 aliases[0]))
                else:
                    for alias in aliases:
                        write('@tindex %s\n'
                              % re.sub(':([0-9]+)', r'(\1)', alias))
                    write('@code{%s} and @code{%s} are aliases'
                          ' for this charset.\n'
                          % ('}, @code{'.join(aliases[:-1]), aliases[-1]))
            for line in self.remark_map[charset]:
                if line[0].islower():
                    line = line[0].upper() + line[1:]
                write(line.replace('@', '@@'))
                if line[-1] != '.':
                    write('.')
                write('\n')

# Handling basic input and output.

class Input:

    def __init__(self, name):
        self.name = name
        self.input = io.open(name, encoding='latin-1')
        self.line_count = 0
        sys.stdout.write("Reading %s\n" % name)

    def readline(self):
        self.line = self.input.readline()
        self.line_count += 1
        if type(self.line) == bytes:
            self.line = self.line.decode('utf-8')
        return self.line

    def warn(self, format, *args):
        if run.verbose:
            sys.stdout.write('%s:%s: %s\n'
                             % (self.name, self.line_count, format % args))

    def die(self, format, *args):
        sys.stdout.write('%s:%s: %s\n'
                         % (self.name, self.line_count, format % args))
        raise 'Fatal'

    def begins(self, text):
        return self.line[:len(text)] == text

    def match(self, pattern):
        return re.match(pattern, self.line)

    def search(self, pattern):
        return re.search(pattern, self.line)

class Output:

    def __init__(self, name, noheader=False):
        self.name = name
        self.write = open(name, 'w', encoding='utf-8').write
        sys.stdout.write("Writing %s\n" % name)
        if not noheader:
            self.write("""\
/* DO NOT MODIFY THIS FILE!  It was generated by `recode/tables.py'.  */

/* Conversion of files between different charsets and surfaces.
   Copyright © 1999 Free Software Foundation, Inc.
   Contributed by François Pinard <pinard@iro.umontreal.ca>, 1993, 1997.

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
   Suite 330, Boston, MA 02111-1307, USA.  */
""")

if __name__ == '__main__':
    main(*sys.argv[1:])

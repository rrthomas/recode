# Makefile for Recode sources.
# Copyright © 1991,92,93,94,95,96,97,98,99, 00 Free Software Foundation, Inc.
# François Pinard <pinard@iro.umontreal.ca>, 1988.

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

AUTOMAKE_OPTIONS = gnits
bin_PROGRAMS = recode
lib_LTLIBRARIES = librecode.la
noinst_LTLIBRARIES = libmerged.la
man_MANS = recode.1
include_HEADERS = recode.h recodext.h

H_STEPS = common.h decsteps.h inisteps.h tersteps.h \
$(H_FALLBACKS) $(H_SURFACES)

noinst_HEADERS = $(H_STEPS) cleaner.h charname.h fr-charname.h iconvdecl.h \
rfc1345.h lat1iso5426.h lat1ansel.h

EXTRA_DIST = stamp-steps stamp-strip $(L_STEPS) mergelex.py $(MANS) lat1ansel.h lat1iso5426.h

CLEANFILES = iconvdecl.h

C_STEPS = african.c afrtran.c atarist.c bangbang.c cdcnos.c \
ebcdic.c ibmpc.c iconqnx.c lat1asci.c lat1iso5426.c lat1ansel.c \
java.c mule.c strip-data.c testdump.c ucs.c utf16.c utf7.c utf8.c varia.c vn.c

ALL_STEPS = $(C_STEPS) $(C_FALLBACKS) $(C_SURFACES)

L_STEPS = ascilat1.l iso5426lat1.l ansellat1.l $(L_FALLBACKS) $(L_SURFACES)

H_FALLBACKS =
C_FALLBACKS = flat.c html.c lat1ltex.c lat1btex.c lat1txte.c rfc1345.c texinfo.c
L_FALLBACKS = ltexlat1.l btexlat1.l txtelat1.l

H_SURFACES = base64.h
C_SURFACES = base64.c dump.c endline.c permut.c quoted.c
L_SURFACES =

recode_SOURCES = main.c mixed.c common.h
recode_LDADD = librecode.la

librecode_la_SOURCES = charname.c combine.c fr-charname.c iconv.c \
names.c outer.c recode.c request.c strip-pool.c task.c $(ALL_STEPS) \
$(include_HEADERS) $(noinst_HEADERS) $(H_STEPS)
librecode_la_LDFLAGS = -version-info $(VERSION_INFO) $(LTLIBINTL) \
	$(LIB_CLOCK_GETTIME) $(LIB_GETRANDOM) $(LIB_HARD_LOCALE) $(LIB_MBRTOWC) $(LIB_SETLOCALE_NULL)
librecode_la_LIBADD = ../lib/libgnu.la libmerged.la

libmerged_la_SOURCES = merged.c
libmerged_la_CPPFLAGS = $(NON_WARN_CFLAGS)

loc:
	cloc \
	charname.c combine.c fr-charname.c iconv.c \
	names.c outer.c recode.c request.c task.c \
	$(ALL_STEPS) $(L_STEPS) common.h $(H_FALLBACKS) $(H_SURFACES) \
	$(top_srcdir)/tables.py mergelex.py $(top_srcdir)/tests/Recode.pyx \
	Makefile.am $(top_srcdir)/configure.ac $(top_srcdir)/Makefile.am \
	$(top_srcdir)/doc/Makefile.am $(top_srcdir)/tests/Makefile.am $(top_srcdir)/bootstrap.conf

# Relative to $(srcdir).
ALAIN = $(KELD)
KELD = ../keld
CHARSETS_DEF = $(KELD)/charsets.def
MNEMONICS_DS = $(KELD)/iso10646.def $(KELD)/other.def $(KELD)/control.def
RFC1345_TXT = $(KELD)/rfc1345.txt
NOMS_CARACS = $(ALAIN)/NomsSeulsfinal.lst

NON_WARN_CFLAGS = -I.. -I$(srcdir) $(ISYSTEM)$(top_builddir)/lib $(ISYSTEM)$(top_srcdir)/lib
AM_CPPFLAGS = $(NON_WARN_CFLAGS) $(WARN_CFLAGS)
DEFS = -DLIBDIR=\"$(libdir)\" @DEFS@

ETAGS_ARGS = $(L_STEPS)
TAGS_DEPENDENCIES = $(L_STEPS)

LEX = @LEX@
PYTHON = @PYTHON@
TABLES_PY = $(PYTHON) $(top_srcdir)/tables.py

merged.c: mergelex.py $(L_STEPS)
	echo '#include "config.h"' > merged.c
	(cd $(srcdir) && cat $(L_STEPS)) \
	| $(PYTHON) $(srcdir)/mergelex.py > merged.tm1
	$(LEX) -t -8 -Plibrecode_yy merged.tm1 > merged.tm2
	grep -av '^# *line [0-9]' merged.tm2 >> merged.c
	rm merged.tm1 merged.tm2

# Depend on recode$(EXEEXT) rather than explicitly make-ing it, as otherwise
# we break parallel builds, as libmerged.la can be built twice in parallel,
# which can fail.
recode.1: main.c $(top_srcdir)/configure.ac recode$(EXEEXT)
## Exit gracefully if recode.1 is not writeable, such as during distcheck!
	$(AM_V_GEN)if ( touch $@.w && rm -f $@.w; ) >/dev/null 2>&1; then \
	  $(top_srcdir)/build-aux/missing --run $(HELP2MAN) --locale=en_US.UTF-8 \
		--name="converts files between character sets" \
		--output=$@ ./recode; \
	fi

main.o: main.c ../config.status

charname.lo: charname.h
charname.h: ../tables.py $(MNEMONICS_DS) $(RFC1345_TXT)
	$(TABLES_PY) -C $(srcdir) -n $(MNEMONICS_DS) $(RFC1345_TXT)

fr-charname.lo: fr-charname.h
fr-charname.h: ../tables.py $(NOMS_CARACS)
	$(TABLES_PY) -C $(srcdir) -Fn $(NOMS_CARACS)

iconv.lo: iconvdecl.h
iconvdecl.h: ../tables.py
	$(TABLES_PY) -i

lat1ansel.lo: lat1ansel.h
lat1ansel.h: ansellat1.l
	$(top_srcdir)/build-aux/invert-l-to-h ansellat1.l > $@

lat1iso5426.lo: lat1iso5426.h
lat1iso5426.h: iso5426lat1.l
	$(top_srcdir)/build-aux/invert-l-to-h iso5426lat1.l > $@

african.lo afrtran.lo atarist.lo bangbang.lo cdcnos.lo \
ebcdic.lo ibmpc.lo iconqnx.lo lat1asci.lo lat1iso5426.lo lat1ansel.lo \
java.lo mule.lo strip-data.lo testdump.lo ucs.lo utf16.lo utf7.lo utf8.lo \
varia.lo vn.lo flat.lo html.lo lat1ltex.lo lat1btex.lo lat1txte.lo rfc1345.lo \
texinfo.lo base64.lo dump.lo endline.lo permut.lo quoted.lo \
iconv.lo outer.lo: decsteps.h inisteps.h tersteps.h

decsteps.h inisteps.h tersteps.h: stamp-steps
stamp-steps: $(ALL_STEPS) merged.c
	@echo "Updating {dec,ini,ter}steps.h"
	@(cd $(srcdir) && grep -h '^module_' $(ALL_STEPS) merged.c) \
	    | sed 's/ .*//' > liste.tmp
	@sed 's/\(.*\)/bool \1 (struct recode_outer *);/' \
	    liste.tmp > decsteps.tmp
	@sed 's/\(.*\)/void del\1 (struct recode_outer *);/' \
	    liste.tmp >> decsteps.tmp
	@sed 's/\(.*\)/  if (!\1 (outer)) return false;/' \
	    liste.tmp > inisteps.tmp
	@sed 's/\(.*\)/  del\1 (outer);/' \
	    liste.tmp > tersteps.tmp
	@rm liste.tmp
	@if cmp -s $(srcdir)/decsteps.h decsteps.tmp; then rm decsteps.tmp; \
	else mv decsteps.tmp $(srcdir)/decsteps.h; fi
	@if cmp -s $(srcdir)/inisteps.h inisteps.tmp; then rm inisteps.tmp; \
	else mv inisteps.tmp $(srcdir)/inisteps.h; fi
	@if cmp -s $(srcdir)/tersteps.h tersteps.tmp; then rm tersteps.tmp; \
	else mv tersteps.tmp $(srcdir)/tersteps.h; fi
	@echo timestamp > $(srcdir)/$@

rfc1345.lo: rfc1345.h
rfc1345.h: ../tables.py $(MNEMONICS_DS)
	$(TABLES_PY) -C $(srcdir) -m $(MNEMONICS_DS)

# There is duplication of work with making doc/rfc1345.texi.
# This used to be combined, but it seems clearer to keep both separate.
strip-pool.c strip-data.c: stamp-strip
stamp-strip: ../tables.py $(MNEMONICS_DS) $(CHARSETS_DEF)
	$(TABLES_PY) -C $(srcdir) -p $(MNEMONICS_DS) $(CHARSETS_DEF)
	@echo timestamp > $(srcdir)/$@

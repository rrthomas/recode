# Makefile for Recode documentation.
# Copyright © 1994-2019 Free Software Foundation, Inc.
# François Pinard <pinard@iro.umontreal.ca>

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
info_TEXINFOS = recode.texi
MAKEINFO = LANG= LANGUAGE= @MAKEINFO@
TEXINFO_TEX = $(srcdir)/../build-aux/texinfo.tex

EXTRA_DIST = rfc1345.texi File-Latin1
MOSTLYCLEANFILES = recode.ops

# Relative to $(srcdir).
KELD = ../keld
CHARSETS_DEF = $(KELD)/charsets.def
MNEMONICS_DS = $(KELD)/iso10646.def $(KELD)/other.def $(KELD)/control.def

TABLES_PY = $(PYTHON) $(top_srcdir)/tables.py

# Disable dvi target, to avoid needing TeX for make distcheck
# (and in any case, no-one wants DVI)
dvi:

$(srcdir)/recode.info: rfc1345.texi
recode.pdf: rfc1345.texi

# There is duplication of work with making src/strip-{pool,data}.c.
# This used to be combined, but it seems clearer to keep both separate.
rfc1345.texi: ../tables.py $(MNEMONICS_DS) $(CHARSETS_DEF)
	$(TABLES_PY) -C $(srcdir) -t $(MNEMONICS_DS) $(CHARSETS_DEF)

fr-rfc1345.texi: ../tables.py $(MNEMONICS_DS) $(CHARSETS_DEF)
	$(TABLES_PY) -C $(srcdir) -Ft $(MNEMONICS_DS) $(CHARSETS_DEF)

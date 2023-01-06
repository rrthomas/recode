# Makefile for Recode regression tests.
# Copyright © 1996-2023 Free Software Foundation, Inc.
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

# LIMIT= may limit testing.  For example, "make check LIMIT='-l 1'" stops
# testing after the first error, "make check LIMIT='-k utf7' runs files
# matched by the "utf7" regexp.  Try "./pytest -h" for a list of options.

SUITE = t21_names.py t22_lists.py t25_subsets.py t30_base64.py \
t30_dumps.py t30_quoted.py t40_african.py t40_combine.py t40_testdump.py \
t40_utf7.py t40_utf8.py t50_methods.py t90_bigauto.py

CYTHON = @CYTHON@
EXTRA_DIST = Recode.c Recode.pyx pytest common.py asan-suppressions.txt $(SUITE)
CLEANFILES = Recode.body.c
DISTCLEANFILES = Recode.c

if HAVE_CYTHON
# TESTS_ENVIRONMENT_EXTRA can be set at configure time
check-local: Recode@pyextext@
	export LD_LIBRARY_PATH=$(top_builddir)/src/@objdir@; \
	export DYLD_LIBRARY_PATH=$(top_builddir)/src/@objdir@; \
	export LSAN_OPTIONS=$$LSAN_OPTIONS:fast_unwind_on_malloc=0:print_suppressions=0:suppressions=$(srcdir)/asan-suppressions.txt; \
	RECODE=$(top_builddir)/src/recode$(EXEEXT) PYTHONPATH=.:$(srcdir) \
	  $(TESTS_ENVIRONMENT_EXTRA) $(PYTHON) $(srcdir)/pytest $(LIMIT) $(srcdir)/t*.py

Recode@pyextext@: Recode.c setup.py
	rm -f *@pyextext@
	$(PYTHON) setup.py build_ext --inplace
	rm -fr build

Recode.c: Recode.pyx Makefile
	rm -f Recode.c && \
	$(CYTHON) -3 -o Recode.body.c $(srcdir)/Recode.pyx && \
	echo '#include "config.h"' > Recode.c && \
	cat Recode.body.c >> Recode.c
else
check-local:
	echo "The tests need cython to run!"
	exit 1
endif

clean-local:
	rm -rf __pycache__ *@pyextext@

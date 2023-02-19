# Main Makefile for Recode.
# Copyright © 1992-2018 Free Software Foundation, Inc.
# François Pinard <pinard@iro.umontreal.ca>, 1992.

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

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = libiconv doc lib src po tests contrib

EXTRA_DIST = COPYING-LIB \
m4/gnulib-cache.m4 tables.py \
keld/NomsSeulsfinal.lst keld/charsets.def keld/control.def \
keld/iso10646.def keld/other.def keld/rfc1345.txt

dist-hook:
	@if test "$(VPATH)" = ""; then \
	  cd libiconv && \
	  make dist && \
	  cd ../$(PACKAGE)-$(VERSION) && \
	  tar -z -x -f ../libiconv/libiconv-*.tar.gz --transform="s|^libiconv-[^/]*|libiconv|"; \
	else \
	  echo "I can't make a proper dist tarball in a VPATH build!"; \
	fi

release: distcheck
	git diff --exit-code && \
	git tag -a -m "Release tag" "v$(VERSION)" && \
	git push && git push --tags && \
	woger github \
		github_user=rrthomas \
		package=$(PACKAGE) \
		version=$(VERSION) \
		dist_type=tar.gz

# Ignore built files that are part of the distribution (specifically,
# src/recode.1), and the libiconv subdirectory.
distcleancheck_listfiles = \
       find . -prune libiconv -type f -exec sh -c 'test -f $(srcdir)/$$1 || echo $$1' \
	    sh '{}' ';'

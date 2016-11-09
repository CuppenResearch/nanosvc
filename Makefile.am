# Copyright (C) 2016  Roel Janssen

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

ACLOCAL_AMFLAGS         = -I m4
AUTOMAKE_OPTIONS 	= subdir-objects
SUBDIRS                 = .

AM_CFLAGS               = -Iinclude $(glib_CFLAGS) $(libinfra_CFLAGS)

nanosvc_SOURCES         = src/main.c 		\
			  src/nanosvc.c		\
			  src/segment.c 	\
			  src/read.c 		\
			  src/breakpoint.c 	\
			  src/trie.c

bin_PROGRAMS 		= nanosvc
check_PROGRAMS          = tests/cigar

nanosvc_LDFLAGS         = $(glib_LIBS) $(libinfra_LIBS)
nanosvc_LDADD           = -lm -ldl

tests_cigar_SOURCES     = tests/cigar.c src/segment.c src/nanosvc.c
tests_cigar_LDFLAGS     = $(nanosvc_LDFLAGS)
tests_cigar_LDADD       = -lm -ldl

dist_data_DATA          = LICENSE \
			  doc/nanosvc.texi \
			  doc/fdl-1.3.texi \
			  doc/version.texi

CLEAN_TARGET            = docs-clean

if ENABLE_MTRACE_OPTION
AM_CFLAGS              += -DENABLE_MTRACE
endif

# Be silent unless VERBOSE is defined.
$(VERBOSE).SILENT:

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status libtool

clean-local: $(CLEAN_TARGET)

pdf-local:
	@cd doc && texi2pdf -q nanosvc.texi -o nanosvc.pdf && \
	$(RM) *.aux *.cp *.fn *.ky *.log *.pg *.toc *.tp *.vr *.fns \
	images/*.pdf images/*.eps

info-local:
	@makeinfo doc/nanosvc.texi -o doc/nanosvc.info

docs-clean:
	-@$(RM) -rf doc/*.aux doc/*.cp doc/*.fn doc/*.ky doc/*.log doc/*.pg \
	doc/*.toc doc/*.tp doc/*.vr

.PHONY: docs-clean

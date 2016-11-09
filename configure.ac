dnl Copyright (C) 2016  Roel Janssen

dnl This program is free software: you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation, either version 3 of the License, or
dnl (at your option) any later version.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.

dnl You should have received a copy of the GNU General Public License
dnl along with this program.  If not, see <http://www.gnu.org/licenses/>.

AC_INIT([nanosvc], [0.1.0], [roel@gnu.org])
AC_CONFIG_AUX_DIR([scripts])
AC_CANONICAL_HOST
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/main.c])
AC_PROG_CC

AM_INIT_AUTOMAKE([1.9 tar-pax foreign -Wall -Werror])
AM_SILENT_RULES([yes])
AM_PROG_CC_C_O
AM_PROG_AR([ar])

# Adopt the new 'default' flags for AR to silence linker warnings.
m4_divert_text([DEFAULTS], [: "${ARFLAGS=cr} ${AR_FLAGS=cr}"])

AC_HEADER_STDC
AC_CHECK_HEADERS([stdio.h])
AC_CONFIG_FILES([Makefile])

AC_SUBST(ENABLE_MTRACE_OPTION)
AC_ARG_ENABLE([mtrace],
AC_HELP_STRING([--enable-mtrace],
[enable mtrace support @<:@default=no@:>@]),
AM_CONDITIONAL([ENABLE_MTRACE_OPTION], [true]), AM_CONDITIONAL([ENABLE_MTRACE_OPTION], [false]))

PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES([glib], [glib-2.0])
PKG_CHECK_MODULES([libinfra], [libinfra])

AC_OUTPUT

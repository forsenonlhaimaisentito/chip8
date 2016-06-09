AC_INIT([chip8], [1.0], [malu@catafratta.org])
AC_CONFIG_SRCDIR([src/main.c])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AM_MAINTAINER_MODE([enable])

AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_LEX
AC_PROG_YACC

PKG_CHECK_MODULES([sdl2], [sdl2])

AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

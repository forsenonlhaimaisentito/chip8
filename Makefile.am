bin_PROGRAMS = c8emu c8as
c8emu_SOURCES = src/main.c src/cpu.c src/util.c src/ui.c
c8as_SOURCES = src/as.c src/dis.c src/util.c src/as_gram.y src/as_lex.l
AM_CFLAGS = -Wall -Wextra -O2 @sdl2_CFLAGS@ # -DDEBUG
AM_LDFLAGS = @sdl2_LIBS@
AM_YFLAGS = -d
SUFFIXES = .l .y .h .c
CLEANFILES = src/as_gram.c src/as_lex.c src/as_gram.h

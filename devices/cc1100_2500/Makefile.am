
noinst_LIBRARIES=libcc1100.a libcc2500.a libcc1101mm.a

INCLUDES=-I$(top_srcdir)

CC1100_2500=	cc1100_2500_dev.c       cc1100_2500_dev.h         \
		cc1100_2500_globals.h   cc1100_2500_internals.h   \
		cc1100_2500_registers.c cc1100_2500_rx.c          \
		cc1100_2500_state.c     cc1100_2500_strobe.c      \
		cc1100_2500_tx.c	cc1100_2500_gdo.c         \
		cc1100_2500_fifo.c	cc1100_2500_debug.c       \
		cc1100_2500_wor.c

libcc1100_a_CFLAGS= -DCC1100
libcc1100_a_SOURCES=$(CC1100_2500)

libcc2500_a_CFLAGS= -DCC2500
libcc2500_a_SOURCES=$(CC1100_2500)

libcc1101mm_a_CFLAGS= -DCC1100 -DCC1101MM -DCC430f6137
libcc1101mm_a_SOURCES=$(CC1100_2500) cc1101mm_dev.c cc1101mm_dev.h

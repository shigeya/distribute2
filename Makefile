# $Id$
#
# Makefile for distribute
#
# Some hacks here courtesy of Brent Chapman, brent@napa.Telebit.COM
#
# Modified by: shin@u-tokyo.ac.jp, toku@dit.co.jp, shigeya@foretune.co.jp
#		and hiro@is.s.u-tokyo.ac.jp
#
# Available options:
#	-DISSUE		include X-Sequence sequence numbering
#	-DSUBJALIAS	put in subject alias like "(MailingListName 1)"
#
OPTIONS= -DISSUE -DSUBJALIAS
#
DESTDIR=
CFLAGS=	-O -g -DISSUE -DSUBJALIAS
LIBS=
MAKE=	make
WHERE=	/usr/local/lib
MANDIR=	/usr/local/man
MANSEC=	1

# C source files
SRCS=		distribute.c header.c
HDRS=		util.h
MISCSRC=	ChangeLog README distribute.1 Makefile Makefile.pmake
KITFILES=	${SRCS} ${HDRS} ${MISCSRC}

all: distribute

distribute: distribute.o header.o
	${CC} ${CFLAGS} -o distribute distribute.o header.o

install: distribute distribute.1
	install -s -o bin -g bin -m 755 distribute ${DESTDIR}${WHERE}
	install -o bin -g bin -m 444 distribute.1 \
	    ${DESTDIR}${MANDIR}/man${MANSEC}/distribute.${MANSEC}

clean:
	rm -f a.out core *.s *.o distribute

depend:
	mkdep ${CFLAGS} ${SRCS}

kit:
	shar ${KITFILES} >distribute.kit

# Makefile for distribute
# Some hacks here courtesy of Brent Chapman, brent@napa.Telebit.COM
#
DESTDIR=
CFLAGS=	-O -g
LIBS=
MAKE=	make
WHERE=	/usr/local2/lib
MANDIR=	/usr/share/local/man
MANSEC=	1

# C source files
SRCS=	distribute.c header.c

all: distribute2

distribute2: distribute.o header.o
	${CC} ${CFLAGS} -o distribute2 distribute.o header.o

install: distribute2 distribute.1
	install -s -o bin -g bin -m 755 distribute2 ${DESTDIR}${WHERE}
#	install -o bin -g bin -m 444 distribute.1 \
#	    ${DESTDIR}${MANDIR}/man${MANSEC}/distribute.${MANSEC}

clean:
	rm -f a.out core *.s *.o distribute

depend:
	mkdep ${CFLAGS} ${SRCS}

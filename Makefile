# $Id$
#
# Makefile for distribute
#
# Some hacks here courtesy of Brent Chapman, brent@napa.Telebit.COM
#
# Modified by: shin@u-tokyo.ac.jp, toku@dit.co.jp, shigeya@foretune.co.jp
#		and hiro@is.s.u-tokyo.ac.jp
#

#
# Available options:
#	-DISSUE		include X-Sequence sequence numbering
#	-DSUBJALIAS	put in subject alias like "(MailingListName 1)"
#
OPTIONS= -DISSUE -DSUBJALIAS
#

#
# DEFAULT parameters -- YOU SHOULD EDIT THESE
#
# DEF_DOMAINNAME	-- default domain name attached to list-name
# [default: undef]	   like MailingListName@My.Domain.Name
#			   Define to your domain name, or undefine it.
#			   If undefined, hostname will be attached.
#
# DEF_SEQ_PATH		-- default path to directory which holds
# [default:			   sequence number files.
#   /usr/lib/mail-list]
#
# DEF_RECIPIENT_PATH	-- default path to directory which holds
# [default:		   recipient list files.
#   /usr/lib/mail-list]
#
# DEF_SEQ_SUFFIX	-- default suffix for sequence files.
# [default: .seq]
#
# DEF_RECIPIENT_SUFFIX	-- default suffix for recipient list files.
# [default: .rec]
#

# EXAMPLE:
## DEFAULTCONFIG=\
##	-DDEF_DOMAINNAME=\"foretune.co.jp\" \
##	-DDEF_SEQ_PATH=\"/proj/proj.mail/seq\" \
##	-DDEF_RECIPIENT_PATH=\"/proj/proj.mail\"

#
DESTDIR=
#
CFLAGS=	-g ${OPTIONS} ${DEFAULTCONFIG}

LIBS=
MAKE=	make
WHERE=	/usr/local/lib
MANDIR=	/usr/local/man
MANSEC=	1

# C source files
SRCS=		distribute.c header.c
HDRS=		util.h
MISCSRC=	ChangeLog README README.FIRST NEWS \
		distribute.1 Makefile Makefile.pmake
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

# $Id$
#
# Makefile for distribute
#
# Some hacks here courtesy of Brent Chapman, brent@napa.Telebit.COM
#
# Modified by: shin@u-tokyo.ac.jp, toku@dit.co.jp, shigeya@foretune.co.jp
#		and hiro@is.s.u-tokyo.ac.jp
#
RCONFIG=-DRELEASESTATE=\"Alpha\"

#
# Available options:
#	-DSYSLOG		include syslog based logging
#	-DISSUE			include X-Sequence sequence numbering
#	-DSUBJALIAS		put in subject alias like "(MailingListName 1)"
#	-DADDVERSION		Add X-Distribute: Version SOMETHING to header
#	-DDEBUGLOG		Debug level logging (in /tmp/distribute.log)
# 	-DSYSLOG_FACILITY=n	-- value for syslogd's facility value
#				   defaulted to LOG_LOCAL4
#
OPTIONS= -DSYSLOG -DISSUE -DSUBJALIAS -DADDVERSION -DSYSLOG_FACILITY=LOG_LOCAL4
#

#
# DEFAULT parameters -- YOU SHOULD EDIT THESE
#
# DEF_DOMAINNAME	-- default domain name attached to list-name
# [default: undef]	   like MailingListName@My.Domain.Name
#			   Define to your domain name, or undefine it.
#			   If undefined, hostname by gethostname or
#			   hostname given by -h will be attached.
#
# DEF_SEQ_PATH		-- default path to directory which holds
# [default:			   sequence number files.
#   /usr/lib/mail-list]
#
# DEF_RECIPIENT_PATH	-- default path to directory which holds
# [default:		   recipient list files.
#   /usr/lib/mail-list]
#
# DEF_MAJORDOMO_RECIPIENT_PATH-- default path to majordomo "lists" directory
# [default:		   recipient file in majordomo style.
#   /usr/lib/mail-list/majordomo/lists]
#
# DEF_SEQ_SUFFIX	-- default suffix for sequence files.
# [default: .seq]
#
# DEF_RECIPIENT_SUFFIX	-- default suffix for recipient list files.
# [default: .rec]
#
# DEF_ALIAS_CHAR_OPTION	-- Define alias char by -B option value.
# [default: NONE]	   (ex: DEF_ALIAS_CHAR_OPTION=\"b\" for brace)
#			   "b" for brace, "c" for curly brace, "p" for paren
#			   or "<>" to specify both open/close char.
#

# CONFIG EXAMPLE:
##DEFAULTCONFIG=\
##	-DDEF_DOMAINNAME=\"foretune.co.jp\" \
##	-DDEF_ALIAS_CHAR_OPTION=\"b\"

#
DESTDIR=
#
CFLAGS=	-g ${OPTIONS} ${DEFAULTCONFIG} ${RCONFIG}

LIBS=
MAKE=	make
WHERE=	/usr/lib
MANDIR=	/usr/man
MANSEC=	1

# Install as
OWNER=	daemon
GROUP=	daemon

# C source files
SRCS=		distribute.c header.c longstr.c
HDRS=		util.h longstr.h cdefs.h patchlevel.h
MISCSRC=	ChangeLog README README.FIRST NEWS \
		distribute.1 Makefile Makefile.pmake \
		resultcheck.pl

KITFILES=	${SRCS} ${HDRS} ${MISCSRC}

OBJS=		distribute.o header.o longstr.o parserecipfile.o

all: distribute

distribute: ${OBJS}
	${CC} ${CFLAGS} -o distribute ${OBJS}

install: distribute distribute.1
	install -s -o ${OWNER} -g ${GROUP} -m 511 distribute \
		${DESTDIR}${WHERE}
	install -m 444 distribute.1 \
		${DESTDIR}${MANDIR}/man${MANSEC}/distribute.${MANSEC}

testinst: distribute
	install -s -o root -g wheel -m 4755 distribute bin

clean:
	rm -f a.out core *.s *.o distribute

depend:
	mkdep ${CFLAGS} ${SRCS}

kit:
	shar ${KITFILES} >distribute.kit

# test suite
test.x:	testlongstr.o longstr.o
	cc -g -o test.x testlongstr.o longstr.o /usr/lib/debug/malloc.o

test:	test.x
	./test.x | perl resultcheck.pl

prtest: parserecipfile.c
	cc -DTEST -g -o prtest parserecipfile.c longstr.o

###
distribute.o:	distribute.c Makefile patchlevel.h longstr.h config.h
header.o:	header.c config.h
longstr.o:	longstr.c longstr.h

# Makefile,v 1.29 1994/08/05 11:49:26 shigeya Exp
#
# Makefile for distribute
#
# See CREDITS for credit and COPYRIGHT for copyright notice.
#
RCONFIG=-DRELEASESTATE=\"Alpha\"

#
# Configuration options:
#
# Available options:
#	-DSYSLOG		include syslog based logging
#	-DISSUE			include X-Sequence sequence numbering
#	-DSUBJALIAS		put in subject alias like "(MailingListName 1)"
#	-DADDVERSION		Add X-Distribute: Version SOMETHING to header
#	-DDEBUGLOG		Debug level logging (in /tmp/distribute.log)
# 	-DSYSLOG_FACILITY=n	-- value for syslogd's facility value
#				   defaulted to LOG_LOCAL4
#	-DUSESUID		include SUID support code
#	-DSTRSTR_MISSING	strstr() missing (Ex: NEWSOS 4.x)
#	-DMSC			MSC Style Subject
#
OPTIONS= -DSYSLOG -DISSUE -DSUBJALIAS -DADDVERSION \
	-DSYSLOG_FACILITY=LOG_LOCAL4 -DCCMAIL
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
PERL=	perl
WHERE=	/usr/lib
MANDIR=	/usr/man
MANSEC=	1

# Install as
OWNER=	daemon
GROUP=	daemon

# C source files
OTHERSRC=	header.c history.c logging.c \
		longstr.c memory.c message.c recipfile.c pathutil.c \
		strutil.c

DISTSRCS= distribute.c ${OTHERSRC}

ARCHIVESRCS= archive.c ${OTHERSRC}

SRCS= archive.c distribute.c header.c history.c logging.c \
		longstr.c memory.c message.c pathutil.c recipfile.c \
		strutil.c

TEST=	testlongstr.c

HDRS= cdefs.h config.h debuglog.h header.h history.h longstr.h \
		memory.h message.h mestab.h patchlevel.h pathutil.h \
		recipfile.h strutil.h util.h
MISCSRC=	ChangeLog README README.FIRST NEWS \
		distribute.1 Makefile Makefile.pmake \
		resultcheck.pl tmpl2c.pl message.tmpl
KITFILES=	${SRCS} ${HDRS} ${MISCSRC}

LIBOBJS=	memory.o history.o pathutil.o strutil.o header.o logging.o \
		longstr.o

DOBJS=		distribute.o recipfile.o message.o ${LIBOBJS}

AOBJS=		archive.o ${LIBOBJS}


all: xdistribute xarchive

xdistribute: ${DOBJS}
	${CC} ${CFLAGS} -o xdistribute ${DOBJS}
	@size xdistribute

xarchive: ${AOBJS}
	${CC} ${CFLAGS} -o xarchive ${AOBJS}
	@size xarchive

install: xdistribute distribute.1 xarchive
	install -c -s -o ${OWNER} -g ${GROUP} -m 511 xdistribute \
		${DESTDIR}${WHERE}/distribute
	install -c -s -o ${OWNER} -g ${GROUP} -m 511 xarchive \
		${DESTDIR}${WHERE}/archive
	install -c -m 444 distribute.1 \
		${DESTDIR}${MANDIR}/man${MANSEC}/distribute.${MANSEC}

testinst: xdistribute xarchive
	install -c -s -o root -g wheel -m 511 distribute test
	install -c -s -o root -g wheel -m 511 archive test

clean:
	rm -f a.out *.core core ${DOBJS} ${AOBJS} xdistribute xarchive mestab.h

depend: mestab.h
	mkmf -CSXW

kit:
	shar ${KITFILES} >distribute.kit

# test suite
test.x:	testlongstr.o longstr.o
	cc -g -o test.x testlongstr.o longstr.o

test:	test.x
	./test.x | ${PERL} resultcheck.pl

lint: distribute.lint archive.lint
distribute.lint: ${DISTSRCS} Makefile
	-lint ${OPTIONS} ${DISTSRCS} >$@
archive.lint: ${ARCHIVESRCS} Makefile
	-lint ${OPTIONS} ${DISTSRCS} >$@

prtest: recipfile.c
	cc -DTEST -g -o prtest recipfile.c longstr.o

htest: history.c memory.o logging.o history.h
	cc -DTEST -g -o htest history.c memory.o logging.o

utest: uidlib.c
	cc -DTEST -g -o utest uidlib.c

mestab.h: message.tmpl tmpl2c.pl
	${PERL} tmpl2c.pl <message.tmpl >mestab.h

###
archive.o: archive.c patchlevel.h config.h memory.h cdefs.h history.h \
	pathutil.h header.h
distribute.o: distribute.c patchlevel.h config.h longstr.h cdefs.h memory.h \
	message.h recipfile.h mestab.h strutil.h pathutil.h \
	history.h header.h
header.o: header.c util.h config.h memory.h cdefs.h
history.o: history.c memory.h cdefs.h history.h
logging.o: logging.c
longstr.o: longstr.c longstr.h cdefs.h
memory.o: memory.c
message.o: message.c message.h cdefs.h longstr.h
pathutil.o: pathutil.c config.h memory.h cdefs.h pathutil.h
recipfile.o: recipfile.c longstr.h cdefs.h
strutil.o: strutil.c

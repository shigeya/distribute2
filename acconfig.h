/* @(#)acconfig.h	8.18 (Berkeley) 7/2/96 */

/* Define to `int' if <sys/types.h> doesn't define.  */
#undef ssize_t

/* Define if you want a debugging version. */
#undef DEBUG

/* Define if you have a System V-style (broken) gettimeofday. */
#undef HAVE_BROKEN_GETTIMEOFDAY

/* Define if you have a Ultrix-style (broken) vdisable. */
#undef HAVE_BROKEN_VDISABLE

/* Define if you have a BSD version of curses. */
#undef HAVE_BSD_CURSES

/* Define if you have the curses(3) addnstr function. */
#undef HAVE_CURSES_ADDNSTR

/* Define if you have the curses(3) beep function. */
#undef HAVE_CURSES_BEEP

/* Define if you have the curses(3) flash function. */
#undef HAVE_CURSES_FLASH

/* Define if you have the curses(3) idlok function. */
#undef HAVE_CURSES_IDLOK

/* Define if you have the curses(3) keypad function. */
#undef HAVE_CURSES_KEYPAD

/* Define if you have the curses(3) newterm function. */
#undef HAVE_CURSES_NEWTERM

/* Define if you have the curses(3) setupterm function. */
#undef HAVE_CURSES_SETUPTERM

/* Define if you have the curses(3) tigetstr/tigetnum functions. */
#undef HAVE_CURSES_TIGETSTR

/* Define if you have the DB __hash_open call in the C library. */
#undef HAVE_DB_HASH_OPEN

/* Define if you have the chsize(2) system call. */
#undef HAVE_FTRUNCATE_CHSIZE

/* Define if you have the ftruncate(2) system call. */
#undef HAVE_FTRUNCATE_FTRUNCATE

/* Define if you have fcntl(2) style locking. */
#undef HAVE_LOCK_FCNTL

/* Define if you have flock(2) style locking. */
#undef HAVE_LOCK_FLOCK

/* Define if you want to compile in the Perl interpreter. */
#undef HAVE_PERL_INTERP

/* Define if your Perl is at least 5.003_01. */
#undef HAVE_PERL_5_003_01

/* Define if you have the Berkeley style revoke(2) system call. */
#undef HAVE_REVOKE

/* Define if you have the Berkeley style strsep(3) function. */
#undef HAVE_STRSEP

/* Define if you have <sys/mman.h> */
#undef HAVE_SYS_MMAN_H

/* Define if you have <sys/select.h> */
#undef HAVE_SYS_SELECT_H

/* Define if you have the System V style pty calls. */
#undef HAVE_SYS5_PTY

/* Define if you want to compile in the Tcl interpreter. */
#undef HAVE_TCL_INTERP

/* Define if your sprintf returns a pointer, not a length. */
#undef SPRINTF_RET_CHARPNT

/* Define if use cc:Mail bounce reject code */
#undef USE_CCMAIL

/* Define if use syslog for logging */
#undef USE_SYSLOG

/* Define if use syslog for logging */
#undef SYSLOG_FACILITY

/* Define if you have TEE */
#undef TEE_COMMAND

/* Define if do an extensive debug logging */
#undef DEBUGLOG

/* we assume subject will no longer than this length */
#define	MAXSUBJLEN	4096

/* we assume message id is no longer than this */
#define	MAXMESSAGEIDLEN	512

/* number of headers allowed */
#define	MAXHEADERLINE	1000

/* The maximum length of a header line that we can handle, including all continuation lines.  */
#define MAXHEADERLEN	16384

/* The maximum length of a address. */
#define MAXADDRLEN	1024

/* Define it to point to sendmail. */
#undef _PATH_SENDMAIL

#define DEF_SEQ_PATH		"/usr/lib/mail-list"

#define DEF_RECIPIENT_PATH	"/usr/lib/mail-list"

#define DEF_ARCHIVE_PATH       "/usr/spool/mail-list"

#define TEE_COMMAND		"/usr/bin/tee"

#define DEF_MAJORDOMO_RECIPIENT_PATH "/usr/lib/mail-list/majordomo/lists"

#define DEF_SEQ_SUFFIX		".seq"

#define DEF_RECIPIENT_SUFFIX	".rec"

#define DEF_ACCEPT_SUFFIX	".acc"

#define DEF_REJECT_SUFFIX	".rej"

#define DEF_INDEX_NAME		"INDEX"

#define DEF_OPENALIAS_CHAR	'('

#define DEF_CLOSEALIAS_CHAR	')'

#define ALIAS_SPACE_CHAR	'-'

/* Define if include X-Sequence sequence numbering (use with SUBJALIAS) */
#undef USE_ISSUE

/* Define if put in subject alias like "(MailingListName 1)" (use with ISSUE) */
#undef USE_SUBJALIAS

#undef DEF_DOMAINNAME

#undef USE_MSC

#undef USE_MIMEKIT

#undef USE_SYS_PARAM_H

#undef USE_NETDB_H

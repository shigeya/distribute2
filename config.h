/* $Id$
 *
 *	Distribute configuration information
 */


/* Default configuration check and define if not defined
 */

#ifndef DEF_SEQ_PATH
# define DEF_SEQ_PATH		"/usr/lib/mail-list"
#endif

#ifndef DEF_RECIPIENT_PATH
# define DEF_RECIPIENT_PATH	"/usr/lib/mail-list"
#endif

#ifndef DEF_ARCHIVE_PATH
# define DEF_ARCHIVE_PATH       "/usr/spool/mail-list"
#endif

#ifndef TEE_COMMAND
# define TEE_COMMAND		"/usr/bin/tee"
#endif

#ifndef DEF_MAJORDOMO_RECIPIENT_PATH
# define DEF_MAJORDOMO_RECIPIENT_PATH "/usr/lib/mail-list/majordomo/lists"
#endif

#ifndef DEF_SEQ_SUFFIX
# define DEF_SEQ_SUFFIX		".seq"
#endif

#ifndef DEF_RECIPIENT_SUFFIX
# define DEF_RECIPIENT_SUFFIX	".rec"
#endif

#ifndef DEF_ACCEPT_SUFFIX
# define DEF_ACCEPT_SUFFIX	".acc"
#endif

#ifndef DEF_INDEX_NAME
# define DEF_INDEX_NAME		"INDEX"
#endif

#ifdef MSC
# define DEF_OPENALIAS_CHAR	'['
# define DEF_CLOSEALIAS_CHAR	']'
#endif

#ifndef DEF_OPENALIAS_CHAR
# define DEF_OPENALIAS_CHAR	'('
#endif
#ifndef DEF_CLOSEALIAS_CHAR
# define DEF_CLOSEALIAS_CHAR	')'
#endif

#ifndef	ALIAS_SPACE_CHAR
# define ALIAS_SPACE_CHAR	'-'
#endif

#ifndef SYSLOG_FACILITY
# define SYSLOG_FACILITY	LOG_LOCAL4
#endif


/*
 * LIMITS
 */

#define	MAXSUBJLEN	4096	/* we assume subject will no
				 * longer than this length */

#define	MAXMESSAGEIDLEN	512	/* we assume message id is no longer than this
				 */

#define	MAXHEADERLINE	1000	/* number of headers allowed */


#define MAXHEADERLEN	16384	/* The maximum length of a header line
				 * that we can handle, including all
				 * continuation lines.  */
#define MAXADDRLEN	1024	/* The maximum length of a address. */

/*
 * Other set-ups
 */

#ifndef _PATH_SENDMAIL
# define	_PATH_SENDMAIL	"/usr/lib/sendmail" /* default */
#endif

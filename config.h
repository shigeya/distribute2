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

#ifndef DEF_MAJORDOMO_RECIPIENT_PATH
# define DEF_MAJORDOMO_RECIPIENT_PATH "/usr/lib/mail-list/majordomo/lists"
#endif

#ifndef DEF_SEQ_SUFFIX
# define DEF_SEQ_SUFFIX		".seq"
#endif

#ifndef DEF_RECIPIENT_SUFFIX
# define DEF_RECIPIENT_SUFFIX	".rec"
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

#ifndef SYSLOG_FACILITY
# define SYSLOG_FACILITY	LOG_LOCAL4
#endif


/*
 * LIMITS
 */

#define	MAXSUBJLEN	4096	/* we assume subject will no
				 * longer than this length */

#define	MAXHEADERLINE	1000	/* number of headers allowed */


#define MAXHEADERLEN	16384	/* The maximum length of a header line
				 * that we can handle, including all
				 * continuation lines.  */
/*
 * Other set-ups
 */

#ifndef _PATH_SENDMAIL
# define	_PATH_SENDMAIL	"/usr/lib/sendmail" /* default */
#endif

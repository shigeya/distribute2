/* $Id$
 *
 * Logging and error handling for distribute+archive
 *
 * Shigeya Suzuki, Dec 1993, October 1997
 * Copyright(c)1993-1997 Shigeya Suzuki
 */

#include <config.h>

#include <stdio.h>
#include <sysexits.h>
#include <stdlib.h>

#ifdef	DEBUGLOG
#include <sys/types.h>
#include <unistd.h>
#endif

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#include "cdefs.h"

#ifdef USE_SYSLOG
# ifdef HAVE_SYSLOG_H
#  include <syslog.h>
# endif
#else
# define   LOG_ERR	1
# define   LOG_WARNING	2
# define   LOG_INFO	3
#endif

#define LOGBUFSIZE	1024	/* for safety */

#include "logging.h"
#include "memory.h"

/* Global
 */
static int print_error = 0;
static char* logbuf = NULL;
static int logbuf_size = LOGBUFSIZE;

/* Initialization
 */
void
init_log(tag)
    char* tag;
{
#ifdef DEBUGLOG
    char logfile[80];
    extern FILE *debuglog;
#endif

#ifdef USE_SYSLOG	
    openlog(tag, LOG_PID, SYSLOG_FACILITY);
#endif
#ifdef DEBUGLOG
    sprintf(logfile, "/tmp/%s.log", tag);
    debuglog = fopen(logfile, "a");
    if (debuglog == NULL) {
	logandexit(EX_UNAVAILABLE, "can't open debug log", logfile);
    }
    fprintf(debuglog, "---\n");
    fprintf(debuglog, "invoked: pid=%ld\n", (long)getpid());
    fflush(debuglog);
#endif
    logbuf = xmalloc(LOGBUFSIZE);
    logbuf_size = LOGBUFSIZE;
}


/* Toggle print error mode
 */
void
logging_setprinterror(flag)
    int flag;
{
    print_error = flag;
}


/* Several error handler
 */

void
logerror_buf(pri, prefix)
    int pri;
    char* prefix;
{
    if (print_error) {
	if (prefix != NULL)
	    fputs(prefix, stderr);
	fputs(logbuf, stderr);
	fputc('\n', stderr);
    }
#ifdef LOGDEBUG
    {
	FILE* fp = fopen("/tmp/distribute.log", "a");
	if (fp != NULL) {
	    if (prefix != NULL)
		fputs(prefix, fp);
	    fputs(logbuf, fp);
	    fputc('\n', fp);
	    fclose(fp);
	}
    }
#endif
#ifdef USE_SYSLOG
    syslog(pri, logbuf);
#endif
}


void
#ifdef __STDC__
logerror(char* fmt, ...)
#else
logerror(fmt, va_alist)
	char *fmt;
	va_dcl
#endif
{
    va_list ap;
    va_start(ap, fmt);
#ifdef HAVE_VSNPRINTF
    vsnprintf(logbuf, logbuf_size, fmt, ap);
#else
    vsprintf(logbuf, fmt, ap);
#endif
    va_end(ap);
    logerror_buf(LOG_ERR, "Error: ");
}


void
#ifdef __STDC__
logwarn(char* fmt, ...)
#else
logwarn(fmt, va_alist)
	char *fmt;
	va_dcl
#endif
{
    va_list ap;
    va_start(ap, fmt);
#ifdef HAVE_VSNPRINTF
    vsnprintf(logbuf, logbuf_size, fmt, ap);
#else
    vsprintf(logbuf, fmt, ap);
#endif
    va_end(ap);
    logerror_buf(LOG_WARNING, "Warning: ");
}


void
#ifdef __STDC__
loginfo(char* fmt, ...)
#else
loginfo(fmt, va_alist)
	char *fmt;
	va_dcl
#endif
{
    va_list ap;
    va_start(ap, fmt);
#ifdef HAVE_VSNPRINTF
    vsnprintf(logbuf, logbuf_size, fmt, ap);
#else
    vsprintf(logbuf, fmt, ap);
#endif
    va_end(ap);
    logerror_buf(LOG_INFO, "Info: ");
}


/* log it then exit with error code
 */

void
#ifdef __STDC__
logandexit(int exitcode, char* fmt, ...)
#else
logandexit(exitcode, fmt, va_alist)
	int exitcode;
	char *fmt;
	va_dcl
#endif
{
    va_list ap;
    va_start(ap, fmt);
#ifdef HAVE_VSNPRINTF
    vsnprintf(logbuf, logbuf_size, fmt, ap);
#else
    vsprintf(logbuf, fmt, ap);
#endif
    va_end(ap);

    logerror_buf(NULL);

    exit(exitcode);
}




/* Abort with program error
 */
void
programerror()
{
    logandexit(EX_UNAVAILABLE, "program error");
}


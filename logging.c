/* $Id$
 *
 * Logging and error handling for distribute+archive
 *
 * Shigeya Suzuki, Dec 1993, October 1997
 * Copyright(c)1993-1997 Shigeya Suzuki
 */

#include <stdio.h>
#include <sysexits.h>
#include <stdlib.h>

#ifdef	DEBUGLOG
#include <sys/types.h>
#include <unistd.h>
#endif

#ifdef __STDC__
#include <stdarg.h>
#endif

#include "cdefs.h"

#ifdef TEST
#undef SYSLOG
#endif

#ifdef SYSLOG
# include <syslog.h>
#endif

#define LOGBUFSIZE	1024

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

#ifdef SYSLOG	
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
#ifdef SYSLOG
    syslog(pri, logbuf);
#endif
}


#ifdef __STDC__

void
logerror(char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    /* vsnprintf is available everywhere?? -- may be.*/
    vsnprintf(logbuf, logbuf_size, fmt, ap);
    va_end(ap);
    logerror_buf(LOG_ERR, "Error: ");
}

#else

void
logerror(fmt, a1, a2)
    char* fmt;
    char* a1;
    char* a2;
{
    sprintf(logbuf, fmt, a1, a2); /* they may not have snprintf also.. */
    logerror_buf(LOG_ERR, "Error: ");
}

#endif


#ifdef __STDC__

void
logwarn(char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(logbuf, logbuf_size, fmt, ap);
    va_end(ap);
    logerror_buf(LOG_WARNING, "Warning: ");
}

#else

void
logwarn(fmt, a1, a2)
    char* fmt;
    char* a1;
    char* a2;
{
    sprintf(logbuf, fmt, a1, a2);
    logerror_buf(LOG_WARNING, "Warning: ");
}

#endif


#ifdef __STDC__

void
loginfo(char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(logbuf, logbuf_size, fmt, ap);
    va_end(ap);
    logerror_buf(LOG_INFO, "Info: ");
}

#else

void loginfo(fmt, a1, a2)
    char* fmt;
    char* a1;
    char* a2;
{
    sprintf(logbuf, fmt, a1, a2);
    logerror_buf(LOG_INFO, "Info: ");
}

#endif


/* log it then exit with error code
 */

#ifdef __STDC__
void
logandexit(int exitcode, char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsprintf(logbuf, fmt, ap);	/* vsnprintf is available everywhere?? */
    va_end(ap);

    logerror_buf(NULL);

    exit(exitcode);
}

#else

void
logandexit(exitcode, fmt, a1, a2)
    int exitcode;
    char* fmt;
    char* a1;
    char* a2;
{
    sprintf(logbuf, fmt, a1, a2);
    logerror_buf(NULL);
    exit(exitcode);
}

#endif




/* Abort with program error
 */
void
programerror()
{
    logandexit(EX_UNAVAILABLE, "program error");
}


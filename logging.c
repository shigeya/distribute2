/* $Id$
 *
 * Logging and error handling for distribute+archive
 *
 * Shigeya Suzuki, Dec 1993
 * Copyright(c)1993 Shigeya Suzuki
 */

#include <stdio.h>
#include <sysexits.h>

#ifdef TEST
#undef SYSLOG
#endif

#ifdef SYSLOG
# include <syslog.h>
#endif

/* We don't make prototype for this.
 * because we don't want to rely on ANSI C at this moment..
 */

/* Global
 */
static int print_error = 0;


/* Initialization
 */
init_log(tag)
    char *tag;
{
#ifdef SYSLOG	
    openlog(tag, LOG_PID, SYSLOG_FACILITY);
#endif
#ifdef DEBUGLOG
    char logfile[80];
    sprintf(logfile, "/tmp/%s.log", tag);
    debuglog = fopen(, "a");
    if (debuglog == NULL) {
	logandexit(EX_UNAVAILABLE, "can't open debug log", logfile);
    }
    fprintf(debuglog, "---\n");
    fprintf(debuglog, "invoked: pid=%d\n", getpid());
    fflush(debuglog);
#endif
}


/* Toggle print error mode
 */
logging_setprinterror(flag)
    int flag;
{
    print_error = flag;
}


/* Several error handler
 */
/*VARARGS1*/
logerror(fmt, a1, a2)
    char *fmt;
    char *a1, *a2;
{
#ifdef SYSLOG
    syslog(LOG_ERR, fmt, a1, a2);
#endif
    if (print_error) {
	fputs("Error: ", stderr);
	fprintf(stderr, fmt, a1, a2);
	fputc('\n', stderr);
    }
}


/*VARARGS1*/
logwarn(fmt, a1, a2)
    char *fmt;
    char *a1, *a2;
{
#ifdef SYSLOG
    syslog(LOG_WARNING, fmt, a1, a2);
#endif
    if (print_error) {
	fputs("Warning: ", stderr);
	fprintf(stderr, fmt, a1, a2);
	fputc('\n', stderr);
    }
}

/*VARARGS1*/
loginfo(fmt, a1, a2)
    char *fmt;
    char *a1, *a2;
{
#ifdef SYSLOG
    syslog(LOG_INFO, fmt, a1, a2);
#endif
    if (print_error) {
	fputs("Info: ", stderr);
	fprintf(stderr, fmt, a1, a2);
	fputc('\n', stderr);
    }
}



/* Abort with program error
 */
programerror()
{
    logandexit(EX_UNAVAILABLE, "program error");
}


/* log it then exit with error code
 */
/*VARARGS2*/
logandexit(exitcode, fmt, a1, a2)
    int exitcode;
    char *fmt;
    char *a1, *a2;
{
    logerror(fmt, a1, a2);
    exit(exitcode);
}

/* $Id$
 *
 * parse recipient file, and constract recipient address buffer
 *
 * by Shigeya Suzuki
 * Copyright(c)1993, Shigeya Suzuki
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sysexits.h>
#include <errno.h>
#include <malloc.h>

#include "longstr.h"

/* external */
char *index();
char *rindex();

void logandexit();
void logwarn();

char *
parserecipfile(filename)
    char *filename;
{
    FILE *recipf;
    char buf[1024];
    struct longstr recipbuf;

    ls_init(&recipbuf);
    ls_reset(&recipbuf);

    if ((recipf = fopen(filename, "r")) == NULL) {
	logandexit(EX_NOINPUT,
		   "can't open receipt address file '%s'\n", filename);
    }
    
    while (fgets(buf, sizeof buf, recipf) != NULL) {
	char *p, *s, *beginp, *namep;

	namep = NULL;
	/* skip comments */
	if ((p = index(buf, '\n')) != NULL)
	    *p = '\0';
	if (buf[0] == '\0' || buf[0] == '#')
	    continue;

	/* has address part -- easy
	 */	
	if ((p = index(buf, '<')) != NULL) {
	    beginp = ++p;
	    if ((p = index(beginp, '>')) != NULL) {
		*p = NULL;
	    }
	    else {
		logwarn("angle mismatch: %s.\n", buf);
		continue;
	    }
	    namep = beginp;
	}
	else {
	    for (p = buf; *p; p++) {
restart:
		if (*p == '\\' && *++p != '\0') /* ignore quoting */
		    continue;
		if (*p == '(') { /* found parenses.. */
		    beginp = p++;
		    while (*p && ! ( *p == ')' && p[-1] != '\\'))
			p++;
		    if (*p != ')') {
			logwarn("paren mismatch: %s.\n", buf);
			goto nextline;
		    }
		    strcpy(beginp, ++p); /* skip these parens.. */
		    p = beginp;
		    goto restart;
		}
		if (*p == '"') { /* double-quote, just skip */
		    p++;
		    while (*p && !(*p == '"' && p[-1] != '\\'))
			p++;
		    if (*p != '"') {
			logwarn("doublequote mismatch: %s.\n", buf);
			goto nextline;
		    }
		}
	    }
	    namep = buf;
	}

	if (namep != NULL) {
	    ls_appendstr(&recipbuf, " ");
	    ls_appendstr(&recipbuf, namep);
	}
nextline:;
    }

    return recipbuf.ls_buf;
}

#ifdef TEST
main(ac, av)
    int ac;
    char **av;
{
    char * p = parserecipfile(av[1]);
    puts(p);
}

void
logandexit(exitcode, fmt, a1, a2)
    int exitcode;
    char *fmt;
    char *a1, *a2;
{
    fprintf(stderr, "[EXITCODE=%d] ", exitcode);
    fprintf(stderr, fmt, a1, a2);
    exit(exitcode);
}

void
logwarn(fmt, a1, a2)
    char *fmt;
    char *a1, *a2;
{
    fprintf(stderr, fmt, a1, a2);
}

/* allocate or fail
 */
void*
xmalloc(len)
    size_t len;
{
    void *p = malloc(len);
    if (p == NULL) {
	fprintf(stderr, "Can't allocate buf. size=%d\n", len);
	exit(1);
    }
    return p;
}

/* reallocate or fail
 */
void*
xrealloc(p, len)
    void *p;
    size_t len;
{
    void *np;

    if (p == NULL || p <= 0) {
	fprintf(stderr, "Invalid pointer for realloc (0x%x)\n", p);
	exit(1);
    }

    np = realloc(p,len);

    if (np == NULL) {
	fprintf(stderr, "Can't reallocate buf. size=%d (%d)\n", len, errno);
	exit(1);
    }
    return np;
}
#endif

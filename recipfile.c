/* recipfile.c,v 1.2 1994/08/05 11:49:32 shigeya Exp
 *
 * parse recipient file, and construct recipient address buffer
 *
 * by Shigeya Suzuki, April 1993
 * Copyright(c)1993, Shigeya Suzuki
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sysexits.h>
#include <errno.h>

#if __STDC__
# include <stdlib.h>
#else
# include <malloc.h>
#endif

#include "longstr.h"

/* external */
char *index();
char *rindex();

extern logandexit();
extern logwarn();


/* normalizeaddr -- check one address and normalize it if necessary.
 */

char *
normalizeaddr(buf)
    char *buf;
{
    char *p, *beginp, *namep;
    
    namep = NULL;
    /* skip comments */
    if ((p = index(buf, '\n')) != NULL)
	*p = '\0';
    if (buf[0] == '\0' || buf[0] == '#') /* ignore comment or null line */
	return NULL;		/* is blank line */
    if ((p = index(buf, '#')) != NULL) /* remove trailing comments */
	*p = '\0';
    
    /* has address part -- easy
     */	
    if ((p = index(buf, '<')) != NULL) {
	beginp = ++p;
	if ((p = index(beginp, '>')) != NULL) {
	    *p = NULL;
	}
	else {
	    logwarn("angle mismatch: %s.\n", buf);
	    return NULL;
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
		    return NULL;
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
		    return NULL;
		}
	    }
	}
	namep = buf;
    }
    return namep;
}

char *
parserecipfile(filename, errormode)
    char *filename;
    int errormode;
{
    FILE *recipf;
    char buf[1024];
    struct longstr recipbuf;
    int first = 1;
    
    if ((recipf = fopen(filename, "r")) == NULL) {
	if (errormode == 1)
	    logandexit(EX_NOINPUT,
		       "can't open receipt address file '%s'\n", filename);
	else
	    return NULL;
    }
    
    ls_init(&recipbuf);
    ls_reset(&recipbuf);

    while (fgets(buf, sizeof buf, recipf) != NULL) {
	char * namep = normalizeaddr(buf);

	if (namep != NULL) {
	    if (!first) {
		ls_appendstr(&recipbuf, " ");
	    }
	    else {
		first = 0;
	    }
	    ls_appendstr(&recipbuf, namep);
	}
    }

#ifdef DEBUGLOG
    {extern FILE* debuglog;
     fprintf(debuglog, "paraserecipfile: %s\n", recipbuf.ls_buf);
    }
#endif
    return recipbuf.ls_buf;
}




#ifdef TEST
main(ac, av)
    int ac;
    char **av;
{
    char * p = parserecipfile(av[1]);
    puts(p);

    puts("\noneliine test:\n");
    test("Shigeya Suzuki <shigeya@foretune.co.jp>");
    test("shigeya@foretune.co.jp (Shigeya Suzuki)");
    test("Shigeya \"too busy\" Suzuki <shigeya@foretune.co.jp>");
    test("\"Shigeya 'too busy' Suzuki\" <shigeya@foretune.co.jp>");
    test("shigeya@foretune.co.jp (Shigeya \"too busy\" Suzuki)");
    test("shigeya@foretune.co.jp (Shigeya 'too busy' Suzuki)");
    test("\":users 1\"@foretune.co.jp");
    test("=?ISO-2022-JP?B?GyRCTmtMWkxQOkgbKEI=?= <shigeya@foretune.co.jp>");
}

test(s1)
    char *s1;
{
    char buf[1024];
    strcpy(buf, s1);
    printf ("target = %s, result='%s'\n", s1, normalizeaddr(buf));
}

logandexit(exitcode, fmt, a1, a2)
    int exitcode;
    char *fmt;
    char *a1, *a2;
{
    fprintf(stderr, "[EXITCODE=%d] ", exitcode);
    fprintf(stderr, fmt, a1, a2);
    exit(exitcode);
}

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

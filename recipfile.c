/* $Id$
 *
 * parse recipient file, and construct recipient address buffer
 *
 * by Shigeya Suzuki, April 1993
 * Copyright(c)1993, Shigeya Suzuki
 *
 */

#if defined(__svr4__) || defined(nec_ews_svr4) || defined(_nec_ews_svr4)
#undef SVR4
#define SVR4
#endif

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

#include "config.h"
#include "longstr.h"
#include "memory.h"

/* external */
#ifdef SVR4
char *strchr();
char *strrchr();
#define	index	strchr
#define	rindex	strrchr
#else
char *index();
char *rindex();
#endif

extern logandexit();
extern logwarn();


/* normalizeaddr -- check one address and normalize it if necessary.
 */

char *
normalizeaddr(buf)
    char *buf;
{
    char* xp;
    char* rp;
    char* beginp;
    char* endp;
    char* namep = NULL;
    char* nbufp = strsave(buf);	/* this buffer will not free'ed.. */
    char* p;

    for (p = nbufp; ; p++) {
      restart:
	if (*p == '\0')
	    break;

	if (*p == '\\' && *++p != '\0') /* ignore quoting */
	    continue;

	if (*p == '<') { /* found non-comment angle bracket */
	    beginp = ++p;
	    if ((p = index(beginp, '>')) != NULL) {
		*p = '\0';
	    }
	    else {
		logwarn("angle mismatch: %s.\n", buf);
		return NULL;
	    }
	    return beginp;
	}
	
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

    rp = rindex(buf, '\0');
    if (rp != NULL) {
	while (rp > buf && *--rp == ' ') /* remove trailing space */
	    *rp = '\0';
    }

    return nbufp;
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
	char *p;
	if ((p = index(buf, '\n')) != NULL)/* remove trailing LF */
	    *p = '\0';
	if ((p = index(buf, '#')) != NULL) /* remove comments, if exists */
	    *p = '\0';

	if (buf[0] != '\0') {	/* if it is *NOT* newline.. */
	    char * namep = normalizeaddr(buf);
	    if (namep != NULL) {
		if (!first) {
		    ls_appendstr(&recipbuf, " ");
		}
		else {
		    first = 0;
		}

		ls_appendstr(&recipbuf, "'"); /* cheat dirty hack */
		ls_appendstr(&recipbuf, namep);
		ls_appendstr(&recipbuf, "'");
	    }
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
#if 0
    if (ac != 1) {
	char * p = parserecipfile(av[1]);
	puts("\noneliine test:\n");
	puts(p);
    }
#endif    
    test("   \"\033$B>H4nL>\033(J \033$BD+CK\033(J\" <jl@foretune.co.jp>");
    test("Shigeya Suzuki <shigeya@foretune.co.jp>");
    test("shigeya@foretune.co.jp (Shigeya Suzuki)");
    test("Shigeya \"too busy\" Suzuki <shigeya@foretune.co.jp>");
    test("\"Shigeya 'too busy' Suzuki\" <shigeya@foretune.co.jp>");
    test("shigeya@foretune.co.jp (Shigeya \"too busy\" Suzuki)");
    test("shigeya@foretune.co.jp (Shigeya 'too busy' Suzuki)");
    test("\":users 1\"@foretune.co.jp");
    test("=?ISO-2022-JP?B?GyRCTmtMWkxQOkgbKEI=?= <shigeya@foretune.co.jp>");
    test("shigeya@foretune.co.jp (<Shigeya Suzuki>)");
    test("=?ISO-2022-JP?B?GyRCTmtMWkxQOkgbKEI=?=\n =?ISO-2022-JP?B?GyRCTmtMWkxQOkgbKEI=?= <shigeya@foretune.co.jp>");
    test("shigeya@foretune.co.jp (=?ISO-2022-JP?B?GyRCTmtMWkxQOkgbKEI=?=\n =?ISO-2022-JP?B?GyRCTmtMWkxQOkgbKEI=?=)");
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

    if (p == NULL) {
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

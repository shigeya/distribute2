/* $Id$
 *
 * parse recipient file, and construct recipient address buffer
 *
 * by Shigeya Suzuki, April 1993
 * Copyright(c)1993, Shigeya Suzuki
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#ifdef STDC_HEADERS
# include <stdio.h>
#else
# ifndef HAVE_STRCHR
#  define strchr index
# endif
# ifndef HAVE_STRRCHR
#  define strrchr rindex
# endif
#endif
#include <sysexits.h>
#include <errno.h>
  
#ifdef STDC_HEADERS
# include <stdlib.h>
#else
# ifdef HAVE_MALLOC_H
#  include <malloc.h>
# endif
#endif

#include "longstr.h"
#include "memory.h"

extern void logandexit();
extern void logwarn();


/* normalizeaddr -- check one address and normalize it if necessary.
 */

char *
normalizeaddr(buf)
    char *buf;
{
    char *rp;
    char *p, *beginp, *tp;
    char *statstr = "";

    if (strlen(buf) >= MAXADDRLEN) {
	logwarn("too large buf\n");
	return NULL;
    }

    p = malloc(sizeof(char) * MAXADDRLEN);
    tp = p;

    strncpy(tp, buf, MAXADDRLEN-1);
    while (*tp == ' ' || *tp == '\t')
	tp++;

    beginp = tp;

    /* delete comment(s) */
    for (; *statstr == '\0'; tp++) {
	if (*tp == '\0')
	    break;

	if (*tp == '\\' && *++tp != '\0') /* ignore quoting */
	    continue;

	if (*tp == '(') { /* found parenses.. */
	    char *pstart = tp;
	    while (*tp && ! ( *tp == ')' && tp[-1] != '\\'))
		tp++;
	    if (*tp != ')') {
		statstr = "paren mismatch";
		break;
	    }
	    strcpy(pstart, tp + 1);
	    tp = pstart;
	}
    }

    /* delete double-quote */
    for (tp = beginp; *statstr == '\0'; tp++) {
	if (*tp == '\0')
	    break;

	if (*tp == '\\' && *++tp != '\0') /* ignore quoting */
	    continue;

	if (*tp == '"') { /* double-quote, just skip */
	    char *qstart = tp;
	    tp++;
	    while (*tp && !(*tp == '"' && tp[-1] != '\\'))
		tp++;
	    if (*tp != '"') {
		statstr = "doublequote mismatch";
		break;
	    }
	    strcpy(tp, tp + 1);
	    strcpy(qstart, qstart + 1);
	}
    }

    /* find address */
    for (tp = beginp; *statstr == '\0'; tp++) {
	if (*tp == '\0')
	    break;

	if (*tp == '\\' && *++tp != '\0') /* ignore quoting */
	    continue;

	if (*tp == '<') { /* found non-comment angle bracket */
	    beginp = ++tp;
	    if ((tp = strchr(beginp, '>')) != NULL) {
		*tp = '\0';
		break;
	    }
	    else {
		statstr = "angle mismatch";
		break;
	    }
	}
    }

    if (*statstr != '\0') {
	logwarn("%s: %s.\n", statstr, buf);
	free(p);
	return NULL;
    }

    /* skip beggining white space(s) */
    while (*beginp == ' ' || *beginp == '\t')
	beginp++;

    if (*beginp == '\0') {
	logwarn("no address: %s.\n", buf);
    }

    /* remove trailing space */
    rp = strrchr(beginp, '\0');
    if (rp != NULL) {
	while (rp > beginp && *--rp == ' ')
	    *rp = '\0';
    }

    tp = malloc(sizeof(char) * MAXADDRLEN);
    strncpy(tp, beginp, MAXADDRLEN-1);
    free(p);

    return tp;
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
	if ((p = strchr(buf, '\n')) != NULL)/* remove trailing LF */
	    *p = '\0';
	if ((p = strchr(buf, '#')) != NULL) /* remove comments, if exists */
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

		ls_appendstr(&recipbuf, namep);
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

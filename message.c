/* $Id$
 *
 * Messaging functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "message.h"
#include "longstr.h"
#include "logging.h"

#ifdef TEST
#define	host		"<host>"
#define	maintainer	"<maintainer>"
#define	dommaintainer	"<dommaintainer>"
#define	list		"<list>"
#define	originator	"<originator>"
#define	programerror()	exit(0);
#else
extern char *host;
extern char maintainer[];
extern char dommaintainer[];
extern char *list;
extern char originator[];
#endif


/* patcompare -- compare function for lookupmestab
 *
 * use tail match.
 * We'll use regular expression later..
 */
int
patcompare(s, pat)
    char *s;
    char *pat;
{
    int slen, plen;
    
    if (*pat == '*')
	return 1;

    slen = strlen(s);
    plen = strlen(pat);

    if (plen > slen)
	return 0;

    if (plen == slen && strcasecmp(s,pat) == 0)
	return 1;

    if (strcasecmp(s+(slen-plen), pat) == 0)
	return 1;

    return 0;
}


struct mestab*
lookupmestab(mestab, target)
    struct mestab *mestab;
    char *target;
{
    struct mestab *p;

    for (p = mestab; p->mestab_pat != NULL; p++) {
	if (patcompare(target, p->mestab_pat)) {
	    return p;
	}
    }
    return NULL;
}

/*VARARGS3*/
void
messageprint(file, mestab, pat, a1, a2)
    FILE *file;
    struct mestab *mestab;
    char *pat;
    char *a1;
    char *a2;
{
    char *result;
    unsigned char *p;
    struct mestab *mt = lookupmestab(mestab, pat);
    char *message;

    struct longstr mesbuf;

    ls_init(&mesbuf);
    ls_reset(&mesbuf);


    if (mt == NULL) {
	programerror();		/* can not happen */
    }

    message = mt->mestab_message; /* use this message */

    for (p=(unsigned char*)message; *p; p++) {
	if (*p == '%') {
	    switch (*++p) {
	    case '1':		/* arg #1 */
		ls_appendstr(&mesbuf, a1);
		break;

	    case '2':		/* arg #2 */
		ls_appendstr(&mesbuf, a2);
		break;

	    case 'H':		/* host */
		ls_appendstr(&mesbuf, host);
		break;

	    case 'M':		/* maintainer */
		ls_appendstr(&mesbuf, dommaintainer);
		break;

	    case 'L':		/* list name */
		ls_appendstr(&mesbuf, list);
		break;

	    case 'O':		/* message originator */
		ls_appendstr(&mesbuf, originator);
		break;
	    }
	}
	else {
	    ls_appendchar(&mesbuf, (int)*p);
	}
    }

    if (mt->mestab_codeconv != NULL) /* if there are convert function exist..*/
	result = (*(mt->mestab_codeconv))(mesbuf.ls_buf);
    else
	result = mesbuf.ls_buf;

    fputs(result, file);
    free(result);
}

char*
euc_to_iso2022jp(from)
    char *from;
{
    struct longstr buf;
    unsigned char *s;
    int inkanji = 0;
    
    ls_init(&buf);
    ls_reset(&buf);

    for (s = (unsigned char*)from; *s; s++) {
	if (*s >= 0x80) {
	    if (!inkanji) {
		ls_appendstr(&buf, JISIN);
		inkanji++;
	    }
	    ls_appendchar(&buf, *s & 0x7f);
	}
	else {
	    if (inkanji) {
		ls_appendstr(&buf, JISOUT);
		inkanji--;
	    }
	    ls_appendchar(&buf, *s);
	}
    }

    if (inkanji)		/* force JIS out */
	ls_appendstr(&buf, JISOUT);

    return buf.ls_buf;
}




#ifdef TEST
char *testarray[] = {
    "shigeya@foretune.co.jp",
    "toku@dit.co.jp",
    "yoshiki@nc.u-tokyo.ac.jp",
    "jun@wide.ad.jp",
    "ram@acri.fr",
    "ram@eiffel.com",
    "rick@uunet.uu.net",
0
};

main()
{
    char **p;
    for (p = testarray; *p; p++) {
	test(*p, ".jp");
	test(*p, "co.jp");
	test(*p, "*");
	test(*p, "ad.jp");
	test(*p, ".edu");
	test(*p, ".com");
    }
}

test(target, pat)
    char *target;
    char *pat;
{
    printf("%s -- %s  -> %d\n", target, pat, patcompare(target, pat));
}
#endif

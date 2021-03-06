/* $Id$
 *
 * Path related support functions
 *
 * By Shigeya Suzuki, Feb 1994
 * Copyright(c)1993,1994 Shigeya Suzuki
 */

#include <config.h>

#include <stdio.h>
#include <sysexits.h>
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
#include <memory.h>
#include <ctype.h>

#include "memory.h"
#include "logging.h"
#include "pathutil.h"

/* make default path
 */
char *
adddefaultpath(defpath, name, suffix, lower)
    char *defpath;
    char *name;
    char *suffix;
    int lower;
{
    int len;
    char *buf;
    char *p;
    
    if (name == NULL) {
	logandexit(EX_UNAVAILABLE, "invalid filename");
    }

    if (*name == '/') {		/* is absolute path */
	return name;		/* use it as is */
    }

    len = strlen(defpath) + strlen(name) + strlen(suffix) + 1 + 1;
    /* one for null, one for "/" */

    buf = xmalloc(len);
    strcpy(buf, defpath);

    p = strrchr(buf, '\0');

    if (p == NULL) { 		/* must not happen */
	programerror();
    }

    if (p > buf && p[-1] != '/')  /* add / if missing */
	strcat(buf, "/");

    if (lower) {
	p = strrchr(buf, '\0');
	strcat(buf, name);
	for (; *p; p++)		/* make the string lowercase */
	    if (isupper(*p))
		*p = tolower(*p);
    }
    else
	strcat(buf, name);

    strcat(buf, suffix);

    return buf;
}

/* make archive path
 */
char*
makearchivepath(ap, ad, name)
    char *ap;
    char *ad;
    char *name;
{
    char *r, *p;

    /* If archivedir is specified and is absolute path, use the path.
     * If archivedir is relative path, use it to relative to default path.
     * If none specified, use DEFAULTPATH/MailinglistName for archive
     */
    if (ad)
	r = adddefaultpath(ap, ad, "", 0);
    else
	r = adddefaultpath(ap, name, "", 0);

    /* Then, ensure there are NO SAPCE character in the path.
     * use character defined in config.h
     */
    for (p=r; *p; p++)
	if (*p == ' ')
	    *p = ALIAS_SPACE_CHAR;

    return r;
}


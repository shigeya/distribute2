/* $Id$
 *
 * Support library for distrbute+archive
 *
 * By Shigeya Suzuki, Dec 1993
 * Copyright(c)1993 Shigeya Suzuki
 */

#include <config.h>

#include <stdio.h>
#include <sys/types.h>
#include <sysexits.h>
#include <memory.h>

#ifndef HAVE_STRSTR
/* Find the first occurrence of find in s.
 */
char *
strstr(s, find)
	register char *s, *find;
{
	register char c, sc;
	register size_t len;

	if ((c = *find++) != 0) {
		len = strlen(find);
		do {
			do {
				if ((sc = *s++) == 0)
					return (NULL);
			} while (sc != c);
		} while (strncmp(s, find, len) != 0);
		s--;
	}
	return ((char *)s);
}
#endif /*HAVE_STRSTR*/

#ifndef HAVE_STRSPN
size_t
strspn(s, cs)
	const char *s;
	const char *cs;
{
	const char *p;
	const char *q;
	size_t cnt;
	int matched;

	p = s;
	while (*p) {
		q = cs;
		matched = 0;
		while (*q) {
			if (*p == *q) {
				matched++;
				break;
			}
			q++;
		}
		if (!matched)
			return p - s;
		p++;
	}

	return p - s;
}
#endif /*HAVE_STRSPN*/

char *
strcpycut(d, s, n)
    char *d, *s;
    size_t n;
{
    int len = strlen(s);
    if (len >= n)
	len = n-1;

    memcpy(d, s, len);
    d[len] = '\0';

    return d;
}

char*
skiptononspace(s)
    char *s;
{
    while (*s == ' ')
	s++;
    return s;
}


/* change characters
 */
char *
changech(s, f, t)
    char *s;
    int f, t;
{
    char *top = s;
    
    for ( ; *s; s++)
	if (*s == f)
	    *s = t;

    return top;
}


/* Chop at LF
 */
char *
chopatlf(s)
    char *s;
{
    char *top = s;

    for (; *s; s++) {
	if (*s == '\n') {
	    *s = '\0';
	    break;
	}
    }
    return top;
}

#ifdef STRSEP_MISSING
# include "strsep.c"
#endif

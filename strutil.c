/* strutil.c,v 1.4 1994/02/10 05:40:55 shigeya Exp
 *
 * Support library for distrbute+archive
 *
 * By Shigeya Suzuki, Dec 1993
 * Copyright(c)1993 Shigeya Suzuki
 */

#include <stdio.h>
#include <sys/types.h>
#include <sysexits.h>
#include <memory.h>

#ifdef STRSTR_MISSING

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

#endif

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

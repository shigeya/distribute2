/* $Id$
 *
 * Memory managers
 */

#include <config.h>

#include <stdio.h>
#include <sys/types.h>
#include <sysexits.h>
#include <stdlib.h>
#include <string.h>

#include "logging.h"
#include "memory.h"

/* allocate or fail
 */
char*
xmalloc(len)
    size_t len;
{
    char *p = malloc(len);
    if (p == NULL) {
	logandexit(EX_UNAVAILABLE, "insufficient memory");
    }
    return p;
}

/* reallocate or fail
 */
char*
xrealloc(p, len)
    char *p;
    size_t len;
{
    char *np = realloc(p,len);
    if (np == NULL) {
	logandexit(EX_UNAVAILABLE, "insufficient memory");
    }
    return np;
}

/* save string or fail
 */
char*
strsave(p)
    char *p;
{
    int len = strlen(p) + 1;
    char *d = xmalloc(len);
    memcpy(d, p, len);
    return d;
}

/* save memory or fail
 */
char*
memsave(p, len)
    char *p;
    size_t len;
{
    char *d = xmalloc(len);
    memcpy(d, p, len);
    return d;
}

/* append string to a "saved" string
 * "saved" string means, string in malloc'ed space
 */ 
char*
strappend(d, s)
    char *d;
    char *s;
{
    int d_len = strlen(d);
    int s_len = strlen(s);

    d = xrealloc(d, d_len+s_len+1);
    strcat(d, s);
    return d;
}

/* append space and a string to "saved" string
 */
char*
strspappend(d, s)
    char *d;
    char *s;
{
    int d_len = strlen(d);
    int s_len = strlen(s);

    d = xrealloc(d, d_len+s_len+1+1);
    strcat(d, " ");
    strcat(d, s);
    return d;
}

/* copy just like strncpy, but spare one byte null pad at the end
 * and do put a null at end of buffer always.
 */

char*
xstrncpy(d,s,n)
    char* d;
    char* s;
    int n;
{
    if (n > 0) {
	strncpy(d,s,n-1);
	d[n-1] = '\0';
    }
    return d;
}
    

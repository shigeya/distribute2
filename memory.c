/* memory.c,v 1.4 1994/08/05 11:49:31 shigeya Exp
 *
 * Memory managers
 */

#include <stdio.h>
#include <sys/types.h>
#include <sysexits.h>
#include <stdlib.h>

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
    bcopy(p, d, len);
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
    bcopy(p, d, len);
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

/* $Id$
 *
 * A test program for longstr
 */

#include <stdio.h>
#include <sys/types.h>
#include "longstr.h"
#include <errno.h>
#include <malloc.h>

struct longstr ls;

main()
{
    int i;
    char buf[256];
    
    malloc_debug(2);

    ls_init(&ls);

    for (i=1; i<=50000; i++) {
	if (i != 1)
	    ls_appendstr(&ls, " ");
	sprintf(buf, "%d@foretune.co.jp", i);
	ls_appendstr(&ls, buf);
    }

    write(1, ls.ls_buf, ls.ls_used);
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

/* longstr.c,v 1.4 1994/02/10 05:40:48 shigeya Exp
 *
 *	Long string handler
 *
 *	Shigeya Suzuki, April 1993
 *	Copyright(c)1993 Shigeya Suzuki
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sysexits.h>
#include <sys/types.h>
#include <sys/param.h>

#include "longstr.h"


/* These must be defined in main program
 */
extern char * xmalloc();
extern char * xrealloc();


void
ls_init(p)
    struct longstr *p;
{
    p->ls_buf = p->ls_ptr = NULL;
    p->ls_size = p->ls_used = 0;
    p->ls_allocsize = LONGSTR_CHUNK;

    ls_reset(p);
}


/* Reset command line buffer
 * We do not re-allocate buffer on reset
 */
void
ls_reset(p)
    struct longstr *p;
{
    if (p->ls_buf == NULL) {
	p->ls_buf = xmalloc(p->ls_allocsize);
	p->ls_size = p->ls_allocsize;
    }

    p->ls_used = 0;
    p->ls_ptr = p->ls_buf;
}

/* Grow command line buffer
 */
void
ls_grow(p, size)
    struct longstr *p;
    size_t size;
{
    /* Rounds, but allocate at least one block */
    size_t diff = size - (p->ls_size - p->ls_used);
    int chunk = diff < p->ls_allocsize ? p->ls_allocsize
	: ((diff / p->ls_allocsize)+1) * p->ls_allocsize;

    if ( (p->ls_size + chunk) >= NCARGS) { /* exceeds limit */
	logandexit(EX_UNAVAILABLE, "limit %d exceeded (%d)",
		   NCARGS, p->ls_size + chunk);
    }

#ifdef DEBUG
    fprintf(stderr, "ls_grow(%d/0x%x:0x%x[%d]): %d -> %d (diff=%d, chunk=%d)\n",
	    size,
	    p->ls_buf, p->ls_ptr, p->ls_ptr - p->ls_buf,
	    p->ls_size, p->ls_size+chunk,
	    diff, chunk);
#endif

    p->ls_buf = xrealloc(p->ls_buf, p->ls_size+chunk);
    p->ls_ptr = p->ls_buf + p->ls_used;
    p->ls_size += chunk;
}

/* ls_append -- add memory block at end
 * string will be null-terminated, thus at least one byte longer.
 */
void
ls_append(p, str, len)
    struct longstr *p;
    char *str;
    size_t len;
{
    if (p->ls_size == 0 || (p->ls_size - p->ls_used) < len) {
	ls_grow(p, len+1);
    }

    bcopy(str, p->ls_ptr, len);
    
    p->ls_ptr += len;
    p->ls_used += len;
    *(p->ls_ptr) = 0;
}

/* ls_appendstr -- add string at end
 */
void
ls_appendstr(p, str)
    struct longstr *p;
    char *str;
{
    ls_append(p, str, strlen(str));
}

/* ls_appendchar -- add a char at end
 */
void
ls_appendchar(p, ch)
    struct longstr *p;
    int ch;
{
    char buf = ch;
    ls_append(p, &buf, 1);
}

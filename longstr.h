/* $Id$
 *
 *	Long string handler --	Definitions
 *
 *	Shigeya Suzuki, April 1993
 */

#ifndef __LONGSTR_H__
#define	__LONGSTR_H__

#ifndef __P
# include "cdefs.h"
#endif

struct longstr {
    char *ls_buf;		/* pointer to head of buffer */
    char *ls_ptr;		/* pointer to current tail (points null ch) */
    size_t ls_allocsize;	/* chunks for allocation */
    size_t ls_size;		/* size of the *ls_buf */
    size_t ls_used;		/* bytes used (actually, ls_ptr - ls_buf +1) */
};

#define	LONGSTR_CHUNK	10240	/* default allocation chunk */

void ls_init __P((struct longstr*));
void ls_reset __P((struct longstr*));
void ls_grow __P((struct longstr*, size_t));
void ls_append __P((struct longstr*, char*, size_t));
void ls_appendstr __P((struct longstr*, char*));
void ls_appendchar __P((struct longstr*, int));

#endif

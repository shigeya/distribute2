/* $Id$
 *
 * Memory related function definitions
 */
#ifndef __MEMORY_H__
#define __MEMORY_H__

#ifndef __P
# include "cdefs.h"
#endif

char *xrealloc __P((char*, size_t));
char *xmalloc __P((size_t));
char *strsave __P((char*));
char *memsave __P((char*, size_t));
char *strappend __P((char*, char*));
char *strspappend __P((char*, char*));

#endif

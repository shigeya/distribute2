/* memory.h,v 1.4 1994/08/05 11:49:31 shigeya Exp
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

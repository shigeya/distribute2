/* $Id$
 *
 * Memory related function definitions
 */
#ifndef __MEMORY_H__
#define __MEMORY_H__

#ifndef __P
# include "cdefs.h"
#endif

extern char *xrealloc __P((char*, size_t));
extern char *xmalloc __P((size_t));
extern char *strsave __P((char*));
extern char *memsave __P((char*, size_t));
extern char *strappend __P((char*, char*));
extern char *strspappend __P((char*, char*));
extern char *xstrncpy __P((char*,char*,int));

#endif

/* $Id$*/

#ifndef __STRUTIL_H__
#define __STRUTIL_H__

#ifndef __P
# include "cdefs.h"
#endif

#ifndef HAVE_STRSTR
extern char *strstr __P((char*, char*));
#endif

#ifndef HAVE_STRSEP
extern char *strsep __P((char**, char*));
#endif

extern char *strcpycut __P((char*, char*, size_t));
extern char *skiptononspace __P((char*));
extern char *chopatcr __P((char*));
extern char *changech __P((char*, int, int));
extern char *chopatlf __P((char*));

#endif

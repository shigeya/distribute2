/* strutil.h,v 1.4 1994/02/13 13:46:52 shigeya Exp */

#ifndef __STRUTIL_H__
#define __STRUTIL_H__

#ifndef __P
# include "cdefs.h"
#endif

#ifdef STRSTR_MISSING
char *strstr __P((char*, char*));
#endif

char *strcpycut __P((char*, char*, size_t));
char *skiptononspace __P((char*));
char *chopatcr __P((char*));
char *changech __P((char*, int, int));
char *chopatlf __P((char*));

#endif

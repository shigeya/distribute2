/* $Id$*/

#ifndef __STRUTIL_H__
#define __STRUTIL_H__

#ifndef __P
# include "cdefs.h"
#endif

#ifdef STRSTR_MISSING
char *strstr __P((char*, char*));
#endif

#ifdef STRSEP_MISSING
char *strsep __P((char**, char*));
#endif

char *strcpycut __P((char*, char*, size_t));
char *skiptononspace __P((char*));
char *chopatcr __P((char*));
char *changech __P((char*, int, int));
char *chopatlf __P((char*));

#endif

/* $Id$ */

#ifndef __PATHUTIL_H__
#define __PATHUTIL_H__

#ifndef __P
# include "cdefs.h"
#endif

char *adddefaultpath __P((char*, char*, char*, int));
char *makearchivepath __P((char*, char*, char*));

#endif

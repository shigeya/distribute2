/* $Id$ */

#ifndef __PATHUTIL_H__
#define __PATHUTIL_H__

#ifndef __P
# include "cdefs.h"
#endif

extern char *adddefaultpath __P((char*, char*, char*, int));
extern char *makearchivepath __P((char*, char*, char*));

#endif

/* $Id$ */

#ifndef __HEADER_H__
#define	__HEADER_H__

#ifndef __P
# include "cdefs.h"
#endif

int head_parse __P((int, char**, FILE*));
void head_norm __P((char*));
void head_blank __P((char*));
char * head_find __P((int, char**, char*));
char * head_delete __P((int, char**, char*));
void head_free __P((int, char**));

#endif
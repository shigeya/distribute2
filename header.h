/* $Id$ */

#ifndef __HEADER_H__
#define	__HEADER_H__

#ifndef __CONFIG_H__
# include "config.h"
#endif

#ifndef __P
# include "cdefs.h"
#endif

extern int head_parse __P((int, char**, FILE*));
extern void head_norm __P((char*));
extern void head_blank __P((char*));
extern char * head_find __P((int, char**, char*));
extern char * head_delete __P((int, char**, char*));
extern void head_free __P((int, char**));

struct msgHeader {
    char from[MAXADDRLEN];
    char replyto[MAXADDRLEN];
    char returnpath[MAXADDRLEN];
    char sender[MAXADDRLEN];
};

#endif

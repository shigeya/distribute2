/* $Id$
 *
 * Message handling functions
 */

#ifndef __MESSAGE_H__
#define	__MESSAGE_H__

#ifndef __P
# include "cdefs.h"
#endif

typedef char* (*ConvFunc)();

struct mestab {
    char *mestab_pat;
    char *mestab_message;
    ConvFunc mestab_codeconv;
};

extern char* euc_to_iso2022jp __P((char*));
extern int patcompare __P((char*, char*));
extern struct mestab* lookupmestab __P((struct mestab*, char*));
extern void messageprint();	/* don't prototype this.. */

#define	JISIN	"\033$B"
#define	JISOUT	"\033(B"

#endif

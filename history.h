/* history.h,v 1.4 1994/02/13 13:46:44 shigeya Exp
 *
 * history related routines definitions
 */

#ifndef __HISTORY_H__
#define	__HISTORY_H__

#ifndef __P
# include "cdefs.h"
#endif

#define	HISTORYBUFLEN		256
#define	ARTICLENUMFORMAT	"%08d"
#define	CHECKSUMMASK		0x7fffffff
#define	HISTORYFORMAT		"%08d %010d %s %s\n"
				/* art, sum, mid, subject */

struct history {
    int 	h_articlenum;
    char	*h_messageid;
    int		h_bodysum;
    char	*h_subject;
};

void openhistory __P((char*, char*));
void closehistory __P(());

struct history* findhistory __P((int));
void appendhistory __P((int, int, char*, char*));
void freehistory __P((struct history*));

int fputs_sum __P((char*, FILE*, int*));

#endif

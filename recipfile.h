/* recipfile.h,v 1.2 1994/08/05 11:49:33 shigeya Exp
 *
 * parse recipfile definitions
 */

#ifndef __RECIPFILE_H__
#define __RECIPFILE_H__

#ifndef __P
# include "cdefs.h"
#endif

extern char * normalizeaddr  __P((char*));
extern char * parserecipfile __P((char*, int));

#endif	/* __PARSERECIPFILE_H__ */

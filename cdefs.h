/* cdefs.h,v 1.1 1993/05/21 09:20:09 shigeya Exp
 *
 *	ANSI C Prototype hacks
 *
 */

#ifndef __P
# ifdef __STDC__
#  define __P(protos)	protos
# else
#  define __P(protos)	()
# endif
#endif


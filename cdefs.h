/* $Id$
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


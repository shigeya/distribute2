#ifndef __LOGGING_H__
#define __LOGGING_H__

#ifndef __P
# include "cdefs.h"
#endif

extern void init_log __P((char*));
extern void logging_setprinterror __P((int));
extern void logerror __P((char*, ...));
extern void logwarn __P((char*, ...));
extern void loginfo __P((char*, ...));
extern void programerror __P((void));
extern void logandexit __P((int, char*, ...));

#endif
    

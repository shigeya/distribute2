/* $Id$
 *
 * history handling routines
 *
 * By Shigeya Suzuki, January 1994
 * Copyright(c)1993 Shigeya Suzuki
 */

/* These routines are slow, but:
 * (1) does not required to call very frequently.
 * (2) We can keep the history file only in text format.
 * (3) Does not waste memory.
 */

#include <config.h>

#include <sys/types.h>
#ifdef HAVE_SYS_FILE_H
# include <sys/file.h>
#endif
#include <sysexits.h>
#include <stdio.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include "memory.h"
#include "history.h"
#include "logging.h"

/* globals
 */
static char * filename = NULL;
static int file_fd = -1;
static FILE *file = NULL;

void
openhistory(name, mode)
    char *name;
    char *mode;
{
    FILE *fp;
    int fd;
    
    if ((fp = fopen(name, mode)) == NULL) {
	logandexit(EX_NOINPUT,
		   "Can't open history file: %s", name);
	return;
    }

    fd = fileno(fp);
#ifdef HAVE_FLOCK
    flock(fd, LOCK_EX);		/* lock the file */
#else
# ifdef HAVE_LOCKF
    lockf(fd, F_LOCK, 0);	/* lock the file */
# endif
#endif
    filename = strsave(name);
    file = fp;
    file_fd = fd;
}

void
closehistory()
{
    if (file != NULL) {
#ifdef HAVE_FLOCK
	flock(file_fd, LOCK_UN);
#else
# ifdef HAVE_LOCKF
	lockf(file_fd, F_ULOCK, 0);
# endif
#endif
	fclose(file);
	file = NULL;
	file_fd = -1;
	free(filename);
	filename = NULL;
    }
}

char*
nexttoken(pp)
    char **pp;
{
    char *p = *pp;
    char *head = p;
    
    while (*p && *p != ' ' && *p != '\t')
	p++;

    if (*p) {
	*p++ = '\0';
	*pp = p;
	return head;
    }
    else {
	*pp = p;
	return head;
    }
}

void
chop(p)
     char* p;
{
    int len = strlen(p);
    if (len > 0)
	p[len-1] = '\0';
}

int
parsehistoryentry(h, buf)
    struct history *h;
    char *buf;
{
    char *p = buf, *v;
    h->h_articlenum = -1;
    h->h_messageid = NULL;
    h->h_bodysum = -1;
    h->h_subject = NULL;
    
    v = nexttoken(&p);
    if (v == 0)
	return 0;
    h->h_articlenum = atoi(v);

    v = nexttoken(&p);
    if (v == 0)
	return 0;
    h->h_bodysum = atoi(v);

    v = nexttoken(&p);
    if (v == 0)
	return 0;
    h->h_messageid = strsave(v);
    if (p == NULL)
	return 0;

    h->h_subject = strsave(p);
    chop(h->h_subject);
    return 1;
}

struct history*
findhistory(article)
    int article;
{
    char buf[HISTORYBUFLEN];
    char numbuf[20];
    struct history h;
    int numbuflen;
    
    sprintf(numbuf, ARTICLENUMFORMAT, article);
    numbuflen = strlen(numbuf);

    rewind(file);

    /* on compare, we assume first field is article number and fixed width.
     */
    while (fgets(buf, sizeof(buf), file) != NULL)
	if (!memcmp(buf, numbuf, numbuflen) && parsehistoryentry(&h, buf))
	    return (struct history*) memsave((char*)&h, sizeof(h));

    return (struct history*)NULL;
}


void
appendhistory(num, sum, id, subj)
    int num;
    int sum;
    char *id;
    char *subj;
{
    char *subjbuf;
    char *p;

    if (file == NULL) {
	programerror();
    }
    
    fseek(file, 0L, SEEK_END);

    subjbuf = strsave(subj);
    for (p = subjbuf; *p; p++)	/* need to remove LF conver to space */
	if (*p == '\n')
	    *p = ' ';
    fprintf(file, HISTORYFORMAT, num, sum, id, subj);
}

void
freehistory(h)
    struct history* h;
{
    if (h->h_messageid != NULL)
	free(h->h_messageid);
    if (h->h_subject != NULL)
	free(h->h_subject);
    free((char*)h);
}

/* fput and make check sum
 */
int
fputs_sum(buf, fp, sump)
    char *buf;
    FILE *fp;
    int *sump;
{
    unsigned char *p;
    int sum = *sump;
    int r = fputs(buf, fp);
    
    for (p = (unsigned char*)buf; *p; p++)
	sum = (sum + *p) & CHECKSUMMASK;

    *sump = sum;

    return r;
}


#ifdef TEST
main()
{
    int i;
    char buf[128];
    char sbuf[128];
    struct history * h;
    
    logging_setprinterror(1);
    
    openhistory("./histtest.dat", "a+");

    printf("Writing..\n");
    for (i=1; i<=1000; i++) {
	sprintf(buf, "<AA%07d.%07d@hook.foretune.co.jp>", i, i);
	sprintf(sbuf, "(B-Class %d) Test Test Test", i);
	appendhistory(i, 10000+i, buf, sbuf);
    }

    printf("Reading..\n");
    for (i=1000; i>0; i--) {
	h = findhistory(i);
	if (h != NULL) {
	    printf("\t%d, %d, %s, %s\n", h->h_articlenum, h->h_bodysum,
		   h->h_messageid, h->h_subject);
	    if (h->h_articlenum != i || h->h_bodysum != i+10000) {
		printf("find: %d BAD\n", i);
	    }
	    else {
		printf("find: %d OK\n", i);
	    }
	    freehistory(h);
	}
	else {
	    printf("Can't find %d\n", i);
	}
    }

    closehistory();
}
#endif

/* $Id$
 *
 * Archiving program for distribute
 *
 * Shigeya Suzuki, Dec 1993
 * Copyright(c)1993-1999 Shigeya Suzuki
 */

#include <config.h>

#include <stdio.h>

#ifndef STDC_HEADERS
# ifndef HAVE_STRCHR
#  define strchr index
# endif
# ifndef HAVE_STRRCHR
#  define strrchr rindex
# endif
#endif
#include <ctype.h>
#include <string.h>
#include <sysexits.h>
#include <sys/types.h>
#ifdef HAVE_SYS_FILE_H
# include <sys/file.h>
#endif
#include <sys/param.h>
  
#ifdef USE_SYSLOG
# ifdef HAVE_SYSLOG_H
#  include <syslog.h>
# endif
#endif


#ifdef DEBUGLOG
FILE	*debuglog;
#endif

#include "patchlevel.h"		/* version identifier */

#include "config.h"
#include "memory.h"
#include "history.h"
#include "pathutil.h"
#include "header.h"
#include "logging.h"

char *progname;

char *Xsequence = NULL;		/* X-Sequence value */
char *XMl_Name = NULL;
char *XMl_Count = NULL;

char *Xdistribute;		/* X-Distribute value */
char *subject;
char *listname = NULL;
char *archive_path = DEF_ARCHIVE_PATH;
char *archivedir = NULL;
char *mltag;
int  mlseq;
char *headv[MAXHEADERLINE];	/* Header vector */
int headc;	/* Number of headers */
char filename[512];
char *index_name = DEF_INDEX_NAME;

int majordomo = 0;
int zaprecv = 0;
int useindex = 0;

/* Forward Declaration
 */
void usage __P((void));


/* Option parser
 */
void
parse_options(argc, argv)
    int argc;
    char **argv;
{
    extern char *optarg;
    int c;

    while ((c = getopt(argc, argv, "M:N:C:Z:Y:jDR?")) != EOF) {
	switch(c) {
	case 'M':	/* generic mailinglist with reply-to */
	case 'N':	/* generic mailinglist without reply-to */
	    listname = optarg;
	    break;
	    
	case 'Y':
	    archive_path = optarg;
	    useindex++;
	    break;

	case 'C':
	    archivedir = optarg;
	    useindex++;
	    break;

	case 'j':	/* majodomo */
	    majordomo++;
	    break;
		
	case 'D':	/* print error message to stderr for debugging */
	    logging_setprinterror(1);
	    break;

	case 'R':	/* zap Received: lines */
	    zaprecv++;
	    break;

	case 'Z':
	    index_name = optarg;
	    break;

	default:
	    usage();
	    logandexit(EX_USAGE, "unknown option: %c", c);
	    break;
	    
	case '?':
	    usage();
	    exit(0);
	    break;	/*NOTREACHED*/
	}
    }
}


void
parse_header(file)
    FILE *file;
{
    /* Read all of the headers and make a header vector
     */
    headc = head_parse(MAXHEADERLINE, headv, file);


    /* Try to find "X-Distribute". If not exists, log and exit.
     * Then, find "X-Sequence", "X-Ml-Name:", "X-Ml-Count:".
     * If none exists, log and exit.
     */
#if 0
    if ((Xdistribute = head_find(headc, headv, "X-Distribute:")) == NULL) {
	logandexit(EX_NOINPUT, "Does not contain X-Distribute tag");
    }
#endif

    Xsequence = head_find(headc, headv, "X-Sequence: ");
    XMl_Name = head_find(headc, headv, "X-Ml-Name: ");
    XMl_Count = head_find(headc, headv, "X-Ml-Count: ");

    if (Xsequence == NULL && (XMl_Name == NULL || XMl_Count == NULL)) {
	logandexit(EX_NOINPUT, "Does not contain any Sequence tag (fatal)");
    }

    if ((subject = head_find(headc, headv, "Subject:")) == NULL) {
	subject = "(No Subject)";
    }
    else {
	subject += sizeof("Subject:");
    }
    
    /* Delete all the Received: lines, if the user said to do so.
     */
    if (zaprecv) {
	while (head_delete(headc, headv, "Received:") != NULL)
	    ;
    }
}


char *
parse_sequence(sequence, ml_n, ml_c, mlseq)
    char *sequence;
    char *ml_n, *ml_c;
    int *mlseq;
{
    char *tag;
    char *endp;
    char *p;

    *mlseq = 0;			/* default error value */

    if (sequence != NULL) {
	if (strncasecmp(sequence,"X-Sequence: ",sizeof("X-Sequence: ")-1) != 0) {
	    return NULL;
	}
	
	tag = strsave(sequence + sizeof("X-Sequence: ")-1);
	
	endp = strrchr(tag, '\0');
	
	for (p=endp; p-- > tag;) {
	    if (!isdigit(*p))
		break;
	}
	if (*p != ' ') {		/* sequence separator must be space */
	    free(tag);
	    return NULL;
	}
	*p++ = '\0';
	*mlseq = atoi(p);
	return tag;
    }
    else {			/* try MSC Style */
	if (strncasecmp(ml_n, "X-Ml-Name: ", sizeof("X-Ml-Name: ")-1) != 0) {
	    return NULL;
	}
	if (strncasecmp(ml_c, "X-Ml-Count: ", sizeof("X-Ml-Count: ")-1) != 0) {
	    return NULL;
	}

	tag = strsave(ml_n + sizeof("X-Ml-Name: ")-1);
	p = ml_c + sizeof("X-Ml-Count: ")-1;
	while (*p && *p == '0')	/* skip leading zer0 */
	    p++;
	*mlseq = atoi(p);
	return tag;
    }
}


/* doarchive -- archive the mail
 */
void
doarchive()
{
    char buf[BUFSIZ];	/* string buffer */
    char tmpname[16];
    char *dir;
    char target[16];
    int sum;
    int fd;
    int i;
    FILE *out = stdout;
    struct history *h = NULL;
    
    mltag = parse_sequence(Xsequence, XMl_Name, XMl_Count, &mlseq);

    if (listname != NULL)
	dir = makearchivepath(archive_path, archivedir, listname);
    else
	dir = makearchivepath(archive_path, archivedir, mltag);
    
    
    if (chdir(dir) == -1) {
	logandexit(EX_NOPERM, "Cannot change directory to %s", dir);
    }

    if (useindex) {
	openhistory(index_name, "r");
	h = findhistory(mlseq);
	closehistory();
	if (h == NULL)
	    logandexit(EX_UNAVAILABLE, "Can't find history entry for article %d", mlseq);
    }

    strcpy(tmpname, "msgXXXXXX");
#ifdef HAVE_MKSTEMP
    mkstemp(tmpname);
#else
    mktemp(tmpname);
#endif
    
    if ((fd = creat(tmpname, 0640)) == -1) {
	logandexit(EX_CANTCREAT, "Cannot make file: %s\n", tmpname);
    }
    out = fdopen(fd, "w");
    
    for (i=0; i<headc; i++) {
	if (headv[i] != NULL) {
	    fputs(headv[i], out);
	    putc('\n', out);
	}
    }
    putc('\n', out);

    sum = 0;
    while (fgets(buf, sizeof buf, stdin) != NULL) {
	fputs_sum(buf, out, &sum);
    }
    fclose(out);

    if (useindex && h && h->h_bodysum != sum) {
	unlink(tmpname);
	logandexit(EX_DATAERR, "body checksum mismatch: incoming: %d", sum);
    }

    sprintf(target, "%d", mlseq);
    rename(tmpname, target);
    sprintf(filename, "%s/%s", dir, target);
}


/* Finally, the Main Entry
 */
int
main(argc, argv)
    int argc;
    char ** argv;
{
/*    chdir("/tmp");  */
    progname = argv[0];

    init_log("archive");
    parse_options(argc, argv);

    parse_header(stdin);
    doarchive();

    loginfo("\"%s\" archived as %s", subject, filename);
    return 0; /*exit(0);*/
}

void
usage()
{
    fprintf(stderr, "usage: %s ", progname);
    fprintf(stderr, "{-M list | -N list}\n(and more...)");
}

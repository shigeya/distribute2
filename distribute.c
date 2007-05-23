/* $Id$
 *
 * distribute - a mailing list distributor: main module
 *
 *
 * This program is originally written by Steve Miller.
 * Later, heaviliy modified by group of people in Japan.
 *
 * Modified portions Copyright (c) 1990-1999 by Shigeya Suzuki,
 * Shin Yoshimura, Yoshitaka Tokugawa, Hiroaki Takada and
 * Susumu Sano, Yoichirou Koga, Junichiro Hagino and other contributors.
 *
 * Permission is granted to use, but not for sell.
 *
 *
 *	Shin Yoshimura			<shin@wide.ad.jp>
 *	Yoshitaka Tokugawa		<toku@wide.ad.jp>
 *	Shigeya Suzuki			<shigeya@foretune.co.jp>
 *	Hiroaki Takada			<hiro@is.s.u-tokyo.ac.jp>
 *      Susumu Sano 			<sano@wide.ad.jp>
 *	Youichirou Koga			<y-koga@isoternet.org>
 *      Junichiro 'itojun' Hagino	<itojun@itojun.org>
 */

#include <config.h>
  
#include <stdio.h>
#include <ctype.h>
#ifdef STDC_HEADERS
# include <stdio.h>
#else
# ifndef HAVE_STRCHR
#  define strchr index
# endif
# ifndef HAVE_STRRCHR
#  define strrchr rindex
# endif
#endif
#include <sys/types.h>
#ifdef USE_SYS_PARAM_H
# include <sys/param.h>
#endif
#ifdef USE_NETDB_H
# include <netdb.h>
#endif
#include <sys/stat.h>
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif
#include "sysexits.h"
#include <sysexits.h>
#include <sys/types.h>
#ifdef HAVE_SYS_FILE_H
# include <sys/file.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <string.h>
  
#ifdef HAVE_PATHS_H
# include <paths.h>
#endif

#ifdef USE_SYSLOG
# ifdef HAVE_SYSLOG_H
#  include <syslog.h>
# endif
#endif

#include "patchlevel.h"		/* version identifier */
#include "config.h"

#include "longstr.h"
#include "logging.h"
#include "memory.h"
#include "message.h"
#include "recipfile.h"
#include "mestab.h"
#include "strutil.h"
#include "pathutil.h"
#include "history.h"
#include "header.h"

char *rcsID = "$Id$";
char *versionID = VERSION;
int patchlevel = PATCHLEVEL;

#ifdef DEBUGLOG
FILE* debuglog;
#endif

/* constants
 */
char *seq_path = DEF_SEQ_PATH;
char *recipient_path = DEF_RECIPIENT_PATH;
char *archive_path = DEF_ARCHIVE_PATH;
char *majordomo_recipient_path = DEF_MAJORDOMO_RECIPIENT_PATH;
char *index_path = DEF_ARCHIVE_PATH;

char *seq_suffix = DEF_SEQ_SUFFIX;
char *recipient_suffix = DEF_RECIPIENT_SUFFIX;
char *accept_suffix = DEF_ACCEPT_SUFFIX;
char *reject_suffix = DEF_REJECT_SUFFIX;
char *index_name = DEF_INDEX_NAME;

char openaliaschar = DEF_OPENALIAS_CHAR;
char closealiaschar = DEF_CLOSEALIAS_CHAR;

#ifndef DEF_DOMAINNAME
char myhostname[MAXHOSTNAMELEN];
#endif

#ifdef REPORTADDR
char *reportaddr = REPORTADDR;
#endif


/*
 * Some keywords to check administrative/noise messages
 */

static char *maybeMlCmdNoise[] = {
    "actives", "add", "address", "append", "bye", "check",
    "config", "delete", "drop", "exit", "end", "get",
    "index", "keyword", "off", "on", "query", "register",
    "review", "send", "set", "stop", "which", "who", "whois",
    NULL
};

static char *mustbeMlCmdNoise[] = {
    "approve", "auth", "subscribe", "unsubscribe",
    NULL
};

static char *mlCmdNoise[] = {
    "addr", "admin", "chaddr", "digest", "gcos", "getdgst",
    "guide", "help", "iam", "info", "intro", "library",
    "list", "lists", "matome", "members", "member", "mget",
    "mkdigest", "msg", "newconfig", "newinfo", "newintro",
    "noskip", "objective", "passwd", "ping", "profile",
    "quit", "remove", "search", "signoff", "skip", "stat",
    "status", "summary", "traffic", "whois-index", "whois-list",
    "writeconfig",
    NULL
};




/* Globals
 */
char *progname;
struct longstr cmdbuf;
char subject[MAXSUBJLEN];
char messageid[MAXMESSAGEIDLEN];
int headc;	/* Number of headers */
char *headv[MAXHEADERLINE];	/* Header vector */
char archivetmp[16];
char archivetgt[16];
char maintainer[MAXADDRLEN];		 /* address of the maintainer */
char dommaintainer[MAXADDRLEN];		 /* address of the maintainer */
char *headererr = NULL;
char *recipientbuf = NULL;
char *acceptbuf = NULL;
char *rejectbuf = NULL;
char subjectbuf[MAXSUBJLEN];
char *originatorreplyto = NULL;
struct msgHeader origMsg;
char originator[MAXADDRLEN];
int bodysum = 0;		/* body check sum */
char *sendmail_path = _PATH_SENDMAIL;

#ifdef USESUID
int archive_uid, archive_gid;
int mailer_uid, mailer_gid;
#endif


/* Option and configuration related globals
 * (previously in main)
 */

FILE *noisef = NULL;

int errorsto = 0;		 /* append Erros-To? or not */

#ifdef USE_ISSUE
int issuenum = -1;		 /* issue number  */
#endif

#ifdef ADDVERSION
int addversion = 1;		 /* Add X-Distribute: header or not */
#endif

#ifdef USE_MIMEKIT
int usemimekit = 1;		 /* use mimekit (default ON) */
#endif

int badhdr = 0;			 /* something is fishy about the header */
int wasadmin = 0;		 /* was a noise message */
int wasrejected = 0;		 /* rejected message */
int badnewsheader = 0;		 /* incoming msg isn't like news2mail article*/
int badoriginator = 0;		 /* origintor address invalid */
int ccmail_error_message = 0;	 /* it's ccmail error message */
int zaprecv = 0;		 /* zap received lines */
int lessnoise = 0;		 /* run ``please add/delete me'' filter */
int forcereplyto = 0;		 /* ignore reply to */
int majordomo = 0;		 /* is NOT majordomo mode in default */
int useowner = 0;		 /* use owner instead of request for sender */
int xsequence = 0;		 /* add x-sequence header */
int addoriginator = 0;		 /* add originator field */
int accept_error = 0;		 /* not on accept list */
char *newstrim = NULL;		 /* trim news header */
char openc, closec;		 /* open/close bracket charcter */
char *precedence = NULL;	 /* precedensce value */
int debug = 0;			 /* debug flag */
int tersemode = 0;		 /* terse mode */
int writeindex = 0;		 /* write index */
int copyacceptfile = 0;		 /* copy recipient list to acceptlist*/

char *aliasid = NULL;		 /*  */
char *replyto = NULL;
char *list = NULL;		 /* Name of the list */
char *host = NULL;		 /* Name of the list's host */
char *senderaddr = NULL;	 /* sender address for Sender: line */
char *header;			 /* A pointer to a header */
char *sendmailargs = NULL;	 /* add'l args to sendmail */
char *issuefile = NULL;
char *recipfile = NULL;
char *acceptfile = NULL;
char *rejectfile = NULL;
char *headerfile = NULL;
char *footerfile = NULL;
char *archivedir = NULL;


/* Macros
 */
#define EQ(a, b) (strcasecmp((a), (b)) == 0)

#define	GETOPT_PATTERN	"a:cdef:h:ijl:m:n:oqr:stu:v:wxA:B:C:DF:H:I:L:M:N:OP:RS:U:VXY:Z:"

/* Forward Declarations
 */
void cleanheader __P((int headc, char** headv));
void write_index __P((void));
int acceptcheck __P((char*, char*));

/*
 * Check to see if a message appears to be an administrative one,
 * (e.g., an add-me or delete-me message).  Returns 0 if it's not,
 * 1 if it is.  Sets *newfile to contain a FILE * for the temp file
 * created to hold the message, and sets that file up so that the
 * message can be re-read from that file.
 * This code is based on signoff.c (by Russ Nelson and Rich Salz),
 * as included in the mail2news distribution.
 */
int
checkadmin(infile, newfile)
    FILE* infile;
    FILE** newfile;
{
    char *p, *s;
    int last_cr = 1;
    int drop_or_add = 0;
    int from_or_to = 0;
    int mail_word = 0;
    int ml_command_noise = 0;
    int maybe_ml_command_noise = 0;
    int mustbe_ml_command_noise = 0;
    int wcount = 0;
    int crcount = 0;
    int charcount = 0;
    int spam = 0;
    int bol = 1;
    int c, i;
    char word[128];

#define MIN_LINES	10
#define MIN_WORDS	20
#define MIN_CHARS	100
#define ISWSP(x)	((x == ' ') || (x == '\t'))

#ifdef HAVE_MKSTEMP
    char tmpnamebuf[MAXPATHLEN];
    strcpy(tmpnamebuf, "adminXXXXXX");
    mkstemp(tmpnamebuf);
    s = tmpnamebuf;
#else
    s = tmpnam((char *) NULL);
#endif
    if ((*newfile = fopen(s, "w+")) == NULL) {
	perror("can't get temp file");
	return(0);	/* better safe than sorry */
    }
    (void) unlink(s);

    p = word;

    while (1) {
	if ((c = getc(infile)) == EOF)
	    break;

	putc(c, *newfile);
	charcount++;

	if (c == '\n')
	    crcount++;

	if (last_cr) {
	    while (ISWSP(c) || (c == '\n') || (c == '#')) {
		if ((c = getc(infile)) == EOF)
		    break;
		putc(c, *newfile);
		charcount++;
		if (c == '\n')
		    crcount++;
	    }
	}

	if (ISWSP(c) || (c == '\n')) {
	    *p = '\0';
	    if (!strlen(word))
		continue;
	    if (p > word)
		wcount++;
	    p = word;
	    if (bol && crcount < MIN_LINES && wcount < MIN_WORDS && charcount < MIN_CHARS) {
		if (EQ(word, "Authenticated"))
		    spam++;
		else if (EQ(word, "from") || EQ(word, "to"))
		    from_or_to++;
		else if (EQ(word, "mail") || EQ(word, "mailing") ||
			 EQ(word, "list"))
		    mail_word++;
		else {
		    for (i = 0; mustbeMlCmdNoise[i] != NULL; i++) {
			if (EQ(word, mustbeMlCmdNoise[i])) {
			    mustbe_ml_command_noise++;
			    break;
			}
		    }
		    if (!mustbe_ml_command_noise) {
			for (i = 0; mlCmdNoise[i] != NULL; i++) {
			    if (EQ(word, mlCmdNoise[i])) {
				ml_command_noise++;
				break;
			    }
			}
		    }
		    if (!(mustbe_ml_command_noise || ml_command_noise)) {
			for (i = 0; maybeMlCmdNoise[i] != NULL; i++) {
			    if (EQ(word, maybeMlCmdNoise[i])) {
				maybe_ml_command_noise++;
				break;
			    }
			}
		    }
		}
	    }
	    bol = 0;
	}
	else if (p < &word[sizeof word - 1])
	    *p++ = c;

	if (c == '\n')
	    bol = 1;

	last_cr = c == '\n';
    }
    rewind(*newfile);

    /* Use fancy-shmancy AI techniques to determine what the message is. */
    return (spam || crcount < 2 || charcount < 20 ||
	    (mustbe_ml_command_noise &&
	     charcount < MIN_CHARS * 2 && crcount < MIN_LINES * 2) ||
	    ((ml_command_noise || (drop_or_add && from_or_to && mail_word)) &&
	     (charcount < MIN_CHARS && crcount < MIN_LINES)) ||
	    (maybe_ml_command_noise &&
	     charcount * 2 < MIN_CHARS && crcount * 2 < MIN_LINES));
}


/* Long argument handling
 */

/* init arguments
 */
void
arginit()
{
    ls_init(&cmdbuf);
}

/* reset arguments
 */
void
argreset()
{
    ls_reset(&cmdbuf);
}

/* Argcat catenates strings to the command buffer
 */
void
argappend(s)
    char *s;
{
    ls_appendstr(&cmdbuf, s);
}

void
argappend_quote(s)
    char *s;
{
    /* append items separated by quote.. slow.. */
    char* p = strsave(s);	/* must make copy, because strsep destroys.. */
    char* bp = p;
    char* sp;
    int count = 0;
    
    while ((sp = strsep(&bp, " ")) != NULL) {
	if (count != 0) {
	    ls_appendstr(&cmdbuf, " ");
	}
	ls_appendstr(&cmdbuf, "'");
	ls_appendstr(&cmdbuf, sp);
	ls_appendstr(&cmdbuf, "'");
	count++;
    }
}

/* return the "long" command string
 */   
char *
argget()
{
    return cmdbuf.ls_buf;
}


/* getaliaschar parses options of alias chars and returns
 * open and close chars
 */
int
getaliaschar(opench, closech, opt)
    char *opench, *closech;
    char *opt;
{
    int opterror = 0;

    switch (strlen(opt)) {
    case 1:	/* -B <typechar> */
	switch (opt[0]) {
	case 'c': case 'C': /* curly brace */
	    *opench = '{'; /* } { */
	    *closech = '}'; 
	    break;
	    
	case 'b': case 'B': /* bracket */
	    *opench = '[';
	    *closech = ']';
	    break;
	    
	case 'a': case 'A': /* angle bracket */
	    *opench = '<';
	    *closech = '>';
	    break;
	    
	case 'p': case 'P': /* paren (default)*/
	default:
	    *opench = '(';
	    *closech = ')';
	    break;
	}
	break;
	
    case 2:	/* -B <opench><closech> */
	if (opt[0] != opt[1]) {
	    *opench = opt[0];
	    *closech = opt[1];
	}
	else {
	    opterror++;
	}
	break;
	
    default:
	opterror++;
	break;
    }

    return opterror == 0;
}

/* Option parser + usage/version print related functions
 */

void
usage()
{
    fprintf(stderr, "usage: %s ", progname);
    fprintf(stderr, "{-M list | -N list | -h host -l list}\n");
    fprintf(stderr, "\t[-f senderaddr] [-H headerfile] [-F footerfile] [-n newsfrom]\n");
    fprintf(stderr, "\t[-r replytoaddr]");
#ifdef USE_ISSUE
    fprintf(stderr, " [-I issuenumfile]");
#endif
#ifdef USE_SUBJALIAS
    fprintf(stderr, " [-a aliasid] [-B brace_lr]");
#endif
    fprintf(stderr, "\n");
    fprintf(stderr, "\t[-P precedence] [-m sendmail-flags]\n");
    fprintf(stderr, "\t[-Z indexfile] [-v seqfile] [-I seqfile]\n");
    fprintf(stderr, "\t[-A accept-addr-file] [-S sendmail-path]\n");
    fprintf(stderr, "\t[-U archive-userid] [-u mailer-uid]\n");
    fprintf(stderr, "\t[-C archive-dir] [-Y archive-path]\n");
    fprintf(stderr, "\t[-DKORVXdeijnoqstuxwc] [-L recip-addr-file | recip-addr ...]\n");

}

void
printversion()
{
    fprintf(stderr, "distribute version %s ", versionID);
#ifdef RELEASESTATE
    fprintf(stderr, "(%s) ", RELEASESTATE);
#endif
    fprintf(stderr, "patchlevel %d\n", patchlevel);
    fprintf(stderr, "%s\n", rcsID);
    fprintf(stderr, "\nOptions:\n\t");
#ifdef SYSLOG
    fprintf(stderr, " [SYSLOG]");
#endif
#ifdef USE_ISSUE
    fprintf(stderr, " [ISSUE]");
#endif
#ifdef USE_MSC
    fprintf(stderr, " [MSC]");
#endif
#ifdef USE_MIMEKIT
    fprintf(stderr, " [MIMEKIT]");
#endif
#ifdef USE_SUBJALIAS
    fprintf(stderr, " [SUBJALIAS]");
#endif
#ifdef ADDVERSION
    fprintf(stderr, " [ADDVERSION]");
#endif
#ifdef DEBUGLOG
    fprintf(stderr, " [DEBUGLOG]");
#endif
    putc('\n', stderr);
    fprintf(stderr, "\t [MAXSUBJLEN=%d] [MAXHEADERLINE=%d] [MAXHEADERLEN=%d]\n",
	    MAXSUBJLEN, MAXHEADERLINE, MAXHEADERLEN);
    fprintf(stderr, "\nDefaults:\n");
#ifdef DEF_ALIAS_CHAR_OPTION
    fprintf(stderr, "\t Alias option: -B%s\n", DEF_ALIAS_CHAR_OPTION);
#endif
#ifdef DEF_DOMAINNAME
    fprintf(stderr, "\t Default Domain Name: %s\n", DEF_DOMAINNAME);
#endif
#ifdef DEF_ARCHIVE_PATH
    fprintf(stderr, "\t Default archive directory path: %s\n", DEF_ARCHIVE_PATH);
#endif
    fprintf(stderr, "\t Recipient file default path: %s\n", DEF_RECIPIENT_PATH);
    fprintf(stderr, "\t Sequence file default path:  %s\n", DEF_SEQ_PATH);
    fprintf(stderr, "\t Majordomo file default path:  %s\n", DEF_MAJORDOMO_RECIPIENT_PATH);
    fprintf(stderr, "\t Default sequence suffix:  %s\n", DEF_SEQ_SUFFIX);
    fprintf(stderr, "\t Default recipient suffix: %s\n", DEF_RECIPIENT_SUFFIX);
    fprintf(stderr, "\t Default accept suffix: %s\n", DEF_ACCEPT_SUFFIX);
    fprintf(stderr, "\t Default reject suffix: %s\n", DEF_REJECT_SUFFIX);
    fprintf(stderr, "\t Default index filename: %s\n", DEF_INDEX_NAME);
    exit(0);
}

void
parse_options(argc, argv)
    int argc;
    char **argv;
{
    extern char *optarg;

    int optionerror = 0;
    int c;

    while ((c = getopt(argc, argv, GETOPT_PATTERN)) != EOF) {
	switch(c) {
	case 'M':	/* generic mailinglist with reply-to */
	    errorsto++;
	    xsequence++;
	    aliasid = optarg;
	    replyto = optarg;
	    list = optarg;
	    issuefile = adddefaultpath(seq_path, optarg,
				       seq_suffix, 0);
	    if (majordomo)
		recipfile = adddefaultpath(majordomo_recipient_path,
					   optarg, "", majordomo);
	    else
		recipfile = adddefaultpath(recipient_path, optarg,
					   recipient_suffix, majordomo);
	    acceptfile = adddefaultpath(recipient_path, optarg,
					accept_suffix, 0);
	    rejectfile = adddefaultpath(recipient_path, optarg,
					reject_suffix, 0);
	    break;
	    
	case 'N':	/* generic maillinglist without reply-to */
	    errorsto++;
	    xsequence++;
	    aliasid = optarg;
	    list = optarg;
	    issuefile = adddefaultpath(seq_path, optarg,
				       seq_suffix, 0);
	    if (majordomo)
		recipfile = adddefaultpath(majordomo_recipient_path,
					   optarg, "", majordomo);
	    else
		recipfile = adddefaultpath(recipient_path, optarg,
					   recipient_suffix, majordomo);
	    acceptfile = adddefaultpath(recipient_path, optarg,
					accept_suffix, 0);
	    rejectfile = adddefaultpath(recipient_path, optarg,
					reject_suffix, 0);
	    break;
	    
	case 'j':	/* MAJORDOMO style recipient file path*/
	    majordomo++;
	    break;
	    
	case 'B':	/* brace def */
	    if (getaliaschar(&openc, &closec, optarg)) {
		openaliaschar = openc;
		closealiaschar = closec;
	    }
	    else {
		optionerror++;
	    }
	    break;
		    
	case 'h':	/* hostname */
	    host = optarg;
	    break;
	    
	case 'f':	/* sender address */
	    senderaddr = optarg;
	    break;
	    
	case 'R':	/* zap Received: lines */
	    zaprecv++;
	    break;
	    
	case 's':	/* be smart about add-me mail */
	    lessnoise++;
	    break;
	    
	case 'e':	/* errorsto header */
	    errorsto++;
	    break;
	    
	case 'l':	/* list name */
	    list = optarg;
	    break;
	    
	case 'H': 	/* file with header */
	    headerfile = optarg;
	    break;
	    
	case 'd':	/* debug */
	    debug++;
	    break;
	    
	case 'D':	/* print error message to stderr for debugging */
	    logging_setprinterror(1);
	    break;

	case 'F':	/* file with footer */
	    footerfile = optarg;
	    break;
	    
	case 'S':
	    sendmail_path = strsave(optarg);
	    break;
	    
	case 'm':	/* add'l args to sendmail */
	    if (sendmailargs == NULL)
		sendmailargs = strsave(optarg);
	    else
		sendmailargs = strspappend(sendmailargs, optarg);
	    break;
	    
#ifdef USE_MIMEKIT
	case 'w':
 	    usemimekit = !usemimekit;
	    break;
#endif

	case 'I':	/* add isssue number */
	case 'v':	/* add vol issue header */
	    issuefile = adddefaultpath(seq_path, optarg,
				       seq_suffix, 0);
	    break;
	    
	case 'i':	/* force ignore replyto */
	    forcereplyto++;
	    break;
	    
	case 'r':	/* replyto header */
	    replyto = optarg;
	    break;
	    
	case 'a':	/* alias-id */
	    aliasid = optarg;
	    break;
	    
	case 'o':	/* use owner */
	    useowner++;
	    break;

	case 'O':	/* add originator field */
	    addoriginator++;
	    break;

	case 'n':	/* trim news header */
	    newstrim = optarg;
	    break;

	case 'L':	/* recip-addr-file */
	    if (majordomo)
		recipfile = adddefaultpath(majordomo_recipient_path,
					   optarg,"", majordomo);
	    else
		recipfile = adddefaultpath(recipient_path, optarg,
					   recipient_suffix, 0);
	    break;
	    
	case 'A':	/* accept-addr-file, in recipient path */
	    acceptfile = adddefaultpath(recipient_path, optarg,
					accept_suffix, 0);
	    break;
	    
	case 'K':	/* reject-addr-file, in recipient path */
	    rejectfile = adddefaultpath(recipient_path, optarg,
					reject_suffix, 0);
	    break;
	    
	case 'P':	/* precedence */
	    precedence = optarg;
	    break;
	    
	case 'V':
	    printversion();
	    break;	/* notreached */
    
#ifdef ADDVERSION
	case 'X':
	    addversion = 0;
	    break;
#endif		    
	case 'C':
	    archivedir = optarg;
#ifndef OLD_ARCHIVE
	    writeindex++;
#endif
	    break;

	case 'Y':
	    archive_path = optarg;
	    writeindex++;
	    break;

	case 'x':
	    writeindex++;
	    break;

	case 'q':
	    xsequence++;
	    break;

#ifdef USESUID
	case 'u':
	    getuandgid(&mailer_uid, &mailer_gid, optarg);
	    break;
	    
	case 'U':
	    getuandgid(&archive_uid, &archive_gid, optarg);
	    break;
#endif

	case 't':
	    tersemode++;
	    break;

	case 'Z':
	    index_name = optarg;
	    break;

	case 'c':
	    copyacceptfile++;
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


    /*
     * We need at least the host name and the list name...
     */
    if (host == NULL || list == NULL || optionerror) {
	usage();
	logandexit(EX_USAGE, "require hostname and list name or bad usage");
    }

    if (copyacceptfile) {
	acceptfile = recipfile;	/* copy recip file path now */
    }
}

/* Initialize Distribute
 */

void
init_distribute()
{
    init_log("distribute");
    
    arginit();
    
    /* setup default */
#ifdef DEF_DOMAINNAME
    host = DEF_DOMAINNAME;
#else

#ifdef HAVE_GETHOSTNAME
    gethostname(myhostname, sizeof(myhostname));
#else
# ifdef HAVE_SYSINFO
    sysinfo(SI_HOSTNAME, myhostname, (long) sizeof(myhostname));
# else
    error
# endif
    
#endif
    host = myhostname;
#endif

#ifdef DEF_ALIAS_CHAR_OPTION
    if (getaliaschar(&openc, &closec, DEF_ALIAS_CHAR_OPTION)) {
	openaliaschar = openc;
	closealiaschar = closec;
    }
    else {
	logandexit(EX_NOINPUT,
		   "Error in compile-in default of -B%s\n",
		   DEF_ALIAS_CHAR_OPTION);
    }
#endif
}


/* main distributie functions
 */

int
parse_and_clean_header(file)
    FILE* file;
{
    /* Read all of the headers and make a header vector
     */
    headc = head_parse(MAXHEADERLINE, headv, file);

    /* Remove news-to-mail style extra headers if From: matches
     */
    if (newstrim) {
	char frombuf[MAXADDRLEN];
	if ((header = head_find(headc, headv, "From:")) != NULL) {
	    xstrncpy(frombuf, header, sizeof(frombuf));
	    header = normalizeaddr(frombuf);

	    if (header != NULL && strcmp(newstrim, header) == 0) {
		head_free(headc, headv);
		headc = head_parse(MAXHEADERLINE, headv, stdin);
	    }
	    else {
		badnewsheader = 1;
	    }
	}
    }

    /* Try to find message originator
     * Try Reply-To:, From:, Sender:, Return-Path: in this order.
     * then send to postmaster
     */
    if ((header = head_find(headc, headv, "Sender:")) != NULL) {
	header = normalizeaddr(header + sizeof("Sender:") - 1);
	if (header != NULL) {
	    xstrncpy(origMsg.sender, header, sizeof(origMsg.sender));
	}
	else {
	    origMsg.sender[0] = '\0';
	}
    }
    if ((header = head_find(headc, headv, "Return-Path:")) != NULL) {
	header = normalizeaddr(header + sizeof("Return-Path:") - 1);
	if (header != NULL) {
	    xstrncpy(origMsg.returnpath, header, sizeof(origMsg.returnpath));
	}
	else {
	    origMsg.returnpath[0] = '\0';
	}
    }
    if ((header = head_find(headc, headv, "From:")) != NULL) {
	header = normalizeaddr(header + sizeof("From:") - 1);
	if (header != NULL) {
	    xstrncpy(origMsg.from, header, sizeof(origMsg.from));
	}
	else {
	    origMsg.from[0] = '\0';
	}
    }
    if ((header = head_find(headc, headv, "Reply-To:")) != NULL) {
	header = normalizeaddr(header + sizeof("Reply-To:") - 1);
	if (header != NULL) {
	    xstrncpy(origMsg.replyto, header, sizeof(origMsg.replyto));
	}
	else {
	    origMsg.replyto[0] = '\0';
	}
    }

    if (origMsg.sender[0] != '\0')
	xstrncpy(originator, origMsg.sender, sizeof(originator));
    else if (origMsg.returnpath[0] != '\0')
	xstrncpy(originator, origMsg.returnpath, sizeof(originator));
    else if (origMsg.from[0] != '\0')
	xstrncpy(originator, origMsg.from, sizeof(originator));
    else if (origMsg.replyto[0] != '\0')
	xstrncpy(originator, origMsg.replyto, sizeof(originator));
    else
	strcpy(originator, "postmaster");

    /*
     * Clean header
     */
    cleanheader(headc, headv);
    

    /*
     * Delete the Reply-To: header
     */
    
    if (replyto != NULL) {
	header = head_delete(headc, headv, "Reply-To:");
	if (forcereplyto) {
	    if (header != NULL)
		free(header);
	}
	else {
	    originatorreplyto = header;
	}
    }
    
    /*
     * Parse Message-ID and Subject
     */
    if ((header = head_find(headc, headv, "Message-ID:")) != NULL) {
	strcpycut(messageid, skiptononspace(header + sizeof("Message-ID:")-1),
		  sizeof(messageid));
	chopatlf(messageid);	/* remove extra lines */
	changech(messageid, ' ', '_'); /* insure no space in Message-ID */
    }
    else
	strcpy(messageid, "<No_Message_ID_in_original>");

    if ((header = head_find(headc, headv, "Subject:")) != NULL) {
	strcpycut(subject, skiptononspace(header+sizeof("Subject:")-1),
		  sizeof(subject));
	head_delete(headc, headv, "Subject:");
    }
    else
	strcpy(subject, "(No Subject in original)");
    
    /*
     * Delete all the Received: lines, if the user said to do so.
     */
    if (zaprecv) {
	while (head_delete(headc, headv, "Received:") != NULL)
	    ;
    }
    
    /* set maintainer
     */
    if (senderaddr != NULL) {
	xstrncpy(maintainer, senderaddr, sizeof(maintainer));
    }
    else {
	if (majordomo || useowner)
#ifdef HAVE_SNPRINTF
	    snprintf(maintainer, sizeof(maintainer), "owner-%s", list);
#else
	    sprintf(maintainer, "owner-%s", list);
#endif
	else
#ifdef HAVE_SNPRINTF
	    snprintf(maintainer, sizeof(maintainer), "%s-request", list);
#else
	    sprintf(maintainer, "%s-request", list);
#endif
    }

    /* Delete the Precedence: header.
     */
    if (precedence != NULL) {
     	if ((header = head_delete(headc, headv, "Precedence:")) != NULL)
	    free(header);
    }

    if (strchr(maintainer, '@') == NULL)
#ifdef HAVE_SNPRINTF
	snprintf(dommaintainer, sizeof(dommaintainer),
		 "%s@%s", maintainer, host);
#else
	sprintf(dommaintainer, 
		 "%s@%s", maintainer, host);
#endif
    else
	xstrncpy(dommaintainer, maintainer, sizeof(dommaintainer));

    return headc;
}


void
prepare_arguments(argc, argv)
    int argc;
    char **argv;
{
    extern int optind;
    
    int i;

    /*
     * Create the command that we are about to run.  We use the sender
     * address given to us by the user (if we were given one); otherwise,
     * the sender address is the list name with the usual -request
     * tacked on.
     */
    
    /* Read-in all recipient files
     */
    if (acceptfile != NULL) {
	acceptbuf = parserecipfile(acceptfile, 0);
    }
    if (recipfile != NULL) {
	recipientbuf = parserecipfile(recipfile, 1);
    }
    if (rejectfile != NULL) {
	rejectbuf = parserecipfile(rejectfile, 0);
    }

#ifdef USE_CCMAIL
    if (strcmp(subject, "cc:Mail SMTPLINK Undeliverable Message") == 0
	|| strcmp(subject, "Message not deliverable") == 0
	|| strcmp(subject, "Unsent Message Returned to Sender") == 0
	|| strncmp(subject, "cc:Mail Link to SMTP Undeliverable Message", 42) == 0
	|| strncmp(subject, "NON-DELIVERY of:", 16) == 0
	|| strncmp(subject, "NDN:", 4) == 0) {

	ccmail_error_message = 1;
    }
#endif

    /* read and add recipient list if exits
     */
    if (acceptcheck(acceptbuf, origMsg.sender) == 0
	&& acceptcheck(acceptbuf, origMsg.from) == 0) {
	accept_error = 1;
    }

    /* Reject check
     * not necessary to copy rejectbuf -- becaust I'm last user.
     */
    if (origMsg.from != NULL) {
	char *rejaddr;
	int fromlen = strlen(origMsg.from);
	while ((rejaddr = strsep(&rejectbuf, " ")) != NULL) {
	    int rejlen = strlen(rejaddr);
	    if (fromlen > rejlen) {
		if (!strncasecmp(origMsg.from, rejaddr, rejlen)) {
		    wasrejected = 1;
		    break;
		}
	    }
	}
    }
    
#if 0
    /*
     * Simple check that the header fields aren't grossly
     * mismatched in terms of parens and angle-brackets.
     */
    for (i = 0; i < headc; i++) {
	if (headv[i] != NULL) {
	    if (checkhdr(headv[i]) == 0) {
		badhdr++;
		break;
	    }
	}
    }
#endif

    /*
     * Give the command its input.
     * Deal with being smart about add-me mail here.
     */
    if (lessnoise) {
	wasadmin = checkadmin(stdin, &noisef);
    }

    /* Now build sendmail command from recipient lists..
     */

    argreset();
    
#ifdef OLD_ARCHIVE
    /*
     * If archiving is requested, change current directory to the
     * directory specified by option '-C', and make temporary file.
     * Archive message is written by 'tee' command. Finaly rename
     * this temporary file as *issue number* file.
     */
    if (archivedir) {
	archivedir = adddefaultpath(archive_path, archivedir, "", majordomo);
	if (chdir(archivedir) == -1) {
	    logerror("%s: cannot change directory\n", archivedir);
	    archivedir = NULL;
	} else {
	    int fd;
	    strcpy(archivetmp, "msgXXXXXX");
#ifdef HAVE_MKSTEMP
	    mktemp(archivetmp);
#else
	    mktemp(archivetmp);
#endif
	    if (debug == 0 && (fd = creat(archivetmp, 0644)) == -1) {
		logerror("%s: cannot make file\n", archivetmp);
		archivedir = NULL;
	    } else {
		if (debug == 0)
		    close(fd);
		argappend(TEE_COMMAND);
		argappend(" ");
		argappend(archivetmp);
		argappend("|");
	    }
	}
    }
#endif

    argappend(sendmail_path);
    argappend(" ");

    if (sendmailargs != NULL)
	argappend(sendmailargs);

    argappend(" -f");
    argappend(dommaintainer);
    
    /* only to maintainer */    
    if (badnewsheader || badoriginator || ccmail_error_message) {
	argappend(" ");
	if (senderaddr != NULL) {
	    argappend(senderaddr);
	}
	else {
	    argappend(dommaintainer);
	}
    }

    /* originator + maintainer */
    else if (badhdr || accept_error || wasrejected || wasadmin) {
	argappend(" ");
	if (senderaddr != NULL) {
	    argappend(senderaddr);
	}
	else {
	    argappend(dommaintainer);
	}
	argappend(" ");
	argappend(originator);

	/* debugging aid */
#ifdef REPORTADDR
	argappend(" ");
	argappend(reportaddr);
#endif
    }

    /* If no error, append regular recipients */
    else {
	if (recipientbuf != NULL) {
	    argappend(" ");
	    argappend_quote(recipientbuf);
	}

	for (i = optind ; i < argc ; i++) {
	    argappend(" ");
	    argappend(argv[i]);
	}
    }
}

int
acceptcheck(buf, pat)
    char *buf;
    char *pat;
{
    char *p;
    int len;

    if (buf == NULL)		/* accept all if no table */
	return 1;

    if (buf[0] == '\0')		/* accept NONE if EMPTY table */
	return 0;

    if (pat == NULL || pat[0] == '\0')
	return 0;

    p = strstr(buf, pat);
    
    if (p == NULL)		/* no match */
	return 0;		/* fail */

    len = strlen(pat);

    /* don't match middle of word.. paranoia? */
    if ((p[len] == ' ' || p[len] == '\0')) {
	if (p == buf)		/* at beginning */
	    return 1;
	else if (p[-1] == ' ')	/* at middle (require space before it) */
	    return 1;
	else			/* else fail */
	    return 0;
    }
    return 0;
}

#ifdef USE_MIMEKIT
/*
 * MIME_makeSubj -- TBW
 */
char*
MIME_makeSubj(s)
    char* s;
{
    /* these are in mime-kit */
    extern int MIME_SPACE_ENCODING;
    extern void MIME_strHeaderDecode(char*, char*, int);
    extern void MIME_strHeaderEncode(char*, char*, int);

    char *p, *t;
    char q[MAXSUBJLEN], r[MAXSUBJLEN * 3];

    if (strstr(s, "\e\e")) {
	return s;		/* mimekit doesn't handle it */
    }
    if (strstr(s, "\n\n")) {
	return "";
    }
    
    MIME_SPACE_ENCODING = 1;
    MIME_strHeaderDecode(s, q, sizeof(q));
    MIME_SPACE_ENCODING = 0;
    MIME_strHeaderEncode(q, r, sizeof(r));

    p = (char*) malloc(sizeof(r));
    if (p == NULL)
	return s;		/* NG -- return as is. */

    xstrncpy(p, r, sizeof(r) - 1);
 
    for (t = p; t = strchr(t, '\n'); t++) {
 	switch (*(t+1)) {
 	case '\0':
 	    *t = '\0';
 	    break;
 	case '\n':
 	    strcpy(t, t+1);
 	    break;
 	case ' ':
 	case '\t':
 	    if (*(t+2) == '\n')
 		strcpy(t, t+2);
 	    break;
 	default:
 	    if (!ISWSP(*(t+1)))
 		return s;
 	}
    }
    return p;
}
#endif


/* AddAliasIDToHeader -- This function format then insert X-SOMETHING style
 * identifier in header.
 */
#ifdef USE_ISSUE
void
AddAliasIDToHeader(pipe, aliasid, issuenum)
    FILE *pipe;
    char *aliasid;
    int issuenum;
{
#ifndef USE_MSC			/* NON-MSC case */
    if (aliasid != NULL) {
	fprintf(pipe, "X-Sequence: %s %d\n",aliasid,issuenum);
    }
    else {
	fprintf(pipe, "X-Sequence: %d\n",issuenum);
    }
#endif
#ifdef USE_MSC
    if (aliasid != NULL) {
	fprintf(pipe, "X-Ml-Count: %05d\n",issuenum);
	fprintf(pipe, "X-Ml-Name: %s\n",aliasid);
    }
    else {
	fprintf(pipe, "X-Ml-Count: %05d\n",issuenum);
    }
#endif
}
#endif


/* AddAliasIDToSubject -- This function formats Subject: line using original
 * subject, alias ID and issue number (if needed)
 */
#ifdef USE_ISSUE
void
AddAliasIDToSubject(subjectbuf, subjectbuf_size, subject, aliasid, issuenum)
    char *subjectbuf;
    int subjectbuf_size;
    char *subject;
    char *aliasid;
    int issuenum;
{
    if (aliasid != NULL && !tersemode) { /* only if non-terse mode */
	char *p = subject;
	int l;
	char buf[MAXSUBJLEN];		/* must be enough. */
	char *openfmt, *subjfmt;
	char seprchr;
	
#ifndef USE_MSC
	openfmt = "%c%s";
	seprchr = ' ';
	subjfmt = "%c%s %d%c %s";
#endif
#ifdef USE_MSC
	openfmt = "%c%s";
	seprchr = ',';
	subjfmt = "%c%s,%05d%c %s";
#endif

	sprintf(buf, openfmt, openaliaschar, aliasid);
	
	l = strlen(buf);
	while (strlen(p) > l + 1) {
	    if (strncmp(p, buf, l) == 0 && (issuenum < 0 ||
			(p[l] == seprchr && isdigit(p[l + 1])))) {
		register char *pp = p, *s = p + l + (issuenum >= 0 ? 2 : 0);
		while (isdigit(*s))
		    s++;
		if (*s == closealiaschar) {
		    s++;
		    while (isspace(*s))
			s++;
		    if (strncmp(s, "Re:", 3) == 0 ||
			strncmp(s, "RE:", 3) == 0) {
			s += 3;
			while (isspace(*s))
			    s++;
		    }
		    do {
			*pp++ = *s;
		    } while (*s++);
		} else {
                    p = s;
                }
	    } else
		p++;
	}
	
	if (issuenum >= 0) {	/* with valid issue number, or error(0) */
#ifdef HAVE_SNPRINTF
	    snprintf(subjectbuf, subjectbuf_size,
		     subjfmt,
		     openaliaschar,
		     aliasid, issuenum,
		     closealiaschar,
		     subject);
#else
	    sprintf(subjectbuf,
		     subjfmt,
		     openaliaschar,
		     aliasid, issuenum,
		     closealiaschar,
		     subject);
#endif
	}
	else {
#ifdef HAVE_SNPRINTF
	    snprintf(subjectbuf, subjectbuf_size,
		     "%c%s%c %s",
		     openaliaschar,
		     aliasid,
		     closealiaschar,
		     subject);
#else
	    sprintf(subjectbuf,
		     "%c%s%c %s",
		     openaliaschar,
		     aliasid,
		     closealiaschar,
		     subject);
#endif
	}
    }
    else {
	xstrncpy(subjectbuf, subject, subjectbuf_size);
    }
}
#endif

int
send_message()
{
    int reject = 0;
    int i;
    char buf[BUFSIZ];	/* string buffer */
    
    FILE *pipe = NULL, *headf = NULL, *footf = NULL;

    int dummysum;	/* to keep sum compatible with older versions */
    
    /* Now, check external configuration file existence then open files
     */
    if (headerfile != NULL) {
	if ((headf = fopen(headerfile, "r")) == NULL) {
	    logandexit(EX_NOINPUT,
		       "can't open header file '%s'\n", headerfile);
	}
    }
    
    if (footerfile != NULL) {
	if ((footf = fopen(footerfile, "r")) == NULL) {
	    logandexit(EX_NOINPUT,
		       "can't open footer file '%s'\n", footerfile);
	}
    }
    
    /*
     * Start this command running.
     */
    
    if (debug) {
	pipe = stdout;
	printf("Command: %s\n", argget());
    }
    else {
	pipe = popen(argget(), "w");
	if (pipe == NULL) {
	    logandexit(EX_UNAVAILABLE, "popen to sendmail failed");
	}
    }
    
    /* If something wrong going on. Bounce the mail to maintainer or
     * originator.
     */
    if (badnewsheader) {
	messageprint(pipe, notanews, dommaintainer);
	logwarn("Unsent message: Not a news article. Forwarded to %s",
		dommaintainer);
	reject = 1;
    } else if (badhdr) {		/* if header is bad */
	messageprint(pipe, badheader, originator, headererr);
	logwarn("Unsent message: Header problem. Bounce to %s and %s",
		originator, dommaintainer);
	reject = 1;
    } else if (wasadmin) {		/* if header is bad */
	messageprint(pipe, administrative, dommaintainer);
	logwarn("Unsent message: Administrative message. Forwarded to %s",
		dommaintainer);
	reject = 1;
    } else if (accept_error) {
	messageprint(pipe, notallowed, originator);
	logwarn("Unsent message: Can't accept message from %s. Bounce to %s also",
		originator, dommaintainer);
	reject = 1;
    } else if (wasrejected) {
	messageprint(pipe, rejected, originator);
	logwarn("Unsent message: Rejected message from %s. Forwarded to %s",
		originator, dommaintainer);
	reject = 1;
    } else if (badoriginator) {
	messageprint(pipe, badoriginator, dommaintainer);
	logwarn("Unsent message: Originator address invalid. Forwarded to %s",
		dommaintainer);
	reject = 1;
    }

#ifdef USE_CCMAIL
    if (ccmail_error_message) {
	messageprint(pipe, ccmailerr, originator);
	logwarn("Unsent message: cc:Mail SMTPLINK Undeliverable Message from %s", originator);
	reject = 1;
    }
#endif

#ifdef USE_ISSUE
    if (reject) {
	issuenum = 0;
    }
    else if (issuefile == NULL) {
	issuenum = -1;
    }
    else {
	int getnextissue();
	issuenum = getnextissue(issuefile);
    }
#endif

    /* Put out the headers.
     */
    for (i=0; i<headc; i++) {
	if (headv[i] != NULL) {
	    fputs(headv[i], pipe);
	    putc('\n', pipe);
	}
    }
    
    /* Add a new Reply-To.
     */
    if (replyto != NULL){
	if (!forcereplyto && originatorreplyto != NULL) {
	    fputs(originatorreplyto, pipe);
	    putc('\n', pipe);
	    free(originatorreplyto); /* sanity */
	}
	else {
	    if (strchr(replyto,'@') == NULL)
		fprintf(pipe, "Reply-To: %s@%s\n", replyto, host);
	    else
		fprintf(pipe, "Reply-To: %s\n", replyto);
	}
    }
    
    /* Add Precedence.
     */
    if (precedence != NULL) {
	fprintf(pipe, "Precedence: %s\n", precedence);
    }

#ifdef ADDVERSION
    /*
     * Add X-distribute
     */
    if (addversion) {
	fprintf(pipe, "X-Distribute: distribute [version %s", versionID);
#ifdef RELEASESTATE
	fprintf(pipe, " (%s)", RELEASESTATE);
#endif
	fprintf(pipe, " patchlevel=%d]\n",patchlevel);
    }	
#endif	
    
#ifdef USE_ISSUE
    if (issuenum >= 0) {
	AddAliasIDToHeader(pipe, aliasid, issuenum);
    }
#endif
    
#ifdef USE_MIMEKIT
    if (usemimekit) {
	xstrncpy(subject, MIME_makeSubj(subject), sizeof(subject));
    }
#endif
    
#ifdef USE_SUBJALIAS
    AddAliasIDToSubject(subjectbuf, sizeof(subjectbuf),
			subject, aliasid, issuenum);
#else
    xstrncpy(subjectbuf, subject, sizeof(subjectbuf));
#endif

#ifdef USE_MIMEKIT
    if (usemimekit) {
	xstrncpy(subjectbuf, MIME_makeSubj(subjectbuf), sizeof(subjectbuf));
    }
#endif

    fprintf(pipe, "Subject: %s\n", subjectbuf);
    
#ifdef DEBUGLOG
    fprintf(debuglog, "Command: %s\n", argget());
    fprintf(debuglog, "Subject: %s\n", subjectbuf);
#endif
    
    if (errorsto) {
	fprintf(pipe, "Errors-To: %s\n", dommaintainer);
    }
    
    /* Add a new Sender: field as requested earlier.
     */
    fprintf(pipe, "Sender: %s\n", dommaintainer);
    if (addoriginator)
	fprintf(pipe, "X-Originator: %s\n", originator);
   
    /* add a blank line separating the header lines from the body of
     * the message.
     */
    putc('\n', pipe);
    
    /* If message have not reject, then write index here.  (since we
     * have subject information here! -- We assume here that sendmail
     * will not fork shell(in this case, archive) before closing pipe. 
     * so archive will run AFTER we correctly wrote index.
     */
    if (reject == 0) {
	write_index();		/* write out index if succeed */
    }
    
    /* Dump the message thru the pipe.  We push out the header(leader),
     * then the message body, then the footer.
     */
    bodysum = 0;
    if (headf != NULL) {
	while (fgets(buf, sizeof buf, headf) != NULL) {
	    /* dot stuffing, but do not sum '.' for backward compatibility */
	    if (*buf == '.') {
		fputs_sum(".", pipe, &dummysum);
	    }
	    fputs_sum(buf, pipe, &bodysum);
	}
    }
    
    while (fgets(buf, sizeof buf, noisef == NULL ? stdin : noisef) != NULL) {
	/* dot stuffing, but do not sum '.' for backward compatibility */
	if (*buf == '.') {
	    fputs_sum(".", pipe, &dummysum);
	}
	fputs_sum(buf, pipe, &bodysum);
    }
    
    if (footf != NULL) {
	while (fgets(buf, sizeof buf, footf) != NULL) {
	    /* dot stuffing, but do not sum '.' for backward compatibility */
	    if (*buf == '.') {
		fputs_sum(".", pipe, &dummysum);
	    }
	    fputs_sum(buf, pipe, &bodysum);
	}
    }
    

    /* clean-up
     */
    pclose(pipe);
    

#ifdef OLD_ARCHIVE
    if (debug == 0 && archivedir) {
	if (issuenum) {
	    sprintf(archivetgt, "%d", issuenum);
	    rename(archivetmp, archivetgt);
	}
	else {
	    unlink(archivetmp);
	}
    }
#endif

    return reject == 0;
}


void
cleanheader(headc, headv)
    int headc;
    char **headv;
{
    int i;
    char *header;		/* A pointer to a header */
	
    /*
     * Make sure that the first space character in each header line
     * is a blank
     */

    for (i=0; i<headc; i++)
	head_blank(headv[i]);
    
    /*
     * The transformations we need to make here are not completely clear.
     * The following table attempts to describe our transformations:
     *
     * 	Field			Action
     * 	-----			------
     *
     *	From_			delete; this is a new message
     *	From:			leave alone; shows the real sender
     *	To:			leave alone; shows the list name
     *	Cc:			leave alone
     * 	Date:			leave alone

     *	Reply-To:		delete and add our own (if "-r")
     *	Message-Id:		leave alone
     *	Return-Receipt-To:	delete to avoid skillions of receipts
     *	Errors-To:		delete (add our own if "-e")
     *	Return-Path:		delete (not needed?)
     *	Received:		optional; can generate spurious
     *				    bounces if sendmail sees too many,
     *				    but is useful for debugging.
     *	Sender:			delete and add our own
     *	Resent-Sender:		leave alone, I suppose.  We should
     *				    think about deleting Resent-*
     *				    (since those fields will in some
     * 				    cases take precedence over our
     *				    own), but not now.  The handling
     *				    of Resent-* fields needs to be
     *				    the topic of a new RFC...
     *  Precedence:		delete and add our own (if -P)
     *	X-Sequence:		delete and add our own
     *	X-Distribute:		our version header
     */
    
    /* 
     * Delete the From_ header.
     */
    if ((header = head_delete(headc, headv, "From ")) != NULL)
	free(header);
    
    /*
     * Delete the Sender: line.
     */
    while ((header = head_delete(headc, headv, "Sender:")) != NULL)
	free(header);
    
    /*
     * Delete the Return-Receipt-To: header.
     */
    while ((header = head_delete(headc, headv, "Return-Receipt-To:")) != NULL)
	free(header);
    
    /*
     * Delete the Errors-To: header.
     */
    while ((header = head_delete(headc, headv, "Errors-To:")) != NULL)
	free(header);
    
    /*
     * Delete the Return-Path: header.
     */
    while ((header = head_delete(headc, headv, "Return-Path:")) != NULL)
	free(header);
    
    /*
     * Delete the X-Volume-Issue
     */
    while ((header = head_delete(headc, headv, "X-Sequence:")) != NULL)
	free(header);
    
    /*
     * Delete X-Distribute (regardless of My configuration)
     */
    while ((header = head_delete(headc, headv, "X-Distribute:")) != NULL)
	free(header);
}



/* getnextissue -- returns next available issue number
 * do exclusive lock also.
 */
#ifdef USE_ISSUE
int
getnextissue(filename)
    char *filename;
{
    char buf[128];		/* enough to hold number */
    int fd;
    int issue;
    char *p;

    if ((fd = open(filename,O_RDWR)) < 0) {
	if ((fd = creat(filename, 0664)) < 0) {
	    logandexit(EX_NOINPUT,
		       "can't create issue file: %s", filename);
	}
	write(fd,"1\n",2);
	close(fd);
	return 1;
    }
#ifdef HAVE_FLOCK
    flock(fd, LOCK_EX);
#else
# ifdef HAVE_LOCKF
    lockf(fd, F_LOCK, 0);
# endif
#endif
    read(fd, buf, sizeof(buf));

    if ((p = strchr(buf,'\n')) != NULL) {	/* failsafe */
	*p = '\0';
	issue = atoi(buf);
	issue++;
    }
    else {
	/* wrong format. reset it */
	issue = 1;
    }
    lseek(fd, 0L, L_SET);
    sprintf(buf, "%d\n", issue);
    write(fd,buf,strlen(buf));
#ifdef HAVE_FLOCK
    flock(fd, LOCK_UN);
#else
# ifdef HAVE_LOCKF
    lockf(fd, F_ULOCK, 0);
# endif
#endif
    close(fd);
    return issue;
}
#endif

/*
 * Check for mismatched parens, angle-brackets, and fields too long for
 * cranky old Sun sendmails.  We presume that the old sendmails can take
 * lines of up to 128 characters, and fields of up to 1024 characters.
 * The latter may be somewhat optimistic.
 */
/*
 * Modified by hchikara@nifs.ac.jp  1998.12.5, modified and merged by shigeya.
 *  1. brackets quoted by "" is not count
 */
char *
checkhdr1(s)
    register char *s;
{
    int nparens = 0;
    int nangles = 0;
    int nquote = 0;
    int linelen = 0, totlen = 0;
    int has_angle = 0;
    int last_char = 0;
    
    if (strncasecmp(s, "To:", 3) != 0
	&& strncasecmp(s, "Cc:", 3) != 0) {
	return NULL;		/* is NOT To/Cc, just pass it */
    }
    
    while (*s != '\0') {
	linelen++;
	totlen++;

	if (*s == '\n') {
	    linelen = 0;
	}
	else if (nquote) {
	    if (*s == '"' && last_char != '\\')
		nquote = 0;
	    /* pass character inside quote.. */
	}
	else {
	    switch(*s) {
	    case '"':
		if (last_char != '\\') { /* if and only if non-quoted char */
		    nquote++;
		}
		break;
		
	    case '(':
		nparens++;
		break;
	    
	    case ')':
		if (nparens <= 0)
		    return "less open paren";
		nparens--;
		break;
		
	    case '<':
		has_angle = 1;
		nangles++;
		break;
	    
	    case '>':
		if (nangles <= 0)
		    return "less open angle";
		nangles--;
		break;
	    }
	}
	last_char = *s++;

	if (totlen > MAXHEADERLEN-1) {/* only check this.. */
	    static char buf[80];
	    sprintf(buf, "too long header (len=%d)", totlen);
	    return buf;
	}
    }

    if (nangles == 0 && nparens == 0)
	return NULL;
    if (nquote)
	return "double quote mismatch";

    return "angle/bracket mismatch";
}

/*
 * Check the headers.
 */
int
checkhdr(s)
    register char *s;
{
    /* using normalizeaddr to see the field is correct.. */
    if (strncasecmp(s, "From:", 5) == 0) { /* From is special.. */
	return normalizeaddr(s + sizeof("From")-1) != NULL;
    }
    return checkhdr1(s) == NULL;
}


/* Write index file to archive directory
 */
void
write_index()
{
    /* write history now. If issuenum == 0, it's error so don't write index.
     */
    if (writeindex && issuenum != 0) {
	char *dir = makearchivepath(archive_path, archivedir, aliasid);
	if (chdir(dir) == -1) {
	    logerror("%s: cannot change directory\n", archivedir);
	    archivedir = NULL;
	}
	else {
	    char sbuf[MAXSUBJLEN];
	    xstrncpy(sbuf, subjectbuf, sizeof(sbuf));
	    changech(sbuf, '\n', ' '); /* don't want LF in history file*/

	    (void)openhistory(index_name, "a+");
	    appendhistory(issuenum, bodysum, messageid, sbuf);
	    closehistory();
	}
    }
}


/* Finally, the Main Entry
 */
int
main(argc, argv)
    int argc;
    char ** argv;
{
    chdir("/tmp");
    progname = argv[0];
    
    init_distribute();
    parse_options(argc, argv);

    headc = parse_and_clean_header(stdin);
    prepare_arguments(argc, argv);

    send_message();	/* really send message */

    loginfo("\"%s\" sent", subjectbuf);

    return 0;	/* exit(0) */;
}

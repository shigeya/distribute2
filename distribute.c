/* $Id$ */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sysexits.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/param.h>

#ifdef SYSLOG
# include <syslog.h>
#endif

#include "patchlevel.h"		/* version identifier */
#include "longstr.h"

/*
 * Send a mail message to users on a distribution list.  The message
 * header is altered mainly to allow error messages to be returned to
 * the list maintainer at the list host.
 *
 * Usage:
 *
 * Put an entry into /etc/aliases of the following form:
 *
 *	list-name: :include:/etc/mail-list/list-name-dist
 *
 * The list-name-dist file should be as the following:
 *
 *	| /usr/local/bin/distribute options user1 user2 ...
 */

/*
 * Modified by:
 *
 *	Shin Yoshimura		<shin@wide.ad.jp>
 *	Yoshitaka Tokugawa	<toku@dit.co.jp>
 *	Shigeya Suzuki		<shigeya@foretune.co.jp>
 *	Hiroaki Takada		<hiro@is.s.u-tokyo.ac.jp>
 */

char *rcsID = "$Id$";
char *versionID = VERSION;
int patchlevel = PATCHLEVEL;

/* Default configuration check and define if not defined
 */
#ifndef DEF_SEQ_PATH
# define DEF_SEQ_PATH		"/usr/lib/mail-list"
#endif

#ifndef DEF_RECIPIENT_PATH
# define DEF_RECIPIENT_PATH	"/usr/lib/mail-list"
#endif

#ifndef DEF_MAJORDOMO_RECIPIENT_PATH
# define DEF_MAJORDOMO_RECIPIENT_PATH "/usr/lib/mail-list/majordomo/lists"
#endif

#ifndef DEF_SEQ_SUFFIX
# define DEF_SEQ_SUFFIX		".seq"
#endif

#ifndef DEF_RECIPIENT_SUFFIX
# define DEF_RECIPIENT_SUFFIX	".rec"
#endif

#ifdef MSC
# define DEF_OPENALIAS_CHAR	'['
# define DEF_CLOSEALIAS_CHAR	']'
#endif

#ifndef DEF_OPENALIAS_CHAR
# define DEF_OPENALIAS_CHAR	'('
#endif
#ifndef DEF_CLOSEALIAS_CHAR
# define DEF_CLOSEALIAS_CHAR	')'
#endif

#ifndef SYSLOG_FACILITY
# define SYSLOG_FACILITY	LOG_LOCAL4
#endif

#define	MAXSUBJLEN	1024	/* we assume subject will no
				 * longer than this length */


extern	int head_parse();
extern	void head_norm();
extern	char * head_find();
extern	char * head_delete();
extern	char * tmpnam();
extern	char * index();
extern	char * rindex();

extern char * parserecipfile();


char *progname;

struct longstr cmdbuf;

#ifdef DEBUGLOG
FILE* debuglog;
#endif

/* constants
 */
char *seq_path = DEF_SEQ_PATH;
char *recipient_path = DEF_RECIPIENT_PATH;
char *majordomo_recipient_path = DEF_MAJORDOMO_RECIPIENT_PATH;

char *seq_suffix = DEF_SEQ_SUFFIX;
char *recipient_suffix = DEF_RECIPIENT_SUFFIX;

#ifndef DEF_DOMAINNAME
char myhostname[MAXHOSTNAMELEN];
#endif

char openaliaschar = DEF_OPENALIAS_CHAR;
char closealiaschar = DEF_CLOSEALIAS_CHAR;


#define EQ(a, b) (strcasecmp((a), (b)) == 0)


#define	GETOPT_PATTERN	"M:N:B:h:f:l:H:F:m:v:I:r:a:L:RsdeijVA"

void
usage() {
    fprintf(stderr, "usage: %s ", progname);
    fprintf(stderr, "{-M list | -N list | -h host -l list} [-f senderaddr]\n");
    fprintf(stderr, "	[-H headerfile] [-F footerfile]\n");
    fprintf(stderr, "	[-r replytoaddr ]");
#ifdef ISSUE
    fprintf(stderr, " [-I issuenumberfile]");
#endif
#ifdef SUBJALIAS
    fprintf(stderr, " [-a aliasid] [-B brace_lr]");
#endif
    fprintf(stderr, "\n");
    fprintf(stderr, "	[-RsdeijVA] [-m sendmail-flags]\n");
    fprintf(stderr, "	{-L recip-addr-file | recip-addr ...}\n");
}

printversion()
{
    fprintf(stderr, "distribute version %s ", versionID);
#ifdef RELEASESTATE
    fprintf(stderr, "(%s) ", RELEASESTATE);
#endif
    fprintf(stderr, "patchlevel %d\n", patchlevel);
    fprintf(stderr, "%s\n", rcsID);
    fprintf(stderr, "\nOptions:");
#ifdef SYSLOG
    fprintf(stderr, " [SYSLOG]");
#endif
#ifdef ISSUE
    fprintf(stderr, " [ISSUE]");
#endif
#ifdef MSC
    fprintf(stderr, " [MSC]");
#endif
#ifdef SUBJALIAS
    fprintf(stderr, " [SUBJALIAS]");
#endif
#ifdef ADDVERSION
    fprintf(stderr, " [ADDVERSION]");
#endif
#ifdef DEBUGLOG
    fprintf(stderr, " [DEBUGLOG]");
#endif
    fprintf(stderr, "\nDefaults:\n");
#ifdef DEF_ALIAS_CHAR_OPTION
    fprintf(stderr, "\tAlias option: -B%s\n", DEF_ALIAS_CHAR_OPTION);
#endif
#ifdef DEF_DOMAINNAME
    fprintf(stderr, "\tDefault Domain Name: %s\n", DEF_DOMAINNAME);
#endif
    fprintf(stderr, "\tRecipient file default path: %s\n", DEF_RECIPIENT_PATH);
    fprintf(stderr, "\tSequence file default path:  %s\n", DEF_SEQ_PATH);
    fprintf(stderr, "\tMajordomo recipient file default path:  %s\n", DEF_MAJORDOMO_RECIPIENT_PATH);

    exit(0);
}


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
	FILE *infile;
	FILE **newfile;
{
	char *p, *s;
	int drop_or_add = 0;
	int from_or_to = 0;
	int mail_word = 0;
	int count = 0;
	int c;
	char word[128];

	s = tmpnam(NULL);
	if ((*newfile = fopen(s, "w+")) == NULL) {
		perror("can't get temp file");
		return(0);	/* better safe than sorry */
	}
	(void) unlink(s);
	for (p = word ; (c = getc(infile)) != EOF; ) {
		putc(c, *newfile);
		if (!isalpha(c)) {
			*p = '\0';
			if (p > word)
				count++;
			p = word;

			if (EQ(word, "remove") || EQ(word, "drop") ||
			    EQ(word, "off") || EQ(word, "subscribe") ||
			    EQ(word, "get") || EQ(word, "add"))
				drop_or_add++;
			else if (EQ(word, "from") || EQ(word, "to"))
				from_or_to++;
			else if (EQ(word, "mail") || EQ(word, "mailing") ||
			    EQ(word, "list") || EQ(word, "dl"))
				mail_word++;
		}
		else if (p < &word[sizeof word - 1])
			*p++ = c;
	}
	rewind(*newfile);

	/* Use fancy-shmancy AI techniques to determine what the message is. */
	return(count < 25 && drop_or_add && from_or_to && mail_word);
}

/* allocate or fail
 */
char*
xmalloc(len)
    size_t len;
{
    char *malloc();
    char *p = malloc(len);
    if (p == NULL) {
	logandexit(EX_UNAVAILABLE, "insufficient memory");
    }
    return p;
}

/* reallocate or fail
 */
char*
xrealloc(p, len)
    char *p;
    size_t len;
{
    char *realloc();
    char *np = realloc(p,len);
    if (np == NULL) {
	logandexit(EX_UNAVAILABLE, "insufficient memory");
    }
    return np;
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

/* return the "long" command string
 */   
char *
argget()
{
    return cmdbuf.ls_buf;
}

/* make default path
 */
char *
adddefaultpath(defpath, name, suffix)
    char *defpath;
    char *name;
    char *suffix;
{
    int len;
    char *buf;
    char *p;
    
    if (name == NULL) {
	logandexit(EX_UNAVAILABLE, "invalid filename");
    }

    if (*name == '/') {		/* is absolute path */
	return name;		/* use it as is */
    }

    len = strlen(defpath) + strlen(name) + strlen(suffix) + 1 + 1;
    /* one for null, one for "/" */

    buf = xmalloc(len);
    strcpy(buf, defpath);

    p = rindex(buf, '\0');

    if (p == NULL) { 		/* must not happen */
	programerror();
    }

    if (p > buf && p[-1] != '/')  /* add / if missing */
	strcat(buf, "/");

    strcat(buf, name);
    strcat(buf, suffix);

    return buf;
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

/* AddAliasIDToHeader -- This function format then insert X-SOMETHING style
 * identifier in header.
 */
#if defined(ISSUE)
AddAliasIDToHeader(pipe, aliasid, issuenum)
    FILE *pipe;
    char *aliasid;
    int issuenum;
{
#ifndef MSC			/* NON-MSC case */
    if (aliasid != NULL) {
	fprintf(pipe, "X-Sequence: %s %d\n",aliasid,issuenum);
    }
    else {
	fprintf(pipe, "X-Sequence: %d\n",issuenum);
    }
#endif
#ifdef MSC
    if (aliasid != NULL) {
	fprintf(pipe, "X-Mail-count: %05d\n",issuenum);
	fprintf(pipe, "X-Ml-name: %s\n",aliasid);
    }
    else {
	fprintf(pipe, "X-Mail-count: %05d\n",issuenum);
    }
#endif
}
#endif


/* AddAliasIDToSubject -- This function formats Subject: line using original
 * subject, alias ID and issue number (if needed)
 */
#if defined(ISSUE)
AddAliasIDToSubject(subjectbuf, subject, aliasid, issuenum)
    char *subjectbuf;
    char *subject;
    char *aliasid;
    int issuenum;
{
    if (aliasid != NULL) {
	char *p = subject;
	int l;
	char buf[MAXSUBJLEN];		/* must be enough. */
	char *openfmt, *subjfmt;
	
#ifndef MSC
	openfmt = "%c%s ";
	subjfmt = "%c%s %d%c%s";
#endif
#ifdef MSC
	openfmt = "%c%s,";
	subjfmt = "%c%s,%05d%c%s";
#endif

	sprintf(buf, openfmt, openaliaschar, aliasid);
	
	l = strlen(buf);
	while (strlen(p) > l) {
	    if (strncmp(p, buf, l) == 0 && isdigit(p[l])) {
		register char *pp = p, *s = p + l;
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
		}
	    } else
		p++;
	}
	
	if (issuenum) {
	    sprintf(subjectbuf, subjfmt,
		    openaliaschar,
		    aliasid,issuenum,
		    closealiaschar,
		    subject);
	}
	else {
	    printf(subjectbuf, "%c%s%c%s",
		   openaliaschar,
		   aliasid,
		   closealiaschar,
		   subject);
	}
    }
    else {
	strcpy(subjectbuf, subject);
    }
}
#endif

/* Main Entry
 */
main(argc, argv)
int argc;
char ** argv;
{
	register int headc;	/* Number of headers */
	register int i;
	register FILE *pipe = NULL, *headf = NULL, *footf = NULL;
	FILE *noisef = NULL;
	FILE *recipf = NULL;
	char *headv[100];	/* Header vector */
	char *list = NULL;	/* Name of the list */
	char *host = NULL;	/* Name of the list's host */
	char *senderaddr = NULL;/* sender address for Sender: line */
	char *header;		/* A pointer to a header */
	char subject[MAXSUBJLEN];
	char subjectbuf[MAXSUBJLEN];
	char *aliasid = NULL;
	char *sendmailargs = NULL;	/* add'l args to sendmail */
	char *issuefile = NULL;
	char *recipfile = NULL;
	char *headerfile = NULL;
	char *footerfile = NULL;
	char *replyto = NULL;
#ifdef ISSUE
	int issuenum = 0;
#endif
#ifdef ADDVERSION
	int addversion = 1;	/* default */
#endif
	int badhdr = 0;		/* something is fishy about the header */
	int wasadmin = 0;	/* was a noise message */
	char buf[1024];
	int debug = 0;
	int zaprecv = 0;	/* zap received lines */
	int lessnoise = 0;	/* run ``please add/delete me'' filter */
	int errorsto = 0;
	int forcereplyto = 0;	/* ignore reply to */
	int majordomo = 0;	/* is NOT majordomo mode in default */
	char *originatorreplyto = NULL;
	int c;
	char openc, closec;
	extern char *optarg;
	extern int optind;
	int optionerror = 0;
	char *recipientbuf;
	
#ifdef SYSLOG	
	openlog("distribute", LOG_PID, SYSLOG_FACILITY);
#endif
#ifdef DEBUGLOG
	debuglog = fopen("/tmp/distribute.log", "a");
	if (debuglog == NULL) {
	    logandexit(EX_UNAVAILABLE, "can't open debug log");
	}
	fprintf(debuglog, "---\n");
	fprintf(debuglog, "invoked: pid=%d\n", getpid());
#endif

	chdir("/tmp");

	arginit();


	/* setup default */
#ifdef DEF_DOMAINNAME
	host = DEF_DOMAINNAME;
#else
	gethostname(myhostname, sizeof(myhostname));
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

	progname = argv[0];
	while ((c = getopt(argc, argv, GETOPT_PATTERN)) != EOF) {
		switch(c) {
		case 'M':	/* generic mailinglist with reply-to */
		    errorsto++;
		    aliasid = optarg;
		    replyto = optarg;
		    list = optarg;
		    issuefile = adddefaultpath(seq_path, optarg,
					       seq_suffix);
		    if (majordomo)
			recipfile = adddefaultpath(majordomo_recipient_path,
						   optarg,"");
		    else
			recipfile = adddefaultpath(recipient_path, optarg,
						   recipient_suffix);
		    break;

		case 'N':	/* generic maillinglist without reply-to */
		    errorsto++;
		    aliasid = optarg;
		    list = optarg;
		    issuefile = adddefaultpath(seq_path, optarg,
					       seq_suffix);
		    if (majordomo)
			recipfile = adddefaultpath(majordomo_recipient_path,
						   optarg,"");
		    else
			recipfile = adddefaultpath(recipient_path, optarg,
						   recipient_suffix);
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

		case 'F':	/* file with footer */
			footerfile = optarg;
			break;

		case 'm':	/* add'l args to sendmail */
			sendmailargs = optarg;
			break;

		case 'I':	/* add isssue number */
		case 'v':	/* add vol issue header */
			issuefile = adddefaultpath(seq_path, optarg, "");
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

		case 'L':	/* recip-addr-file */
			recipfile = adddefaultpath(recipient_path, optarg, "");
			break;

		case 'V':
		    printversion();
		    break;	/* notreached */

#ifdef ADDVERSION
		case 'A':
		    addversion = 0;
		    break;
#endif		    
		default:
		    usage();
		    logandexit(EX_USAGE, "unknown option: %c", c);
		    break;

		case '?':
		    usage();
		    exit(0);
		    break;	/* notreached */
		}
	}

	/* external configuration file existence check & file open
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

	if (recipfile != NULL) {
	    recipientbuf = parserecipfile(recipfile);
	}


	/*
	 * We need at least the host name and the list name...
	 */
	if (host == NULL || list == NULL || optionerror) {
	    usage();
	    logandexit(EX_USAGE, "require hostname and list name or bad usage");
	}

	/*
	 * Read all of the headers and make a header vector
	 */
	headc = head_parse(100, headv, stdin);

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
	 * Parse Subject
	 */
	if ((header = head_find(headc, headv, "Subject:")) != NULL) {
		strcpy(subject,index(header,' '));
		head_delete(headc, headv, "Subject:");
	}
	else
		strcpy(subject," (No Subject in original)");

	/*
	 * Delete all the Received: lines, if the user said to do so.
	 */
	if (zaprecv) {
		while (head_delete(headc, headv, "Received:") != NULL)
			;
	}

	/*
	 * Create the command that we are about to run.  We use the sender
	 * address given to us by the user (if we were given one); otherwise,
	 * the sender address is the list name with the usual -request
	 * tacked on.
	 */
	argreset();
	argappend("/usr/lib/sendmail ");
	if (sendmailargs != NULL)
		argappend(sendmailargs);
	argappend(" -f");
	if (senderaddr != NULL)
		argappend(senderaddr);
	else {
		argappend(list);
		argappend("-request");
	}

	/* add recipients */
	if (recipientbuf != NULL) {
	    argappend(" ");
	    argappend(recipientbuf);
	}

	for (i = optind ; i < argc ; i++)
	{
		argappend(" ");
		argappend(argv[i]);
	}

	/*
	 * Simple check that the header fields aren't grossly
	 * mismatched in terms of parens and angle-brackets.
	 */
	for (i=0; i<headc; i++) {
		if (headv[i] != NULL)
		{
			if (checkhdr(headv[i])) {
			    argreset();
			    if (senderaddr != NULL) {
				argappend("/usr/lib/sendmail -f");
				argappend(senderaddr);
				argappend(" ");
				argappend(senderaddr);
			    }
			    else {
				argappend("/usr/lib/sendmail -f");
				argappend(list);
				argappend("-request");
				argappend(" ");
				argappend(list);
				argappend("-request");
			    }

			    badhdr++;
			    break;
			}
		}
	}

	/*
	 * Deal with being smart about add-me mail here.
	 */
	if (lessnoise) {
		if (wasadmin = checkadmin(stdin, &noisef)) {
		    argreset();
		    if (senderaddr != NULL) {
			argappend("/usr/lib/sendmail -f");
			argappend(senderaddr);
			argappend(" ");
			argappend(senderaddr);
		    }
		    else {
			argappend("/usr/lib/sendmail -f");
			argappend(list);
			argappend("-request");
			argappend(" ");
			argappend(list);
			argappend("-request");
		    }
		}
	}

#ifdef ISSUE
	if (!(badhdr || wasadmin) && (issuefile != NULL)) {
		int getnextissue();
		issuenum = getnextissue(issuefile);
	}
#endif

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

	/*
	 * Give the command its input.
	 */

	/*
	 * Put out the headers.
	 */
	for (i=0; i<headc; i++) {
		if (headv[i] != NULL) {
			fputs(headv[i], pipe);
			putc('\n', pipe);
		}
	}

	/*
	 * Add a new Reply-To.
	 */
	if (replyto != NULL){
	    if (!forcereplyto && originatorreplyto != NULL) {
		fputs(originatorreplyto, pipe);
		putc('\n', pipe);
		free(originatorreplyto); /* sanity */
	    }
	    else {
		if (index(replyto,'@') == NULL)
		    fprintf(pipe, "Reply-To: %s@%s\n", replyto, host);
		else
		    fprintf(pipe, "Reply-To: %s\n", replyto);
	    }
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

#if defined(ISSUE)
	if (issuenum)
	    AddAliasIDToHeader(pipe, aliasid, issuenum);
#endif

#if defined(SUBJALIAS)
	AddAliasIDToSubject(subjectbuf, subject, aliasid, issuenum);
#else
	strcpy(subjectbuf, subject);
#endif

	fprintf(pipe, "Subject: %s\n", subjectbuf);

#ifdef DEBUGLOG
	fprintf(debuglog, "Command: %s\n", argget());
	fprintf(debuglog, "Subject: %s\n", subjectbuf);
#endif

	if (errorsto)
	    fprintf(pipe, "Errors-To: %s-request@%s\n", list, host);

	/*
	 * Add a new Sender: field as requested earlier.  Also add
	 * a blank line separating the header lines from the body of
	 * the message.
	 */
	if (senderaddr != NULL){
		if (index(senderaddr,'@') == NULL)
			fprintf(pipe, "Sender: %s@%s\n\n", senderaddr, host);
		else
			fprintf(pipe, "Sender: %s\n\n", senderaddr);
	}
	else
		fprintf(pipe, "Sender: %s-request@%s\n\n", list, host);

	/*
	 * If something was wrong, tell the list maintainer.
	 */
	if (badhdr)
		logwarn("unsent message: header problem.\n\n");
	if (wasadmin)
		logwarn("unsent message: was administrivia.\n\n");

	/*
	 * Dump the message thru the pipe.  We push out the header, then
	 * the message body, then the footer.
	 */
	if (headf != NULL) {
	    while (fgets(buf, sizeof buf, headf) != NULL) {
#ifdef DEBUGLOG		
		fputs(buf, debuglog);
#endif
		fputs(buf, pipe);
	    }
	}

	while (fgets(buf, sizeof buf, noisef == NULL ? stdin : noisef) != NULL)
		fputs(buf, pipe);

	if (footf != NULL) {
		while (fgets(buf, sizeof buf, footf) != NULL)
			fputs(buf, pipe);
	}

	fclose(pipe);

#ifdef SYSLOG
	syslog(LOG_INFO, "\"%s\" sent", subjectbuf);
#endif
	exit(0);
}


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
    if ((header = head_delete(headc, headv, "Sender:")) != NULL)
	free(header);
    
    /*
     * Delete the Return-Receipt-To: header.
     */
    if ((header = head_delete(headc, headv, "Return-Receipt-To:")) != NULL)
	free(header);
    
    /*
     * Delete the Errors-To: header.
     */
    if ((header = head_delete(headc, headv, "Errors-To:")) != NULL)
	free(header);
    
    /*
     * Delete the Return-Path: header.
     */
    if ((header = head_delete(headc, headv, "Return-Path:")) != NULL)
	free(header);
    
    /*
     * Delete the X-Volume-Issue
     */
    if ((header = head_delete(headc, headv, "X-Sequence:")) != NULL)
	free(header);
    
#ifdef ADDVERSION
    /*
     * Delete X-Distribute
     */
    if ((header = head_delete(headc, headv, "X-Distribute:")) != NULL)
	free(header);
#endif	
}



/* getnextissue -- returns next available issue number
 * do exclusive lock also.
 */
#ifdef ISSUE
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
	flock(fd, LOCK_EX);
	read(fd, buf, sizeof(buf));

	if ((p = index(buf,'\n')) != NULL) {	/* failsafe */
		*p = '\0';
		issue = atoi(buf);
		issue++;
	}
	else {
		/* wrong format. reset it */
		issue = 1;
	}
	lseek(fd, 0, L_SET);
	sprintf(buf, "%d\n", issue);
	write(fd,buf,strlen(buf));
	flock(fd, LOCK_UN);
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
int
checkhdr(s)
	register char *s;
{
	int nparens = 0;
	int nangles = 0;
	int linelen = 0, totlen = 0;

	if (strncasecmp(s, "From:", 5) && strncasecmp(s, "To:", 3) &&
	    strncasecmp(s, "Cc:", 3))
		return(0);	/* ??? */

	while (*s != '\0') {
		linelen++;
		totlen++;
		switch(*s) {
			case '\n':
				linelen = 0;
				break;

			case '(':
				nparens++;
				break;

			case ')':
				if (nparens <= 0)
					return(1);
				nparens--;
				break;

			case '<':
				nangles++;
				break;

			case '>':
				if (nangles <= 0)
					return(1);
				nangles--;
				break;
		}
		s++;
		if (linelen > 127 || totlen > 1023)
			return(1);
	}
	if (nangles == 0 && nparens == 0)
		return(0);
	return(1);
}


logandexit(exitcode, fmt, a1, a2)
    int exitcode;
    char *fmt;
    char *a1, *a2;
{
    syslog(LOG_ERR, fmt, a1, a2);
    exit(exitcode);
}

programerror()
{
    logandexit(EX_UNAVAILABLE, "program error");
}

logwarn(fmt, a1, a2)
    char *fmt;
    char *a1, *a2;
{
    syslog(LOG_WARNING, fmt, a1, a2);
}

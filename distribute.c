/* $Id$ */
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sysexits.h>
#include <sys/types.h>
#include <sys/file.h>

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

#define	MAXCMD	10240

extern	int head_parse();
extern	void head_norm();
extern	char * head_find();
extern	char * head_delete();
extern	char * tmpnam();
extern	char * index();
char *progname;

#define EQ(a, b) (strcasecmp((a), (b)) == 0)

void
usage() {
	fprintf(stderr, "usage: %s -h host -l list [ -f senderaddr ]\n",
		progname);
	fprintf(stderr, "    [ -H headerfile ] [ -F footerfile ]\n");
	fprintf(stderr, "    [ -r replytoaddr ]");
#ifdef ISSUE
	fprintf(stderr, " [ -I issuenumberfile ]");
#endif
#ifdef SUBJALIAS
	fprintf(stderr, " [ -a aliasid ]");
#endif
	fprintf(stderr, "\n");
	fprintf(stderr, "    [ -Rsde ] [ -m sendmail-flags ]\n");
	fprintf(stderr, "    [ -L recip-addr-file ] recip-addr ...\n");
	exit(EX_USAGE);
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
	char subject[1024];
	char *aliasid = NULL;
	char *sendmailargs = NULL;	/* add'l args to sendmail */
	char *issuefile = NULL;
	char *replyto = NULL;
#ifdef ISSUE
	int issuenum = 0;
#endif
	int badhdr = 0;		/* something is fishy about the header */
	int wasadmin = 0;	/* was a noise message */
	char buf[1024];
	char cmdbuf[MAXCMD];
	int debug = 0;
	int zaprecv = 0;	/* zap received lines */
	int lessnoise = 0;	/* run ``please add/delete me'' filter */
	int errorsto = 0;
	int c;
	extern char *optarg;
	extern int optind;

	chdir("/tmp");

	progname = argv[0];
	while ((c = getopt(argc, argv, "h:f:Rsel:H:dF:m:v:I:r:a:L:")) != EOF) {
		switch(c) {
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
			if ((headf = fopen(optarg, "r")) == NULL) {
				fprintf(stderr, "can't open header file '%s'\n", optarg);
				exit(EX_NOINPUT);
			}
			break;

		case 'd':	/* debug */
			debug++;
			break;

		case 'F':	/* file with footer */
			if ((footf = fopen(optarg, "r")) == NULL) {
				fprintf(stderr, "can't open footer file '%s'\n", optarg);
				exit(EX_NOINPUT);
			}
			break;

		case 'm':	/* add'l args to sendmail */
			sendmailargs = optarg;
			break;

		case 'I':	/* add isssue number */
		case 'v':	/* add vol issue header */
			issuefile = optarg;
			break;

		case 'r':	/* replyto header */
			replyto = optarg;
			break;

		case 'a':	/* alias-id */
			aliasid = optarg;
			break;

		case 'L':	/* recip-addr-file */
			if ((recipf = fopen(optarg, "r")) == NULL) {
				fprintf(stderr, "can't open receipt address file '%s'\n", optarg);
				exit(EX_NOINPUT);
			}
			break;

		default:
		case '?':
			usage();
		}
	}

	/*
	 * We need at least the host name and the list name...
	 */
	if (host == NULL || list == NULL)
		usage();

	/*
	 * Read all of the headers and make a header vector
	 */
	headc = head_parse(100, headv, stdin);

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

	/*
	 * Delete the Reply-To: header
	 */
	if (replyto != NULL) {
		if ((header = head_delete(headc, headv, "Reply-To:")) != NULL)
			free(header);
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
	strcpy(cmdbuf, "/usr/lib/sendmail ");
	if (sendmailargs != NULL)
		strcat(cmdbuf, sendmailargs);
	strcat(cmdbuf, " -f");
	if (senderaddr != NULL)
		strcat(cmdbuf, senderaddr);
	else {
		strcat(cmdbuf, list);
		strcat(cmdbuf, "-request");
	}
	if (recipf != NULL) {
		while (fgets(buf, sizeof buf, recipf) != NULL) {
			register char *p;

			if ((p = index(buf, '\n')) != NULL)
				*p = '\0';
			if (buf[0] == '\0' || buf[0] == '#')
				continue;
			strcat(cmdbuf, " ");
			strcat(cmdbuf, buf);
		}
	}
	for (i = optind ; i < argc ; i++)
	{
		strcat(cmdbuf, " ");
		strcat(cmdbuf, argv[i]);
	}

	/*
	 * Simple check that the header fields aren't grossly
	 * mismatched in terms of parens and angle-brackets.
	 */
	for (i=0; i<headc; i++) {
		if (headv[i] != NULL)
		{
			if (checkhdr(headv[i])) {
				if (senderaddr != NULL)
					sprintf(cmdbuf, "/usr/lib/sendmail -f%s %s", senderaddr, senderaddr);
				else
					sprintf(cmdbuf, "/usr/lib/sendmail -f%s-request %s-request", list, list);

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
			if (senderaddr != NULL)
				sprintf(cmdbuf, "/usr/lib/sendmail -f%s %s",
				    senderaddr, senderaddr);
			else
				sprintf(cmdbuf, "/usr/lib/sendmail -f%s-request %s-request", list, list);
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
		printf("Command: %s\n", cmdbuf);
	}
	else {
		pipe = popen(cmdbuf, "w");
		if (pipe == NULL) {
			perror("distribute: popen failed");
			exit(1);
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
		if (index(replyto,'@') == NULL)
			fprintf(pipe, "Reply-To: %s@%s\n", replyto, host);
		else
			fprintf(pipe, "Reply-To: %s\n", replyto, host);
	}
		
#ifdef ISSUE
	if (issuenum) {
#ifdef SUBJALIAS
		if (aliasid != NULL)
			fprintf(pipe, "X-Sequence: %s %d\n",aliasid,issuenum);
		else
#endif
			fprintf(pipe, "X-Sequence: %d\n",issuenum);
	}
#endif

#if defined(ISSUE) && defined(SUBJALIAS)
	if (aliasid != NULL) {
		register char *p = subject;
		register int l;
		char buf[256];
		sprintf(buf, "(%s ", aliasid);
		l = strlen(buf);
		while (strlen(p) > l) {
			if (strncmp(p, buf, l) == 0 && isdigit(p[l])) {
				register char *pp = p, *s = p + l;
				while (isdigit(*s))
					s++;
				if (*s == ')') {
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
		if (issuenum)
			fprintf(pipe, "Subject: (%s %d)%s\n",aliasid,issuenum,subject);
		else
			fprintf(pipe, "Subject: (%s)%s\n",aliasid,subject);
	} else
#endif
		fprintf(pipe, "Subject:%s\n",subject);

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
			fprintf(pipe, "Sender: %s\n\n", senderaddr, host);
	}
	else
		fprintf(pipe, "Sender: %s-request@%s\n\n", list, host);

	/*
	 * If something was wrong, tell the list maintainer.
	 */
	if (badhdr)
		fprintf(pipe, "WARNING: unsent message: header problem.\n\n");
	if (wasadmin)
		fprintf(pipe, "WARNING: unsent message: was administrivia.\n\n");

	/*
	 * Dump the message thru the pipe.  We push out the header, then
	 * the message body, then the footer.
	 */
	if (headf != NULL) {
		while (fgets(buf, sizeof buf, headf) != NULL)
			fputs(buf, pipe);
	}
	while (fgets(buf, sizeof buf, noisef == NULL ? stdin : noisef) != NULL)
		fputs(buf, pipe);
	if (footf != NULL) {
		while (fgets(buf, sizeof buf, footf) != NULL)
			fputs(buf, pipe);
	}
	exit(0);
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
			perror("distribute: can't create issue file");
			exit(EX_NOINPUT);
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

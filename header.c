/* header.c,v 1.7 1994/08/05 11:49:30 shigeya Exp */

/* Routines to read & munge mail & news headers.

   A header starts with a non space character in the first column.
   Continuation lines start with a space character.
   The first line of a message must not be a continuation line.
   The headers are seprated from the body of the message with a space
	line.

   Most of these routines all work with 'header counts' and 'header
   vectors' (headc, headv).  These are similar to argument counts and
   argument vectors (argc, argv).

   Routines are:
	int head_parse(headc, headv, fp)
			Read header lines and stick them into a header
			vector.
	void head_norm(line)
			Normalize a header line by removing
			continuation lines, and extra blanks.
	void head_blank(line)
			Make sure that the first space character in a
			header line is a blank (it could also be a tab
			or newline or something).  Do not need to do
			this if head_norm() has been called.
	char * head_find(headc, headv, header)
			Find and return a pointer to the indicated
			header in the header vector.
	char * head_delete(headc, headv, header)
			Delete and return a pointer to the indicated
			header in the header vector.
*/
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sysexits.h>

#include "util.h"
#include "config.h"

#include "memory.h"

extern logandexit();

/* Run through a message grabbing all of the header lines.  Stop at the
   end of the headers.

   Note: This routine malloc's space for each header line.

   This routine returns the number of header lines that it finds.

   Note: The caller of this routine is responsible for calling it with
   an (empty) head vector that is large enough for all of the expected
   headers.

   Bug: If the first line of the message is not a header, but starts
   with a space character, then that line is LOST!

   Bug: If there are more header lines than there is space in the header
   vector, then the first header line that can NOT fit in the header
   vector is LOST!

   Bug: If a header is longer than MAXHEADERLEN, than the rest of it
   is LOST!

   Makes sure that all continuation lines start with a tab.

   Deletes the final newline ('\n') from the end of all header lines.
*/
int head_parse(headc, headv, fp)
int headc;	/* Number of spaces for headers in the headv. */
char ** headv;	/* Header vector, filled out and returned. */
FILE * fp;	/* File pointer to the start of the message. */
{
	char head_line[MAXHEADERLEN+1];/* Current header line. */
	char line[MAXHEADERLEN+1];	/* Current line. */
	register int head_no;		/* Number of headers so far. */
	register int head_len;		/* Length of the current header. */


	/* Make sure that we were passed a place to put some header
	   lines.
	*/
	if ((headc <= 0) || (headv == (char **)NULL))
	{
	    logandexit(EX_UNAVAILABLE, "head_parse: Null args");
	}

	/* Make sure the header vector is empty.
	*/
	for (head_no=0; head_no < headc; head_no++)
	{
		headv[head_no] = (char *)NULL;
	}

	/* We start off with the first header and the first character
	   of that header.
	*/
	head_no = 0;
	head_len = 0;

	/* Our loop reading lines & dealing with them.  We break out of
	   this loop if we run out of places to put more header lines
	   (headv is not long enough), if we get a header line longer
	   than MAXHEADERLEN, if we reach the end of the file, or if
	   we reach the end of the header lines.
	*/
	while (1)
	{

		/* Read a line.
		*/
		if (fgets(line, MAXHEADERLEN, fp) == NULL)
		{
			/* We are at the end of the headers.
			*/
			break;
		}

		/* Check to see if this might be either a continuation
		   line or a blank line.
		*/
		if (isascii(*line) && isspace(*line))
		{

			/* The first header must not be a continuation
			   line.
			*/
			if ((head_no <= 0) && (head_len <= 0))
			{
				/* We are at the end of the headers.
				*/
				break;
			}

			/* According to RFC822, WHITESPACE CRLF sequnce
			 * is just a continuation line. So, Just ignore it.
			 * Also, RFC822 do not care about Form Feed.
			 */
			if (strspn(line, "\n\r") == strlen(line)) {
			    break; /* this is end of line */
			}

			if (strspn(line, " \t\n\r") == strlen(line)) {
			    continue; /* just ignore and continue */
			}

			/* This is a continuation line.  If any of it
			   will fit into the current header line, add
			   it.  Make sure that it starts with a tab.
			   Add as much as will fit to the current
			   header line.  Toss anything beyond that.
			*/
			/* We do not think it is good idea to replace
			 * leading character to TAB. Thus, commented out.
			 * We try to avoid to touch header.
			 */
			if (head_len < MAXHEADERLEN)
			{
/*				*line = '\t'; */
				(void)strncat(&head_line[head_len], line,
					MAXHEADERLEN - head_len);
				head_len += min(MAXHEADERLEN - head_len,
					strlen(line));
			}
			else
			{
			}

		}
		else
		{

			/* This is a new header line.
			*/

			/* Save the old one, if any.
			*/
			if (head_len > 0)
			{

				/* Make sure that the header line does
				   not end with a newline.  Make sure
				   that the header line does end with a
				   NUL.
				*/
				if (head_line[head_len - 1] == '\n')
				{
					head_len--;
				}
				head_line[head_len] = '\0';

				/* Grab some space for it.
				*/
				headv[head_no] = xmalloc((unsigned)head_len+1);

				/* Copy the header line into this space.
				*/
				(void)strcpy(headv[head_no], head_line);

				/* We are done with this header line.
				*/
				head_len = 0;

				/* Onto the next header line.  If we
				   are out of space in the header
				   vector, then quit.
				*/
				if (++head_no >= headc)
				{
					/* We are at the end of the
					   headers.
					*/
					break;
				}
			}

			/* Save this new header line.
			*/
			(void)strcpy(head_line, line);
			head_len = strlen(line);

		}

	} /* End of while(1). */

	/* Save the last header line, if any.
	*/
	if (head_len > 0)
	{

		/* Make sure that the header line does not end with a
		   newline.  Make sure that the header line does end
		   with a NUL.
		*/
		if (head_line[head_len - 1] == '\n')
		{
			head_len--;
		}
		head_line[head_len] = '\0';

		/* Grab some space for it.
		*/
		headv[head_no] = xmalloc((unsigned)head_len+1);

		/* Copy the header line into this space.
		*/
		(void)strcpy(headv[head_no], head_line);

		head_no++;
	}

	/* Return the number of header lines that we found.
	*/
	return (head_no);
}

/* This routine 'normalizes' header lines - it gets rid of continuation
   lines, and makes sure that the only space characters are single
   blanks.

   Note: This overwrites the line!!

   Note: It is NOT very efficient!  If the line needs NOT to be
   changed, it still copies it on top of itself.

   Makes sure that the end of the header line is not a space character.
*/
void head_norm(line)
register char * line;
{
	register char * wr_ptr;		/* Write pointer in the line. */


	/* Sanity check - we were not passed a NULL pointer.
	*/
	if ((line == (char *)NULL) || (*line == '\0'))
	{
		return;
	}

	/* Run down the line changing all space characters to a single
	   blank.

	   Note: we use 'line' as the read pointer.
	*/
	wr_ptr = line;
	while (*line != '\0')
	{

		/* Replace all strings of space characters with one
		   blank.
		*/
		if (isascii(*line) && isspace(*line))
		{

			/* Put in one blank.
			*/
			*wr_ptr++ = ' ';

			/* Skip past this character and all other space
			   characters.

			   Note: We do not need to check for NUL here
			   since NUL is not a space character.
			*/
			line++;
			while (isascii(*line) && isspace(*line))
			{
				line++;
			}
		}

		/* Copy the character that we now have and move up
		   one.  Do not copy the NUL.
		*/
		if (*line != '\0')
		{
			*wr_ptr++ = *line++;
		}
	}

	/* Stick a NUL at the end.
	*/
	*wr_ptr = '\0';

	/* If the last character is a blank, get rid of it.
	*/
	if (*--wr_ptr == ' ')
	{
		*wr_ptr = '\0';
	}

	/* All done.
	*/
	return;
}

/* This routine makes sure that the first space charater in a header
   line is a blank.  This does not need to be called if head_norm() has
   already been run on the line.

   This is usefull if you want to search for the "From: " lines, and do
   not want to normalize the header lines, but do not want to worry
   about having the first space character be anything other than a
   blank.
*/
void head_blank(line)
register char * line;
{

	/* Sanity check - we were not passed a NULL pointer.
	*/
	if ((line == (char *)NULL) || (*line == '\0'))
	{
		return;
	}

	/* Run down the line until the end.
	*/
	while (*line != '\0')
	{

		/* If this character is a space character, replace it
		   with a blank and quit.
		*/
		if (isascii(*line) && isspace(*line))
		{

			/* Replace it with a blank.
			*/
			*line = ' ';

			/* All done.
			*/
			break;
		}

		/* On to the next character in the line.
		*/
		line++;
	}

	/* All done.
	*/
	return;
}

/* This routine finds and returns a pointer to the indicated header in
   a header vector.

   This is similar to getenv(3) but it finds headers in the header
   vector instead of environment variables in the environment.

   If there is no such header, this routine returns NULL.

   It finds the first such header only.

   Note: If you are looking for a header such as "Subject: ", the given
   header string must be exactly that: "Subject: ".

   The routine actually matches the passed string with the first part
   of each header line, and returns to first such header line.

   It would actually be possible to look for "Subject: Re: ", even
   though this is not (properly speaking) a real header.

   Note: "Subject: " WILL NOT MATCH "Subject:\t".  It is probably
   useful to run all of the headers through head_norm() before using
   this routine.
*/
char * head_find(headc, headv, header)
int headc;	/* Number of headers in the header vector. */
register char ** headv;	/* Header vector. */
char * header;	/* Header to look for. */
{

	/* Make sure that we were passed some header lines and that we
	   were passed a header to look for.
	*/
	if ((headc <= 0) || (headv == (char **)NULL)
		|| (header == (char *)NULL) || (*header == '\0'))
	{
		return ((char *)NULL);
	}

	/* Run down the header vector looking for the first header that
	   matches.
	*/
	while (headc-- > 0)
	{

		/* If this header does not exist, skip it.
		*/
		if ((*headv == (char *)NULL) || (**headv == '\0'))
		{
			headv++;
			continue;
		}

		/* Check this header against our target.
		*/
		if (strncasecmp(*headv, header, strlen(header)) == 0)
		{
			/* We found it!
			*/
			return (*headv);
		}

		/* On to the next header.
		*/
		headv++;
	}

	/* We did not find it.
	*/
	return ((char *)NULL);
}

/* This routine deletes the indicated header in a header vector.  It
   returns a pointer to the deleted header.

   If there is no such header, this routine returns NULL.

   It finds the first such header only.

   Note: If you are looking for a header such as "Subject: ", the given
   header string must be exactly that: "Subject: ".

   The routine actually matches the passed string with the first part
   of each header line, and returns to first such header line.

   It would actually be possible to look for "Subject: Re: ", even
   though this is not (properly speaking) a real header.

   Note: "Subject: " WILL NOT MATCH "Subject:\t".  It is probably
   useful to run all of the headers through head_norm() before using
   this routine.
*/
char * head_delete(headc, headv, header)
int headc;	/* Number of headers in the header vector. */
register char ** headv;	/* Header vector. */
char * header;	/* Header to delete. */
{

	/* Make sure that we were passed some header lines and that we
	   were passed a header to look for.
	*/
	if ((headc <= 0) || (headv == (char **)NULL)
		|| (header == (char *)NULL) || (*header == '\0'))
	{
		return ((char *)NULL);
	}

	/* Run down the header vector looking for the first header that
	   matches.
	*/
	while (headc-- > 0)
	{

		/* If this header does not exist, skip it.
		*/
		if ((*headv == (char *)NULL) || (**headv == '\0'))
		{
			/* On to the next header.
			*/
			headv++;
			continue;
		}

		/* Check this header against our target.
		*/
		if (strncasecmp(*headv, header, strlen(header)) == 0)
		{

			/* We found it!  Remove it from the list &
			   return a pointer to it.
			*/
			header = *headv;
			*headv = (char *)NULL;
			return (header);
		}

		/* On to the next header.
		*/
		headv++;
	}

	/* We did not find it.
	*/
	return ((char *)NULL);
}


/* free whole header vector
 */
void head_free(headc, headv)
    register int headc;
    register char ** headv;
{
    register int i;

    for(i=0; i<headc; i++) {
	free(headv[i]);
	headv[i] = NULL;
    }
}

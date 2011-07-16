/* See COPYING file for copyright, license and warranty details. */

#define VERSION "prealpha"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.c"

#define usage "getgbook " VERSION " - a google books downloader\n" \
              "usage: getgbook [-p|-a] isbn\n" \
              "  -p print all available pages\n" \
              "  -a download all available pages\n" \
              "  otherwise, all pages in stdin will be downloaded\n"

#define hostname "books.google.com"

#define URLMAX 1024
#define BOOKID_LEN 12

typedef struct {
	char *name;
	char *code;
} pgtype;

pgtype pgtypes[] = {
	{"cover", "PP"},
	{"preface", "PR"},
	{"page", "PA"},
	{"postface", "PA"},
};

char *getbookid(char *isbn)
{
	char url[URLMAX];
	char *buf, *bookid, *c;

	/* NOTE: new api returns json, and looks like this:
	 * http://www.googleapis.com/books/v1/volumes?q=isbn:1589235126
	 * (this needs https, which we don't yet support) */

	snprintf(url, URLMAX, "/books/feeds/volumes?q=isbn:%s", isbn);

	bookid = malloc(sizeof(char *) * BOOKID_LEN);

	if(!get("books.google.com", url, &buf))
		return NULL;

	if((c = strstr(buf,"<dc:identifier>")) == NULL)
		return NULL;
	strncpy(bookid, c+15, BOOKID_LEN);
	bookid[BOOKID_LEN] = '\0';
	free(buf);

	return bookid;
}

char *getpageurl(char *bookid, char *pg)
{
	char url[URLMAX];
	char *buf, *c, *d, m[80], *pageurl, *p;

	snprintf(url, URLMAX, "/books?id=%s&pg=%s&jscmd=click3", bookid, pg);

	if(!get("books.google.com", url, &buf))
		return NULL;

	snprintf(m, 80, "\"pid\":\"%s\"", pg);
	if((c = strstr(buf,m)) == NULL)
		return NULL;
	if(strncmp(c+strlen(m)+1, "\"src\"", 5) != 0)
		return NULL;

	pageurl = malloc(sizeof(char *) * URLMAX);
	for(p=pageurl, d=c+strlen(m)+8; *d && *d != '"'; *d++, *p++) {
		if(!strncmp(d, "\\u0026", 6)) {
			*p = '&';
			d+=5;
		} else
			*p = *d;
	}
	*p = '\0';
	free(buf);

	return pageurl;
}

int main(int argc, char *argv[])
{
	char *bookid, *url, pg[12];

	if(argc < 2 || argc > 3)
		die(usage);

	if(argv[1][0] == '-') {
		if((argv[1][1] != 'p' && argv[1][1] != 'a') || argc < 3)
			die(usage);

		if((bookid = getbookid(argv[2])) == NULL)
			die("Could not find book\n");
		printf("bookid is %s\n", bookid);
	} else {
		if((bookid = getbookid(argv[1])) == NULL)
			die("Could not find book\n");
		printf("bookid is %s\n", bookid);

		strncpy(pg, "PA2", 12);
		if((url = getpageurl(bookid, pg)) == NULL)
			fprintf(stderr, "Could not find page %s\n", pg);
		else {
			printf("page %s url is %s\n", pg, url);
			gettofile(url, "test.png");
		}
	}

	free(bookid);
	free(url);
	return EXIT_SUCCESS;
}

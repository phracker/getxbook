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
	int i;
	FILE *srv;
	char *buf, *bookid, *c;

	i = dial("books.google.com", "80");
	srv = fdopen(i, "r+");

	/* NOTE: new api returns json, and looks like this:
	 * http://www.googleapis.com/books/v1/volumes?q=isbn:1589235126
	 * (this needs https, which we don't yet support) */

	snprintf(url, URLMAX, "/books/feeds/volumes?q=isbn:%s", isbn);

	bookid = malloc(sizeof(char *) * BOOKID_LEN);

	if((buf = get(srv, "books.google.com", url)) == NULL)
		return NULL;

	if((c = strstr(buf,"<dc:identifier>")) == NULL)
		return NULL;
	strncpy(bookid, c+15, BOOKID_LEN);
	bookid[BOOKID_LEN] = '\0';
	free(buf);

	return bookid;
}

char *getpageurl(char *bookid, char *pagetype, int pagenum)
{
	char url[URLMAX];
	int i, l;
	FILE *srv;
	char *buf, *c, *d, m[80], *pageurl;

	i = dial("books.google.com", "80");
	srv = fdopen(i, "r+");

	snprintf(url, URLMAX, "/books?id=%s&pg=%s%i&jscmd=click3", bookid, pagetype, pagenum);

	if((buf = get(srv, "books.google.com", url)) == NULL)
		return NULL;

	snprintf(m, 80, "\"pid\":\"%s%i\"", pagetype, pagenum);
	if((c = strstr(buf,m)) == NULL)
		return NULL;
	if(strncmp(c+strlen(m)+1, "\"src\"", 5) != 0)
		return NULL;
	for(l=0, d=c+strlen(m)+8; *d && *d != '"'; *d++, l++);

	pageurl = malloc(sizeof(char *) * l);
	strncpy(pageurl, c+strlen(m)+8, l);
	pageurl[l] = '\0';
	free(buf);

	return pageurl;
}

int main(int argc, char *argv[])
{
	char *bookid, *url;

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

		if((url = getpageurl(bookid, "PA", 2)) != NULL)
			printf("page 2 url is %s\n", url);
		else
			fprintf(stderr, "Could not find page %s %i\n", "PA", 2);
	}

	free(bookid);
	free(url);
	return EXIT_SUCCESS;
}

/* See COPYING file for copyright, license and warranty details. */

#define VERSION "prealpha"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.c"

#define usage "getgbook - a google books downloader\n" \
              "getgbook [-p|-a] isbn\n" \
              "  -p print all available pages\n" \
              "  -a download all available pages\n" \
              "  otherwise, all pages given in stdin will be downloaded"

#define hostname "books.google.com"

#define URLMAX 1024
#define BOOKID_LEN 12

char *getgbookid(char *isbn)
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
	else {
		if((c = strstr(buf,"<dc:identifier>")) == NULL)
			return NULL;
		strncpy(bookid, c+15, BOOKID_LEN);
		bookid[BOOKID_LEN] = '\0';
		free(buf);
	}

	return bookid;
}

int main(int argc, char *argv[])
{
	char *bookid, isbn[16];

	if(argc < 2 || argc > 3 || !strncmp(argv[1], "-h", 2))
		die("usage: " usage "\n");

	if(!strncmp(argv[1], "-p", 2)) {
		if(argc != 3) die("usage: " usage "\n");
		printf("I'd love to print a list of available pages\n");
		argv++;
	} else if(!strncmp(argv[1], "-a", 2)) {
		if(argc != 3) die("usage: " usage "\n");
		printf("I'd love to download all available pages\n");
		argv++;
	} else {
		printf("I'd love to download all pages from stdin\n");
	}

	strncpy(isbn,argv[1],16);

	if((bookid = getgbookid(isbn)) == NULL)
		die("Could not find book\n");
	printf("bookid is %s\n", bookid);

	free(bookid);
	return 0;
}

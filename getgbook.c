/* See COPYING file for copyright, license and warranty details. */

#define VERSION "prealpha"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.c"

#define usage "getgbook isbn"

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
		fprintf(stderr,"Error downloading page\n");
	else {
		c = strstr(buf,"<dc:identifier>");
		strncpy(bookid, c+15, BOOKID_LEN);
		bookid[BOOKID_LEN] = '\0';
		free(buf);
	}

	return bookid;
}

int main(int argc, char *argv[])
{
	int i;
	char *bookid, isbn[16];
	FILE *srv;

	if(argc != 2)
		die("usage: " usage "\n");

	strncpy(isbn,argv[1],16);

	i = dial(hostname, "80");
	srv = fdopen(i, "r+");

	bookid = getgbookid(isbn);
	printf("bookid is %s\n", bookid);

	free(bookid);
	return 0;
}

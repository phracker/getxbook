/* See COPYING file for copyright, license and warranty details. */

#define VERSION "prealpha"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.c"

#define usage "getgbook bookid"

#define hostname "books.google.com"

int main(int argc, char *argv[])
{
	int i, s;
	char *bookid, url[80];
	char *curpage;
	FILE *srv;

	if(argc != 2)
		die("usage: " usage "\n");

	bookid = argv[1];

	i = dial(hostname, "80");
	srv = fdopen(i, "r+");

	snprintf(url, 80, "/books?id=%s&pg=%s&jscmd=click3", bookid, "PA1");
	if((curpage = get(srv, hostname, url)) == NULL)
		fprintf(stderr,"Error downloading page\n");
	else {
		fputs(curpage,stdout);
		free(curpage);
	}

	return 0;
}

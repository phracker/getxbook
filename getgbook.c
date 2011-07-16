/* See COPYING file for copyright, license and warranty details. */

/* NOTE: there's now a new api that returns json.
 * it requires https, which we don't yet support.
 * https://www.googleapis.com/books/v1/volumes?q=isbn:1589235126
 * https://www.googleapis.com/books/v1/volumes/jglfL_eVG4cC */


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
	int num;
	char url[URLMAX];
	char name[80];
} page;

char *getbookid(char *isbn)
{
	char url[URLMAX];
	char *buf, *bookid, *c;

	snprintf(url, URLMAX, "/books/feeds/volumes?q=isbn:%s", isbn);

	if(!get("books.google.com", url, &buf))
		return NULL;

	if((c = strstr(buf,"<dc:identifier>")) == NULL)
		return NULL;
	bookid = malloc(sizeof(char *) * BOOKID_LEN);
	strncpy(bookid, c+15, BOOKID_LEN);
	bookid[BOOKID_LEN] = '\0';
	free(buf);

	return bookid;
}

int gettotalpages(char *bookid)
{
	char url[URLMAX];
	char *buf, *c;
	int total;

	snprintf(url, URLMAX, "/books/feeds/volumes/%s", bookid);

	bookid = malloc(sizeof(char *) * BOOKID_LEN);

	if(!get("books.google.com", url, &buf))
		return 0;

	if((c = strstr(buf," pages</dc:format>")) == NULL)
		return 0;
	while(*c && *c != '>') *c--;
	sscanf(c+1, "%d ", &total);

	return total;
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
	char *bookid, *url, pg[16], buf[1024];
	int totalpages, i;

	if(argc < 2 || argc > 3 ||
	   (argv[1][0]=='-' && ((argv[1][1]!='p' && argv[1][1]!='a') || argc < 3)))
		die(usage);

	if((bookid = getbookid(argv[argc-1])) == NULL)
		die("Could not find book\n");

	if(!(totalpages = gettotalpages(bookid)))
		die("Book has no pages\n");

	if(argv[1][0] == '-') {
		/* note this isn't the best way, not least because it misses the
		 * non PA pages. best is to crawl around the json grabbing everything
		 * available, by starting on PP1, and filling in by going through
		 * all pages in totalpages. then crawl through the pages struct. */
		for(i=1; i<totalpages; i++) {
			snprintf(pg, 16, "%s%d", "PA", i);
			if((url = getpageurl(bookid, pg)) == NULL) {
				fprintf(stderr, "%d failed\n", i);
				continue;
			}
			if(argv[1][1] == 'a') {
				strncat(pg, ".png", 16);
				gettofile(url, pg);
				printf("Downloaded page %d\n", i);
			} else
				printf("%d\n", i);
		}
	} else {
		while(fgets(buf, 1024, stdin)) {
			sscanf(buf, "%d", &i);
			snprintf(pg, 16, "%s%d", "PA", i);
			if((url = getpageurl(bookid, pg)) == NULL) {
				fprintf(stderr, "%d failed\n", i);
				continue;
			}
			strncat(pg, ".png", 16);
			gettofile(url, pg);
			printf("Downloaded page %d\n", i);
		}
	}

	free(bookid);
	free(url);

	return EXIT_SUCCESS;
}

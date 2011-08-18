/* See COPYING file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

#define usage "getgbook " VERSION " - a google books downloader\n" \
              "usage: getgbook [-] bookid\n" \
              "  - download pages from stdin\n" \
              "  otherwise, all available pages will be downloaded\n"

#define URLMAX 1024
#define STRMAX 1024
#define PGCODELEN 3
#define RETRYNUM 5
#define COOKIENUM 5

typedef struct {
	int num;
	char url[URLMAX];
	char name[STRMAX];
} Page;

char pagecodes[][PGCODELEN] = { "PP", "PR", "PA", "PT", "\0" };

int getpagelist(char *bookid, Page **pages)
{
	char url[URLMAX];
	char *buf;
	char *s;
	int i;
	Page *p;

	snprintf(url, URLMAX, "/books?id=%s&printsec=frontcover", bookid);

	if(!get("books.google.com", url, NULL, NULL, &buf))
		return -1;

	if((s = strstr(buf, "_OC_Run({\"page\":[")) == NULL)
		return -1;
	s+=strlen("_OC_Run({\"page\":[");

	for(i=0, p=pages[0];*s; s++) {
		if(*s == ']')
			break;
		if(!strncmp(s, "\"pid\"", 5)) {
			sscanf(s+6, "\"%[^\"]\",", p->name);
			for(;*s; s++) {
				if(*s == '}')
					break;
				if(!strncmp(s, "\"order\"", 7))
					sscanf(s+8, "%d,", &(p->num));
			}
			p=pages[++i];
		}
	}

	return i;
}

Page *getpagedetail(char *bookid, char *pg, char *cookie)
{
	char url[URLMAX], m[STRMAX];
	char *c, *d, *p, *buf = NULL;
	Page *page;

	snprintf(url, URLMAX, "/books?id=%s&pg=%s&jscmd=click3&q=subject:a", bookid, pg);

	if(!get("books.google.com", url, cookie, NULL, &buf))
		return NULL;

	snprintf(m, STRMAX, "\"pid\":\"%s\"", pg);
	if(!(c = strstr(buf,m)))
		return NULL;

	page = malloc(sizeof(*page));
	strncpy(page->name, pg, STRMAX);
	page->url[0] = '\0';
	page->num = -1;

	if(!strncmp(c+strlen(m)+1, "\"src\"", 5)) {
		for(p=page->url, d=c+strlen(m)+8; *d && *d != '"'; d++, p++) {
			if(!strncmp(d, "\\u0026", 6)) {
				*p = '&';
				d+=5;
			} else
				*p = *d;
		}
		strncpy(p, "&q=subject:a", 12);
	} else
		d=c;

	for(; *d; d++) {
		if(*d == '}') {
			break;
		}
		if(!strncmp(d, "\"order\"", 7)) {
			sscanf(d+8, "%d,", &(page->num));
			break;
		}
	}

	free(buf);
	return page;
}

int main(int argc, char *argv[])
{
	char *bookid, *tmp, cookies[COOKIENUM][COOKIEMAX];
	int i, a;

	if(argc < 2 || argc > 3 || (argc == 3 && argv[1][0]!='-')) {
		fputs(usage, stdout);
		return 1;
	}

	/* get cookies */
	for(i=0;i<COOKIENUM;i++) {
		if(get("books.google.com", "/", NULL, cookies[i], &tmp))
			free(tmp);
	}

	bookid = argv[argc-1];

	if(argc == 2) {
		/* download all pages */
		/* - fill page struct with names & nums
		 * - loop through each struct
		 * - if there's not a file matching num, try downloading, if dl failure, try with a different cookie */
		/*
			cookie management:
			use up to 5 cookies. (number might change)
			complexity comes with a page which is not available; that shouldn't cause us to use up all the cookies
			so:
			 - save 5 cookies immediately
			 - use first until it fails
				 - then use next. if it succeeds, drop previous. if not, try next, etc. if all failed, don't drop any, and continue to next page, and +1 to retry
			 - maybe: when retry is 5, quit as it looks like we won't get anything more from any cookies
		*/

		Page **page;
		page = malloc(sizeof(*page) * 1000);
		for(i=0; i<1000; i++) page[i] = malloc(sizeof(**page));
		if(!(i = getpagelist(bookid, page))) {
			fprintf(stderr, "Could not find pages for %s\n", bookid);
			return 1;
		}
		for(a=0; a<i; a++)
			printf("page name: %s page num: %d\n", page[a]->name, page[a]->num);
	}

	return EXIT_SUCCESS;
}

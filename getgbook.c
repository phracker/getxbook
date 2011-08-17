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

int getpagelist(char *bookid, Page *pages)
{
	/* TODO */
	/*http://books.google.com/books?id=h3DSQ0L10o8C&printsec=frontcover*/
	return 1;
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
	char *bookid, *tmp, *code, cookies[COOKIENUM][COOKIEMAX];
	char pg[STRMAX], buf[BUFSIZ], n[STRMAX], cookie[COOKIEMAX] = "";
	int i, c, retry;

	if(argc < 2 || argc > 3 || (argc == 3 && argv[1][0]!='-')) {
		fputs(usage, stdout);
		return 1;
	}

	/* get cookies */
	for(i=0;i<COOKIENUM;i++) {
		get("books.google.com", "/", NULL, cookies[i], &tmp);
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

		Page page[10000];
		if(!getpagelist(bookid, page)) {
			fprintf(stderr, "Could not find pages for %s\n", bookid);
			return 1;
		}



		/* OLD CODE */
		code = pagecodes[0];
		c = i = retry = 0;
		while(++i) {
			snprintf(pg, STRMAX, "%s%d", code, i);
			if(!(page = getpagedetail(bookid, pg, cookie))) {
				/* no more pages with that code */
				code = pagecodes[++c];
				if(code[0] == '\0') break;
				i=0;
				continue;
			}
			if(!page->url[0]) {
				free(page);
				/* try with fresh cookie */
				if(retry < RETRYNUM) {
					get("books.google.com", "/", NULL, cookie, &tmp);
					free(tmp);
					retry++;
					i--;
				} else {
					fprintf(stderr, "%s not available\n", pg);
					retry=0;
				}
				continue;
			}
			retry=0;
			if(argv[1][1] == 'a') {
				if(page->num != -1)
					snprintf(n, STRMAX, "%04d.png", page->num);
				else
					snprintf(n, STRMAX, "%s.png", page->name);
				if(gettofile("books.google.com", page->url, cookie, NULL, n))
					fprintf(stderr, "%s failed\n", pg);
				else
					printf("Downloaded page %d\n", page->num);
			} else {
				printf("%s ", page->name);
				if(page->num != -1) printf("%d", page->num);
				printf("\n");
				fflush(stdout);
			}
			free(page);
		}
	} else {
		/* download pages from stdin */
		/* TODO: rewrite using cookies as above */
		Page *page;
		while(fgets(buf, BUFSIZ, stdin)) {
			sscanf(buf, "%15s", pg);
			for(retry = 0; retry < RETRYNUM; retry++) {
				get("books.google.com", "/", NULL, cookie, &tmp);
				if((page = getpagedetail(bookid, pg, cookie)) && page->url[0]) {
					snprintf(n, STRMAX, "%04d.png", page->num);
					if(gettofile("books.google.com", page->url, cookie, NULL, n))
						continue;
					printf("Downloaded page %d\n", page->num);
					free(page);
					break;
				}
				if(page) free(page);
			}
			if(retry == RETRYNUM)
				fprintf(stderr, "%s failed\n", pg);
		}
	}

	return EXIT_SUCCESS;
}

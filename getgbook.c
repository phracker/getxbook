/* See COPYING file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

#define usage "getgbook " VERSION " - a google books downloader\n" \
              "usage: getgbook [-p|-a] bookid\n" \
              "  -p print all available pages\n" \
              "  -a download all available pages\n" \
              "  otherwise, all pages in stdin will be downloaded\n"

#define URLMAX 1024
#define STRMAX 1024
#define PGCODELEN 3
#define RETRYNUM 5

typedef struct {
	int num;
	char url[URLMAX];
	char name[STRMAX];
} Page;

char pagecodes[][PGCODELEN] = { "PP", "PR", "PA", "PT", "\0" };

Page *getpagedetail(char *bookid, char *pg, char *cookie)
{
	char url[URLMAX], m[STRMAX];
	char *c, *d, *p, *buf = NULL;
	Page *page;

	snprintf(url, URLMAX, "/books?id=%s&pg=%s&jscmd=click3", bookid, pg);

	if(!get("books.google.com", url, cookie, NULL, &buf))
		return NULL;

	snprintf(m, STRMAX, "\"pid\":\"%s\"", pg);
	if(!(c = strstr(buf,m)))
		return NULL;

	page = malloc(sizeof(Page));
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
		*p = '\0';
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
	char *bookid, *tmp, *code;
	char pg[STRMAX], buf[BUFSIZ], n[STRMAX], cookie[COOKIEMAX] = "";
	int i, c, retry;
	Page *page;

	if(argc < 2 || argc > 3 ||
	   (argv[1][0]=='-' && ((argv[1][1]!='p' && argv[1][1]!='a') || argc < 3))) {
		fputs(usage, stdout);
		return 1;
	}

	bookid = argv[argc-1];

	if(argv[1][0] == '-') {
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
			}
			free(page);
		}
	} else {
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

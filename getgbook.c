/* See COPYING file for copyright, license and warranty details. */
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

typedef struct {
	int num;
	char url[URLMAX];
	char name[80];
} Page;

char pagecodes[][3] = { "PP", "PR", "PA", "PT", "\0" };

Page *getpagedetail(char *bookid, char *pg, char *cookie)
{
	char url[URLMAX];
	char *buf, *c, *d, m[80], *p;
	Page *page;

	snprintf(url, URLMAX, "/books?id=%s&pg=%s&jscmd=click3", bookid, pg);

	if(!get("books.google.com", url, cookie, NULL, &buf))
		return NULL;

	snprintf(m, 80, "\"pid\":\"%s\"", pg);
	if(!(c = strstr(buf,m)))
		return NULL;

	page = malloc(sizeof(Page));
	strncpy(page->name, pg, 80);
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
	}

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
	char *bookid, *tmp, pg[16], buf[1024], n[80], code[3], cookie[COOKIEMAX], u[1024];
	int i, c, retry;
	Page *page;

	if(argc < 2 || argc > 3 ||
	   (argv[1][0]=='-' && ((argv[1][1]!='p' && argv[1][1]!='a') || argc < 3))) {
		fputs(usage, stdout);
		return 1;
	}

	bookid = argv[argc-1];

	if(argv[1][0] == '-') {
		strncpy(code, pagecodes[0], 3);
		c = i = retry = 0;
		while(++i) {
			snprintf(pg, 15, "%s%d", code, i);
			if(!(page = getpagedetail(bookid, pg, cookie))) {
				/* no more pages with that code */
				strncpy(code, pagecodes[++c], 3);
				if(code[0] == '\0') break;
				i=0;
				continue;
			}
			if(!page->url[0]) {
				free(page);
				/* try with fresh cookie */
				if(!retry) {
					snprintf(u, URLMAX, "/books?id=%s", bookid);
					get("books.google.com", u, NULL, cookie, &tmp);
					free(tmp);
					retry=1;
					i--;
				} else {
					fprintf(stderr, "%s not available\n", pg);
					retry=0;
				}
				continue;
			}
			if(argv[1][1] == 'a') {
				snprintf(n, 80, "%05d.png", page->num);
				gettofile("books.google.com", page->url, cookie, NULL, n);
				printf("Downloaded page %d\n", page->num);
			} else if(page->num != -1)
				printf("%s %d\n", page->name, page->num);
			free(page);
		}
	} else {
		while(fgets(buf, 1024, stdin)) {
			sscanf(buf, "%15s", pg);
			if(!(page = getpagedetail(bookid, pg, cookie)) || !page->url[0]) {
				fprintf(stderr, "%s failed\n", pg);
				free(page);
				continue;
			}
			snprintf(n, 80, "%05d.png", page->num);
			gettofile("books.google.com", page->url, cookie, NULL, n);
			printf("Downloaded page %d\n", page->num);
			free(page);
		}
	}

	return EXIT_SUCCESS;
}

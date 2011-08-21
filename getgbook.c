/* See COPYING file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

#define usage "getgbook " VERSION " - a google books downloader\n" \
              "usage: getgbook [-c|-n] bookid\n" \
              "  -c download pages from codes in stdin (TODO)\n" \
              "  -n download pages from numbers in stdin (TODO)\n" \
              "  otherwise, all available pages will be downloaded\n"

#define URLMAX 1024
#define STRMAX 1024
#define MAXPAGES 9999
#define COOKIENUM 5

typedef struct {
	int num;
	char url[URLMAX];
	char name[STRMAX];
	char cookie[COOKIEMAX];
} Page;

Page **pages;
int totalpages;
char cookies[COOKIENUM][COOKIEMAX];
char *bookid;

int getpagelist()
{
	char url[URLMAX], m[STRMAX];
	char *buf = NULL;
	char *s;
	int i;
	Page *p;

	snprintf(url, URLMAX, "/books?id=%s&printsec=frontcover", bookid);

	if(!get("books.google.com", url, NULL, NULL, &buf))
		return 0;

	if((s = strstr(buf, "_OC_Run({\"page\":[")) == NULL)
		return 0;
	s+=strlen("_OC_Run({\"page\":[");

	for(i=0, p=pages[0];*s; s++) {
		p->url[0] = '\0';
		if(*s == ']')
			break;
		if(!strncmp(s, "\"pid\"", 5)) {
			snprintf(m, STRMAX, "\"%%%d[^\"]\"", STRMAX-1);
			sscanf(s+6, m, p->name);
			for(;*s; s++) {
				if(*s == '}')
					break;
				if(!strncmp(s, "\"order\"", 7))
					sscanf(s+8, "%d,", &(p->num));
			}
			p=pages[++i];
		}
	}

	free(buf);
	return i;
}

int getpageurls(char *pagecode, char *cookie) {
	char url[URLMAX], code[STRMAX], m[STRMAX];
	char *c, *d, *p, *buf = NULL;
	int i, j;

	snprintf(url, URLMAX, "/books?id=%s&pg=%s&jscmd=click3&q=subject:a", bookid, pagecode);

	if(!get("books.google.com", url, cookie, NULL, &buf))
		return 1;

	c = buf;
	while(*c && (c = strstr(c, "\"pid\":"))) {
		snprintf(m, STRMAX, "\"pid\":\"%%%d[^\"]\"", STRMAX-1);
		if(!sscanf(c, m, code))
			break;
		for(; *c; c++) {
			if(*c == '}') {
				break;
			}
			j = -1;
			if(!strncmp(c, "\"src\"", 5)) {
				for(i=0; i<totalpages; i++) {
					if(!strncmp(pages[i]->name, code, STRMAX)) {
						j = i;
						break;
					}
				}
				if(j == -1) /* TODO: it would be good to add new page on the end */
					break;  /*       of structure rather than throw it away. */
				for(p=pages[j]->url, d=c+strlen("\"src\":")+1; *d && *d != '"'; d++, p++) {
					if(!strncmp(d, "\\u0026", 6)) {
						*p = '&';
						d+=5;
					} else
						*p = *d;
				}
				strncpy(p, "&q=subject:a", 13);
				strncpy(pages[j]->cookie, cookie, COOKIEMAX);
				break;
			}
		}
	}

	free(buf);
	return 0;
}

int getpage(Page *page)
{
	char path[STRMAX];
	snprintf(path, STRMAX, "%04d.png", page->num);

	if(page->url[0] == '\0') {
		fprintf(stderr, "%s not found\n", page->name);
		return 1;
	}

	if(gettofile("books.google.com", page->url, page->cookie, NULL, path)) {
		fprintf(stderr, "%s failed\n", page->name);
		return 1;
	}

	printf("%d downloaded\n", page->num);
	return 0;
}

void searchpage(Page *page) {
	int i, j;

	if(page->url[0] != '\0')
		return;

	for(i=0; i<COOKIENUM; i++) {
		if(cookies[i][0] == '\0') /* dead cookie */
			continue;
		getpageurls(page->name, cookies[i]);
		if(page->url[0] != '\0') {
			/* invalidate old cookies if one succeeded */
			for(j=0; j<i; j++)
				cookies[j][0] = '\0';
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	char *tmp;
	char buf[BUFSIZ], pgpath[STRMAX];
	char in[16];
	int a, i, n;
	FILE *f;

	if(argc < 2 || argc > 3 || (argc == 3 && (argv[1][0]!='-'
	   || (argv[1][1] != 'c' && argv[1][1] != 'n')))
	   || (argc >= 2 && argv[1][0] == '-' && argv[1][1] == 'h')) {
		fputs(usage, stdout);
		return 1;
	}

	/* get cookies */
	for(i=0;i<COOKIENUM;i++) {
		if(get("books.google.com", "/", NULL, cookies[i], &tmp))
			free(tmp);
	}

	bookid = argv[argc-1];

	pages = malloc(sizeof(*pages) * MAXPAGES);
	for(i=0; i<MAXPAGES; i++) pages[i] = malloc(sizeof(**pages));
	if(!(totalpages = getpagelist(bookid, pages))) {
		fprintf(stderr, "Could not find any pages for %s\n", bookid);
		return 1;
	}

	if(argc == 2) {
		for(i=0; i<totalpages; i++) {
			snprintf(pgpath, STRMAX, "%04d.png", pages[i]->num);
			if((f = fopen(pgpath, "r")) != NULL) {
				fclose(f);
				continue;
			}
			searchpage(pages[i]);
			getpage(pages[i]);
		}
	} else if(argv[1][0] == '-') {
		while(fgets(buf, BUFSIZ, stdin)) {
			sscanf(buf, "%15s", in);
			i = -1;
			if(argv[1][1] == 'c') {
				for(a=0; a<totalpages; a++) {
					if(strncmp(pages[a]->name, in, STRMAX) == 0) {
						i = a;
						break;
					}
				}
			} else if(argv[1][1] == 'n') {
				sscanf(in, "%d", &n);
				for(a=0; a<totalpages; a++) {
					if(pages[a]->num == n) {
						i = a;
						break;
					}
				}
			}
			if(i == -1) {
				fprintf(stderr, "%s not found\n", in);
				continue;
			}
			searchpage(pages[i]);
			getpage(pages[i]);
		}
	}

	for(i=0; i<MAXPAGES; i++) free(pages[i]);
	free(pages);

	return EXIT_SUCCESS;
}

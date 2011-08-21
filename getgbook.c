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

int getpagelist(char *bookid, Page **pages)
{
	char url[URLMAX], m[STRMAX];
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

	return i;
}

int getpageurls(char *bookid, Page **pages, int totalpages, char *pagecode, char *cookie) {
	char url[URLMAX], code[STRMAX], m[STRMAX];
	char *c, *d, *p, *buf = NULL;
	int i;

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
			if(!strncmp(c, "\"src\"", 5)) {
				for(i=0; i<totalpages; i++)
					if(!strncmp(pages[i]->name, code, STRMAX))
						break;
				for(p=pages[i]->url, d=c+strlen("\"src\":")+1; *d && *d != '"'; d++, p++) {
					if(!strncmp(d, "\\u0026", 6)) {
						*p = '&';
						d+=5;
					} else
						*p = *d;
				}
				strncpy(p, "&q=subject:a", 13);
				strncpy(pages[i]->cookie, cookie, COOKIEMAX);
				break;
			}
		}
	}

	free(buf);
	return 0;
}

int main(int argc, char *argv[])
{
	char *bookid, *tmp, cookies[COOKIENUM][COOKIEMAX];
	char pgpath[STRMAX];
	int a, i, j, totalpages;
	FILE *f;

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
		Page **page;
		page = malloc(sizeof(*page) * MAXPAGES);
		for(i=0; i<MAXPAGES; i++) page[i] = malloc(sizeof(**page));
		if(!(totalpages = getpagelist(bookid, page))) {
			fprintf(stderr, "Could not find pages for %s\n", bookid);
			return 1;
		}
		for(i=0; i<totalpages; i++) {
			snprintf(pgpath, STRMAX, "%04d.png", page[i]->num);
			if((f = fopen(pgpath, "r")) != NULL) {
				fclose(f);
				continue;
			}
			if(page[i]->url[0] == '\0') {
				for(j=0; j<COOKIENUM; j++) {
					if(cookies[j][0] == '\0') /* dead cookie */
						continue;
					getpageurls(bookid, page, totalpages, page[i]->name, cookies[j]);
					if(page[i]->url[0] != '\0') {
						/* invalidate old cookies if one succeeded */
						for(a=0; a<j; a++)
							cookies[a][0] = '\0';
						break;
					}
				}
			}
			if(page[i]->url[0] == '\0')
				fprintf(stderr, "%s not found\n", page[i]->name);
			else {
				if(gettofile("books.google.com", page[i]->url, page[i]->cookie, NULL, pgpath))
					fprintf(stderr, "%s failed\n", page[i]->name);
				else
					printf("%d downloaded\n", page[i]->num);
			}
		}
	}

	return EXIT_SUCCESS;
}

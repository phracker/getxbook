/* See COPYING file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

#define usage "getabook " VERSION " - an amazon look inside the book downloader\n" \
              "usage: getabook [-c|-n] asin\n" \
              "  -n download pages from numbers in stdin\n" \
              "  otherwise, all available pages will be downloaded\n"

#define URLMAX 1024
#define STRMAX 1024
#define MAXPAGES 9999

typedef struct {
	int num;
	char url[URLMAX];
} Page;

Page **pages;
int numpages;
char *bookid;

int getpagelist()
{
	char url[URLMAX], b[STRMAX];
	char *buf = NULL;
	char *s, *c;
	int i;
	Page *p;

	snprintf(url, URLMAX, "/gp/search-inside/service-data?method=getBookData&asin=%s", bookid);

	if(!get("www.amazon.com", url, NULL, NULL, &buf))
		return 0;

	if((s = strstr(buf, "\"litbPages\":[")) == NULL)
		return 0;
	s+=strlen("\"litbPages\":[");

	for(i=0, p=pages[0];*s && i<MAXPAGES; s++) {
		for(c = b; *s != ',' && *s != ']'; s++, c++) *c = *s;
		*(c+1) = '\0';
		p=pages[i++]=malloc(sizeof(**pages));;
		sscanf(b, "%d,", &(p->num));
		if(s[0] == ']')
			break;
		p->url[0] = '\0';
	}
	free(buf);
	return i;
}

int getpageurls(int pagenum) {
	char url[URLMAX], m[STRMAX];
	char *c, *s, *buf = NULL;
	size_t l;
	int i;

	snprintf(url, URLMAX, "/gp/search-inside/service-data?method=goToPage&asin=%s&page=%d", bookid, pagenum);

	if(!(l = get("www.amazon.com", url, NULL, NULL, &buf)))
		return 1;

	s = strstr(buf, "\"jumboImageUrls\":{") + strlen("\"jumboImageUrls\":{");

	for(i=0; *s && i<numpages; i++) {
		c = s;

		snprintf(m, STRMAX, "\"%d\":", pages[i]->num);
		
		while(strncmp(c, m, strlen(m)) != 0) {
			while(*c && *c != '}' && *c != ',')
				c++;
			if(*c == '}')
				break;
			c++;
		}
		if(*c == '}')
			continue;

		c += strlen(m);
		if(!sscanf(c, "\"%[^\"]\"", pages[i]->url))
			continue;
	}

	free(buf);
	return 0;
}

/*
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
	fflush(stdout);
	return 0;
}
*/

int main(int argc, char *argv[])
{
	/*char *tmp;
	char buf[BUFSIZ], pgpath[STRMAX];
	char in[16];
	int a, i, n;
	FILE *f;*/

	int i;

	if(argc < 2 || argc > 3 ||
	   (argc == 3 && (argv[1][0]!='-' || argv[1][1] != 'n'))
	   || (argc >= 2 && argv[1][0] == '-' && argv[1][1] == 'h')) {
		fputs(usage, stdout);
		return 1;
	}

	bookid = argv[argc-1];

	pages = malloc(sizeof(*pages) * MAXPAGES);
	if(!(numpages = getpagelist(bookid, pages))) {
		fprintf(stderr, "Could not find any pages for %s\n", bookid);
		return 1;
	}

	for(i=0; i<numpages; i++) {
		if(pages[i]->url[0] == '\0')
			getpageurls(pages[i]->num);
		printf("page %d is %s\n", pages[i]->num, pages[i]->url);
	}

	return 0;

/*
	if(argc == 2) {
		for(i=0; i<numpages; i++) {
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
				for(a=0; a<numpages; a++) {
					if(strncmp(pages[a]->name, in, STRMAX) == 0) {
						i = a;
						break;
					}
				}
			} else if(argv[1][1] == 'n') {
				sscanf(in, "%d", &n);
				for(a=0; a<numpages; a++) {
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
*/

	for(i=0; i<numpages; i++) free(pages[i]);
	free(pages);

	return EXIT_SUCCESS;
}

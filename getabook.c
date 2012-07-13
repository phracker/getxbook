/* See COPYING file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#ifdef WINVER
#define mkdir(D, M) _mkdir(D)
#endif
#include "util.h"

#define usage "getabook " VERSION " - an amazon look inside the book downloader\n" \
              "usage: getabook [-n] asin\n" \
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
char bookid[STRMAX];
char *bookdir;

int fillurls(char *buf) {
	char m[STRMAX];
	char *c, *s;
	int i;

	if(!(s = strstr(buf, "\"jumboImageUrls\":{"))) {
		return 1;
	}
	s += strlen("\"jumboImageUrls\":{");

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
		if(!sscanf(c, "\"//sitb-images.amazon.com%[^\"]\"", pages[i]->url))
			continue;
	}

	return 0;
}

int getpagelist()
{
	char url[URLMAX];
	char b[STRMAX] = "";
	char *buf = NULL;
	char *s, *c;
	int i;
	Page *p;

	snprintf(url, URLMAX, "/gp/search-inside/service-data?method=getBookData&asin=%s", bookid);

	if(!get("www.amazon.com", url, NULL, NULL, &buf, 1))
		return 1;

	/* amazon have a canonical asin, which is needed to get all available pages */
	if((s = strstr(buf, "\"ASIN\":\"")) != NULL) {
		s+=strlen("\"ASIN\":\"");
		strncpy(bookid, s, 10);
		bookid[10] = '\0';
	}

	if((s = strstr(buf, "\"litbPages\":[")) == NULL) {
		free(buf);
		return 1;
	}
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
	numpages = i;

	fillurls(buf);

	free(buf);
	return 0;
}

int getpageurls(int pagenum) {
	char url[URLMAX];
	char query[URLMAX];
	char *buf = NULL;

	strncpy(url, "/gp/search-inside/service-data", URLMAX);
	snprintf(query, URLMAX, "method=goToPage&asin=%s&page=%d", bookid, pagenum);

	if(!post("www.amazon.com", url, NULL, NULL, query, &buf, 1))
		return 1;

	fillurls(buf);

	free(buf);
	return 0;
}

int getpage(Page *page)
{
	char path[STRMAX];
	snprintf(path, STRMAX, "%s/%04d.png", bookdir, page->num);

	if(page->url[0] == '\0') {
		fprintf(stderr, "%d not found\n", page->num);
		return 1;
	}

	if(gettofile("sitb-images.amazon.com", page->url, NULL, NULL, path, 0)) {
		fprintf(stderr, "%d failed\n", page->num);
		return 1;
	}
	renameifjpg(path);

	printf("%d downloaded\n", page->num);
	fflush(stdout);
	return 0;
}

int main(int argc, char *argv[])
{
	char buf[BUFSIZ], pgpath[STRMAX], pgpath2[STRMAX];
	char in[16];
	int a, i, n;
	FILE *f;
	DIR *d;

	if(argc < 2 || argc > 3 ||
	   (argc == 3 && (argv[1][0]!='-' || argv[1][1] != 'n'))
	   || (argc >= 2 && argv[1][0] == '-' && argv[1][1] == 'h')) {
		fputs(usage, stdout);
		return 1;
	}

	strncpy(bookid, argv[argc-1], STRMAX-1);
	bookid[STRMAX-1] = '\0';
	bookdir = argv[argc-1];

	pages = malloc(sizeof(*pages) * MAXPAGES);
	if(getpagelist()) {
		fprintf(stderr, "Could not find any pages for %s\n", bookid);
		return 1;
	}

	if(!((d = opendir(bookdir)) || !mkdir(bookdir, S_IRWXU))) {
		fprintf(stderr, "Could not create directory %s\n", bookdir);
		return 1;
	}
	if(d) closedir(d);

	if(argc == 2) {
		for(i=0; i<numpages; i++) {
			snprintf(pgpath, STRMAX, "%s/%04d.png", bookdir, pages[i]->num);
			snprintf(pgpath2, STRMAX, "%s/%04d.jpg", bookdir, pages[i]->num);
			if((f = fopen(pgpath, "r")) != NULL || (f = fopen(pgpath2, "r")) != NULL) {
				fclose(f);
				continue;
			}
			if(pages[i]->url[0] == '\0')
				getpageurls(pages[i]->num);
			getpage(pages[i]);
		}
	} else if(argv[1][0] == '-' && argv[1][1] == 'n') {
		while(fgets(buf, BUFSIZ, stdin)) {
			sscanf(buf, "%15s", in);
			i = -1;
			sscanf(in, "%d", &n);
			for(a=0; a<numpages; a++) {
				if(pages[a]->num == n) {
					i = a;
					break;
				}
			}
			if(i == -1) {
				fprintf(stderr, "%s not found\n", in);
				continue;
			}
			if(pages[i]->url[0] == '\0')
				getpageurls(pages[i]->num);
			getpage(pages[i]);
		}
	}

	for(i=0; i<numpages; i++) free(pages[i]);
	free(pages);

	return EXIT_SUCCESS;
}

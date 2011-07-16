/* See COPYING file for copyright, license and warranty details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.c"

#define usage "getgbook " VERSION " - a google books downloader\n" \
              "usage: getgbook [-p|-a] isbn\n" \
              "  -p print all available pages\n" \
              "  -a download all available pages\n" \
              "  otherwise, all pages in stdin will be downloaded\n"

#define URLMAX 1024
#define BOOKID_LEN 12

typedef struct {
	int num;
	char url[URLMAX];
	char name[80];
} Page;

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
	while(*c && *c != '>') c--;
	sscanf(c+1, "%d ", &total);

	return total;
}

Page *getpagedetail(char *bookid, char *pg)
{
	char url[URLMAX];
	char *buf, *c, *d, m[80], *p;
	Page *page;

	snprintf(url, URLMAX, "/books?id=%s&pg=%s&jscmd=click3", bookid, pg);

	if(!get("books.google.com", url, &buf))
		return NULL;

	snprintf(m, 80, "\"pid\":\"%s\"", pg);
	if((c = strstr(buf,m)) == NULL) {
		free(buf); return NULL;
	}

	page = malloc(sizeof(Page));
	strncpy(page->name, pg, 80);
	page->url[0] = '\0';
	page->num = 0;

	if(strncmp(c+strlen(m)+1, "\"src\"", 5) != 0) {
		free(buf); return page;
	}

	for(p=page->url, d=c+strlen(m)+8; *d && *d != '"'; d++, p++) {
		if(!strncmp(d, "\\u0026", 6)) {
			*p = '&';
			d+=5;
		} else
			*p = *d;
	}
	*p = '\0';

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
	char *bookid, pg[16], buf[1024], n[80];
	int totalpages, i;
	Page *page;

	if(argc < 2 || argc > 3 ||
	   (argv[1][0]=='-' && ((argv[1][1]!='p' && argv[1][1]!='a') || argc < 3)))
		die(usage);

	if((bookid = getbookid(argv[argc-1])) == NULL)
		die("Could not find book\n");

	if(argv[1][0] == '-') {
		/* note this isn't the best way, not least because it misses the
		 * non PA pages. best is to crawl around the json grabbing everything
		 * available, by starting on PP1, and filling in by going through
		 * all pages in totalpages. */
		if(!(totalpages = gettotalpages(bookid)))
			die("Book has no pages\n");

		for(i=1; i<totalpages; i++) {
			snprintf(pg, 16, "%s%d", "PA", i);
			if((page = getpagedetail(bookid, pg)) == NULL || page->url[0] == '\0') {
				fprintf(stderr, "%s failed\n", pg);
				free(page);
				continue;
			}
			if(argv[1][1] == 'a') {
				snprintf(n, 80, "%05d.png", page->num);
				gettofile(page->url, n);
				printf("Downloaded page %d\n", page->num);
			} else
				printf("%d\n", page->num);
			free(page);
		}
	} else {
		/* todo: find the page based on its order number, rather than using PA%d */
		while(fgets(buf, 1024, stdin)) {
			sscanf(buf, "%d", &i);
			snprintf(pg, 16, "%s%d", "PA", i);
			if((page = getpagedetail(bookid, pg)) == NULL || page->url[0] == '\0') {
				fprintf(stderr, "%d failed\n", i);
				free(page);
				continue;
			}
			snprintf(n, 80, "%05d.png", page->num);
			gettofile(page->url, n);
			printf("Downloaded page %d\n", page->num);
			free(page);
		}
	}

	free(bookid);

	return EXIT_SUCCESS;
}

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

#define usage "getbnbook " VERSION " - a barnes and noble book downloader\n" \
              "usage: getbnbook [-n] isbn13\n" \
              "  -n download pages from numbers in stdin\n" \
              "  otherwise, all available pages will be downloaded\n"

#define URLMAX 1024
#define STRMAX 1024
#define MAXPAGES 9999

int pages[MAXPAGES];
int numpages;
char urlpath[STRMAX];
char bookid[STRMAX];
char *bookdir;
char cookies[COOKIEMAX];

int getpagelist()
{
	char url[URLMAX];
	char *buf = NULL;
	char *s, *l;
	int i, num;
	char avail[STRMAX];
	char dummy1[STRMAX], dummy2[STRMAX];

	numpages = 0;

	snprintf(url, URLMAX, "/DigBooks/viewer/bookviewmanager.aspx?op=getbookinfo&ean=%s", bookid);

	if(!get("search2.barnesandnoble.com", url, NULL, cookies, &buf))
		return 1;

	/* find page url structure */
	if((s=strstr(buf, "<imagesize name=\"med\"><param name=\"path\">")) == NULL) {
		free(buf);
		return 1;
	}
	s+=strlen("<imagesize name=\"med\"><param name=\"path\">");
	l = strchr(s, '<');
	for(i=0; s<l && i<STRMAX; s++,i++)
		urlpath[i] = *s;
	urlpath[i] = '\0';

	/* find available pages */
	for(i=0, s=buf;*s && i<MAXPAGES; s++) {
		if((s = strstr(s, "<page ")) == NULL)
			break;
		sscanf(s, "<page sequence=\"%d\" type=\"bitmap\" toc=\"%[^\"]\" folio=\"%[^\"]\" freevendstatus=\"%[^\"]\" />", &num, dummy1, dummy2, avail);

		if(strncmp(avail, "true", STRMAX) == 0)
			pages[i++] = num;
	}

	numpages = i;

	free(buf);
	return 0;
}

int getpage(int pagenum)
{
	char path[STRMAX], pageurl[STRMAX];
	char *s;

	snprintf(path, STRMAX, "%s/%04d.png", bookdir, pagenum);

	s=strchr(urlpath+7, '/');
	snprintf(pageurl, STRMAX, s, pagenum);

	if(gettofile("search2.barnesandnoble.com", pageurl, cookies, NULL, path)) {
		fprintf(stderr, "%d failed\n", pagenum);
		return 1;
	}
	renameifjpg(path);

	printf("%d downloaded\n", pagenum);
	fflush(stdout);
	return 0;
}

int main(int argc, char *argv[])
{
	char *tmp;
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

	strncpy(bookid, argv[argc-1], STRMAX);
	bookdir = argv[argc-1];

	/* get cookie */
	if(get("www.barnesandnoble.com", "/", NULL, cookies, &tmp))
		free(tmp);

	if(getpagelist(bookid, pages)) {
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
			snprintf(pgpath, STRMAX, "%s/%04d.png", bookdir, pages[i]);
			snprintf(pgpath2, STRMAX, "%s/%04d.jpg", bookdir, pages[i]);
			if((f = fopen(pgpath, "r")) != NULL || (f = fopen(pgpath2, "r")) != NULL) {
				fclose(f);
				continue;
			}
			getpage(pages[i]);
		}
	} else if(argv[1][0] == '-' && argv[1][1] == 'n') {
		while(fgets(buf, BUFSIZ, stdin)) {
			sscanf(buf, "%15s", in);
			i = -1;
			sscanf(in, "%d", &n);
			for(a=0; a<numpages; a++) {
				if(pages[a] == n) {
					i = a;
					break;
				}
			}
			if(i == -1) {
				fprintf(stderr, "%s not found\n", in);
				continue;
			}
			getpage(pages[i]);
		}
	}

	return EXIT_SUCCESS;
}

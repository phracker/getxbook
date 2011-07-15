#define usage "getgbook bookid"

#define hostname "books.google.com"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.c"

char * get(FILE *srv, char *host, char *path) {
	size_t l, res;
	int fd, i;
	char *buf, *c, *p;

	fprintf(srv, "GET %s HTTP/1.0\r\nUser-Agent: getgbook-prealpha (not mozilla)\r\nHost: %s\r\n\r\n", path, host);

	fflush(srv);

	l=0;
	fd = fileno(srv);

	buf = malloc(sizeof(char *) * 4096);
	for(i=0; (res = read(fd, buf+l, 4096)) > 0; l+=res, i++)
		buf = realloc(buf, sizeof(char *) * (l+4096));

	/* check that it's a 200 */
	if(strncmp(buf+9, "200 ", 4)) {
		free(buf);
		return NULL;
	}

	/* exclude header */
	for(p = buf; *p && *(p+1) && *(p+2) && *(p+3); p++)
		if(!strncmp(p, "\r\n\r\n", 4)) break;
	p+=4;
	
	i = l - (p - buf);
	c = malloc(i);
	memcpy(c, p, i);
	free(buf);

	return c;
}

int main(int argc, char *argv[])
{
	int i, s;
	char *bookid, url[80];
	char *curpage;
	FILE *srv;

	if(argc != 2)
		die("usage: " usage "\n");

	bookid = argv[1];

	i = dial(hostname, "80");
	srv = fdopen(i, "r+");

	snprintf(url, 80, "/books?id=%s&pg=%s&jscmd=click3", bookid, "PA1");
	if((curpage = get(srv, hostname, url)) == NULL)
		fprintf(stderr,"Error downloading page\n");
	else {
		fputs(curpage,stdout);
		free(curpage);
	}

	return 0;
}

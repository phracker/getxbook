/* See COPYING file for copyright and license details. */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "util.h"

/* plundered from suckless' sic */
int dial(char *host, char *port) {
	static struct addrinfo hints;
	int srv;
	struct addrinfo *res, *r;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if(getaddrinfo(host, port, &hints, &res) != 0) {
		fprintf(stderr, "error: cannot resolve hostname %s\n", host);
		return -1;
	}
	for(r = res; r; r = r->ai_next) {
		if((srv = socket(r->ai_family, r->ai_socktype, r->ai_protocol)) == -1)
			continue;
		if(connect(srv, r->ai_addr, r->ai_addrlen) == 0)
			break;
		close(srv);
	}
	freeaddrinfo(res);
	if(!r) {
		fprintf(stderr, "error: cannot connect to host %s\n", host);
		return -1;
	}
	return srv;
}

int get(char *host, char *path, char *sendcookie, char *savecookie, char **buf) {
	size_t l, res;
	int fd, i, p;
	char h[HEADERMAX] = "\0";
	char c[COOKIEMAX] = "";
	FILE *srv;

	if((fd = dial(host, "80")) == -1) return 0;
	srv = fdopen(fd, "r+");

	if(sendcookie)
		snprintf(c, COOKIEMAX, "\r\nCookie: %s", sendcookie);
	fprintf(srv, "GET %s HTTP/1.0\r\nUser-Agent: getxbook-"VERSION \
	             " (not mozilla)\r\nHost: %s%s\r\n\r\n", path, host, c);
	fflush(srv);

	while(h[0] != '\r') {
		if(!fgets(h, HEADERMAX, srv)) return 0;
		if(sscanf(h, "HTTP/%d.%d %d", &i, &i, &p) == 3 && p != 200)
			return 0;
		if(savecookie != NULL && sscanf(h, "Set-Cookie: %s;", c))
			strncat(savecookie, c, COOKIEMAX);
	}

	*buf = malloc(sizeof(char *) * BUFSIZ);
	for(l=0; (res = fread(*buf+l, 1, BUFSIZ, srv)) > 0; l+=res)
		*buf = realloc(*buf, sizeof(char *) * (l+BUFSIZ));

	fclose(srv);
	return l;
}

int gettofile(char *host, char *url, char *sendcookie, char *savecookie, char *savepath) {
	char *buf = 0;
	FILE *f;
	size_t i, l;

	if(!(l = get(host, url, sendcookie, savecookie, &buf))) {
		fprintf(stderr, "Could not download %s\n", url);
		return 1;
	}
	if((f = fopen(savepath, "w")) == NULL) {
		fprintf(stderr, "Could not create file %s\n", savepath);
		free(buf); return 1;
	}

	for(i=0; i < l; i+=512)
		if(!fwrite(buf+i, l-i > 512 ? 512 : l-i, 1, f)) {
			fprintf(stderr, "Error writing file %s\n", savepath);
			free(buf); fclose(f); return 1;
		}

	free(buf);
	fclose(f);

	return 0;
}

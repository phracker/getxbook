/* See COPYING file for copyright and license details. */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "util.h"
#ifndef WINVER
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

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
	char h[HDRMAX] = "\0";
	char c[COOKIEMAX] = "";
	char t[BUFSIZ];
	char *t2;

	if((fd = dial(host, "80")) == -1) return 0;

	if(sendcookie && sendcookie[0])
		snprintf(c, COOKIEMAX, "\r\nCookie: %s", sendcookie);
	snprintf(h, HDRMAX, "GET %s HTTP/1.0\r\nUser-Agent: getxbook-"VERSION \
	                    " (not mozilla)\r\nHost: %s%s\r\n\r\n", path, host, c);
	if(!send(fd, h, HDRMAX, 0)) return 0;

	*buf = NULL;
	l = 0;
	while((res = recv(fd, t, 1024, 0)) > 0) {
		if(sscanf(t, "HTTP/%d.%d %d", &i, &i, &p) == 3 && p != 200)
			return 0;
		if(savecookie != NULL && sscanf(t, "Set-Cookie: %s;", c))
			strncat(savecookie, c, COOKIEMAX);
		if((t2 = strstr(t, "\r\n\r\n") + 4)) {
			*(t2-1) = '\0';
			l = res - strlen(t) - 1;
			*buf = malloc(sizeof(char *) * l);
			memcpy(*buf, t2, l);
			break;
		}
	}

	*buf = realloc(*buf, sizeof(char *) * (l+BUFSIZ));
	for(; (res = recv(fd, *buf+l, BUFSIZ, 0)) > 0; l+=res)
		*buf = realloc(*buf, sizeof(char *) * (l+BUFSIZ));

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

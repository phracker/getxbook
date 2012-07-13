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

	#ifdef WINVER
	WSADATA w;
	if(WSAStartup(MAKEWORD(2,2), &w)!= 0) {
		fprintf(stderr, "error: failed to start winsock\n");
		return -1;
	}
	#endif

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

int request(char *host, char *request, char *savecookie, char **body, int istext) {
	size_t l, res;
	int fd, i, p;
	char m[256];
	char c[COOKIEMAX] = "";
	char *headpos;
	size_t headsize, bodysize;
	char headline[BUFSIZ] = "";
	char *buf, *cur, *pos;

	if((fd = dial(host, "80")) == -1) return 0;

	if(!send(fd, request, strlen(request), 0)) return 0;

	/* download everything into buf */
	l = 0;
	buf = malloc(sizeof(char *) * BUFSIZ);
	for(; buf != NULL && (res = recv(fd, buf+l, BUFSIZ, 0)) > 0; l+=res)
		buf = realloc(buf, sizeof(char *) * (l+BUFSIZ));

	/* strstr to find end of header */
	if((headpos = strstr(buf, "\r\n\r\n")) == NULL) {
		free(buf);
		return 0;
	}
	headpos += 4;
	headsize = headpos - buf;
	bodysize = l - headsize;
	if(istext) {
		bodysize++; /* for null terminator */
	}

	/* memcopy from there into a large enough buf */
	if((*body = malloc(sizeof(char *) * bodysize)) == NULL) {
		free(buf);
		return 0;
	}
	memcpy(*body, headpos, sizeof(char *) * (l - headsize));
	if(istext) {
		(*body)[bodysize - 1] = '\0';
	}

	/* parse header as needed */
	snprintf(m, 256, "Set-Cookie: %%%ds;", COOKIEMAX-1);
	cur = buf;
	while((pos = strstr(cur, "\r\n")) != NULL && cur < (headpos - 4)) {
		strncpy(headline, cur, pos - cur);
		headline[pos - cur] = '\0';
		cur = pos + 2;

		if(sscanf(headline, "HTTP/%d.%d %d", &i, &i, &p) == 3 && p != 200) {
			if(p == 403)
				fprintf(stderr, "403 forbidden: your IP address may be temporarily blocked\n");
			free(buf);
			free(*body);
			return 0;
		}

		if(savecookie != NULL && sscanf(headline, m, c)) {
			strncat(savecookie, c, COOKIEMAX - strlen(savecookie) - 1);
		}
	}

	free(buf);

	return l;
}

int get(char *host, char *path, char *sendcookie, char *savecookie, char **body, int istext) {
	char h[BUFSIZ] = "";
	char c[COOKIEMAX] = "";

	if(sendcookie && sendcookie[0])
		snprintf(c, COOKIEMAX, "\r\nCookie: %s", sendcookie);
	snprintf(h, BUFSIZ, "GET %s HTTP/1.0\r\nUser-Agent: getxbook-"VERSION \
	                    " (not mozilla)\r\nHost: %s%s\r\n\r\n", path, host, c);

	return request(host, h, savecookie, body, istext);
}

int post(char *host, char *path, char *sendcookie, char *savecookie, char *data, char **body, int istext) {
	char h[BUFSIZ] = "";
	char c[COOKIEMAX] = "";

	if(sendcookie && sendcookie[0])
		snprintf(c, COOKIEMAX, "\r\nCookie: %s", sendcookie);
	snprintf(h, BUFSIZ, "POST %s HTTP/1.0\r\nUser-Agent: getxbook-"VERSION \
	                    " (not mozilla)\r\nContent-Length: %d\r\n" \
	                    "Content-Type: application/x-www-form-urlencoded\r\n" \
	                    "Host: %s%s\r\n\r\n%s\r\n",
	                    path, (int)strlen(data), host, c, data);

	return request(host, h, savecookie, body, istext);
}

int gettofile(char *host, char *url, char *sendcookie, char *savecookie, char *savepath, int istext) {
	char *buf = 0;
	FILE *f;
	size_t i, l;

	if(!(l = get(host, url, sendcookie, savecookie, &buf, istext))) {
		fprintf(stderr, "Could not download %s\n", url);
		return 1;
	}
	if((f = fopen(savepath, "wb")) == NULL) {
		fprintf(stderr, "Could not create file %s\n", savepath);
		free(buf); return 1;
	}

	for(i=0; i < l; i+=BUFSIZ)
		if(!fwrite(buf+i, l-i > BUFSIZ ? BUFSIZ : l-i, 1, f)) {
			fprintf(stderr, "Error writing file %s\n", savepath);
			free(buf); fclose(f); return 1;
		}

	free(buf);
	fclose(f);

	return 0;
}

int renameifjpg(char *path) {
	FILE *f;
	char *newpath, *c;

	if((f = fopen(path, "rb")) == NULL)
		return 1;

	if(fgetc(f) == 255) {
		if((newpath = malloc(strlen(path) + 1)) == NULL) {
			fclose(f);
			return 1;
		}
		strncpy(newpath, path, strlen(path)+1);
		c = strrchr(newpath, '.');
		strncpy(c+1, "jpg\0", 4);

		if(rename(path, newpath)) {
			free(newpath);
			fclose(f);
			return 1;
		}
		free(newpath);
	}

	fclose(f);

	return 0;
}

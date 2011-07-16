/* See COPYING file for copyright, license and warranty details. */

#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

void die(char *msg) {
	fputs(msg, stderr);
	exit(EXIT_FAILURE);
}

/* plundered from suckless' sic */
static int dial(char *host, char *port) {
	static struct addrinfo hints;
	int srv;
	struct addrinfo *res, *r;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if(getaddrinfo(host, port, &hints, &res) != 0)
		die("error: cannot resolve hostname\n");
	for(r = res; r; r = r->ai_next) {
		if((srv = socket(r->ai_family, r->ai_socktype, r->ai_protocol)) == -1)
			continue;
		if(connect(srv, r->ai_addr, r->ai_addrlen) == 0)
			break;
		close(srv);
	}
	freeaddrinfo(res);
	if(!r)
		die("error: cannot connect to host\n");
	return srv;
}

int get(char *host, char *path, char **gotbuf) {
	size_t l, res;
	int fd, i;
	char *buf, *c, *p;
	FILE *srv;

	fd = dial(host, "80");
	srv = fdopen(fd, "r+");

	fprintf(srv, "GET %s HTTP/1.0\r\nUser-Agent: getgbook-"VERSION" (not mozilla)\r\nHost: %s\r\n\r\n", path, host);
	fflush(srv);

	buf = malloc(sizeof(char *) * 4096);
	for(i=0, l=0; (res = read(fd, buf+l, 4096)) > 0; l+=res, i++)
		buf = realloc(buf, sizeof(char *) * (l+4096));

	/* check that it's a 200 */
	if(strncmp(buf+9, "200 ", 4)) {
		free(buf);
		return 0;
	}

	/* exclude header */
	for(p = buf; *p && *(p+1) && *(p+2) && *(p+3); p++)
		if(!strncmp(p, "\r\n\r\n", 4)) break;
	p+=4;

	i = l - (p - buf);
	c = malloc(i+1);
	memcpy(c, p, i);
	free(buf);

	*gotbuf = c;

	return i;
}

int gettofile(char *url, char *savepath) {
	char *buf = 0;
	FILE *f;
	size_t i, l;

	if(!(l = get("books.google.com", url, &buf))) {
		fprintf(stderr, "Could not download %s\n", url);
		return 1;
	}

	if((f = fopen(savepath, "w")) == NULL) {
		fprintf(stderr, "Could not create file %s\n", savepath);
		return 1;
	}

	for(i=0; i < l; i+=512)
		if(!fwrite(buf+i, l-i > 512 ? 512 : l-i, 1, f)) {
			fprintf(stderr, "Error writing file %s\n", savepath);
			free(buf); fclose(f);
			return 1;
		}

	free(buf);
	fclose(f);

	return 0;
}

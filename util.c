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

int get(char *host, char *path, char **buf) {
	size_t l, res;
	int fd, i;
	char h[1024] = "\0";
	FILE *srv;

	fd = dial(host, "80");
	srv = fdopen(fd, "r+");

	fprintf(srv, "GET %s HTTP/1.0\r\nUser-Agent: getgbook-"VERSION \
	             " (not mozilla)\r\nHost: %s\r\n\r\n", path, host);
	fflush(srv);

	while(h[0] != '\r') {
		fgets(h, 1024, srv);
		if(sscanf(h, "HTTP/%d.%d %d", &i, &i, &l) == 3 && l != 200)
			return 1;
	}

	*buf = malloc(sizeof(char *) * 4096);
	for(l=0; (res = fread(*buf+l, 1, 4096, srv)) > 0; l+=res)
		*buf = realloc(*buf, sizeof(char *) * (l+4096));

	fclose(srv);
	return l;
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

/* See LICENSE file for license details. */
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

void die(char *msg) {
	fputs(msg, stderr);
	exit(EXIT_FAILURE);
}

/* plundered from suckless' sic */
static int
dial(char *host, char *port) {
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

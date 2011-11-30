/* See COPYING file for copyright and license details. */
#define COOKIEMAX 1024
#define HDRMAX 1024
int dial(char *host, char *port);
int get(char *host, char *path, char *sendcookie, char *savecookie, char **buf);
int gettofile(char *host, char *url, char *sendcookie, char *savecookie, char *savepath);
int post(char *host, char *path, char *data, char **buf);
int renameifjpg(char *path);

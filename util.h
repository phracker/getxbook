/* See COPYING file for copyright and license details. */
#define COOKIEMAX 1024
int dial(char *host, char *port);
int get(char *host, char *path, char *sendcookie, char *savecookie, char **body);
int post(char *host, char *path, char *sendcookie, char *savecookie, char *data, char **body);
int gettofile(char *host, char *url, char *sendcookie, char *savecookie, char *savepath);
int renameifjpg(char *path);

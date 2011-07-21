/* See COPYING file for copyright, license and warranty details. */
int dial(char *host, char *port);
int get(char *host, char *path, char **buf);
int gettofile(char *host, char *url, char *savepath);

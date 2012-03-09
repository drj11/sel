/*  sel
    a command line tool for select(2)
 */

#define _POSIX_C_SOURCE 200112L
#include <sys/select.h>
#include <unistd.h>

#include <math.h>
#include <stdlib.h>
#include <string.h>

int sel_string(char *rs,
  char *ws, char *es, double timeout);

int
main(int argc, char **argv)
{
    int opt;
    char *rs=NULL, *ws=NULL, *es=NULL, *t=NULL;
    double timeout = -1;

    while((opt=getopt(argc, argv, "r:w:e:t:")) != -1) {
        switch(opt) {
            case 'r':
                rs = optarg;
                break;
            case 'w':
                ws = optarg;
                break;
            case 'e':
                es = optarg;
                break;
            case 't':
                t = optarg;
                break;
            case '?':
                return 99;
        }
    }

    if(t) {
        timeout = atof(t);
    }

    return sel_string(rs, ws, es, timeout);
}

/*  *target* should be a comma separated string of non-negative
    integers.  The fd_set *s is updated by setting each FD that
    appears in the list.

    The max fd is returned.
 */
int
to_fd_set(char *target, fd_set *s)
{
    char *p;
    int fd;
    int max = -1;

    p = strtok(target, ",");
    while(p) {
        fd = strtol(p, NULL, 0);
        FD_SET(fd, s);
        max = (int)fmax(max, fd);
        p = strtok(NULL, ",");
    }
    return max;
}

/*  *rs*, *ws*, and *es* are comma separated strings of integer
    FDs.  *timeout* is the, floating point, timeout value for the
    select; if negative the select(3) call will not time out.
 */
int
sel_string(char *rs, char *ws, char *es, double timeout)
{
    fd_set rfds;
    fd_set wfds;
    fd_set efds;

    int max = -1;
    fd_set *rarg=NULL, *warg=NULL, *earg=NULL;

    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_ZERO(&efds);

    if(rs) {
        max = (int)fmax(max, to_fd_set(rs, &rfds));
        rarg = &rfds;
    }
    if(ws) {
        max = (int)fmax(max, to_fd_set(ws, &wfds));
        warg = &wfds;
    }
    if(es) {
        max = (int)fmax(max, to_fd_set(es, &efds));
        earg = &efds;
    }
    printf("max %d\n", max);
    return 0;
}

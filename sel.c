/*  sel
    a command line tool for select(2)
 */

#define _POSIX_C_SOURCE 200112L
#include <sys/select.h>
#include <unistd.h>

#include <errno.h>
#include <math.h>
#include <stdio.h>
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

/*  Count number of set FDs, and return the count. */
int
any_fd_set(int nfds, fd_set *s)
{
    int i;
    int n=0;

    for(i=0; i<nfds; ++i) {
        if(FD_ISSET(i, s)) {
            n += 1;
        }
    }
    return n;
}

/*  Print the first *nset* FDs that are set. */
void
print_fd(int nfds, fd_set *s)
{
    int i=0;

    while(nfds) {
        if(FD_ISSET(i, s)) {
            nfds -= 1;
            printf("%d", i);
            if(nfds) {
                printf(", ");
            }
        }
        ++i;
    }
    return;
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
    struct timeval tv;
    /* When select returns, the number of set FDs in each of
       rfds, wfds, efds. */
    int nr=0, nw=0, ne=0;
    int ret;

    int max = -1;
    fd_set *rarg=NULL, *warg=NULL, *earg=NULL;
    struct timeval *targ=NULL;

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
    if(timeout >= 0.0) {
        tv.tv_sec = (long)timeout;
        tv.tv_usec = (long)(1000000*(timeout-tv.tv_sec));
        targ = &tv;
    }

    ret = select(max+1, rarg, warg, earg, targ);
    if(ret == -1) {
        perror("select");
        return errno;
    }

    printf("[\n");
    if(rarg) {
        nr = any_fd_set(max+1, rarg);
    }
    if(warg) {
        nw = any_fd_set(max+1, warg);
    }
    if(earg) {
        ne = any_fd_set(max+1, earg);
    }
    if(rarg && nr) {
        printf(" \"read\": [");
        print_fd(nr, rarg);
        printf("]");
        if(nw || ne) {
            printf(",");
        }
        printf("\n");
    }
    if(warg && nw) {
        printf(" \"write\": [");
        print_fd(nw, warg);
        printf("]");
        if(ne) {
            printf(",");
        }
        printf("\n");
    }
    if(earg && ne) {
        printf(" \"error\": [");
        print_fd(ne, earg);
        printf("]\n");
    }
    printf("]\n");

    return 0;
}

#include <stdio.h>
#include <sys/epoll.h>
/* epoll is still a blocking I/O call, same as select & poll,
 * the difference is the decoupling of register a fd listener and
 * the actual listening behavior.
 */
int main() {
    
    // initialize a epoll instance, and associates it with a fd, and return the fd
    // epfd has no relationship to the real fd. Think of it as what programmers 
    // oftens as "tmp" variable
    int epfd = epoll_create1(0);
    if(epfd < 0) {
        perror("epoll_create1");
        return -1;
    }

    // control the epoll to watch the STDIN
    struct epoll_event event;
    int ret;
    
    event.data.fd = STDIN_FILENO;
    event.events = EPOLLIN | EPOLLOUT;

    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &event);
    if(ret) {
        perror("epoll_ctl");
        return -1;
    }




    return 0;
}

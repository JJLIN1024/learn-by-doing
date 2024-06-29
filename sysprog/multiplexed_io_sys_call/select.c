#include <unistd.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>

#define TIMEOUT 5
#define BUF_LEN 1024

int main() {
    // select 
    // on return:
    // 1. timeval is modified, so user has to reinitialized it
    // 2. fd_set is modified, so user has to reinitialized it
    // 
    // User has to manually calculate the biggest fd number and pass it
    // as the first paramter, and, fd_set is statically initialized, it is 
    // inefficient when dealing with fd that has large value, since the kernel
    // does not know the fd_set is sparse or not, it'll has to check it
    // manually, which is costly, for example, use select to wait on a single fd 
    // with value 10000.
    struct timeval tv;
    fd_set readfds;
    int ret;

    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;
    
    // blocking read on selected fd(s), in this case
    // only waiting on STDIN
    ret = select(STDIN_FILENO + 1,
            &readfds,
            NULL,
            NULL,
            &tv);
    if(ret == -1) {
        perror("select");
        return -1;
    } else if(!ret) {
        printf("%d seconds elapsed.\n", TIMEOUT);
        return 0;
    }

    if(FD_ISSET(STDIN_FILENO, &readfds)) {
        char buf[BUF_LEN];
        int len = read(STDIN_FILENO, buf, BUF_LEN);
        if(len == -1) {
            perror("read");
            return -1;
        }

        if(len) {
            buf[len] = '\0';
            printf("read: %s\n", buf);
        }
    }
    return 0;
}

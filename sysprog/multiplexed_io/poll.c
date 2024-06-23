#include <unistd.h>
#include <stdio.h>
#include <poll.h>

#define TIMEOUT 5

int main() {
    // poll is superior to select, but less portable
    struct pollfd fds[2];
    int ret;
    // watch for read
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    // watch for write 
    fds[1].fd = STDOUT_FILENO;
    fds[1].events = POLLOUT;

    // blocking
    ret = poll(fds, 2, TIMEOUT * 1000);
    if(ret == -1) {
        perror("poll");
        return 1;
    } else if(!ret) {
        printf("%d seconds elapsed.\n", TIMEOUT * 1000);
        return 0;
    }

    if(fds[0].revents & POLLIN) {
        printf("stdin is readable\n");
    }
    if(fds[1].revents & POLLOUT) {
        printf("stdout is writable\n");
    }
    return 0;
}

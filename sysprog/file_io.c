#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
    int fd;
    fd = open("", O_RDONLY);
    if(fd == -1) {
        perror("fail to read file");
        return -1;
    }
    return 0;
}

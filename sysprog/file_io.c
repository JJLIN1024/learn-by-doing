#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

int main() {
    // create a write only file
    int fd;
    fd = creat("random_text.txt", 0644);
    // this is equivalent to:
    // fd = open("random_text.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(fd == -1) {
        perror("fail to create file");
        return -1;
    }

    // write to file
    // a write to regular file is gauranteed to perform the entire requested write 
    // unless an error occure, while write to a special file like socket will need
    // a while loop
    ssize_t ret;
    const char* message = "Hello World!";
    size_t message_len = strlen(message);
    char* m_ptr = message;
    int len = message_len;
    while((len != 0) && (ret = write(fd, m_ptr, len)) != 0) {
        if(ret == -1) {
            if(errno == EINTR) 
                continue;
            perror("write");
            break;
        }

        len -= ret;
        m_ptr += ret;
    }
    
    // fdatasync only writes data and essential meta back to disk
    // while fsync writes both data and inode back to disk, which
    // is more expensive. One can use O_SYNC to open the file, to 
    // force every write operation to be syncronized.
    ret = fdatasync(fd);
    // ret = fsync(fd);
    if(ret == -1) {
        perror("fail to write file back to disk");
        return -1;
    }

    // sync() will flush all buffers, not just file corresponding to fd, to the disk
    // which is way more expensive than fsync();
    // sync();

    // close fd for write
    if(close(fd) == -1) {
        perror("close");
    }
    
    fd = open("random_text.txt", O_RDONLY);
    if(fd == -1) {
        perror("fail to read file");
        return -1;
    }
    // blocking read
    len = message_len;
    char buf[len];
    char* buf_ptr = buf;
    while((len != 0) && (ret = read(fd, buf_ptr, len)) != 0) { // read return 0 on EOF
        if(ret == -1) {
            if(errno == EINTR) 
                continue;
            perror("read");
            break;
        }

        len -= ret;
        buf_ptr += ret;
    }
    
    for(size_t i = 0; i < message_len; i++) {
        printf("%c", buf[i]);
    }

    // close fd for read
    if(close(fd) == -1) {
        perror("close");
    }

    return 0;
}

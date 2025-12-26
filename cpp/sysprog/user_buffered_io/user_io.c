#include <stdio.h>

#define LINE_MAX 128
int main() {
    // FILE is a file pointer, which maps to a file descriptor
    FILE* stream = fopen("test.txt", "r");
    // one can convert a already opened fd to a FILE, for example: fdopen(fd, "r");
    if(!stream) {
        perror("fopen");
        return -1;
    }

    int c = fgetc(stream);
    if(c == EOF) {
        perror("fgetc");
        return -1;
    } else {
        printf("the first character is: %c\n", (char)c);
    }

    char line[LINE_MAX];
    if(!fgets(line, LINE_MAX, stream)) {
        perror("fgets");
        return -1;
    } else {
        printf("the first line is: %s\n", line);
    }

    fclose(stream);

    return 0;
}

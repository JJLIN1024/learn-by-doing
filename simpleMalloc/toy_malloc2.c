#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/resource.h>

#define ALIGNMENT 16
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define CHUNK_SIZE (1 << 14)
#define CHUNK_ALIGN(size) (((size) + (CHUNK_SIZE - 1)) & ~(CHUNK_SIZE - 1))
#define OVERHEAD sizeof(block_header)
#define HDRP(bp) ((char *)(bp) - sizeof(block_header))
#define GET_SIZE(p) ((block_header *)(p))->size
#define GET_ALLOC(p) ((block_header *)(p))->allocated
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(HEPR(bp)))


// total 16 bytes(plus padding)
typedef struct _block_header{
    size_t size;            // 8 bytes
    char allocated;         // 1 byte
} block_header;


void *first_bp;

int mm_init() {
    sbrk(sizeof(block_header));
    first_bp = sbrk(0);

    GET_SIZE(HDRP(first_bp)) = 0;
    GET_ALLOC(HDRP(first_bp)) = 1;
    return 0;
}

void extend(size_t new_size) {
    size_t chunk_size = CHUNK_ALIGN(new_size);
    void *bp = sbrk(chunk_size);

    GET_SIZE(HDRP(bp)) = chunk_size;
    GET_ALLOC(HDRP(bp)) = 0;

    GET_SIZE(HDRP(NEXT_BLKP(bp))) = 0;
    GET_ALLOC(HDRP(NEXT_BLKP(bp))) = 1;
}

int main() {
    printf("%lu \n", sizeof(block_header));
    printf("%lu \n", OVERHEAD);
    return 0;
}
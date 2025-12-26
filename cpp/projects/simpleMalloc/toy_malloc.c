#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <unistd.h>

#define BLOCK_SIZE 24
void *first_block = NULL;

size_t align8(size_t s) {
  if ((s & 0x7) == 0)
    return s;
  // remove last three bits, add one is essentially adding a 8.
  return ((s >> 3) + 1) << 3;
}

typedef struct s_block {
  size_t size;
  struct s_block *next;
  int free;
  int padding; // explicit define? will the compiler do it for you?
  char data[1];
} * t_block;

/* First Fit */
t_block find_block(t_block *last, size_t size) {
  t_block b = first_block;
  while (b && !(b->free && b->size >= size)) {
    *last = b;
    b = b->next;
  }
  return b;
}

t_block extend_heap(t_block last, size_t s) {
  t_block b;
  b = sbrk(0);
  if (sbrk(BLOCK_SIZE + s) == (void *)-1) {
    return NULL;
  }
  b->size = s;
  b->next = NULL;
  if (last) {
    last->next = b;
    b->free = 0;
  }
  return b;
}

t_block split_block(t_block b, size_t s) {
  t_block new;
  new = b->data + s;
  new->size = b->size - s - BLOCK_SIZE;
  new->next = b->next;
  new->free = 1;
  b->size = s;
  b->next = new;
}

void *malloc(size_t size) {
  t_block b, last;
  size_t s;
  s = align8(size);
  if (first_block) {
    last = first_block;
    b = find_block(&last, s);
    if (b) {
      if ((b->size - s) >= (BLOCK_SIZE + 8))
        split_block(b, s);
      b->free = 0;
    } else {
      b = extend_heap(last, s);
      if (!b)
        return NULL;
    }
  } else {
    b = extend_heap(NULL, s);
    if (!b)
      return NULL;
    first_block = b;
  }
  return b->data;
}

int main(int argc, char **argv) {
  void *buf = malloc(sizeof(int));
  if (buf) {
    printf("malloc suceed!");
  }
}

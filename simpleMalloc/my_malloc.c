#include <assert.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#define BLOCK_SIZE 24
void* first_block = NULL;

typedef struct s_block* t_block;

struct s_block {
  size_t size;
  t_block prev; /*指向上個塊的指針*/
  t_block next; /*指向下個塊的指針*/
  int free;
  int padding;
  void* ptr;
  char data[1];
};

/* first fit */
t_block find_block(t_block* last, size_t size) {
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
  if (sbrk(BLOCK_SIZE + s) == (void*)-1) {
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

void split_block(t_block b, size_t s) {
  t_block new;
  new = b->data + s;
  new->size = b->size - s - BLOCK_SIZE;
  new->next = b->next;
  new->free = 1;
  b->size = s;
  b->next = new;
}

size_t align8(size_t s) {
  if (s & 0x7 == 0)
    return s;
  return ((s >> 3) + 1) << 3;
}

void* malloc(size_t size) {
  t_block b, last;
  size_t s;
  /*對齊地址*/
  s = align8(size);
  if (first_block) {
    /*尋找合適的block */
    last = first_block;
    b = find_block(&last, s);
    if (b) {
      /*如果可以，則分裂*/
      if ((b->size - s) >= (BLOCK_SIZE + 8))
        split_block(b, s);
      b->free = 0;
    } else {
      /*沒有合適的block，開闢一個新的*/
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

void* calloc(size_t number, size_t size) {
  size_t* new;
  size_t s8, i;
  new = malloc(number * size);
  if (new) {
    s8 = align8(number * size) >> 3;
    for (i = 0; i < s8; i++)
      new[i] = 0;
  }
  return new;
}

t_block get_block(void* p) {
  char* tmp;
  tmp = p;
  return (p = tmp -= BLOCK_SIZE);
}

int valid_addr(void* p) {
  if (first_block) {
    if (p > first_block && p < sbrk(0)) {
      return p == (get_block(p))->ptr;
    }
  }
  return 0;
}

t_block fusion(t_block b) {
  if (b->next && b->next->free) {
    b->size += BLOCK_SIZE + b->next->size;
    b->next = b->next->next;
    if (b->next)
      b->next->prev = b;
  }
  return b;
}

void free(void* p) {
  t_block b;
  if (valid_addr(p)) {
    b = get_block(p);
    b->free = 1;
    if (b->prev && b->prev->free)
      b = fusion(b->prev);
    if (b->next)
      fusion(b);
    else {
      if (b->prev)
        b->prev->prev = NULL;
      else
        first_block = NULL;
      brk(b);
    }
  }
}

void copy_block(t_block src, t_block dst) {
  size_t *sdata, *ddata;
  size_t i;
  sdata = src->ptr;
  ddata = dst->ptr;
  for (i = 0; (i * 8) < src->size && (i * 8) < dst->size; i++)
    ddata[i] = sdata[i];
}

void* realloc(void* p, size_t size) {
  size_t s;
  t_block b, new;
  void* newp;
  if (!p)
    /*根據標準庫文檔，當p傳入NULL時，相當於調用malloc */
    return malloc(size);
  if (valid_addr(p)) {
    s = align8(size);
    b = get_block(p);
    if (b->size >= s) {
      if (b->size - s >= (BLOCK_SIZE + 8))
        split_block(b, s);
    } else {
      /*看是否可進行合併*/
      if (b->next && b->next->free &&
          (b->size + BLOCK_SIZE + b->next->size) >= s) {
        fusion(b);
        if (b->size - s >= (BLOCK_SIZE + 8))
          split_block(b, s);
      } else {
        /*新malloc */
        newp = malloc(s);
        if (!newp)
          return NULL;
        new = get_block(newp);
        copy_block(b, new);
        free(p);
        return (newp);
      }
    }
    return (p);
  }
  return NULL;
}

void* toy_malloc(size_t size) {
  void* p = sbrk(0);
  printf("%p\n", p);
  void* p2 = sbrk(size);
  printf("%p\n", p2);
  if (p2 == (void*)-1) {
    return NULL;
  } else {
    assert(p == p2);
    printf("\n");
    return p;
  }
}

int main() {
  void* p = toy_malloc(1024);
  void* p2 = toy_malloc(1024);
  void* p3 = toy_malloc(1024);
  // printf("%p\n", p);
  // printf("%p\n", p2);
  // printf("%p\n", p3);
  return 0;
}
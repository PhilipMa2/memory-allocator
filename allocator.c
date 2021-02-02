#define _GNU_SOURCE 1

#include <assert.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <math.h>

// The minimum size returned by malloc
#define MIN_MALLOC_SIZE 16

// Round a value x up to the next multiple of y
#define ROUND_UP(x,y) ((x) % (y) == 0 ? (x) : (x) + ((y) - (x) % (y)))
#define ROUND_DOWN(x,y) ((x) % (y) == 0 ? (x) : (x) - (x) % (y))

// The size of a single page of memory, in bytes
#define PAGE_SIZE 0x1000

#define MAGIC_NUM 0xF00DFACE

typedef struct node {
  struct node *next;
} node_t;

typedef struct header {
  size_t size;
  int magic;
} header_t;

size_t xxmalloc_usable_size(void* ptr);
node_t* freelist[8] = {NULL};

int roundUp(unsigned int x) {
    if (x <= 16)
        return 16;
    else {
        x--;
        x |= x >> 1;
        x |= x >> 2;
        x |= x >> 4;
        x |= x >> 8;
        x |= x >> 16;
        x++;
        return x;
    }
}

/**
 * Allocate space on the heap.
 * \param size  The minimium number of bytes that must be allocated
 * \returns     A pointer to the beginning of the allocated space.
 *              This function may return NULL when an error occurs.
 */
void* xxmalloc(size_t size) {
  if (size <= 2048) {
    size = roundUp(size);
    int bucket = (int) log2(size) - 4;
    if (freelist[bucket] != NULL) {
      void* ret = freelist[bucket];
      freelist[bucket] = freelist[bucket]->next;
      return ret;
    } else {
      header_t* hp = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

      if (hp == MAP_FAILED) {
        fputs("mmap failed! Giving up.\n", stderr);
        exit(2);
      }

      hp->size = size;
      hp->magic = MAGIC_NUM;
      int num = PAGE_SIZE / size;
      node_t* p = (node_t *) hp + size/8;
      freelist[bucket] = p;
      p->next = p + size/8;
      for (int i = 2; i < num; i++) {
        p = p->next;
        p->next = p + size/8;
      }
      p->next = NULL;
      return xxmalloc(size);
    }
  } else {
    // Round the size up to the next multiple of the page size
    size = ROUND_UP(size, PAGE_SIZE);
    
    // Request memory from the operating system in page-sized chunks
    void* p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    // Check for errors
    if(p == MAP_FAILED) {
      fputs("mmap failed! Giving up.\n", stderr);
      exit(2);
    }
    
    return p;
  }
}

/**
 * Free space occupied by a heap object.
 * \param ptr   A pointer somewhere inside the object that is being freed
 */
void xxfree(void* ptr) {
  if (ptr == NULL) return;
  
  size_t size = xxmalloc_usable_size(ptr);
  node_t* start = (node_t*) ROUND_DOWN((intptr_t) ptr, size);
  int bucket = (int) log2(size) - 4;
  start->next = freelist[bucket];
  freelist[bucket] = ptr;
}

/**
 * Get the available size of an allocated object
 * \param ptr   A pointer somewhere inside the allocated object
 * \returns     The number of bytes available for use in this object
 */
size_t xxmalloc_usable_size(void* ptr) {
  if (ptr == NULL)
    return 0;
  else {
    header_t* header = (header_t*) ROUND_DOWN((intptr_t) ptr, PAGE_SIZE);
    if (header->magic == MAGIC_NUM) {
      return header->size;
    } else {
      fputs("magic number incorrect. Giving up.\n", stderr);
      exit(2);
    }
  } 
    
}

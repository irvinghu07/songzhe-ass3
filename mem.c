#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "mem.h"
#include <unistd.h>
#include <string.h>

int m_error;

void *heap_addr;

void *next_avilable;

void *block_head;

void *block_tail;

static int init_flag = 0;

static int region_size = 0;

struct mem_block
{
    int usage;              // 4 bytes
    int block_sz;           // 4 bytes
    struct mem_block *prev; // 8 bytes
    struct mem_block *next; // 8 bytes
    void *block_addr;       // 8 bytes
};

void print_block(struct mem_block *blk)
{
    printf("{Block: %p\n", blk);
    printf("Usage: %d\n", blk->usage);
    printf("Block size: %d\n", blk->block_sz);
    printf("Prev: %p\n", blk->prev);
    printf("Next: %p\n", blk->next);
    printf("Block address: %p}\n", blk->block_addr);
}

int initialize()
{
    if (init_flag == 0)
    {
        init_flag = 1;
        return 1;
    }
    return 0;
}
// Ceil the size of the region to the nearest page size
int align_memory(int region_sz, int blk_sz)
{
    return (blk_sz + region_sz - 1) / blk_sz * blk_sz;
}

void advance_next_available(int size)
{
    next_avilable = (char *)next_avilable + size;
}

int mem_init(int size_of_region)
{
    if (0 == initialize())
    {
        m_error = E_BAD_ARGS;
        return -1;
    }
    if (size_of_region <= 0)
    {
        m_error = E_BAD_ARGS;
        return -1;
    }
    size_of_region = align_memory(size_of_region, getpagesize());
    if ((heap_addr = mmap(NULL, size_of_region, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)) == MAP_FAILED)
    {
        perror("mmap");
        return -1;
    }
    // Explicitly set the chunk of memory to 0 to prevent unexpected behavior
    memset(heap_addr, 0, size_of_region);
    next_avilable = heap_addr;
    region_size = size_of_region;
    return 0;
}

void *mem_alloc(int size, int style)
{
    int chunk_sz = size += sizeof(struct mem_block);
    chunk_sz = align_memory(chunk_sz, 8);
    if ((char *)next_avilable + chunk_sz > ((char *)heap_addr + region_size))
    {
        m_error = E_NO_SPACE;
        return NULL;
    }
    struct mem_block *blk = (struct mem_block *)next_avilable;

    blk->usage = 1;
    blk->block_sz = size;
    blk->block_addr = (char *)next_avilable + sizeof(struct mem_block);
    advance_next_available(chunk_sz);

    if (blk->prev == NULL)
    {
        // First block in the list
        blk->next = NULL;
    }
    else
    {
        // Not the first block, link it to the previous one
        blk->prev->next = blk;
    }

    blk->prev = NULL; // It's the new head of the list

    return blk->block_addr;
}

int mem_free(void *ptr)
{
    return 0;
}
void mem_dump()
{
}

int main()
{
    mem_init(4096);
    unsigned long *node1 = mem_alloc(sizeof(unsigned long), 0);
    *node1 = 1;
    unsigned long *node2 = mem_alloc(sizeof(unsigned long), 0);
    *node2 = 2;
    unsigned long *node3 = mem_alloc(sizeof(unsigned long), 0);
    *node3 = 3;
    printf("Node 1: %lu\nNode 2: %lu\nNode 3: %lu\n", *node1, *node2, *node3);
    return 0;
}

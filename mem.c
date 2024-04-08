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
    int request_sz;         // 4 bytes
    int data_sz;            // 4 bytes
    struct mem_block *prev; // 8 bytes
    struct mem_block *next; // 8 bytes
    void *block_addr;       // 8 bytes
};

void print_block(struct mem_block *blk);

void *best_fit(int size)
{
    struct mem_block *current = block_head;
    struct mem_block *best_fit = NULL;
    while (current != NULL && current->usage == 1 && current->request_sz < size)
    {
        if (best_fit == NULL || current->request_sz < best_fit->request_sz)
        {
            best_fit = current;
        }
        current = current->next;
    }
    return best_fit;
}

void *worst_fit(int size)
{
    struct mem_block *current = block_head;
    struct mem_block *worst_fit = NULL;
    while (current != NULL && current->usage == 1 && current->request_sz < size)
    {
        if (worst_fit == NULL || current->request_sz > worst_fit->request_sz)
        {
            worst_fit = current;
        }
        current = current->next;
    }
    return worst_fit;
}

void *first_fit(int size)
{
    struct mem_block *current = block_head;
    while (current != NULL && current->usage == 1 && current->request_sz < size)
        current = current->next;
    return current;
}

void *try_fit(struct mem_block *blk, int size, int style)
{
    switch (style)
    {
    case M_BESTFIT:
        blk = (struct mem_block *)best_fit(size);
        if (blk != NULL)
        {
            blk->usage = 1;
            blk->data_sz = size;
            blk->block_addr = (char *)blk + sizeof(struct mem_block);
            return blk->block_addr;
        }
        break;
    case M_WORSTFIT:
        blk = worst_fit(size);
        if (blk != NULL)
        {
            blk->usage = 1;
            blk->data_sz = size;
            blk->block_addr = (char *)blk + sizeof(struct mem_block);
            return blk->block_addr;
        }
        break;
    case M_FIRSTFIT:
        blk = first_fit(size);
        if (blk != NULL)
        {
            blk->usage = 1;
            blk->data_sz = size;
            blk->block_addr = (char *)blk + sizeof(struct mem_block);
            return blk->block_addr;
        }
        break;
    }
    return blk;
}

void coalescing(struct mem_block *blk)
{
    struct mem_block *prev_blk = blk->prev;
    struct mem_block *next_blk = blk->next;
    if (prev_blk != NULL && prev_blk->usage == 0)
    {
        prev_blk->next = next_blk;
        if (next_blk != NULL)
        {
            next_blk->prev = prev_blk;
        }
        prev_blk->data_sz += blk->data_sz + sizeof(struct mem_block);
    }
    if (next_blk != NULL && next_blk->usage == 0)
    {
        next_blk->prev = prev_blk;
        if (prev_blk != NULL)
        {
            prev_blk->next = next_blk;
        }
        blk->data_sz += next_blk->data_sz + sizeof(struct mem_block);
    }
}

int region_empty()
{
    struct mem_block *current = block_head;
    while (current != NULL)
    {
        if (current->usage == 1)
        {
            return 0;
        }
        current = current->next;
    }
    return 0;
}

void *get_block_from_ref(void *target)
{
    struct mem_block *current = block_head;
    while (current != NULL && current->block_addr != target)
        current = current->next;
    return current;
}

void traverse_blocks()
{
    struct mem_block *current = block_head;
    while (current != NULL)
    {
        print_block(current);
        current = current->next;
    }
    printf("\n=====================================\n");
}

void print_block(struct mem_block *blk)
{
    printf("Block: %p\n", blk);
    printf("Usage: %d\n", blk->usage);
    printf("Request size: %d\n", blk->request_sz);
    printf("Data size: %d\n", blk->data_sz);
    printf("Prev: %p\n", blk->prev);
    printf("Next: %p\n", blk->next);
    printf("Block address: %p\n", blk->block_addr);
    printf("\n");
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
    next_avilable = heap_addr;
    region_size = size_of_region;
    return 0;
}

void *mem_alloc(int size, int style)
{
    // Trying to reuse block
    struct mem_block *blk = NULL;
    blk = try_fit(blk, size, style);
    if (blk != NULL)
    {
        return blk->block_addr;
    }

    // Trying to allocate new block
    int chunk_sz = size + sizeof(struct mem_block);
    chunk_sz = align_memory(chunk_sz, 8);
    if ((char *)next_avilable + chunk_sz > ((char *)heap_addr + region_size))
    {
        m_error = E_NO_SPACE;
        return NULL;
    }
    blk = (struct mem_block *)next_avilable;
    blk->usage = 1;
    blk->request_sz = size;
    blk->data_sz = size;
    blk->block_addr = (char *)next_avilable + sizeof(struct mem_block);
    advance_next_available(chunk_sz);

    // First block in the list
    if (block_head == NULL)
    {
        block_head = blk;
        block_tail = blk;
    }
    // Not the first block, link it to the previous one
    else
    {
        struct mem_block *prev_blk = block_tail;
        prev_blk->next = blk;
        blk->prev = prev_blk;
        block_tail = blk;
    }
    return blk->block_addr;
}

int mem_free(void *ptr)
{
    // No operation on freeing a NULL pointer
    if (ptr == NULL)
    {
        return 0;
    }
    struct mem_block *target = get_block_from_ref(ptr);
    if (NULL == target)
    {
        perror("Internal error with the Linked-List");
        return -1;
    }
    target->usage = 0;
    // Coalescing adjacent free blocks
    coalescing(target);
    if (region_empty())
    {
        if (munmap(heap_addr, region_size) == -1)
        {
            perror("munmap");
            return -1;
        }
        return 0;
    }
    return 0;
}

void mem_dump()
{
}

int main()
{
    mem_init(4096);
    unsigned long *node1 = mem_alloc(sizeof(unsigned long), 0);
    memset(node1, 0, sizeof(unsigned long));
    *node1 = 1;
    traverse_blocks();
    unsigned long *node2 = mem_alloc(sizeof(unsigned long), 0);
    memset(node2, 0, sizeof(unsigned long));
    *node2 = 2;
    traverse_blocks();
    unsigned long *node3 = mem_alloc(sizeof(unsigned long), 0);
    memset(node3, 0, sizeof(unsigned long));
    *node3 = 3;
    traverse_blocks();
    printf("Node 1: %lu\nNode 2: %lu\nNode 3: %lu\n", *node1, *node2, *node3);
    mem_free(NULL);
    return 0;
}

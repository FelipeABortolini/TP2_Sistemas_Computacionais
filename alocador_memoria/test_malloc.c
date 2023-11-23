#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Maximum number of memory blocks that can be allocated */
#define MAX_ALLOCATIONS 1000  

struct allocation_t{
    void *ptr;
    unsigned int size;
} allocations[MAX_ALLOCATIONS];

#define MEMORY_SIZE 1024  // Size of the fixed memory block

#pragma pack(push, 1)
struct mem_block {
    int is_free;
    size_t size;
    void *mem_ptr;
    struct mem_block *next;
};
#pragma pack(pop)

char* memory = NULL;  // Pointer to the memory block
struct mem_block* head = NULL;  // Head of the linked list

void* smalloc(size_t size) {

    if (!head) {
        head = (struct mem_block *)memory;
        head->is_free = 1;
        head->size = MEMORY_SIZE - sizeof(struct mem_block);
        // head->mem_ptr = head + 1;
        head->mem_ptr = head + sizeof(struct mem_block);
        head->next = NULL;
    }

    struct mem_block* curr = head;

    while (curr) {
        if (curr->is_free && curr->size >= size) {
            if (curr->size > size + sizeof(struct mem_block)) {
                // Split the block
                struct mem_block* new_block = (struct mem_block*)((char*)curr + sizeof(struct mem_block) + size);
                new_block->is_free = 1;
                new_block->size = curr->size - size - sizeof(struct mem_block);
                new_block->mem_ptr = (char*)new_block + sizeof(struct mem_block);
                new_block->next = curr->next;
                curr->size = size;
                curr->is_free = 0;
                curr->next = new_block;
            }
            return curr->mem_ptr;
        }
        curr = curr->next;
    }
    // Insufficient memory
    printf("Insufficient memory to allocate %zu bytes\n", size);
    return NULL;
}

void sfree(void* ptr) {
    struct mem_block* curr = head;
    while (curr) {
        if (curr->mem_ptr == ptr) {
            curr->is_free = 1;
            // Coalesce adjacent free blocks
            while (curr->next && curr->next->is_free) {
                curr->size += sizeof(struct mem_block) + curr->next->size;
                curr->next = curr->next->next;
            }
            break;
        }
        curr = curr->next;
    }
}

int main(int argc, char *argv[]) {
    unsigned int currentMemory = 0;
    int allocCount = 0;
    unsigned int maxMemory;  
    unsigned int size;

    if (argc < 2){
        printf("Usage: %s <max memory in bytes>\n", argv[0]);
        return 1;
    }

    maxMemory = atoi(argv[1]);

    srand(time(NULL));

    while(1){
        if (rand() % 2 == 0 && allocCount < MAX_ALLOCATIONS - 1){
            /* Allocate memory */
            size = rand() % 100;
            if (currentMemory + size > maxMemory){
                printf("Out of memory\n");
                continue;
            }
            /* Change malloc() by smalloc() */
            allocations[allocCount].ptr = smalloc(size);
            allocations[allocCount].size = size;
            currentMemory += size;
            allocCount++;
            printf("Allocated %u bytes, total %u, position %i\n", size, currentMemory, allocCount);
        }else{
            /* Free memory */
            if (allocCount > 0){
                allocCount--;
                size = allocations[allocCount].size;
                currentMemory -= size;
                /* Change free() by sfree() */
                sfree(allocations[allocCount].ptr);
                printf("Free %u bytes, total %u, position %i\n", size, currentMemory, allocCount);
            }
        }
    }

    return 0;
}

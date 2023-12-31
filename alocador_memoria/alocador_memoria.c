#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/* Maximum number of memory blocks that can be allocated */
#define MAX_ALLOCATIONS 1000

struct allocation_t{
    void *ptr;
    unsigned int size;
} allocations[MAX_ALLOCATIONS];

#pragma pack(push, 1)
struct mem_block {
    int is_free;
    size_t size;
    void *mem_ptr;
    struct mem_block *next;
};
#pragma pack(pop)

unsigned int maxMemory;
struct mem_block* head = NULL;  // Head of the linked list

void* smalloc(size_t size) {

    if (!head) {
        char* memory = sbrk(maxMemory);
        head = (struct mem_block *)memory;
        if (head == (void*)-1) {
            perror("sbrk");
            return 1;  // Error allocating memory
        }
        head->is_free = 1;
        head->size = maxMemory - sizeof(struct mem_block);
        head->mem_ptr = head + sizeof(struct mem_block);
        head->next = NULL;
    }

    struct mem_block* curr = head;

    while (curr) {
        if (curr->is_free && curr->next && curr->next->is_free) {
            curr->next = curr->next->next;
            curr->size = curr->size + sizeof(struct mem_block) + curr->next->size;
        }
        if (curr->is_free && curr->size >= size + sizeof(struct mem_block)) {
            if((int)curr->size - (int)size <= 0) {
                curr->next = NULL;
            } else {
                struct mem_block* new_block = (struct mem_block*)((char*)curr + sizeof(struct mem_block) + size);
                new_block->size = curr->size - size - sizeof(struct mem_block);
                new_block->is_free = 1;
                new_block->mem_ptr = (char*)new_block + sizeof(struct mem_block);
                new_block->next = curr->next;
                curr->next = new_block;
            }
            curr->size = size;
            curr->is_free = 0;
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
        if (curr->is_free && curr->next && curr->next->is_free) {
            curr->size = curr->size + sizeof(struct mem_block) + curr->next->size;
            curr->next = curr->next->next;
        }
        if (curr->mem_ptr == ptr) {
            curr->is_free = 1;
            // Coalesce adjacent free blocks
            if (curr->next && curr->next->is_free) {
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

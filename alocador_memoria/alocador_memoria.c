#include <stdio.h>
#include <unistd.h>

#define MEMORY_SIZE 1024  // Size of the fixed memory block

#pragma pack(push, 1)
struct mem_block {
    int is_free;
    size_t size;
    void *mem_ptr;
    struct mem_block *next;
};
#pragma pack(pop)

// char* memory = NULL;  // Pointer to the memory block
struct mem_block* head = NULL;  // Head of the linked list

void* smalloc(size_t size) {

    if (!head) {
        // head = (struct mem_block *)memory;
        head = sbrk(MEMORY_SIZE);
        if (head == (void*)-1) {
            perror("sbrk");
            return 1;  // Error allocating memory
        }
        head->is_free = 1;
        head->size = MEMORY_SIZE - sizeof(struct mem_block);
        // head->mem_ptr = head + 1;
        head->mem_ptr = head + sizeof(struct mem_block);
        head->next = NULL;
    }

    struct mem_block* curr = head;

    while (curr) {
        // if (curr->is_free && curr->next && curr->next->is_free) {
        //     curr->next = curr->next->next;
        //     curr->size = curr->size + sizeof(struct mem_block) + curr->next->size;
        // }
        if (curr->is_free && curr->size >= size + sizeof(struct mem_block)) {
            // if (curr->size > size + sizeof(struct mem_block)) {
            // if (curr->size >= size) {
                // Split the block
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
            // }
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
        // if (curr->is_free && curr->next && curr->next->is_free) {
        //     curr->size = curr->size + sizeof(struct mem_block) + curr->next->size;
        //     curr->next = curr->next->next;
        // }
        if (curr->mem_ptr == ptr) {
            curr->is_free = 1;
            // Coalesce adjacent free blocks
            if (curr->next && curr->next->is_free) {
                curr->size += sizeof(struct mem_block) + curr->next->size;
                curr->next = curr->next->next;
            }
            // if (curr->next && curr->next->next && curr->next->next->is_free) {
            //     curr->next->size += sizeof(struct mem_block) + curr->next->next->size;
            //     curr->next = curr->next->next->next;
            // }
            break;
        }
        curr = curr->next;
    }
}

// void sfree(void* ptr) {
//     struct mem_block* curr = head;
//     while (curr) {
//         if (curr->mem_ptr == ptr) {
//             curr->is_free = 1;
//             // Coalesce adjacent free blocks
//             while (curr->next && curr->next->is_free) {
//                 curr->size += sizeof(struct mem_block) + curr->next->size;
//                 curr->next = curr->next->next;
//             }
//             break;
//         }
//         curr = curr->next;
//     }
// }

int main() {
    // Allocate the fixed-size memory block using sbrk
    // memory = sbrk(MEMORY_SIZE);
    // if (memory == (void*)-1) {
    //     perror("sbrk");
    //     return 1;  // Error allocating memory
    // }

    // Simulating memory allocation and deallocation
    void* ptr1 = smalloc(200);  // Allocate 200 bytes
    void* ptr2 = smalloc(300);  // Allocate 300 bytes
    // void* ptr3 = smalloc(167);  // Allocate 200 bytes
    // void* ptr4 = smalloc(5);
    // sfree(ptr4);
    // void* ptr5 = smalloc(213);  // Allocate 300 bytes
    // sfree(ptr1);  // Free the first block
    void* ptr3 = smalloc(167);  // Allocate 150 bytes (should reuse the freed block)
    sfree(ptr2);  // Free the second block
    void* ptr4 = smalloc(5);  // Allocate 500 bytes (should coalesce with the freed block)
    void* ptr5 = smalloc(183);
    void* ptr6 = smalloc(1);
    void* ptr7 = smalloc(7);
    void* ptr8 = smalloc(183);
    // void* ptr4 = smalloc(5);
    // void* ptr5 = smalloc(213);

    return 0;
}
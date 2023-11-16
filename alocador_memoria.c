#include <stdio.h>
#include <unistd.h>

#define MEM_SIZE 1024

#pragma pack(push, 1)
struct mem_block {
    int is_free;
    size_t size;
    void *mem_ptr;
    struct mem_block *next;
};
#pragma pack(pop)

static char memory[MEM_SIZE];
static struct mem_block *head = NULL;

void *smalloc(size_t size) {
    struct mem_block *curr, *prev;
    void *mem;

    if (size <= 0) {
        printf("Tamanho inválido de alocação de memória\n");
        return NULL;
    }

    if (!head) {
        head = (struct mem_block *)memory;
        head->is_free = 0;
        head->size = MEM_SIZE - sizeof(struct mem_block);
        head->mem_ptr = head + 1;
        head->next = NULL;
    }

    curr = head;
    while (curr && !(curr->is_free && curr->size >= size)) {
        prev = curr;
        curr = curr->next;
    }

    if (!curr) {
        printf("Memória insuficiente para alocar %zu bytes\n", size);
        return NULL;
    }

    if (curr->size > size + sizeof(struct mem_block)) {
        struct mem_block *new_block = (struct mem_block *)((char *)(curr->mem_ptr) + size);
        new_block->is_free = 1;
        new_block->size = curr->size - size - sizeof(struct mem_block);
        new_block->mem_ptr = new_block + 1;
        new_block->next = curr->next;
        curr->size = size;
        curr->next = new_block;
    }

    curr->is_free = 0;
    mem = curr->mem_ptr;

    return mem;
}

void sfree(void *ptr) {
    if (!ptr) {
        return;
    }

    struct mem_block *curr = (struct mem_block *)memory;
    struct mem_block *prev = NULL;
    struct mem_block *next = NULL;

    while (curr) {
        if (curr->mem_ptr == ptr) {
            curr->is_free = 1;
            break;
        }
        prev = curr;
        curr = curr->next;
    }

    // Verifica se o bloco atual e o próximo estão livres e adjacentes
    if (curr && curr->next && curr->is_free && curr->next->is_free) {
        curr->size += sizeof(struct mem_block) + curr->next->size;
        curr->next = curr->next->next;
    }

    // Verifica se o bloco anterior e o atual estão livres e adjacentes
    if (prev && curr && prev->is_free && curr->is_free) {
        prev->size += sizeof(struct mem_block) + curr->size;
        prev->next = curr->next;
    }
}

int main() {
    // Simulação de chamadas smalloc() e sfree()
    void *ptr1 = smalloc(200);
    void *ptr2 = smalloc(300);
    sfree(ptr1);
    void *ptr3 = smalloc(150);
    sfree(ptr2);
    void *ptr4 = smalloc(100);

    return 0;
}

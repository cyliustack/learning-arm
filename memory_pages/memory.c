#include "memory.h"
#include "debug.h"
#include "print.h"
#include "lib.h"
#include "stddef.h"
#include "stdbool.h"

static struct Page free_memory;
extern char end;
void load_pgd(uint64_t map);


static void free_region(uint64_t v, uint64_t e)
{
    for (uint64_t start = PA_UP(v); start + PAGE_SIZE <= e; start += PAGE_SIZE) {
        if (start + PAGE_SIZE <= MEMORY_END) {
            kfree(start);
        }
    }
}

void kfree(uint64_t v)
{
    ASSERT(v % PAGE_SIZE == 0);
    ASSERT(v >= (uint64_t)&end);
    ASSERT(v + PAGE_SIZE <= MEMORY_END);

    struct Page *page_address = (struct Page*)v;
    page_address->next = free_memory.next;
    free_memory.next = page_address;
}

void* kalloc(void)
{
    struct Page *page_address = free_memory.next;

    if (page_address != NULL) {
        ASSERT((uint64_t)page_address % PAGE_SIZE == 0);
        ASSERT((uint64_t)page_address >= (uint64_t)&end);
        ASSERT((uint64_t)page_address + PAGE_SIZE <= MEMORY_END);
        
        free_memory.next = page_address->next;
    }

    return page_address;
}

void checkmm(void)
{
    struct Page *v = free_memory.next;
    uint64_t size = 0;
    uint64_t i = 0;

    while (v != NULL) {
        size += PAGE_SIZE;
        printk("%d base is %x \r\n", i++, v);
        v = v->next;
    }

    printk("memory size is %u \r\n", size/1024/1024);
}

static uint64_t* find_pgd_entry(uint64_t map, uint64_t v, int alloc, uint64_t attribute)
{
    uint64_t *ptr = (uint64_t*)map;
    void *addr = NULL;
    unsigned int index = (v >> 39) & 0x1ff;

    if (ptr[index] & ENTRY_V) {
        addr = (void*)P2V(PDE_ADDR(ptr[index]));
    }
    else if (alloc == 1) {
        addr = kalloc();
        if (addr != NULL) {
            memset(addr, 0, PAGE_SIZE);
            ptr[index] = (V2P(addr) | attribute | TABLE_ENTRY);
        }
    }

    return addr;
}

static uint64_t* find_pud_entry(uint64_t map, uint64_t v, int alloc, uint64_t attribute)
{
    uint64_t *ptr = NULL;
    void *addr = NULL;
    unsigned int index = (v >> 30) & 0x1ff;

    ptr = find_pgd_entry(map, v, alloc, attribute);
    if (ptr == NULL) {
        return NULL;
    }

    if (ptr[index] & ENTRY_V) {
        addr = (void*)P2V(PDE_ADDR(ptr[index]));
    }
    else if (alloc == 1) {
        addr = kalloc();
        if (addr != NULL) {
            memset(addr, 0, PAGE_SIZE);
            ptr[index] = (V2P(addr) | attribute | TABLE_ENTRY);
        }
    }

    return addr;
}

bool map_page(uint64_t map, uint64_t v, uint64_t pa, uint64_t attribute)
{
    uint64_t vstart = PA_DOWN(v);
    uint64_t *ptr = NULL;

    ASSERT(vstart + PAGE_SIZE < MEMORY_END);
    ASSERT(pa % PAGE_SIZE == 0);
    ASSERT(pa + PAGE_SIZE <= V2P(MEMORY_END));

    ptr = find_pud_entry(map, vstart, 1, attribute);
    if (ptr == NULL) {
        return false;
    }

    unsigned int index = (vstart >> 21) & 0x1ff;
    ASSERT((ptr[index] & ENTRY_V) == 0);

    ptr[index] = (pa | attribute | BLOCK_ENTRY);

    return true;
}

bool setup_uvm(void)
{
    bool status = false;

    return status;
}

void switch_vm(uint64_t map)
{
    load_pgd(V2P(map));
}

void init_memory(void)
{
    free_region((uint64_t)&end, MEMORY_END);
    //checkmm();
}
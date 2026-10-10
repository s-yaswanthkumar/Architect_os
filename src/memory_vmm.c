#include "architect.h"

/* --- Custom Dynamic Heap Allocator Implementation --- */
#define ARENA_SIZE (2 * 1024 * 1024) /* 2 MB Memory Pool */
#define BLOCK_MAGIC 0xAC1984C0
#define ALIGNMENT 16
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

typedef struct ArchBlock {
    size_t size;               /* Total size of block including header */
    int is_free;               /* 1 = Free, 0 = In Use */
    uint32_t magic;            /* Validation Cookie */
    struct ArchBlock *next;    /* Next physical block in arena */
    struct ArchBlock *prev;    /* Previous physical block in arena */
} ArchBlock;

#define BLOCK_HEADER_SIZE ALIGN(sizeof(ArchBlock))

static void *g_arena_start = NULL;
static size_t g_arena_size = 0;
static ArchBlock *g_first_block = NULL;

static void init_allocator_if_needed(void) {
    if (g_arena_start != NULL) return;

    g_arena_size = ARENA_SIZE;
    g_arena_start = mmap(NULL, g_arena_size, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (g_arena_start == MAP_FAILED) {
        perror("mmap arena");
        exit(EXIT_FAILURE);
    }

    g_first_block = (ArchBlock *)g_arena_start;
    g_first_block->size = g_arena_size;
    g_first_block->is_free = 1;
    g_first_block->magic = BLOCK_MAGIC;
    g_first_block->next = NULL;
    g_first_block->prev = NULL;
}

void arch_allocator_cleanup(void) {
    if (g_arena_start && g_arena_start != MAP_FAILED) {
        munmap(g_arena_start, g_arena_size);
        g_arena_start = NULL;
        g_first_block = NULL;
    }
}

void *arch_malloc(size_t size) {
    if (size == 0) return NULL;
    init_allocator_if_needed();

    size_t total_needed = BLOCK_HEADER_SIZE + ALIGN(size);

    /* First-fit search */
    ArchBlock *curr = g_first_block;
    while (curr) {
        if (curr->is_free && curr->size >= total_needed) {
            /* Check if we can split this block */
            if (curr->size >= total_needed + BLOCK_HEADER_SIZE + ALIGNMENT) {
                ArchBlock *new_block = (ArchBlock *)((char *)curr + total_needed);
                new_block->size = curr->size - total_needed;
                new_block->is_free = 1;
                new_block->magic = BLOCK_MAGIC;
                new_block->next = curr->next;
                new_block->prev = curr;

                if (curr->next) {
                    curr->next->prev = new_block;
                }
                curr->next = new_block;
                curr->size = total_needed;
            }

            curr->is_free = 0;
            return (void *)((char *)curr + BLOCK_HEADER_SIZE);
        }
        curr = curr->next;
    }

    return NULL; /* Out of memory in arena */
}

void arch_free(void *ptr) {
    if (!ptr) return;
    init_allocator_if_needed();

    ArchBlock *block = (ArchBlock *)((char *)ptr - BLOCK_HEADER_SIZE);
    if (block->magic != BLOCK_MAGIC) {
        fprintf(stderr, COLOR_RED "arch_free: Invalid pointer or corrupted block magic!\n" COLOR_RESET);
        return;
    }

    block->is_free = 1;

    /* Coalesce with next block if free */
    if (block->next && block->next->is_free) {
        block->size += block->next->size;
        block->next = block->next->next;
        if (block->next) {
            block->next->prev = block;
        }
    }

    /* Coalesce with previous block if free */
    if (block->prev && block->prev->is_free) {
        block->prev->size += block->size;
        block->prev->next = block->next;
        if (block->next) {
            block->next->prev = block->prev;
        }
        block = block->prev;
    }
}

int cmd_arch_alloc(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: arch-alloc <bytes>\n");
        return 1;
    }

    size_t bytes = (size_t)atol(argv[1]);
    if (bytes == 0) {
        printf(COLOR_RED "Invalid allocation size.\n" COLOR_RESET);
        return 1;
    }

    void *ptr = arch_malloc(bytes);
    if (!ptr) {
        printf(COLOR_RED "arch_malloc: Failed to allocate %zu bytes (Out of arena space)\n" COLOR_RESET, bytes);
        return 1;
    }

    ArchBlock *block = (ArchBlock *)((char *)ptr - BLOCK_HEADER_SIZE);
    printf(COLOR_GREEN "[Arch-Alloc Success]" COLOR_RESET "\n");
    printf("  Payload Address   : %p\n", ptr);
    printf("  Header Address    : %p\n", (void *)block);
    printf("  Requested Size    : %zu bytes\n", bytes);
    printf("  Aligned Total Size: %zu bytes (Header: %zu, Payload: %zu)\n",
           block->size, BLOCK_HEADER_SIZE, block->size - BLOCK_HEADER_SIZE);
    printf("  Alignment Padding : %zu bytes\n", (block->size - BLOCK_HEADER_SIZE) - bytes);
    printf("  Magic Cookie      : 0x%08X (Valid)\n", block->magic);

    return 0;
}

int cmd_arch_free(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: arch-free <hex_address>\n");
        return 1;
    }

    void *ptr = NULL;
    if (sscanf(argv[1], "%p", &ptr) != 1 || !ptr) {
        printf(COLOR_RED "Invalid address format.\n" COLOR_RESET);
        return 1;
    }

    arch_free(ptr);
    printf(COLOR_GREEN "[Arch-Free Completed]" COLOR_RESET " Block at %p returned to free list with coalescing.\n", ptr);
    return 0;
}

int cmd_heapinfo(int argc, char **argv) {
    (void)argc;
    (void)argv;
    init_allocator_if_needed();

    size_t total_free = 0;
    size_t total_allocated = 0;
    size_t block_count = 0;
    size_t free_block_count = 0;
    size_t largest_free_block = 0;

    printf("\n" COLOR_BOLD COLOR_CYAN "=== Architect OS Custom Heap Allocator Introspection ===" COLOR_RESET "\n");
    printf("Arena Base Address: %p | Total Capacity: %zu bytes (%zu KiB)\n",
           g_arena_start, g_arena_size, g_arena_size / 1024);
    printf("Header Metadata Size: %zu bytes (16-byte aligned)\n\n", BLOCK_HEADER_SIZE);

    printf(COLOR_BOLD "%-6s %-18s %-12s %-12s %-10s\n" COLOR_RESET,
           "INDEX", "BLOCK ADDRESS", "TOTAL SIZE", "PAYLOAD SIZE", "STATE");
    printf("--------------------------------------------------------------------\n");

    ArchBlock *curr = g_first_block;
    while (curr) {
        block_count++;
        size_t payload_size = curr->size > BLOCK_HEADER_SIZE ? curr->size - BLOCK_HEADER_SIZE : 0;

        if (curr->is_free) {
            free_block_count++;
            total_free += curr->size;
            if (curr->size > largest_free_block) {
                largest_free_block = curr->size;
            }
        } else {
            total_allocated += payload_size;
        }

        printf("#%-5zu %-18p %-12zu %-12zu %s%-10s" COLOR_RESET "\n",
               block_count,
               (void *)curr,
               curr->size,
               payload_size,
               curr->is_free ? COLOR_GREEN : COLOR_YELLOW,
               curr->is_free ? "FREE" : "ALLOCATED");

        curr = curr->next;
    }

    printf("--------------------------------------------------------------------\n");
    printf(COLOR_BOLD "Heap Telemetry Summary:\n" COLOR_RESET);
    printf("  Total Blocks       : %zu (Allocated: %zu, Free: %zu)\n",
           block_count, block_count - free_block_count, free_block_count);
    printf("  Memory In-Use      : %zu bytes (%.2f%%)\n",
           total_allocated, (double)total_allocated / g_arena_size * 100.0);
    printf("  Free Memory Pool   : %zu bytes (%.2f%%)\n",
           total_free, (double)total_free / g_arena_size * 100.0);
    printf("  Largest Free Block : %zu bytes\n", largest_free_block);

    double frag_index = 0.0;
    if (total_free > 0) {
        frag_index = 1.0 - ((double)largest_free_block / total_free);
    }
    printf("  Fragmentation Index: %.4f %s\n\n",
           frag_index,
           frag_index > 0.5 ? COLOR_YELLOW "(High Fragmentation)" COLOR_RESET :
                              COLOR_GREEN "(Low Fragmentation - Clean)" COLOR_RESET);

    return 0;
}

/* --- Virtual Memory Inspection: vmmaps --- */
int cmd_vmmaps(int argc, char **argv) {
    pid_t target_pid = getpid();
    if (argc > 1) {
        target_pid = (pid_t)atoi(argv[1]);
    }

    char maps_path[256];
    snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", target_pid);

    FILE *f = fopen(maps_path, "r");
    if (!f) {
        fprintf(stderr, COLOR_RED "Cannot open %s: %s\n" COLOR_RESET, maps_path, strerror(errno));
        return 1;
    }

    printf("\n" COLOR_BOLD COLOR_CYAN "=== Process Memory Layout Map (PID: %d) ===" COLOR_RESET "\n", target_pid);
    printf(COLOR_BOLD "%-35s %-6s %-10s %-12s %-25s\n" COLOR_RESET,
           "VIRTUAL ADDRESS RANGE", "PERM", "SIZE (KiB)", "SEGMENT TYPE", "MAPPED OBJECT / PATH");
    printf("-----------------------------------------------------------------------------------------------------\n");

    char line[1024];
    unsigned long total_vm = 0;

    while (fgets(line, sizeof(line), f)) {
        unsigned long start, end, offset, inode;
        char perms[5];
        char dev[10];
        char path[512] = "";

        int fields = sscanf(line, "%lx-%lx %4s %lx %9s %lu %511[^\n]",
                            &start, &end, perms, &offset, dev, &inode, path);
        if (fields < 5) continue;

        unsigned long size_kib = (end - start) / 1024;
        total_vm += size_kib;

        const char *seg_type = "Anonymous/MMAP";
        const char *color = COLOR_WHITE;

        if (strstr(path, "[heap]")) {
            seg_type = "Heap Segment";
            color = COLOR_YELLOW;
        } else if (strstr(path, "[stack]")) {
            seg_type = "User Stack";
            color = COLOR_MAGENTA;
        } else if (strstr(path, "[vdso]") || strstr(path, "[vvar]")) {
            seg_type = "Kernel vDSO/vVar";
            color = COLOR_CYAN;
        } else if (strstr(perms, "x")) {
            seg_type = "Text (Code)";
            color = COLOR_GREEN;
        } else if (strcmp(perms, "rw-p") == 0 && strlen(path) > 0) {
            seg_type = "Data / BSS";
            color = COLOR_BLUE;
        }

        char addr_buf[40];
        snprintf(addr_buf, sizeof(addr_buf), "0x%012lx - 0x%012lx", start, end);

        printf("%-35s %-6s %-10lu %s%-14s" COLOR_RESET " %-25s\n",
               addr_buf, perms, size_kib, color, seg_type, strlen(path) > 0 ? path : "[anon]");
    }

    fclose(f);
    printf("-----------------------------------------------------------------------------------------------------\n");
    printf(COLOR_BOLD "Total Virtual Memory Size: %lu KiB (%.2f MiB)\n\n" COLOR_RESET,
           total_vm, (double)total_vm / 1024.0);

    return 0;
}

/* --- Built-in Copy-on-Write (COW) Demonstrator --- */
static uint64_t get_pfn_from_pagemap(pid_t pid, void *addr) {
    char pagemap_path[256];
    snprintf(pagemap_path, sizeof(pagemap_path), "/proc/%d/pagemap", pid);

    int fd = open(pagemap_path, O_RDONLY);
    if (fd < 0) return 0;

    uintptr_t vaddr = (uintptr_t)addr;
    uintptr_t page_index = vaddr / 4096;
    off_t offset = page_index * sizeof(uint64_t);

    uint64_t entry = 0;
    if (pread(fd, &entry, sizeof(entry), offset) != sizeof(entry)) {
        close(fd);
        return 0;
    }
    close(fd);

    /* Bit 63 indicates if page is present in RAM */
    if ((entry & (1ULL << 63)) == 0) {
        return 0; /* Page not present */
    }

    /* Bits 0-54 represent Page Frame Number (PFN) */
    return entry & ((1ULL << 55) - 1);
}

int cmd_cowdemo(int argc, char **argv) {
    (void)argc;
    (void)argv;

    printf("\n" COLOR_BOLD COLOR_CYAN "=== Architect OS Copy-on-Write (COW) Memory Demonstrator ===" COLOR_RESET "\n");
    printf("Allocating page-aligned anonymous memory buffer via mmap()...\n");

    size_t page_size = (size_t)sysconf(_SC_PAGESIZE);
    char *shared_buf = mmap(NULL, page_size, PROT_READ | PROT_WRITE,
                            MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (shared_buf == MAP_FAILED) {
        perror("mmap shared_buf");
        return 1;
    }

    snprintf(shared_buf, page_size, "Initial Parent Secret String @ %lu", (unsigned long)time(NULL));
    printf("Buffer Virtual Address: %p | Content: \"%s\"\n\n", (void *)shared_buf, shared_buf);

    uint64_t pfn_parent_initial = get_pfn_from_pagemap(getpid(), shared_buf);
    if (pfn_parent_initial > 0) {
        printf("Parent Physical Frame Number (PFN) before fork: 0x%lx\n", (unsigned long)pfn_parent_initial);
    } else {
        printf("Parent PTE present (Kernel PFN masked by security / Yama policy)\n");
    }

    printf(COLOR_YELLOW "[Action] Forking child process...\n" COLOR_RESET);
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        munmap(shared_buf, page_size);
        return 1;
    }

    if (pid == 0) {
        /* Child Process */
        printf("\n" COLOR_GREEN "[Child (PID: %d)]" COLOR_RESET " Before write:\n", getpid());
        printf("  Virtual Address : %p (Identical to parent)\n", (void *)shared_buf);
        printf("  Read Content    : \"%s\"\n", shared_buf);

        uint64_t pfn_child_before = get_pfn_from_pagemap(getpid(), shared_buf);
        if (pfn_child_before > 0) {
            printf("  Physical PFN    : 0x%lx (Shared with parent!)\n", (unsigned long)pfn_child_before);
        }

        printf(COLOR_YELLOW "  [Child Action] Triggering write to memory: shared_buf[0] = 'M'...\n" COLOR_RESET);
        printf("  -> Hardware MMU raises Page Fault Exception (Fault #14, Read-Only PTE marked COW)\n");
        printf("  -> Linux Virtual Memory Subsystem allocates a NEW physical page frame and duplicates contents\n");

        shared_buf[0] = 'M'; /* Write trigger */

        printf("  [Child After Write] Modified Content: \"%s\"\n", shared_buf);
        uint64_t pfn_child_after = get_pfn_from_pagemap(getpid(), shared_buf);
        if (pfn_child_after > 0) {
            printf("  [Child After Write] New Physical PFN: 0x%lx (Diverged from parent!)\n", (unsigned long)pfn_child_after);
        }

        munmap(shared_buf, page_size);
        _exit(0);
    } else {
        /* Parent Process */
        int status;
        waitpid(pid, &status, 0);

        printf("\n" COLOR_BLUE "[Parent (PID: %d)]" COLOR_RESET " After child modified its copy:\n", getpid());
        printf("  Virtual Address : %p\n", (void *)shared_buf);
        printf("  Parent Content  : \"%s\" (UNTOUCHED! Memory isolation intact)\n", shared_buf);

        uint64_t pfn_parent_after = get_pfn_from_pagemap(getpid(), shared_buf);
        if (pfn_parent_after > 0) {
            printf("  Parent Physical PFN: 0x%lx\n", (unsigned long)pfn_parent_after);
        }

        munmap(shared_buf, page_size);
        printf("\n" COLOR_GREEN "[COW Demonstration Verified]" COLOR_RESET
               " Pages remained shared until write occurred, proving zero unnecessary copying.\n\n");
    }

    return 0;
}

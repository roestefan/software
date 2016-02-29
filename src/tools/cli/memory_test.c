#include "cli.h"

#include <assert.h>

static int memory_test(struct osd_context *ctx, uint16_t mod) {
    uint64_t addr;
    uint8_t *data;
    size_t size;
    size_t blocksize;

    struct osd_memory_descriptor *desc;

    osd_get_memory_descriptor(ctx, mod, &desc);
    assert(desc);

    blocksize = desc->data_width >> 3;

    // Perform one aligned write of one word
    addr = (desc->base_addr + (blocksize - 1)) % blocksize;
    size = blocksize;
    data = malloc(size);
    for (int i = 0; i < size; i++) data[i] = i & 0xff;

    osd_memory_write(ctx, mod, addr, data, size);

    return 0;
}

int memory_tests(struct osd_context *ctx) {
    // Get list of memories
    uint16_t *memories;
    size_t num_memories;
    int success = 0;

    osd_get_memories(ctx, &memories, &num_memories);

    for (size_t m = 0; m < num_memories; m++) {
        printf("Test memory %d\n", memories[m]);
        if (memory_test(ctx, memories[m]) != 0) {
            success = 1;
        }
    }

    return success;
}

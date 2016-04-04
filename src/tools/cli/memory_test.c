#include "cli.h"

#include <assert.h>

static int memory_test(struct osd_context *ctx, uint16_t mod) {
    uint64_t addr;
    uint8_t *wdata, *rdata;
    size_t size;
    size_t blocksize;

    struct osd_memory_descriptor *desc;

    wdata = malloc(1024*1024);
    rdata = malloc(1024*1024);

    osd_get_memory_descriptor(ctx, mod, &desc);
    assert(desc);

    // Bytes per block
    blocksize = desc->data_width >> 3;

    // Perform one aligned write of one word
    addr = desc->base_addr;
    size = blocksize;
    for (size_t i = 0; i < size; i++) wdata[i] = i & 0xff;

    osd_memory_write(ctx, mod, addr, wdata, size);

    // Write the next ten blocks
    addr = desc->base_addr + blocksize;
    size = blocksize * 10;
    for (size_t i = 0; i < size; i++) wdata[i] = i & 0xff;

    osd_memory_write(ctx, mod, addr, wdata, size);

    // Read back the first block
    addr = desc->base_addr;
    size = blocksize;

    osd_memory_read(ctx, mod, addr, rdata, size);

    for (size_t i = 0; i < size; i++) printf("  %02x %02x\n", wdata[i], rdata[i]);

    // Read back the next ten blocks
    addr = desc->base_addr + blocksize;
    size = blocksize * 10;

    osd_memory_read(ctx, mod, addr, rdata, size);

    for (size_t i = 0; i < size; i++) printf("  %02x %02x\n", wdata[i], rdata[i]);

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

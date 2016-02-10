#include "osd-private.h"
#include <libglip.h>

#include <assert.h>
#include <stdio.h>

static void* receiver_thread_function(void* arg);

int osd_send_packet_standalone(struct osd_context *ctx, uint16_t *packet,
                               size_t size) {
    struct glip_ctx *gctx = ctx->ctx.standalone->glip_ctx;

    uint16_t *data = packet - 1;
    data[0] = size;

    size_t actual;
    glip_write_b(gctx, 0, (size+1)*2, (void*) data, &actual, 0);

    return OSD_SUCCESS;
}

int osd_connect_standalone(struct osd_context *ctx) {
    struct glip_ctx *gctx = ctx->ctx.standalone->glip_ctx;

    glip_open(gctx, 1);

    pthread_create(&ctx->ctx.standalone->receiver_thread, 0,
                   receiver_thread_function, ctx);

    return 0;
}

static void* receiver_thread_function(void* arg) {
    struct osd_context *ctx = (struct osd_context*) arg;
    struct glip_ctx *gctx = ctx->ctx.standalone->glip_ctx;

    uint16_t packet[64];
    size_t size, actual;

    int rv;

    while (1) {
        rv = glip_read_b(gctx, 0, 2, (void*) packet, &actual, 0);
        assert(rv == 0);

        size = *((uint16_t*) &packet[0]);

        rv = glip_read_b(gctx, 0, size*2, (void*) packet, &actual, 0);
        assert(rv == 0);

        osd_handle_packet(ctx, packet, size);
    }
}

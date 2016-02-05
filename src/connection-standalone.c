#include "osd-private.h"
#include <libglip.h>

#include <assert.h>
#include <stdio.h>

static void* receiver_thread_function(void* arg);

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

        uint8_t type = (packet[1] >> 10);

        if ((type >> 4) == 0) {
            // Register access
            pthread_mutex_lock(&ctx->reg_access.lock);

            memcpy(&ctx->reg_access.resp_packet, packet, size*2);

            ctx->reg_access.size = size;

            pthread_cond_signal(&ctx->reg_access.cond_complete);

            pthread_mutex_unlock(&ctx->reg_access.lock);
        } else {
            uint16_t mod_id = packet[1] & 0x3ff;
            uint16_t ev_size = (type & 0xf);

            if (size != ev_size + 2) {
                fprintf(stderr, "Incorrect event size packet received\n");
                continue;
            }

            if ((type >> 4) == OSD_INCOMING_PACKET) {
                void *arg = ctx->module_handlers[mod_id]->packet_handler.arg;
                if (!ctx->module_handlers[mod_id]->packet_handler.call) {
                    fprintf(stderr, "No module handler\n");
                    continue;
                }
                ctx->module_handlers[mod_id]->packet_handler.call(ctx, arg, packet, size);
            } else if ((type >> 4) == OSD_INCOMING_TRACE) {
            }
        }


    }
}

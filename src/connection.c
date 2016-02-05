#include "osd-private.h"
#include <libglip.h>

OSD_EXPORT
int osd_connect(struct osd_context *ctx) {
    pthread_mutex_init(&ctx->reg_access.lock, 0);
    pthread_cond_init(&ctx->reg_access.cond_complete, 0);

    int rv = ctx->functions.connect(ctx);

    if (rv != 0) {
        return OSD_E_GENERIC;
    }

    osd_system_enumerate(ctx);

    control_init(ctx);

    return OSD_SUCCESS;
}

OSD_EXPORT
int osd_send_packet(struct osd_context *ctx, uint16_t *data,
                    size_t size) {
    return ctx->functions.send(ctx, data, size);
}

int osd_send_packet_standalone(struct osd_context *ctx, uint16_t *packet,
                               size_t size) {
    struct glip_ctx *gctx = ctx->ctx.standalone->glip_ctx;

    uint16_t *data = packet - 1;
    data[0] = size;

    size_t actual;
    glip_write_b(gctx, 0, (size+1)*2, (void*) data, &actual, 0);

    return OSD_SUCCESS;
}

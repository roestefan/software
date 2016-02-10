#include "osd-private.h"
#include <libglip.h>

#include <stdio.h>

OSD_EXPORT
int osd_connect(struct osd_context *ctx) {
    pthread_mutex_init(&ctx->reg_access.lock, 0);
    pthread_cond_init(&ctx->reg_access.cond_complete, 0);

    int rv = ctx->functions.connect(ctx);

    if (rv != 0) {
        return rv;
    }

    rv = osd_system_enumerate(ctx);
    if (rv != OSD_SUCCESS) {
        return rv;
    }

    control_init(ctx);

    return OSD_SUCCESS;
}

OSD_EXPORT
int osd_send_packet(struct osd_context *ctx, uint16_t *data,
                    size_t size) {
    return ctx->functions.send(ctx, data, size);
}

void osd_handle_packet(struct osd_context *ctx, uint16_t *packet,
                       size_t size) {
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
        size_t ev_size = (type & 0xf);

        if (size != ev_size + 2) {
            fprintf(stderr, "Incorrect event size packet received\n");
            return;
        }

        if ((type >> 4) == OSD_EVENT_PACKET) {
            void *parg = ctx->module_handlers[mod_id]->packet_handler.arg;
            if (!ctx->module_handlers[mod_id]->packet_handler.call) {
                fprintf(stderr, "No module handler\n");
                return;
            }
            ctx->module_handlers[mod_id]->packet_handler.call(ctx, parg, packet, size);
        } else if ((type >> 4) == OSD_EVENT_TRACE) {
        }
    }
}

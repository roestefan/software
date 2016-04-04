#include "osd-private.h"

#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "tools/daemon/daemon-packets.h"

void control_init(struct osd_context *ctx) {
    size_t sz = sizeof(struct module_handler*);
    ctx->module_handlers = calloc(ctx->system_info->num_modules, sz);

    sz = sizeof(struct module_handler);
    for (int i = 0; i < ctx->system_info->num_modules + 1; i++) {
        ctx->module_handlers[i] = calloc(1, sz);
    }
}

OSD_EXPORT
int osd_module_claim(struct osd_context *ctx, uint16_t id) {
    return ctx->functions.claim(ctx, id);
}

int claim_standalone(struct osd_context *ctx, uint16_t id) {
    return 0;
}

int claim_daemon(struct osd_context *ctx, uint16_t id) {
    uint16_t packet[4];
    packet[0] = 3;
    packet[1] = 0xffff;
    packet[2] = OSD_DP_CLAIM;
    packet[3] = id;

    send(ctx->ctx.daemon->socket, packet, 8, 0);

    return 0;
}

OSD_EXPORT
int osd_module_register_handler(struct osd_context *ctx, uint16_t id,
                                enum osd_event_type type, void *arg,
                                osd_incoming_handler handler) {
    struct module_callback *cb;

    if (type == OSD_EVENT_PACKET) {
        cb = &ctx->module_handlers[id]->packet_handler;
    } else {
        return -1;
    }

    cb->call = handler;
    cb->arg = arg;

    return 0;
}

OSD_EXPORT
int osd_reset_system(struct osd_context *ctx, int halt_cores) {
    uint16_t scm;

    if (osd_get_scm(ctx, &scm) != OSD_SUCCESS) {
        return OSD_E_GENERIC;
    }

    osd_reg_write16(ctx, scm, 0x203, 0x3);

    if (halt_cores) {
        osd_reg_write16(ctx, scm, 0x203, 0x2);
    } else {
        osd_reg_write16(ctx, scm, 0x203, 0x0);
    }

    return OSD_SUCCESS;
}

OSD_EXPORT
int osd_start_cores(struct osd_context *ctx) {
    uint16_t scm;

    if (osd_get_scm(ctx, &scm) != OSD_SUCCESS) {
        return OSD_E_GENERIC;
    }

    osd_reg_write16(ctx, scm, 0x203, 0x0);

    return OSD_SUCCESS;
}

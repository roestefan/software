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
        cb = &ctx->module_handlers[id]->packet_handler;
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

    uint16_t addr = osd_modid2addr(ctx, scm);

    osd_reg_write16(ctx, addr, 0x203, 0x3);

    if (halt_cores) {
        osd_reg_write16(ctx, addr, 0x203, 0x2);
    } else {
        osd_reg_write16(ctx, addr, 0x203, 0x0);
    }

    return OSD_SUCCESS;
}

OSD_EXPORT
int osd_start_cores(struct osd_context *ctx) {
    uint16_t scm;

    if (osd_get_scm(ctx, &scm) != OSD_SUCCESS) {
        return OSD_E_GENERIC;
    }

    uint16_t addr = osd_modid2addr(ctx, scm);

    osd_reg_write16(ctx, addr, 0x203, 0x0);

    return OSD_SUCCESS;
}

OSD_EXPORT
int osd_module_stall(struct osd_context *ctx, uint16_t id) {
    osd_reg_write16(ctx, osd_modid2addr(ctx, id), OSD_REG_CS, OSD_CS_STALL);
    return 0;
}

OSD_EXPORT
int osd_module_unstall(struct osd_context *ctx, uint16_t id) {
    printf("UNSTALL\n");
    printf("to: %d\n", osd_modid2addr(ctx, id));
    osd_reg_write16(ctx, osd_modid2addr(ctx, id), OSD_REG_CS, OSD_CS_UNSTALL);
    return 0;
}

static void stm_log_handler (struct osd_context *ctx, void* arg, uint16_t* packet) {
    FILE *fh = (FILE*) arg;
    uint32_t timestamp;
    uint16_t id;
    uint64_t value;

    timestamp = (packet[4] << 16) | packet[3];
    id = packet[5];
    value = ((uint64_t)packet[9] << 48) | ((uint64_t)packet[8] << 32) | ((uint64_t)packet[7] << 16) | packet[6];

    fprintf(fh, "%08x %04x %016lx\n", timestamp, id, value);
    return;
}

OSD_EXPORT
int osd_stm_log(struct osd_context *ctx, uint16_t modid, char *filename) {
    FILE *fh = fopen(filename, "w");
    osd_module_claim(ctx, modid);
    osd_module_register_handler(ctx, modid, OSD_EVENT_TRACE, (void*) fh,
                                stm_log_handler);
    osd_module_unstall(ctx, modid);
    return 0;
}

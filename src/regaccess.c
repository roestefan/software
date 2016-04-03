#include "osd-private.h"

#include <assert.h>

OSD_EXPORT
int osd_reg_access(struct osd_context *ctx, uint16_t* packet) {

    pthread_mutex_lock(&ctx->reg_access.lock);

    osd_send_packet(ctx, packet);

    pthread_cond_wait(&ctx->reg_access.cond_complete,
                      &ctx->reg_access.lock);

    memcpy(packet, ctx->reg_access.resp_packet, (ctx->reg_access.size+1)*2);
    pthread_mutex_unlock(&ctx->reg_access.lock);

    return OSD_SUCCESS;
}

OSD_EXPORT
int osd_reg_read16(struct osd_context *ctx, uint16_t mod,
                   uint16_t addr, uint16_t *value) {

    uint16_t packet[4];
    size_t size = 3;

    packet[0] = size;
    packet[1] = mod & 0x3ff;
    packet[2] = (REG_READ16 << 10);
    packet[3] = addr;

    osd_reg_access(ctx, packet);
    assert(packet[0] == 3);

    *value = packet[3];

    return OSD_SUCCESS;
}

OSD_EXPORT
int osd_reg_write16(struct osd_context *ctx, uint16_t mod,
                    uint16_t addr, uint16_t value) {
    uint16_t packet[5];
    size_t size = 4;

    packet[0] = size;
    packet[1] = mod & 0x3ff;
    packet[2] = (REG_WRITE16 << 10);
    packet[3] = addr;
    packet[4] = value;

    osd_reg_access(ctx, packet);

    return OSD_SUCCESS;
}

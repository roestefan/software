#include "osd-private.h"

#include <assert.h>

OSD_EXPORT
int osd_reg_access(struct osd_context *ctx, uint16_t* packet,
                      size_t req_size, size_t *resp_size) {

    pthread_mutex_lock(&ctx->reg_access.lock);

    osd_send_packet(ctx, packet, req_size);

    pthread_cond_wait(&ctx->reg_access.cond_complete,
                      &ctx->reg_access.lock);

    if (*resp_size < ctx->reg_access.size) {
        return OSD_E_GENERIC;
    }

    *resp_size = ctx->reg_access.size;

    memcpy(packet, ctx->reg_access.resp_packet, *resp_size*2);
    pthread_mutex_unlock(&ctx->reg_access.lock);

    return OSD_SUCCESS;
}

OSD_EXPORT
int osd_reg_read16(struct osd_context *ctx, uint16_t mod,
                   uint16_t addr, uint16_t *value) {

    uint16_t data[4];
    uint16_t *packet = &data[1];
    size_t size = 3;

    packet[0] = mod & 0x3ff;
    packet[1] = (REG_READ16 << 10);
    packet[2] = addr;

    osd_reg_access(ctx, packet, 3, &size);
    assert(size == 3);

    *value = packet[2];

    return OSD_SUCCESS;
}

OSD_EXPORT
int osd_reg_write16(struct osd_context *ctx, uint16_t mod,
                    uint16_t addr, uint16_t value) {
    uint16_t data[5];
    uint16_t *packet = &data[1];
    size_t size = 4;

    packet[0] = mod & 0x3ff;
    packet[1] = (REG_WRITE16 << 10);
    packet[2] = addr;
    packet[3] = value;

    osd_reg_access(ctx, packet, 4, &size);

    return OSD_SUCCESS;
}

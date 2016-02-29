#include "osd-private.h"
#include <libglip.h>

static int memory_write_bulk(struct osd_context *ctx, uint16_t mod,
                             uint64_t addr,
                             uint8_t* data, size_t size) {
    uint16_t psize = osd_get_max_pkt_len(ctx);
    uint16_t wordsperpacket = psize - 2;

    uint16_t *pdata = malloc((psize+1)*2);
    uint16_t *packet = &pdata[1];

    struct osd_memory_descriptor *mem;
    mem = ctx->system_info->modules[mod].descriptor.memory;

    size_t hlen = 1; // control word
    hlen += ((mem->addr_width + 15) >> 4);
    uint16_t *header = &packet[2];

    header[0] = 0xc000 | size;
    header[1] = addr & 0xff;
    if (mem->addr_width > 16)
        header[2] = (addr >> 8) & 0xff;
    if (mem->addr_width > 32)
        header[3] = (addr >> 16) & 0xff;
    if (mem->addr_width > 48)
        header[4] = (addr >> 16) & 0xff;

    // Static for packets
    packet[0] = mod;
    packet[1] = 1 << 14;

    for (int i = 0; i < hlen + 2; i++)
        printf("  %04x\n", packet[i]);

    osd_send_packet(ctx, packet, hlen + 2);

    size_t numwords = size/2;
    printf("numwords: %d\n", numwords);
    int curword = 0;

    for (int i = 0; i < numwords; i++) {
        packet[2+curword] = (data[i*2] << 8) | data[i*2+1];
        curword++;

        if (curword > wordsperpacket) {
            osd_send_packet(ctx, packet, psize);
            curword = 0;
        }
    }

    if (curword != 0) {
        osd_send_packet(ctx, packet, curword + 2);
    }
}

OSD_EXPORT
int osd_memory_write(struct osd_context *ctx, uint16_t mod, uint64_t addr,
                     uint8_t* data, size_t size) {

    // TODO: prolog

    // bulk part
    memory_write_bulk(ctx, mod, addr, data, size);

    // TODO: epilog
    return 0;
}

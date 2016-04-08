#include "osd-private.h"
#include <libglip.h>

#include <gelf.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

static int memory_write_bulk(struct osd_context *ctx, uint16_t mod,
                             uint64_t addr,
                             uint8_t* data, size_t size) {

    if (size > 16383) {
        return -1;
    }

    uint16_t psize = osd_get_max_pkt_len(ctx);
    uint16_t wordsperpacket = psize - 2;
    size_t numwords = size/2;

    uint16_t *packet = malloc((psize+1)*2);

    struct osd_memory_descriptor *mem;
    mem = ctx->system_info->modules[mod].descriptor.memory;

    size_t hlen = 1; // control word
    hlen += ((mem->addr_width + 15) >> 4);
    uint16_t *header = &packet[3];

    header[0] = 0xc000 | numwords;
    header[1] = addr & 0xffff;
    if (mem->addr_width > 16)
        header[2] = (addr >> 16) & 0xffff;
    if (mem->addr_width > 32)
        header[3] = (addr >> 32) & 0xffff;
    if (mem->addr_width > 48)
        header[4] = (addr >> 48) & 0xffff;

    // Static for packets
    packet[0] = hlen + 2;
    packet[1] = mod;
    packet[2] = 1 << 14;

    osd_send_packet(ctx, packet);

    int curword = 0;

    for (size_t i = 0; i < numwords; i++) {
        packet[3+curword] = (data[i*2+1] << 8) | data[i*2];
        curword++;

        if (curword == wordsperpacket) {
            packet[0] = psize;
            osd_send_packet(ctx, packet);
            curword = 0;
        }
    }

    if (curword != 0) {
        packet[0] = curword + 2;
        osd_send_packet(ctx, packet);
    }

    return 0;
}

static int memory_write_single(struct osd_context *ctx, uint16_t mod,
                             uint64_t addr,
                             uint8_t* data, size_t size) {
    return 0;
}

static void memory_read_cb(struct osd_context *ctx, void* arg,
                           uint16_t* packet) {
    size_t num = packet[0] - 2;

    for (size_t i = 0; i < num; i++) {
        size_t idx = ctx->mem_access.count + i*2;
        ctx->mem_access.data[idx] = packet[3+i] >> 8;
        ctx->mem_access.data[idx+1] = packet[3+i] & 0xff;
    }

    ctx->mem_access.count += num;

    if (ctx->mem_access.count >= ctx->mem_access.size/2) {
        pthread_mutex_lock(&ctx->mem_access.lock);
        pthread_cond_signal(&ctx->mem_access.cond_complete);
        pthread_mutex_unlock(&ctx->mem_access.lock);
    }
}

static int memory_read_bulk(struct osd_context *ctx, uint16_t mod,
                            uint64_t addr,
                            uint8_t* data, size_t size) {
    uint16_t psize = osd_get_max_pkt_len(ctx);
    size_t numwords = size/2;

    uint16_t *packet = malloc((psize+1)*2);

    struct osd_memory_descriptor *mem;
    mem = ctx->system_info->modules[mod].descriptor.memory;

    size_t hlen = 1; // control word
    hlen += ((mem->addr_width + 15) >> 4);
    uint16_t *header = &packet[3];

    header[0] = 0x4000 | numwords;
    header[1] = addr & 0xffff;
    if (mem->addr_width > 16)
        header[2] = (addr >> 16) & 0xffff;
    if (mem->addr_width > 32)
        header[3] = (addr >> 32) & 0xffff;
    if (mem->addr_width > 48)
        header[4] = (addr >> 48) & 0xffff;

    osd_module_claim(ctx, mod);
    osd_module_register_handler(ctx, mod, OSD_EVENT_PACKET, 0, memory_read_cb);

    // Static for packets
    packet[0] = hlen + 2;
    packet[1] = mod;
    packet[2] = 1 << 14;

    pthread_mutex_lock(&ctx->mem_access.lock);
    ctx->mem_access.size = size;
    ctx->mem_access.data = data;
    ctx->mem_access.count = 0;

    osd_send_packet(ctx, packet);

    pthread_cond_wait(&ctx->mem_access.cond_complete,
                      &ctx->mem_access.lock);

    pthread_mutex_unlock(&ctx->mem_access.lock);

    return 0;
}

OSD_EXPORT
int osd_memory_write(struct osd_context *ctx, uint16_t mod, uint64_t addr,
                     uint8_t* data, size_t size) {
    struct osd_memory_descriptor *mem;
    mem = ctx->system_info->modules[mod].descriptor.memory;

    size_t blocksize = mem->data_width >> 3;

    int prolog = (blocksize - addr) % blocksize;
    int epilog = ((addr + size) % blocksize);
    int bulk = size - prolog - epilog;

    printf("write, prolog: %d, bulk: %d, epilog: %d\n", prolog, bulk, epilog);

    if (prolog) {
        memory_write_single(ctx, mod, addr, data, prolog);
    }

    if (bulk) {
        for (size_t i = 0; i < size; i += 16383) {
            size_t s = 16383;

            if ((i+s) > size) s = size - i - 1;

            printf("bulk part size %d\n", s);

            memory_write_bulk(ctx, mod, addr, data, s);
        }
    }

    if (epilog) {
        memory_write_single(ctx, mod, addr+prolog+bulk, &data[prolog+bulk], epilog);
    }

    return 0;
}

OSD_EXPORT
int osd_memory_read(struct osd_context *ctx, uint16_t mod, uint64_t addr,
                     uint8_t* data, size_t size) {
    struct osd_memory_descriptor *mem;
    mem = ctx->system_info->modules[mod].descriptor.memory;

    size_t blocksize = mem->data_width >> 3;

    int prolog = (blocksize - addr) % blocksize;
    int epilog = ((addr + size) % blocksize);
    int bulk = size - prolog - epilog;

    printf("read, prolog: %d, bulk: %d, epilog: %d\n", prolog, bulk, epilog);

    if (prolog) {
        uint8_t *tmp = malloc(blocksize);
        memory_read_bulk(ctx, mod, addr - blocksize + prolog, tmp, blocksize);
        memcpy(data, &tmp[prolog], prolog);
        free(tmp);
    }

    if (bulk) {
        memory_read_bulk(ctx, mod, addr, &data[prolog], bulk);
    }

    if (epilog) {
        uint8_t *tmp = malloc(blocksize);
        memory_read_bulk(ctx, mod, addr + prolog + bulk, tmp, blocksize);
        memcpy(data, &tmp[prolog+bulk], epilog);
        free(tmp);
    }

    return 0;
}

OSD_EXPORT
int osd_memory_loadelf(struct osd_context *ctx, uint16_t mod, char *filename) {
    int fd;
    Elf *elf_object;
    size_t num;
    int rv;

    printf("Open %s\n", filename);

    fd = open(filename, O_RDONLY , 0);
    if (fd < 0) {
        return -1;
    }

    if (elf_version(EV_CURRENT) == EV_NONE) {
        rv = -1;
        goto error_file;
    }

    printf("Get elf object\n");

    elf_object = elf_begin(fd , ELF_C_READ , NULL);
    if (elf_object == NULL) {
        printf("%s\n", elf_errmsg(-1));
        rv = -1;
        goto error_file;
    }

    printf("Get phdrnum\n");

    // Load program headers
    if (elf_getphdrnum(elf_object, &num)) {
        rv = -1;
        goto error_elf;
    }

    printf("phdrnum = %zu\n", num);

    for (size_t i = 0; i < num; i++) {
        printf("Load program header %zu\n", i);
        GElf_Phdr phdr;
        Elf_Data *data;
        if (gelf_getphdr(elf_object, i, &phdr) != &phdr) {
            rv = -1;
            goto error_elf;
        }

        data = elf_getdata_rawchunk(elf_object, phdr.p_offset, phdr.p_filesz, ELF_T_BYTE);
        if (data) {
            printf("Load elf program header to %lx, size %zu\n", phdr.p_paddr, data->d_size);
            osd_memory_write(ctx, mod, phdr.p_paddr, data->d_buf, data->d_size);
        }
    }

    return 0;
    for (size_t i = 0; i < num; i++) {
        printf("Verify program header %zu\n", i);
        GElf_Phdr phdr;
        Elf_Data *data;
        if (gelf_getphdr(elf_object, i, &phdr) != &phdr) {
            rv = -1;
            goto error_elf;
        }

        data = elf_getdata_rawchunk(elf_object, phdr.p_offset, phdr.p_filesz, ELF_T_BYTE);

        uint8_t *memory_data = malloc(data->d_size);
        osd_memory_read(ctx, mod, phdr.p_paddr, memory_data, data->d_size);
    }

    return 0;

    error_elf:
    elf_end(elf_object);
    error_file:
    close(fd);
    return rv;
}

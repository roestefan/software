#include "osd-private.h"
#include <libglip.h>

const struct module_types module_lookup[4] = {
        { .name = "HOST" },
        { .name = "SCM" },
        { .name = "DEM - UART" },
        { .name = "MAM" }
};

int osd_system_enumerate(struct osd_context *ctx) {
    uint16_t mod1_id, mod_num;
    osd_reg_read16(ctx, 1, 0, &mod1_id);

    if (mod1_id != 0x1) {
        return OSD_E_CANNOTENUMERATE;
    }

    osd_reg_read16(ctx, 1, 0x201, &mod_num);
    mod_num += 1;

    size_t size = sizeof(struct osd_system_info);
    size += sizeof(struct osd_module_info) * mod_num;

    ctx->system_info = calloc(1, size);

    ctx->system_info->num_modules = mod_num;

    osd_reg_read16(ctx, 1, 0x200, &ctx->system_info->identifier);

    osd_reg_read16(ctx, 1, 0x202, &ctx->system_info->max_pkt_len);

    ctx->system_info->modules[0].addr = 0;
    ctx->system_info->modules[0].type = 0;
    ctx->system_info->modules[0].version = 0;

    for (size_t i = 1; i < mod_num; i++) {
        struct osd_module_info *mod = &ctx->system_info->modules[i];
        mod->addr = i;
        osd_reg_read16(ctx, i, 0, &mod->type);
        if (mod->type == OSD_MOD_MAM) {
            ctx->system_info->num_memories++;

            struct osd_memory_descriptor *mem;
            mem = calloc(1, sizeof(struct osd_memory_descriptor));
            mod->descriptor.memory = mem;

            osd_reg_read16(ctx, i, 0x200, &mem->data_width);
            osd_reg_read16(ctx, i, 0x201, &mem->addr_width);

            uint16_t r;
            osd_reg_read16(ctx, i, 0x202, &r);
            mem->base_addr = r;
            osd_reg_read16(ctx, i, 0x203, &r);
            mem->base_addr |= ((uint64_t) r << 16);
            osd_reg_read16(ctx, i, 0x204, &r);
            mem->base_addr |= ((uint64_t) r << 32);
            osd_reg_read16(ctx, i, 0x205, &r);
            mem->base_addr |= ((uint64_t) r << 48);

            osd_reg_read16(ctx, i, 0x206, &r);
            mem->size = r;
            osd_reg_read16(ctx, i, 0x207, &r);
            mem->size |= ((uint64_t) r << 16);
            osd_reg_read16(ctx, i, 0x208, &r);
            mem->size |= ((uint64_t) r << 32);
            osd_reg_read16(ctx, i, 0x209, &r);
            mem->size |= ((uint64_t) r << 48);
        }

        osd_reg_read16(ctx, i, 1, &mod->version);
    }

    return OSD_SUCCESS;
}

int osd_get_scm(struct osd_context *ctx, uint16_t *addr) {
    if (ctx->system_info->modules[1].type != OSD_MOD_SCM) {
        return OSD_E_GENERIC;
    }

    *addr = 1;
    return OSD_SUCCESS;
}

OSD_EXPORT
int osd_get_memories(struct osd_context *ctx,
                            uint16_t **memories, size_t *num) {

    *num = ctx->system_info->num_memories;

    *memories = malloc(sizeof(uint16_t) * ctx->system_info->num_memories);

    uint16_t num_mod = ctx->system_info->num_modules;

    for (uint16_t i = 0, m = 0; (i < num_mod) && (m < *num); i++) {
        if (ctx->system_info->modules[i].type == OSD_MOD_MAM) {
            *memories[m++] = i;
        }
    }

    return OSD_SUCCESS;
}

OSD_EXPORT
int osd_get_memory_descriptor(struct osd_context *ctx, uint16_t addr,
                              struct osd_memory_descriptor **desc) {
    if (ctx->system_info->modules[addr].type != OSD_MOD_MAM) {
        return OSD_E_GENERIC;
    }

    size_t sz = sizeof(struct osd_memory_descriptor);
    *desc = malloc(sz);

    memcpy(*desc, ctx->system_info->modules[addr].descriptor.memory, sz);

    return OSD_SUCCESS;
}

OSD_EXPORT
int osd_get_system_identifier(struct osd_context *ctx, uint16_t *id) {
    *id = ctx->system_info->identifier;
    return OSD_SUCCESS;
}

OSD_EXPORT
size_t osd_get_max_pkt_len(struct osd_context *ctx) {
    return ctx->system_info->max_pkt_len;
}

OSD_EXPORT
int osd_get_num_modules(struct osd_context *ctx, uint16_t *n) {
    *n = ctx->system_info->num_modules;
    return OSD_SUCCESS;
}

OSD_EXPORT
int osd_get_module_name(struct osd_context *ctx, uint16_t id,
                        char **name) {
    uint16_t type = ctx->system_info->modules[id].type;
    if (type > modules_max_id) {
        *name = strdup("UNKNOWN");
    }
    *name = strdup(module_lookup[type].name);

    return OSD_SUCCESS;
}

OSD_EXPORT
int osd_print_module_info(struct osd_context *ctx, uint16_t addr,
                          FILE* fh, int indent) {
    struct osd_module_info *mod = &ctx->system_info->modules[addr];

    if (!mod) {
        return OSD_E_GENERIC;
    }

    char *indentstring = malloc(indent+1);
    memset(indentstring, 0x20, indent);
    indentstring[indent] = 0;

    fprintf(fh, "%sversion: %04x\n", indentstring, mod->version);

    struct osd_memory_descriptor *mem;
    switch (mod->type) {
        case OSD_MOD_MAM:
            mem = mod->descriptor.memory;
            fprintf(fh, "%sdata width: %d, ", indentstring,
                    mem->data_width);
            fprintf(fh, "address width: %d\n", mem->addr_width);
            fprintf(fh, "%sbase address: 0x%016lx, ", indentstring,
                    mem->base_addr);
            fprintf(fh, "memory size: %ld Bytes\n", mem->size);
            break;
        default:
            break;
    }

    return OSD_SUCCESS;
}

OSD_EXPORT
int osd_module_is_terminal(struct osd_context *ctx, uint16_t id) {
    uint16_t type = ctx->system_info->modules[id].type;

    switch (type) {
        case OSD_MOD_DEM_UART:
            return 1;
            break;
        default:
            return 0;
            break;
    }

    return OSD_SUCCESS;
}


#include "osd-private.h"
#include <libglip.h>

uint16_t modules_max_id;

const struct module_types module_lookup[3] = {
        { .name = "HOST" },
        { .name = "SCM" },
        { .name = "DEM - UART" }
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
    ctx->system_info->modules[0].identifier = 0;
    ctx->system_info->modules[0].version = 0;

    for (size_t i = 1; i < mod_num; i++) {
        struct osd_module_info *mod = &ctx->system_info->modules[i];
        mod->addr = i;
        osd_reg_read16(ctx, i, 0, &mod->identifier);
        if (mod->identifier == OSD_MOD_MAM) {
            ctx->system_info->num_memories++;
        }
        osd_reg_read16(ctx, i, 1, &mod->version);
    }

    return OSD_SUCCESS;
}

int osd_module_get_scm(struct osd_context *ctx, uint16_t *addr) {
    if (ctx->system_info->modules[1].identifier != OSD_MOD_SCM) {
        return OSD_E_GENERIC;
    }

    *addr = 1;
    return OSD_SUCCESS;
}

int osd_module_get_memories(struct osd_context *ctx,
                            uint16_t **memories, size_t *num) {

}

OSD_EXPORT
int osd_get_system_identifier(struct osd_context *ctx, uint16_t *id) {
    *id = ctx->system_info->identifier;
    return OSD_SUCCESS;
}

OSD_EXPORT
int osd_get_max_pkt_len(struct osd_context *ctx, uint16_t *len) {
    *len = ctx->system_info->max_pkt_len;
    return OSD_SUCCESS;
}

OSD_EXPORT
int osd_get_num_modules(struct osd_context *ctx, uint16_t *n) {
    *n = ctx->system_info->num_modules;
    return OSD_SUCCESS;
}

OSD_EXPORT
int osd_get_module_name(struct osd_context *ctx, uint16_t id,
                        char **name) {
    uint16_t type = ctx->system_info->modules[id].identifier;
    if (type > modules_max_id) {
        *name = strdup("UNKNOWN");
    }
    *name = strdup(module_lookup[type].name);

    return OSD_SUCCESS;
}

OSD_EXPORT
int osd_module_is_terminal(struct osd_context *ctx, uint16_t id) {
    uint16_t type = ctx->system_info->modules[id].identifier;

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


#include "daemon.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    INFO("Open SoC Debug Daemon");

    struct osd_context *ctx;

    if (argc < 2) {
        ERR("Usage: opensocdebugd <backend> [<backend_option> ..]");
    }

    INFO("Backend: %s", argv[1]);

    struct osd_mode_option *options = calloc(argc-1, sizeof(struct osd_mode_option));
    options[0].name = "backend";
    options[0].value = argv[1];

    for (int i = 2; i < argc; i++) {
        options[i-1].name = "backend_option";
        options[i-1].value = argv[i];
    }

    osd_new(&ctx, OSD_MODE_STANDALONE, argc-1, options);


    osd_connect(ctx);

    uint16_t systemid, num_modules;
    osd_get_system_identifier(ctx, &systemid);
    INFO("System ID: %04x", systemid);

    osd_get_num_modules(ctx, &num_modules);
    INFO("%d debug modules found:", num_modules);

    for (int i = 0; i < num_modules; i++) {
        uint16_t addr;
        char *name;

        osd_get_module_addr(ctx, i, &addr);
        osd_get_module_name(ctx, i, &name);

        INFO(" [%d]: %s", addr, name);

        free(name);
    }

    for (int i = 0; i < num_modules; i++) {
        if (osd_module_is_terminal(ctx, i)) {
            uint16_t addr;
            osd_get_module_addr(ctx, i, &addr);
            INFO("Open terminal for module %d", addr);
            struct terminal *term_arg;
            terminal_open(&term_arg);

            osd_module_claim(ctx, i);
            osd_module_register_handler(ctx, addr, OSD_INCOMING_PACKET,
                                        term_arg, terminal_ingress);

            osd_reg_write16(ctx, addr, 0x3, 0x1 << 11);
        }
    }

    while(1) {}
}

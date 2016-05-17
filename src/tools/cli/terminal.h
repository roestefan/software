#ifndef __TERMINAL_H__
#define __TERMINAL_H__

#include <stdint.h>

struct terminal {
    int socket_listen;
    int socket;
    char *path;
    pid_t child;
};

int terminal_open(struct terminal **term);

void terminal_ingress(struct osd_context *ctx, void* mod_arg,
                      uint16_t *packet);

#endif

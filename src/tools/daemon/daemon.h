#ifndef _DAEMON_H_
#define _DAEMON_H_

#include <opensocdebug.h>

#include <stdio.h>
#include <err.h>

#define INFO(format, ...) fprintf (stdout, format "\n", ##__VA_ARGS__)
#define WARN(format, ...) warnx(format, ##__VA_ARGS__)
#define ERR(format, ...) errx(1, format, ##__VA_ARGS__)

struct terminal;

int terminal_open(struct terminal **term);

void terminal_ingress(struct osd_context *ctx, void* mod_arg,
                      uint16_t *packet);

#endif

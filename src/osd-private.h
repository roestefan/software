#ifndef _OSD_PRIVATE_H_
#define _OSD_PRIVATE_H_

#include <libglip.h>

#include <pthread.h>
#include <stdlib.h>

#include "include/opensocdebug.h"

#define OSD_EXPORT __attribute__ ((visibility("default")))

struct module_handler;

struct osd_module_info {
    uint16_t addr;
    uint16_t type;
    uint16_t version;

    union {
        struct osd_memory_descriptor *memory;
    } descriptor;
};

struct osd_system_info {
    uint16_t identifier;
    uint16_t max_pkt_len;
    uint16_t num_modules;
    uint16_t num_memories;
    struct osd_module_info modules[];
};

struct osd_context_standalone {
    struct glip_ctx *glip_ctx;
    pthread_t receiver_thread;
};

struct osd_context_daemon {
    char* host;
    int port;
    int socket;
    pthread_t receiver_thread;
};

struct osd_mode_functions {
    int (*connect)(struct osd_context *);
    int (*send)(struct osd_context *, uint16_t *, size_t);
    int (*claim)(struct osd_context *ctx, uint16_t);
};

struct osd_context {
    enum osd_mode mode;
    union {
        struct osd_context_standalone *standalone;
        struct osd_context_daemon *daemon;
    } ctx;
    struct osd_mode_functions functions;

    struct {
        pthread_mutex_t lock;
        pthread_cond_t cond_complete;
        size_t size;
        uint16_t resp_packet[10];
    } reg_access;

    struct osd_system_info *system_info;

    struct module_handler **module_handlers;
};

int osd_connect_standalone(struct osd_context *ctx);
int osd_connect_daemon(struct osd_context *ctx);

int osd_send_packet_standalone(struct osd_context *ctx, uint16_t *data,
                               size_t size);
int osd_send_packet_daemon(struct osd_context *ctx, uint16_t *data,
                               size_t size);

void osd_handle_packet(struct osd_context *ctx, uint16_t *packet, size_t size);

int osd_system_enumerate(struct osd_context *ctx);

static const uint8_t REG_READ16 = 0x0;
static const uint8_t REG_READ32 = 0x1;
static const uint8_t REG_READ64 = 0x2;
static const uint8_t REG_READ128 = 0x3;
static const uint8_t REG_WRITE16 = 0x4;
static const uint8_t REG_WRITE32 = 0x5;
static const uint8_t REG_WRITE64 = 0x6;
static const uint8_t REG_WRITE128 = 0x7;

static const uint16_t modules_max_id = 4;
struct module_types {
    const char *name;
};
extern const struct module_types module_lookup[4];

struct module_callback {
    osd_incoming_handler call;
    void *arg;
};

struct module_handler {
    struct module_callback packet_handler;
    size_t num_trace_handlers;
    struct module_callback trace_handlers[];
};

void control_init(struct osd_context *ctx);
int claim_standalone(struct osd_context *ctx, uint16_t id);
int claim_daemon(struct osd_context *ctx, uint16_t id);

#endif

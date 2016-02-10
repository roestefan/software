/* Copyright (c) 2012-2016 by the author(s)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * ============================================================================
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

#ifndef _OPENSOCDEBUG_H_
#define _OPENSOCDEBUG_H_

#include <stdarg.h>
#include <inttypes.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Opaque context object
 *
 * This object contains all state information. Create and initialize a new
 * object with opensocdebug_new() and delete it with opensocdebug_free().
 */
struct osd_context;

struct osd_mode_option {
    char* name;
    char* value;
};

enum osd_mode {
    OSD_MODE_STANDALONE,
    OSD_MODE_DAEMON
};

int osd_new(struct osd_context **ctx, enum osd_mode standalone,
            size_t num_mode_options, struct osd_mode_option *options);

int osd_connect(struct osd_context *ctx);

struct osd_dp {
    size_t payload_size;
    uint8_t type;
    uint16_t dest;
    uint16_t src;
    uint16_t payload[];
};

enum osd_module_types {
    OSD_MOD_HOST = 0,
    OSD_MOD_SCM = 1,
    OSD_MOD_DEM_UART = 2,
    OSD_MOD_MAM = 3
};

static const int OSD_SUCCESS = 0;
static const int OSD_E_GENERIC = 1;
static const int OSD_E_CANNOTENUMERATE = 2;

/*static inline
struct osd_dp *osd_dp_alloc(size_t payload_size) {
    size_t size = sizeof(struct osd_dp) + payload_size * 2;
    uint16_t *packet = malloc(size + 2);
    return (struct osd_dp*) &packet[1];
}

static inline
int osd_dp_extract(struct osd_dp* dp, uint16_t *data, size_t size) {
    dp->payload_size = size - 2;

    dp->dest = data[0];
    dp->type = data[1] >> 10;
    dp->src = data[1] & 0x3ff;
    memcpy(dp->payload, &data[2], size - 2);

    return 0;
}

static inline
int osd_dp_assemble(struct osd_dp* dp, uint16_t *data, size_t *size) {
    if (*size < (dp->payload_size + 2)) {
        return OSD_E_GENERIC;
    }

    *size = dp->payload_size + 2;

    data[0] = dp->dest & 0x3ff;
    data[1] = (dp->type << 10) | (dp->src & 0x3ff);
    memcpy(&data[2], dp->payload, dp->payload_size*2);

    return OSD_SUCCESS;
}*/

int osd_reset_system(struct osd_context *ctx, int halt_cores);
int osd_start_cores(struct osd_context *ctx);

int osd_send_packet(struct osd_context *ctx, uint16_t *data,
                    size_t size);

int osd_reg_access(struct osd_context *ctx, uint16_t* packet,
                      size_t req_size, size_t *resp_size);
int osd_reg_read16(struct osd_context *ctx, uint16_t mod,
                   uint16_t addr, uint16_t *value);
int osd_reg_write16(struct osd_context *ctx, uint16_t mod,
                    uint16_t addr, uint16_t value);

int osd_get_system_identifier(struct osd_context *ctx, uint16_t *id);
int osd_get_max_pkt_len(struct osd_context *ctx, uint16_t *len);
int osd_get_num_modules(struct osd_context *ctx, uint16_t *n);

int osd_get_module_name(struct osd_context *ctx, uint16_t addr,
                        char **name);

int osd_module_is_terminal(struct osd_context *ctx, uint16_t addr);

int osd_module_get_scm(struct osd_context *ctx, uint16_t *addr);
int osd_module_get_memories(struct osd_context *ctx,
                            uint16_t **memories, size_t *num);

int osd_module_claim(struct osd_context *ctx, uint16_t addr);

enum osd_event_type {
    OSD_EVENT_PACKET = 1,
    OSD_EVENT_TRACE = 2
};

typedef void (*osd_incoming_handler)(struct osd_context *ctx,
        void* arg, uint16_t* packet, size_t size);

int osd_module_register_handler(struct osd_context *ctx, uint16_t id,
                                enum osd_event_type type, void *arg,
                                osd_incoming_handler handler);

int osd_memory_write(struct osd_context *ctx, uint64_t addr, uint8_t* data, size_t size);
int osd_memory_read(struct osd_context *ctx, uint64_t addr, uint8_t* data, size_t size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

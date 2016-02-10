#include "osd-private.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

static void* receiver_thread_function(void* arg);

int osd_send_packet_daemon(struct osd_context *ctx, uint16_t *packet,
                               size_t size) {
    uint16_t *data = packet - 1;
    data[0] = size;

    send(ctx->ctx.daemon->socket, data, (size+1)*2, 0);

    return OSD_SUCCESS;
}

int osd_connect_daemon(struct osd_context *ctx) {
    struct osd_context_daemon *c = ctx->ctx.daemon;
    struct hostent *server;

    struct sockaddr_in addr;

    c->socket = socket(AF_INET, SOCK_STREAM, 0);
    if (c->socket < 0) {
        return OSD_E_GENERIC;
    }

    server = gethostbyname(c->host);
    if (server == NULL) {
        return OSD_E_GENERIC;
    }

    /* build the server's Internet address */
    bzero((char *) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&addr.sin_addr.s_addr, server->h_length);
    addr.sin_port = htons(c->port);

    /* connect: create a connection with the server */
    if (connect(c->socket, &addr, sizeof(addr)) < 0) {
      return OSD_E_GENERIC;
    }

    pthread_create(&c->receiver_thread, 0,
                   receiver_thread_function, ctx);

    return 0;
}

static void* receiver_thread_function(void* arg) {
    struct osd_context *ctx = (struct osd_context*) arg;
    struct osd_context_daemon *dctx = ctx->ctx.daemon;

    uint16_t packet[64];
    size_t size;

    int rv;

    while (1) {
        rv = recv(dctx->socket, packet, 2, 0);
        assert(rv == 2);

        size = *((uint16_t*) &packet[0]);

        rv = recv(dctx->socket, packet, size*2, 0);
        assert(rv == size*2);

        osd_handle_packet(ctx, packet, size);
    }
}

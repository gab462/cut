#ifndef INCLUDE_TCP_TASK_H
#define INCLUDE_TCP_TASK_H

#include "task.h"
#include "tcp.h"
#include "cut.h"
#include <stdio.h>

typedef void (*tcp_handler_t)(struct task_context *ctx, socket_t sock, struct sockaddr_in addr);

static inline
void
tcp_server(struct task_context *ctx, socket_t sock, tcp_handler_t handler)
{
    struct tcp_client {
        struct task_context ctx;
        socket_t sock;
        struct sockaddr_in addr;
    };

    struct tcp_client **clients = task_ctx_alloc(ctx, struct tcp_client *);

    task_begin(ctx);

    struct sockaddr_in addr;
    socket_t client_sock = tcp_accept(sock, &addr); // TODO: accept more than one client per tick

    if(client_sock != SOCK_INVALID){
        printf("Accepted connection %d.\n", da_len(*clients));
        da_push(clients, { .sock = client_sock, .addr = addr });
        printf("Total connections: %d.\n", da_len(*clients));
    }

    for(int i = 0; i < da_len(*clients); i++){
        struct tcp_client *client = &(*clients)[i];

        handler(&client->ctx, client->sock, client->addr);

        if(!client->ctx.running){
            da_swap_delete(clients, i);
            i--;

            printf("Total connections: %d.\n", da_len(*clients));

            // Cleanup memory when no clients connected
            if(da_len(*clients) == 0)
                da_reset(clients);
        }
    }

    task_return(ctx);

    task_end(ctx);
}

#endif

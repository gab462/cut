#ifndef INCLUDE_TCP_TASK_H
#define INCLUDE_TCP_TASK_H

#include "task.h"
#include "tcp.h"
#include "cut.h"
#include <stdio.h>

typedef bool (*tcp_handler_t)(void **ctx, int fd, struct sockaddr_in addr);

static inline
void
tcp_server(void **ctx, int fd, tcp_handler_t handler)
{
    struct tcp_client {
        void *ctx;
        int fd;
        struct sockaddr_in addr;
    };

    task_context_begin();
    struct tcp_client *clients;
    task_context_end();

    task_begin(ctx);

    struct sockaddr_in addr;
    int client_fd = tcp_accept(fd, &addr); // only accepts one client per tick

    if(client_fd != -1){
        printf("Accepted connection %d.\n", len(task_ctx(ctx)->clients));
        da_push(&task_ctx(ctx)->clients, { .fd = client_fd, .addr = addr });
        printf("Total connections: %d.\n", len(task_ctx(ctx)->clients));
    }

    for(int i = 0; i < da_len(task_ctx(ctx)->clients); i++){
        struct tcp_client *client = &task_ctx(ctx)->clients[i];

        bool done = handler(&client->ctx, client->fd, client->addr);

        if(done){
            da_swap_delete(&task_ctx(ctx)->clients, i);
            i--;

            printf("Total connections: %d.\n", len(task_ctx(ctx)->clients));

            // Cleanup memory when no clients connected
            if(da_len(task_ctx(ctx)->clients) == 0)
                da_reset(&task_ctx(ctx)->clients);
        }
    }

    return;

    task_end(ctx);
}

#endif

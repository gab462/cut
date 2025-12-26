#ifndef INCLUDE_TCP_TASK_H
#define INCLUDE_TCP_TASK_H

#include "task.h"
#include "tcp.h"
#include "cut.h"
#include <sys/socket.h>
#include <stdio.h>

typedef bool (*tcp_handler_t)(int fd, struct sockaddr_in addr, void **data);

struct tcp_cli_conn {
    struct task interface;
    int fd;
    struct sockaddr_in addr;
    tcp_handler_t handler;
};

static inline
bool
tcp_cli_conn_poll(struct task *interface)
{
    struct tcp_cli_conn *self = (struct tcp_cli_conn *) interface;

    return(self->handler(self->fd, self->addr, &interface->data));
}

static inline
struct task *
tcp_cli_conn(int fd, struct sockaddr_in addr, tcp_handler_t handler)
{
    struct tcp_cli_conn task = {
        .interface.poll = tcp_cli_conn_poll,
        .fd = fd,
        .addr = addr,
        .handler = handler
    };

    return(cut_memdup(&task, sizeof(task)));
}

struct tcp_acceptor {
    struct task interface;
    int fd;
    tcp_handler_t handler;
};

static inline
bool
tcp_acceptor_poll(struct task *interface)
{
    struct tcp_acceptor *self = (struct tcp_acceptor *) interface;
    struct task ***client_tasks = interface->data;

    struct sockaddr_in addr;
    int client = tcp_accept(self->fd, &addr);

    if(client != -1){
        printf("Accepted connection %d.\n", len(*client_tasks));
        da_push(client_tasks, tcp_cli_conn(client, addr, self->handler));
    }

    return(false);
}

static inline
struct task *
tcp_acceptor(short port, tcp_handler_t handler)
{
    struct tcp_acceptor task = {
        .interface.poll = tcp_acceptor_poll,
        .fd = tcp_listen(port),
        .handler = handler
    };

    return(cut_memdup(&task, sizeof(task)));
}

static inline
struct task *
tcp_server(short port, tcp_handler_t handler)
{
    struct task_group *task = (struct task_group *) task_group(tcp_acceptor(port, handler));
    task->interface.data = &task->list;
    return &task->interface;
}

#endif

#include "../cut.c"
#include "../sock.c"
#include "../tcp.c"
#include "../task.c"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define IP "127.0.0.1"
#define PORT "8080"

struct handler {
    struct task interface;
    int fd;
    struct sockaddr_in addr;
};

bool
handler_poll(struct task *interface)
{
    struct handler *self = (struct handler *) interface;
    char **msg = (char **) &interface->data;

    ssize_t received = sock_read(self->fd, msg);

    if(received == 0 || (received == -1 && errno != EAGAIN)){ // Connection closed or error
        perror("Lost connection");
        close(self->fd);
        da_reset(msg);
        return(true);
    }

    ssize_t sent = sock_write(self->fd, msg);

    if(sent > 0)
        printf("Sent %ld bytes\n", sent);

    return(false);
}

struct task *
handler(int fd, struct sockaddr_in addr)
{
    struct handler task = {
        .interface.poll = handler_poll,
        .fd = fd,
        .addr = addr
    };

    void *out = malloc(sizeof(task));
    return(memcpy(out, &task, sizeof(task)));
}

struct acceptor {
    struct task interface;
    int fd;
};

bool
acceptor_poll(struct task *interface)
{
    struct acceptor *self = (struct acceptor *) interface;
    struct task ***client_tasks = (struct task ***) interface->data;

    struct sockaddr_in addr;
    int client = tcp_accept(self->fd, &addr);

    if(client != -1){
        printf("Accepted connection %d.\n", len(*client_tasks));
        push(client_tasks, handler(client, addr));
    }

    return(false);
}

struct task *
acceptor(short port)
{
    struct acceptor task = {
        .interface.poll = acceptor_poll,
        .fd = tcp_listen(port)
    };

    void *out = malloc(sizeof(task));
    return(memcpy(out, &task, sizeof(task)));
}

int
main(void)
{
    short port = atoi(PORT);

    struct task_group *task
        = (struct task_group *) task_group(acceptor(port));

    printf("Listening on %s:%s...\n", IP, PORT);

    task->interface.data = &task->list;

    while(!task_poll(&task->interface))
        usleep(8000);

    free(task);

    return(0);
}

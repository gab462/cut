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

struct server_client_task {
    struct task interface;
    int fd;
    struct sockaddr_in addr;
    char *msg;
};

bool
server_client_task_poll(struct task *task)
{
    struct server_client_task *client = (struct server_client_task *) task;

    ssize_t received = sock_read(client->fd, &client->msg);

    if(received == -1 && errno != EAGAIN){ // Connection error, close
        printf("Lost connection.\n");
        close(client->fd);
        da_reset(&client->msg);
        return(true);
    }

    sock_write(client->fd, &client->msg);

    return(false);
}

struct task *
server_client_task(int fd, struct sockaddr_in addr)
{
    struct server_client_task *task = calloc(1, sizeof(struct server_client_task));
    task->interface.poll = server_client_task_poll;
    task->fd = fd;
    task->addr = addr;

    return(&task->interface);
}

struct server_accept_task {
    struct task interface;
    int fd;
};

bool
server_accept_task_poll(struct task *task)
{
    struct server_accept_task *server = (struct server_accept_task *) task;
    struct task ***client_tasks = (struct task ***) task->data;

    struct sockaddr_in addr;
    int client = tcp_accept(server->fd, &addr);

    if(client != -1){
        printf("Accepted connection.\n");
        push(client_tasks, server_client_task(client, addr));
    }

    return(false);
}

struct task *
server_accept_task(short port)
{
    struct server_accept_task *task = calloc(1, sizeof(struct server_accept_task));
    task->interface.poll = server_accept_task_poll;
    task->fd = tcp_listen(port);

    return(&task->interface);
}

int
main(void)
{
    short port = atoi(PORT);

    struct concurrent_task *task
        = (struct concurrent_task *) task_group(server_accept_task(port));

    printf("Listening on %s:%s...\n", IP, PORT);

    task->interface.data = &task->list;

    while(!task_poll(&task->interface))
        usleep(8000);

    free(task);

    return(0);
}

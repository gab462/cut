#include "../cut.c"
#include "../sock.c"
#include "../tcp.c"
#include "../task.c"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#define IP "127.0.0.1"
#define PORT "8080"

struct connection {
    struct task interface;
    int fd;
};

bool
connection_poll(struct task *interface)
{
    struct connection *self = (struct connection *) interface;
    char **msg = (char **) &interface->data;

    char buf[4096];

    ssize_t received = read(self->fd, buf, sizeof(buf));

    if(received == 0 || (received == -1 && errno != EAGAIN)){ // Connection closed or error
        perror("Lost connection");
        close(self->fd);
        da_reset(msg);
        return(true);
    }

    if(received > 0)
        write(STDOUT_FILENO, buf, received);

    if(len(*msg) > 0)
        sock_write(self->fd, msg);

    return(false);
}

struct task *
connection(char *ip, char *port)
{
    struct connection task = {
        .interface.poll = connection_poll,
        .fd = tcp_connect(ip, port)
    };

    void *out = malloc(sizeof(task));
    return(memcpy(out, &task, sizeof(task)));
}

int
main(void)
{
    struct task *task = connection(IP, PORT);
    char **msg = (char **) &task->data;

    printf("Connected to %s:%s\n", IP, PORT);

    sock_set_nonblock(STDIN_FILENO);

    while(!task_poll(task)){
        char buf[64];
        ssize_t count = read(STDIN_FILENO, buf, sizeof(buf) - 1);

        if(count == -1){
            assert(errno == EAGAIN);
        }else if(count > 0){
            push_items(msg, buf, count);
        }

        usleep(8000);
    }

    free(task);

    return(0);
}

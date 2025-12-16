#include "../cut.c"
#include "../tcp.c"
#include "../task.c"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#define IP "127.0.0.1"
#define PORT "8080"

struct client_task {
    struct task interface;
    int fd;
    char *msg;
};

bool
client_task_poll(struct task *task)
{
    struct client_task *client = (struct client_task *) task;

    char buf[4096];

    ssize_t received = recv(client->fd, buf, sizeof(buf), 0);

    if(received == -1 && errno != EAGAIN){ // Connection error, close
        close(client->fd);
        da_reset(&client->msg);
        return(true);
    }

    if(received > 0)
        write(STDOUT_FILENO, buf, received);

    if(len(client->msg) > 0){
        ssize_t sent = send(client->fd, client->msg, len(client->msg), 0);

        if(sent > 0){
            memmove(client->msg, client->msg + sent, len(client->msg) - sent);
            da_header(client->msg)->length -= sent;
        }
    }

    return(false);
}

struct task *
client_task(char *ip, char *port)
{
    struct client_task *task = calloc(1, sizeof(struct client_task));
    task->interface.poll = client_task_poll;
    task->fd = tcp_connect(ip, port);

    return(&task->interface);
}

int
main(void)
{
    struct client_task *task = (struct client_task *) client_task(IP, PORT);

    tcp_set_nonblock(STDIN_FILENO);

    while(!task_poll(&task->interface)){
        char buf[64];
        ssize_t count = read(STDIN_FILENO, buf, sizeof(buf) - 1);

        if(count == -1){
            assert(errno == EAGAIN);
        }else if(count > 0){
            push_items(&task->msg, buf, count);
        }

        usleep(8000);
    }

    free(task);

    return(0);
}

#include "../cut.c"
#include "../sock.c"
#include "../tcp_task.c"
#include "../task.c"
#include <stdio.h>
#include <errno.h>

#define IP "127.0.0.1"
#define PORT "8080"

bool
handler(int fd, struct sockaddr_in addr, void **data)
{
    (void) addr;
    char **msg = (char **) data;

    ssize_t received = sock_read(fd, msg);

    if(received == 0 || (received == -1 && errno != EAGAIN)){ // Connection closed or error
        perror("Lost connection");
        close(fd);
        da_reset(msg);
        return(true);
    }

    ssize_t sent = sock_write(fd, msg);

    if(sent > 0)
        printf("Sent %ld bytes\n", sent);

    return(false);
}

int
main(void)
{
    short port = atoi(PORT);

    struct task *task = tcp_server(port, handler);

    printf("Listening on %s:%s...\n", IP, PORT);

    while(!task_poll(task))
        usleep(8000);

    free(task);

    return(0);
}

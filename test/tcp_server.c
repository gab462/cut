#include "../cut.h"
#include "../sock.h"
#include "../tcp_task.h"
#include "../task.h"
#include <stdio.h>
#include <errno.h>

#define IP "127.0.0.1"
#define PORT "8080"

void
handler(struct task_context *ctx, int fd, struct sockaddr_in addr)
{
    (void) addr;

    char **msg = task_ctx_alloc(ctx, char *);

    task_begin(ctx);

    ssize_t received = sock_read(fd, msg);

    if(received == 0 || (received == -1 && errno != EAGAIN)){ // Connection closed or error
        perror("Lost connection");
        close(fd);
        da_reset(msg);
        task_abort(ctx);
    }

    ssize_t sent = sock_write(fd, msg);

    if(sent > 0)
        printf("Sent %ld bytes\n", sent);

    task_return(ctx);

    task_end(ctx);
}

int
main(void)
{
    short port = atoi(PORT);
    int fd = tcp_listen(port);
    assert(fd != -1);

    printf("Listening on %s:%s...\n", IP, PORT);

    struct task_context ctx = {0};
    for(;;){
        tcp_server(&ctx, fd, handler);
        usleep(8000);
    }

    return(0);
}

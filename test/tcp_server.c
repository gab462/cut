#include "../cut.h"
#include "../sock.h"
#include "../tcp_task.h"
#include "../task.h"
#include <stdio.h>
#include <errno.h>

#define IP "127.0.0.1"
#define PORT "8080"

bool
handler(void **ctx, int fd, struct sockaddr_in addr)
{
    (void) addr;

    task_context_begin();
    char *msg;
    task_context_end();

    task_begin(ctx);

    ssize_t received = sock_read(fd, &task_ctx(ctx)->msg);

    if(received == 0 || (received == -1 && errno != EAGAIN)){ // Connection closed or error
        perror("Lost connection");
        close(fd);
        da_reset(&task_ctx(ctx)->msg);
        task_abort(ctx, true);
    }

    ssize_t sent = sock_write(fd, &task_ctx(ctx)->msg);

    if(sent > 0)
        printf("Sent %ld bytes\n", sent);

    return(false);

    task_end(ctx, true);
}

int
main(void)
{
    short port = atoi(PORT);
    int fd = tcp_listen(port);
    assert(fd != -1);

    printf("Listening on %s:%s...\n", IP, PORT);

    void *ctx = NULL;
    for(;;){
        tcp_server(&ctx, fd, handler);
        usleep(8000);
    }

    return(0);
}

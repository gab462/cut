#include "../cut.h"
#include "../sock.h"
#include "../tcp_task.h"
#include "../task.h"
#include <stdio.h>

#define IP "127.0.0.1"
#define PORT "8080"

void
handler(struct task_context *ctx, socket_t sock, struct sockaddr_in addr)
{
    (void) addr;

    char **msg = task_ctx_alloc(ctx, char *);

    task_begin(ctx);

    int received = sock_read(sock, msg);

    if(received == 0 || (received == -1 && sock_error() != SOCK_WOULDBLOCK)){ // Connection closed or error
        perror("Lost connection");
        sock_close(sock);
        da_reset(msg);
        task_abort(ctx);
    }

    int sent = sock_write(sock, msg);

    if(sent > 0)
        printf("Sent %d bytes\n", sent);

    task_return(ctx);

    task_end(ctx);
}

int
main(void)
{
    sock_init();

    short port = atoi(PORT);
    socket_t sock = tcp_listen(port);
    assert(sock != SOCK_INVALID);

    printf("Listening on %s:%s...\n", IP, PORT);

    struct task_context ctx = {0};
    for(;;){
        tcp_server(&ctx, sock, handler);
        usleep(8000);
    }

    return(0);
}

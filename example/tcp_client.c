#include "../cut.h"
#include "../sock.h"
#include "../tcp.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define IP "127.0.0.1"
#define PORT "8080"

bool
connection(socket_t sock, char **msg)
{
    char buf[4096];

    int received = recv(sock, buf, sizeof(buf), 0);

    if(received == 0 || (received == -1 && sock_error() != SOCK_WOULDBLOCK)){ // Connection closed or error
        perror("Lost connection");
        sock_close(sock);
        da_reset(msg);
        return(true);
    }

    if(received > 0)
        printf("%.*s\n", received, buf);

    if(len(*msg) > 0)
        sock_write(sock, msg);

    return(false);
}

int
main(void)
{
    sock_init();

    socket_t sock = tcp_connect(IP, PORT);
    assert(fd != -1);

    printf("Connected to %s:%s\n", IP, PORT);

    sock_set_nonblock(STDIN_FILENO);

    char *msg = NULL;
    while(!connection(sock, &msg)){
        char buf[64];
        ssize_t count = read(STDIN_FILENO, buf, sizeof(buf) - 1);

        if(count == -1){
            assert(sock_error() == SOCK_WOULDBLOCK);
        }else if(count > 0){
            push_items(&msg, buf, count);
        }

        usleep(8000);
    }

    return(0);
}

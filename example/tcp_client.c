#include "../cut.h"
#include "../sock.h"
#include "../tcp.h"
#include "../task.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#define IP "127.0.0.1"
#define PORT "8080"

bool
connection(int fd, char **msg)
{
    char buf[4096];

    ssize_t received = read(fd, buf, sizeof(buf));

    if(received == 0 || (received == -1 && errno != EAGAIN)){ // Connection closed or error
        perror("Lost connection");
        close(fd);
        da_reset(msg);
        return(true);
    }

    if(received > 0)
        write(STDOUT_FILENO, buf, received);

    if(len(*msg) > 0)
        sock_write(fd, msg);

    return(false);
}

int
main(void)
{
    int fd = tcp_connect(IP, PORT);
    assert(fd != -1);

    printf("Connected to %s:%s\n", IP, PORT);

    sock_set_nonblock(STDIN_FILENO);

    char *msg = NULL;
    while(!connection(fd, &msg)){
        char buf[64];
        ssize_t count = read(STDIN_FILENO, buf, sizeof(buf) - 1);

        if(count == -1){
            assert(errno == EAGAIN);
        }else if(count > 0){
            push_items(&msg, buf, count);
        }

        usleep(8000);
    }

    return(0);
}

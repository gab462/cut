#ifndef INCLUDE_TCP_H
#define INCLUDE_TCP_H

#include "sock.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define tcp_check_err(ret)      \
    do{                         \
        if(ret == -1){          \
            perror(__func__);   \
            return(ret);        \
        }                       \
    }while(0)

static inline
int
tcp_listen(short port)
{
    int err;

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    tcp_check_err(fd);

    sock_set_nonblock(fd);

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = INADDR_ANY
    };

    int reuse_addr = 1;
    err = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));
    tcp_check_err(err);

    err = bind(fd, (struct sockaddr *) &addr, sizeof(addr));
    tcp_check_err(err);

    err = listen(fd, SOMAXCONN);
    tcp_check_err(err);

    return(fd);
}

static inline
int
tcp_accept(int server, struct sockaddr_in *addr)
{
    socklen_t addr_len = sizeof(*addr);
    int client = accept(server, (struct sockaddr *) addr, &addr_len);

    if(client != -1)
        sock_set_nonblock(client);

    return(client);
}

static inline
int
tcp_connect(char *ip, char *port)
{
    int err;

    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM
    };

    struct addrinfo *res;
    err = getaddrinfo(ip, port, &hints, &res);

    if(err != 0){
        perror(__func__);
        return err;
    }

    for(struct addrinfo *p = res; p != NULL; p = p->ai_next){
        int conn = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(conn == -1) continue;

        err = connect(conn, p->ai_addr, p->ai_addrlen);
        if(err == -1) continue;

        sock_set_nonblock(conn);

        freeaddrinfo(res);

        return(conn);
    }

    fprintf(stderr, "Could not connect\n");
    freeaddrinfo(res);
    return(-1);
}

#endif

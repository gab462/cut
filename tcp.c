#ifndef INCLUDE_TCP_C
#define INCLUDE_TCP_C

#include "cut.c"
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>

void
tcp_set_nonblock(int fd)
{
    int err;

    int flags = fcntl(fd, F_GETFL, 0);
    assert(flags != -1);
    err = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    assert(err != -1);
}

static inline
int
tcp_listen(short port)
{
    int err;

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(fd != -1);

    tcp_set_nonblock(fd);

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = INADDR_ANY
    };

    int reuse_addr = 1;
    err = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));
    assert(err != -1);

    err = bind(fd, (struct sockaddr *) &addr, sizeof(addr));
    assert(err != -1);

    err = listen(fd, SOMAXCONN);
    assert(err != -1);

    return(fd);
}

static inline
int
tcp_accept(int server, struct sockaddr_in *addr)
{
    socklen_t addr_len = sizeof(*addr);
    int client = accept(server, (struct sockaddr *) addr, &addr_len);

    if(client < 0)
        return(client);

    tcp_set_nonblock(client);

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
    assert(err == 0);

    for (struct addrinfo *p = res; p != NULL; p = p->ai_next) {
        int conn = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(conn == -1) continue;

        err = connect(conn, p->ai_addr, p->ai_addrlen);
        if(err == -1) continue;

        tcp_set_nonblock(conn);

        freeaddrinfo(res);

        return(conn);
    }

    assert(false && "Could not connect");
    freeaddrinfo(res);
}

#endif

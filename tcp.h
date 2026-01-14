#ifndef INCLUDE_TCP_H
#define INCLUDE_TCP_H

#include "sock.h"

#ifndef _WIN32

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#endif

#define tcp_check_err(ret)          \
    do{                             \
        if(ret == -1){              \
            sock_report();          \
            return(SOCK_INVALID);   \
        }                           \
    }while(0)

static inline
socket_t
tcp_listen(short port)
{
    int err;

    socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == SOCK_INVALID){
        sock_report();
        return(sock);
    }

    sock_set_nonblock(sock);

#ifndef _WIN32
    int reuse_addr = 1;
    err = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));
    tcp_check_err(err);
#endif

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = INADDR_ANY
    };

    err = bind(sock, (struct sockaddr *) &addr, sizeof(addr));
    tcp_check_err(err);

    err = listen(sock, SOMAXCONN);
    tcp_check_err(err);

    return(sock);
}

static inline
socket_t
tcp_accept(socket_t server, struct sockaddr_in *addr)
{
    socklen_t addr_len = sizeof(*addr);
    socket_t client = accept(server, (struct sockaddr *) addr, &addr_len);

    if(client != SOCK_INVALID)
        sock_set_nonblock(client);

    return(client);
}

static inline
socket_t
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
        fprintf(stderr, "%s error: %s\n", __func__, gai_strerror(err));
        return(SOCK_INVALID);
    }

    for(struct addrinfo *p = res; p != NULL; p = p->ai_next){
        socket_t conn = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(conn == SOCK_INVALID) continue;

        err = connect(conn, p->ai_addr, p->ai_addrlen);
        if(err == -1) continue;

        sock_set_nonblock(conn);

        freeaddrinfo(res);

        return(conn);
    }

    fprintf(stderr, "Could not connect\n");
    freeaddrinfo(res);
    return(SOCK_INVALID);
}

#endif

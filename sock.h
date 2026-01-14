#ifndef INCLUDE_SOCK_H
#define INCLUDE_SOCK_H

#include "cut.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define sock_report() fprintf(stderr, "%s error: %d\n", __func__, sock_error())

#ifdef _WIN32

// Prevent raylib conflicts
#define WIN32_LEAN_AND_MEAN
#define NOUSER
#define NOGDI

#include <winsock2.h>
#include <ws2tcpip.h>

#undef WIN32_LEAN_AND_MEAN
#undef NOUSER
#undef NOGDI

#define sock_error() WSAGetLastError()
#define SOCK_WOULDBLOCK WSAEWOULDBLOCK
#define SOCK_INVALID INVALID_SOCKET

typedef SOCKET socket_t;

static inline
void
sock_init(void)
{
    WSADATA wsa;
    int err = WSAStartup(MAKEWORD(2, 2), &wsa);
    assert(err == 0);
    atexit((void (*)(void)) WSACleanup);
}

static inline
void
sock_set_nonblock(socket_t sock)
{
    u_long mode = 1;
    int err = ioctlsocket(sock, FIONBIO, &mode);
    if(err != 0) sock_report();
}

static inline
void
sock_close(socket_t sock)
{
    closesocket(sock);
}

#else

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <errno.h>

#define sock_error() errno
#define SOCK_WOULDBLOCK EAGAIN
#define SOCK_INVALID (-1)

typedef int socket_t;

static inline
void
sock_init(void) {}

static inline
void
sock_set_nonblock(socket_t sock)
{
    int flags = fcntl(sock, F_GETFL, 0);
    if(flags == -1) perror(__func__);
    int err = fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    if(err == -1) perror(__func__);
}

static inline
void
sock_close(socket_t sock)
{
    close(sock);
}

#endif

#define SOCK_BUF_SIZE 4096

static inline
int
sock_read(socket_t sock, char **sb)
{
    char buf[SOCK_BUF_SIZE];

    int count = recv(sock, buf, sizeof(buf), 0);

    if(count > 0)
        da_push_items(sb, buf, count);

    return(count);
}

static inline
int
sock_write(socket_t sock, char **sb)
{
    int count = send(sock, *sb, da_len(*sb), 0);

    if(count > 0)
        sb_consume(sb, count);

    return(count);
}

#endif

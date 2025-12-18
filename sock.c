#ifndef INCLUDE_SOCK_C
#define INCLUDE_SOCK_C

#include "cut.c"
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define SOCK_BUF_SIZE 4096

static inline
void
sock_set_nonblock(int fd)
{
    int err;

    int flags = fcntl(fd, F_GETFL, 0);
    assert(flags != -1);
    err = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    assert(err != -1);
}

ssize_t
sock_read(int fd, char **sb)
{
    char buf[SOCK_BUF_SIZE];

    ssize_t count = read(fd, buf, sizeof(buf));

    if(count > 0)
        da_push_items(sb, buf, count);

    return(count);
}

ssize_t
sock_write(int fd, char **sb)
{
    ssize_t count = write(fd, *sb, da_len(*sb));

    if(count > 0){
        memmove(*sb, *sb + count, da_len(*sb) - count);
        da_header(*sb)->length -= count;
    }

    return(count);
}

#endif

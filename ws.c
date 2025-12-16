#ifndef INCLUDE_WS_C
#define INCLUDE_WS_C

#include "cut.c"
#include <poll.h>
#include <assert.h>
#include <curl/curl.h>

static inline
CURL *
ws_connect(const char *url)
{
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 2L);
    curl_easy_perform(curl);
    return(curl);
}

static inline
short
ws_poll(CURL *curl, short events)
{
    curl_socket_t socket_fd;
    CURLcode res = curl_easy_getinfo(curl, CURLINFO_ACTIVESOCKET, &socket_fd);
    assert(res == CURLE_OK);

    struct pollfd fd = {
        .fd = socket_fd,
        .events = events
    };

    int ret = poll(&fd, 1, -1);
    assert(ret > 0);

    return(fd.revents);
}

static inline
void
ws_poll_write(CURL *curl)
{
    short revents = ws_poll(curl, POLLOUT);
    assert(revents & POLLOUT);
}

static inline
void
ws_poll_read(CURL *curl)
{
    short revents = ws_poll(curl, POLLIN);
    assert(revents & POLLIN);
}

static inline
CURLcode
ws_send(CURL *curl, char *content, int len)
{
    while(len > 0){
        size_t sent;
        CURLcode res = curl_ws_send(curl, content, len, &sent, 0, CURLWS_TEXT);

        if(res == CURLE_OK){
            content += sent;
            len -= sent;
        }else if(res == CURLE_AGAIN){
            ws_poll_write(curl);
        }else{
            return(res);
        }
    }

    return(CURLE_OK);
}

static inline
char *
ws_recv(CURL *curl)
{
    char buf[4096];
    char *sb = NULL;

    for(;;){
        const struct curl_ws_frame *meta;
        size_t received;
        CURLcode res = curl_ws_recv(curl, buf, 4096, &received, &meta);

        if(res == CURLE_OK){
            da_push_items(&sb, buf, (int) received);
            if(meta->bytesleft == 0 && !(meta->flags & CURLWS_CONT))
                break;
        }else if(res == CURLE_AGAIN){
            ws_poll_read(curl);
        }else{
            fprintf(stderr, "ws_recv() failed: %s\n", curl_easy_strerror(res));
            da_reset(&sb);
            return(NULL);
        }
    }

    return(sb);
}

#endif

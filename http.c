#ifndef INCLUDE_HTTP_C
#define INCLUDE_HTTP_C

#include "cut.c"
#include <curl/curl.h>

static inline
size_t
http_sb_write(void *data, size_t, size_t count, void *userp)
{
    char **sb = userp;
    da_push_items(sb, data, (int) count);
    return(count);
}

static inline
char *
http_get(char *url)
{
    char *sb = NULL;
    CURL *curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_sb_write);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &sb);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    // curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, CURLFOLLOW_ALL);
    CURLcode ret = curl_easy_perform(curl);

    if(ret != CURLE_OK){
        fprintf(stderr, "GET %s failed: %s\n", url, curl_easy_strerror(ret));
        da_reset(&sb);
    }

    return(sb);
}

static inline
char *
http_post(char *url, char *data)
{
    char *sb = NULL;
    CURL *curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_sb_write);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &sb);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    CURLcode ret = curl_easy_perform(curl);

    if(ret != CURLE_OK){
        fprintf(stderr, "POST %s failed: %s\n", url, curl_easy_strerror(ret));
        da_reset(&sb);
    }

    return(sb);
}

#endif

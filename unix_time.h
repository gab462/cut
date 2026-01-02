#ifndef INCLUDE_UNIX_TIME_H
#define INCLUDE_UNIX_TIME_H

#include <time.h>
#include <stdint.h>

static inline
int64_t
time_secs(void)
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return((int64_t)ts.tv_sec);
}

static inline
int64_t
time_millis(void)
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return((int64_t)ts.tv_sec * 1000 + (int64_t)ts.tv_nsec / 1000000);
}

static inline
int64_t
time_micros(void) {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return((int64_t)ts.tv_sec * 1000000 + (int64_t)ts.tv_nsec / 1000);
}

static inline
int64_t
time_nanos(void) {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return((int64_t)ts.tv_sec * 1000000000 + (int64_t)ts.tv_nsec);
}

#endif

#include "../cut.h"
#include "../task.h"
#include <stddef.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

bool
sleeper(void **ctx, float until, float dt){
    task_context_begin();
    int64_t total;
    task_context_end();

    task_begin(ctx);

    task_ctx(ctx)->total = until * 1000.f;
    task_ctx(ctx)->total -= dt;

    if(task_ctx(ctx)->total <= 0)
        task_abort(ctx, true);

    task_yield(ctx, false);

    task_ctx(ctx)->total -= dt;

    if(task_ctx(ctx)->total <= 0)
        task_abort(ctx, true);
    else
        return(false);

    task_end(ctx, true);
}

int64_t
current_time_millis(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return((int64_t)tv.tv_sec * 1000 + (int64_t)tv.tv_usec / 1000);
}

bool
task(void **ctx, float dt)
{
    task_context_begin();
    void *child_ctx[3];
    task_context_end();

    task_begin(ctx);

    bool done = sleeper(&task_ctx(ctx)->child_ctx[0], 1.f, dt);
    if(!done)
        return(false);
    else{
        task_yield(ctx, false);
    }

    bool done_a = sleeper(&task_ctx(ctx)->child_ctx[1], 2.f, dt);
    bool done_b = sleeper(&task_ctx(ctx)->child_ctx[2], 2.f, dt);
    if(!done_a && !done_b) return(false);

    task_end(ctx, true);
}

int
main(void)
{
    int64_t previous = current_time_millis();
    int64_t dt = 0;

    void *ctx = NULL;
    while(!task(&ctx, dt)){
        int64_t now = current_time_millis();
        dt = now - previous;
        previous = now;
        usleep(100000);
    }

    return(0);
}

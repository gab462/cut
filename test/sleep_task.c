#include "../cut.h"
#include "../task.h"
#include <stddef.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

void
sleeper(void **ctx, float until, int64_t dt){
    task_context_begin();
    int64_t total;
    task_context_end();

    task_begin(ctx);

    task_ctx(ctx)->total = until * 1000.f;
    task_ctx(ctx)->total -= dt;

    if(task_ctx(ctx)->total <= 0)
        task_abort(ctx);

    task_yield_while(ctx, (task_ctx(ctx)->total -= dt) > 0);

    task_end(ctx);
}

int64_t
current_time_millis(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return((int64_t)tv.tv_sec * 1000 + (int64_t)tv.tv_usec / 1000);
}

void
task(void **ctx, float dt)
{
    task_context_begin();
    void *child_ctx[3];
    task_context_end();

    task_begin(ctx);

    task_yield_from(ctx, sleeper, &task_ctx(ctx)->child_ctx[0], 1.f, dt);

    task_yield(ctx);

    sleeper(&task_ctx(ctx)->child_ctx[1], 2.f, dt);
    sleeper(&task_ctx(ctx)->child_ctx[2], 2.f, dt);
    if(!task_done(task_ctx(ctx)->child_ctx[1])
       || !task_done(task_ctx(ctx)->child_ctx[2]))
        return;

    task_end(ctx);
}

int
main(void)
{
    int64_t previous = current_time_millis();
    int64_t dt = 0;

    void *ctx = NULL;
    do{
        usleep(1000);

        int64_t now = current_time_millis();
        dt = now - previous;

        task(&ctx, dt);

        previous = now;
    }while(!task_done(ctx));

    return(0);
}

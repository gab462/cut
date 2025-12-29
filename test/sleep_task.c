#include "../cut.h"
#include "../task.h"
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

void
sleeper(struct task_context *ctx, float until, int64_t dt)
{
    int64_t *total = task_ctx_alloc(ctx, int64_t);

    task_begin(ctx);

    *total = until * 1000.f;
    *total -= dt;

    if(*total <= 0)
        task_abort(ctx);

    task_yield_while(ctx, (*total -= dt) > 0);

    task_end(ctx);
}

void
task(struct task_context *ctx, float dt)
{
    struct task_context *child_ctx = task_ctx_alloc(ctx, struct task_context, .count = 3);

    task_begin(ctx);

    task_yield_from(ctx, sleeper, &child_ctx[0], 1.f, dt);

    task_yield(ctx);

    sleeper(&child_ctx[1], 2.f, dt);
    sleeper(&child_ctx[2], 2.f, dt);
    if(!task_done(child_ctx[1])
       || !task_done(child_ctx[2]))
        task_return(ctx);

    task_end(ctx);
}

int
main(void)
{
    int64_t previous = unix_millis();
    int64_t dt = 0;

    struct task_context ctx = {0};
    do{
        usleep(1000);

        int64_t now = unix_millis();
        dt = now - previous;

        task(&ctx, dt);

        previous = now;
    }while(!task_done(ctx));

    return(0);
}

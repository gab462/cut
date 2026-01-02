#include "../task.h"

struct range_opt {
    int from;
    int step;
};

#define range(ctx, to, ...) \
    range_impl(ctx, to, (struct range_opt){ .from = 0, .step = 1, __VA_ARGS__ })

int
range_impl(struct task_context *ctx, int to, struct range_opt opt)
{
    int *i = task_ctx_alloc(ctx, int);

    task_begin(ctx);

    for(*i = opt.from; *i < to; *i += opt.step)
        task_yield(ctx, *i);

    task_end(ctx, to);
}

int
numbers(struct task_context *ctx)
{
    struct task_context *child_ctx = task_ctx_alloc(ctx, struct task_context);
    int ret;

    task_begin(ctx);

    while(ret = range(child_ctx, 5), child_ctx->running)
        task_yield(ctx, ret);

    do{
        task_yield(ctx, range(child_ctx, 20, .from = 10, .step = 2));
    }while(child_ctx->running);

    task_end(ctx, 21);
}

int
main(void)
{
    struct task_context ctx = {0};

    do{
        int n = numbers(&ctx);
        printf("%d\n", n);
    }while(ctx.running);

    return(0);
}

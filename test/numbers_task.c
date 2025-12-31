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
    int *current = task_ctx_alloc(ctx, int);

    task_begin(ctx);

    *current = opt.from;

    task_yield_while(ctx, *current < to, (*current += opt.step) - opt.step);

    task_end(ctx, to);
}

int
numbers(struct task_context *ctx)
{
    struct task_context *child_ctx = task_ctx_alloc(ctx, struct task_context);

    task_begin(ctx);

    task_yield_with(ctx, range, child_ctx, 5);
    task_yield_with(ctx, range, child_ctx, 19, .from = 10, .step = 2);

    task_end(ctx, 20);
}

int
main(void)
{
    struct task_context ctx = {0};

    do{
        int n = numbers(&ctx);
        printf("%d\n", n);
    }while(!task_done(ctx));

    return(0);
}

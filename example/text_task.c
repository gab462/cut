#include "../unix_time.h"
#include "../task.h"
#include "../sock.h"
#include "../term.h"
#include <string.h>
#include <stddef.h>
#include <unistd.h>

void
sleeper(struct task_context *ctx, int64_t millis)
{
    int64_t *until = task_ctx_alloc(ctx, int64_t);

    task_begin(ctx);

    *until = time_millis() + millis;

    while(time_millis() < *until)
        task_yield(ctx);

    task_end(ctx);
}

void
text_writer(struct task_context *ctx, char *text)
{
    size_t *i = task_ctx_alloc(ctx, size_t);
    struct task_context *sleep_ctx = task_ctx_alloc(ctx, struct task_context);

    task_begin(ctx);

    for(*i = 0; *i < strlen(text); ++*i){
        write(STDOUT_FILENO, text + *i, 1);

        while(sleeper(sleep_ctx, 50), !task_done(*sleep_ctx))
            task_yield(ctx);
    }

    task_end(ctx);
}

void
key_waiter(struct task_context *ctx, char c)
{
    task_begin(ctx);

    while(getchar() != c)
        task_yield(ctx);

    task_end(ctx);
}

void
presenter(struct task_context *ctx, char **text, int count)
{
    struct task_context *child_ctx = task_ctx_alloc(ctx, struct task_context);
    int *i = task_ctx_alloc(ctx, int);

    task_begin(ctx);

    for(*i = 0; *i < count; ++*i){
        while(text_writer(child_ctx, text[*i]), !task_done(*child_ctx))
            task_yield(ctx);

        while(key_waiter(child_ctx, 'n'), !task_done(*child_ctx))
            task_yield(ctx);
    }

    task_end(ctx);
}

int
main(void)
{
    char *text[] = {
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit.\n",
        "In vitae elementum odio.\n",
        "Integer tincidunt laoreet sapien vel fermentum.\n"
    };

    sock_set_nonblock(STDIN_FILENO);
    term_set_raw();

    struct task_context ctx = {0};
    do{
        presenter(&ctx, text, sizeof(text)/sizeof(text[0]));
        usleep(1000);
    }while(!task_done(ctx));

    term_set_canon();

    return(0);
}

#include "../cut.h"
#include "../task.h"
#include "../sock.h"
#include "../term.h"
#include <string.h>
#include <stddef.h>
#include <unistd.h>

void
sleeper(struct task_context *ctx, int64_t millis)
{
    int64_t *remaining = task_ctx_alloc(ctx, int64_t);
    int64_t *previous = task_ctx_alloc(ctx, int64_t);

    task_begin(ctx);

    *remaining = millis;
    *previous = unix_millis();

    task_yield_while(ctx, ({
        int64_t now = unix_millis();
        int64_t dt = now - *previous;
        *previous = now;

        (*remaining -= dt) > 0;
    }));

    task_end(ctx);
}

void
text_writer(struct task_context *ctx, char *text)
{
    size_t *offset = task_ctx_alloc(ctx, size_t);
    struct task_context *sleep_ctx = task_ctx_alloc(ctx, struct task_context);

    task_begin(ctx);

    for(*offset = 0; *offset < strlen(text); ++*offset){
        write(STDOUT_FILENO, text + *offset, 1);

        task_yield_while(ctx, (
            sleeper(sleep_ctx, 50),
            !task_done(*sleep_ctx)
        ));
    }

    task_end(ctx);
}

void
key_waiter(struct task_context *ctx, char c)
{
    task_begin(ctx);

    task_yield_while(ctx, getchar() != c);

    task_end(ctx);
}

void
presenter(struct task_context *ctx, char **text, int count)
{
    struct task_context *child_ctx = task_ctx_alloc(ctx, struct task_context);
    int *n = task_ctx_alloc(ctx, int);

    task_begin(ctx);

    for(*n = 0; *n < count; ++*n){
        task_yield_while(ctx, (
            text_writer(child_ctx, text[*n]),
            !task_done(*child_ctx)
        ));

        task_yield_while(ctx, (
            key_waiter(child_ctx, 'n'),
            !task_done(*child_ctx)
        ));
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

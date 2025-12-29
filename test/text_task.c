#include "../task.h"
#include "../sock.h"
#include <sys/time.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <termios.h>

int64_t
current_time_millis(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return((int64_t)tv.tv_sec * 1000 + (int64_t)tv.tv_usec / 1000);
}

void
sleeper(struct task_context *ctx, int64_t millis){
    int64_t *remaining = task_ctx_alloc(ctx, int64_t);
    int64_t *previous = task_ctx_alloc(ctx, int64_t);

    task_begin(ctx);

    *remaining = millis;
    *previous = current_time_millis();

    task_yield_while(ctx, ({
        int64_t now = current_time_millis();
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

    for(*offset = 0; *offset < strlen(text); (*offset)++){
        write(STDOUT_FILENO, text + *offset, 1);

        task_yield_from(ctx, sleeper, sleep_ctx, 50);
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

    for(*n = 0; *n < count; (*n)++){
        task_yield_from(ctx, text_writer, child_ctx, text[*n]);
        task_yield_from(ctx, key_waiter, child_ctx, 'n');
    }

    task_end(ctx);
}

struct termios
term_set_canon(void)
{
    struct termios old, new;

    tcgetattr(STDIN_FILENO, &old);

    new = old;
    new.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(STDIN_FILENO, TCSANOW, &new);

    return(old);
}

void
term_restore(struct termios cfg)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &cfg);
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
    struct termios cfg = term_set_canon();

    struct task_context ctx = {0};
    do{
        presenter(&ctx, text, sizeof(text)/sizeof(text[0]));
        usleep(1000);
    }while(!task_done(ctx));

    term_restore(cfg);

    return(0);
}

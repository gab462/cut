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
sleeper(void **ctx, int64_t millis){
    task_context_begin();
    int64_t remaining;
    int64_t previous;
    task_context_end();

    task_begin(ctx);

    task_ctx(ctx)->remaining = millis;
    task_ctx(ctx)->previous = current_time_millis();

    task_yield_while(ctx, ({
        int64_t now = current_time_millis();
        int64_t dt = now - task_ctx(ctx)->previous;
        task_ctx(ctx)->previous = now;

        (task_ctx(ctx)->remaining -= dt) > 0;
    }));

    task_end(ctx);
}

void
text_writer(void **ctx, char *text)
{
    task_context_begin();
    size_t offset;
    void *sleep_ctx;
    task_context_end();

    task_begin(ctx);

    for(task_ctx(ctx)->offset = 0;
        task_ctx(ctx)->offset < strlen(text);
        task_ctx(ctx)->offset++){
        write(STDOUT_FILENO, text + task_ctx(ctx)->offset, 1);

        task_yield_from(ctx, sleeper, &task_ctx(ctx)->sleep_ctx, 50);
    }

    task_end(ctx);
}

void
key_waiter(void **ctx, char c)
{
    task_context_begin();
    task_context_end();

    task_begin(ctx);

    task_yield_while(ctx, getchar() != c);

    task_end(ctx);
}

void
presenter(void **ctx, char **text, int count)
{
    task_context_begin();
    void *child_ctx;
    int n;
    task_context_end();

    task_begin(ctx);

    for(task_ctx(ctx)->n = 0; task_ctx(ctx)->n < count; task_ctx(ctx)->n++){
        task_yield_from(ctx, text_writer, &task_ctx(ctx)->child_ctx, text[task_ctx(ctx)->n]);
        task_yield_from(ctx, key_waiter, &task_ctx(ctx)->child_ctx, 'n');
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

    return old;
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

    void *ctx = NULL;
    do{
        presenter(&ctx, text, sizeof(text)/sizeof(text[0]));
        usleep(1000);
    }while(!task_done(ctx));

    term_restore(cfg);

    return(0);
}

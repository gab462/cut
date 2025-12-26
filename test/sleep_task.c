#include "../cut.h"
#include "../task.h"
#include <stddef.h>
#include <stdlib.h>
#include <sys/time.h>

struct sleeper {
    struct task interface;
    int64_t until;
};

bool
sleeper_poll(struct task *interface){
    struct sleeper *self = (struct sleeper *) interface;
    const int64_t *dt = interface->data;

    self->until -= *dt;

    return(self->until <= 0);
}

struct task *
sleeper(float until)
{
    struct sleeper task = {
        .interface.poll = sleeper_poll,
        .until = until * 1000.f
    };

    void *out = malloc(sizeof(task));
    return(memcpy(out, &task, sizeof(task)));
}

int64_t
current_time_millis(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return((int64_t)tv.tv_sec * 1000 + (int64_t)tv.tv_usec / 1000);
}

int
main(void)
{
    struct task *task =
        task_sequence(
            sleeper(1.f),
            task_group(
                sleeper(2.f),
                sleeper(2.f)
            )
        );

    int64_t previous = current_time_millis();
    int64_t dt = 0;

    task->data = &dt;

    while(!task_poll(task)){
        int64_t now = current_time_millis();
        dt = now - previous;
        previous = now;
    }

    free(task);

    return(0);
}

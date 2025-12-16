#include "../cut.c"
#include "../task.c"
#include <sys/time.h>

struct sleep_task {
    struct task interface;
    int64_t until;
};

static inline
bool
sleep_task_poll(struct task *task)
{
    struct sleep_task *sleeper = (struct sleep_task *) task;
    const int64_t *dt = task->data;

    sleeper->until -= *dt;

    return(sleeper->until <= 0);
}

static inline
struct task *
sleep_task(float until)
{
    struct sleep_task *sleeper = calloc(1, sizeof(struct sleep_task));
    sleeper->interface.poll = sleep_task_poll;
    sleeper->until = until * 1000.f;

    return(&sleeper->interface);
}

static inline
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
            sleep_task(1.f),
            task_group(
                sleep_task(2.f),
                sleep_task(2.f)
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
